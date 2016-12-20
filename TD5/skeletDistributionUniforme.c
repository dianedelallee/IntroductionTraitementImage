/******************************************************************************/
/* NAME                                                                       */
/* skelet is the program skeleton used to test image processing algorithms on */
/* a standard TrueColor visual.                                               */
/******************************************************************************/
/* SYNOPSIS                                                                   */
/* COLOR DISPLAY                                                              */
/* skelet [ <image_red> <image_green> <image_blue> [ <line_number>            */
/*                                                 [ <pixel_number> ] ] ]     */
/* GRAY-SCALE DISPLAY                                                         */
/* skelet [ <image_gray> [ <line_number>  [ <pixel_number> ] ] ]              */
/******************************************************************************/
/* DESCRIPTION                                                                */
/* This process connects to the X server and displays a RGB raster image from */
/* the three file names provided in input or a GRAY-SCALE image from the      */
/* single file provided in input.                                             */
/* Examples:                                                                  */
/* . skelet roissy.1 roissy.2 roissy.3 512 512                                */
/* . skelet girl 512 512                                                      */
/*                                                                            */
/* Images provided are supposed to have the same size (<line_number> and      */
/* <pixel_number>) given as last parameters.                                  */
/* These images in input must be in BSQ (Bit Sequential, also called DUMP)   */
/* format. In such organization, pixels are stored in the file as shown in the*/
/* figure.                                                                    */
/* Let (i,j) i=0,N-1 j=0,M-1 be the value of point in line i and pixel ,      */
/* data are stored in the file in the following order:                        */
/* (0,0) (0,1) ... (0,M-1) (1,0) (1,1) ... (1,M-1) ... (N-1,0) ... (N-1,M-1)  */
/*               0                       j        M-1                         */
/*             0 +-----------------------|---------+                          */
/*               |                       |         |                          */
/*               |                       |         |                          */
/*               |                       |         |                          */
/*               |                       |         |                          */
/*             i ------------------------*         |                          */
/*               |                                 |                          */
/*               |                                 |                          */
/*               |                                 |                          */
/*               |                                 |                          */
/*           N-1 +---------------------------------+                          */
/* Pixel representation expected in input is 8 bits per pixel.                */
/* As a consequence, size of input file must exactly match N x M bytes.       */
/******************************************************************************/
/* ADMINISTRATION                                                             */
/* Serge RIAZANOFF  | 28.01.00 | v00.01 | Creation of the SW component        */
/* Serge RIAZANOFF  | 14.02.00 | v00.02 | Adaptation on Sun Solaris 2.5       */
/* Serge RIAZANOFF  | 02.10.00 | v01.01 | Correction ROUGE/ROUGE/ROUGE        */
/* Serge RIAZANOFF  | 18.10.00 | v01.02 | Elaboration of skelet.c             */
/* Serge RIAZANOFF  | 18.10.01 | v02.01 | Added InitFrameBuffer function      */
/* Serge RIAZANOFF  | 20.10.01 | v02.02 | Adaptation on Sun Solaris 2.8       */
/* Serge RIAZANOFF  | 04.10.07 | v02.03 | Checked version                     */
/******************************************************************************/

/******************************************************************************/
/* Standard inclusion files                                                   */
/******************************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <errno.h>
#include  <memory.h>

#include  <X11/X.h>
#include  <X11/Xlib.h>
#include  <X11/Intrinsic.h>

/******************************************************************************/
/* Constant definitions                                                       */
/******************************************************************************/
#define MAX_COLOR   255                 /* Greatest pixel value */

/******************************************************************************/
/* Macro definitions                                                          */
/******************************************************************************/
#define nint(float_value)  (((float_value)-(int)(float_value) > 0.5)?          \
                            (int)(float_value)+1 : (int)(float_value))

/******************************************************************************/
/* Forward declarations                                                       */
/******************************************************************************/
int InitFrameBuffer ( );


/******************************************************************************/
/* Application core                                                           */
/******************************************************************************/
int main (
   int              argc,               /* argument count */
   char             **argv)             /* argument list */
{
/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/
   Display          *display;           /* display returned from connection */
   int              screen;             /* default screen of connection */
   unsigned int     display_planes;     /* screen color planes */
   XImage           *origin_ximage;     /* origin XImage structure */
   XImage           *processed_ximage;  /* processed XImage structure */
   Visual           *visual;            /* true-color visual */
   XVisualInfo      visual_info;        /* structure used to get visual info */
   unsigned char    *origin_frame_buffer;    /* origin frame buffer */
   unsigned char    *processed_frame_buffer; /* processed frame buffer */
   XEvent           event;              /* standard event structure */
   Window           window;             /* window XID */
   GC               gc;                 /* default graphic context */

   char             file_name[3][80];   /* name of RGB image files */
   FILE             *fp[3];             /* red image file pointer */
   char             window_title[3*80]; /* title reported in window bar */
   unsigned char    *origin_image[3];   /* image array: ORIGIN IMAGE */
   unsigned char    *processed_image[3];/* image array: PROCESSED IMAGE */

   int              channel_number;     /* number of channels (1 or 3) */
   int              nliin;              /* input line number */
   int              npxin;              /* input pixel number */
   int              ichannel;           /* index among channels */
   int              ili;                /* index among lines */
   int              ipx;                /* index among pixels */
   int              nread;              /* number of bytes actually read */

   int              required_depth;     /* expected depth when getting visual */
   int              status;             /* status returned by X function call */

   int              red_colormap_entries; /* nb.of possible values for Red */
   int              red_offset;         /* left offset to match the Red mask*/
   int              green_colormap_entries; /* nb.of possible values for Green*/
   int              green_offset;       /* left offset to match the Green mask*/
   int              blue_colormap_entries; /* nb.of possible values for Blue */
   int              blue_offset;        /* left offset to match the Blue mask*/
   int              bits_per_rgb;       /* bits nb.per RGB pixel in frame buf*/
   int              bytes_per_rgb;      /* bytes nb.per RGB pixel in frame bu*/

   XSetWindowAttributes window_attributes; /* used to set window attributes */
   Colormap         colormap;           /* Colormap used for TrueColor display*/
   XGCValues        GC_values;          /* structure used to initialize GC */

/******************************************************************************/
/* Connect to X server                                                        */
/******************************************************************************/
   if ((display=XOpenDisplay(NULL)) == NULL)
   {
      fprintf (stderr,"skelet : Cannot connect to X server.\n");
      exit (1);
   }
   screen         = DefaultScreen (display);
   display_planes = DisplayPlanes (display,screen);
/******************************************************************************/
/* Get the TrueColor visual                                                   */
/******************************************************************************/
   required_depth = 24;
   do
   {
      if ((status=XMatchVisualInfo(display,screen,required_depth,TrueColor,
           &visual_info)) == 0)
      {
         required_depth = required_depth - 1;
      }
   } while ((status == 0) && (required_depth >= 8));
   if (required_depth < 8)
   {
      fprintf (stderr,
     "skelet : Cannot get a TrueColor visual for whatever depth.\n");
      exit (1);
   }
/*============================================================================*/
/* Check that the number of bits per pixels in frame buffer is multiple of 8  */
/*============================================================================*/
   bits_per_rgb = BitmapUnit(display);
   if (required_depth == 16)
      bits_per_rgb = 16;
   if (bits_per_rgb % 8 != 0)
   {
      fprintf (stderr,"skelet : Only number of bits per pixels in ");
      fprintf (stderr,"frame buffer multiple of 8 are supported.\n");
      exit (1);
   }
   bytes_per_rgb = bits_per_rgb / 8;
/******************************************************************************/
/* Analyze the way Red, Green and Blue component are mapped in frame buffer   */
/******************************************************************************/
/* Red component                                                              */
/*----------------------------------------------------------------------------*/
   red_colormap_entries = visual_info.red_mask;
   red_offset           = 0;
   while ((red_colormap_entries & 0x00000001) == 0)
   {
      red_offset           = red_offset + 1;
      red_colormap_entries = (red_colormap_entries >> 1);
   }
   red_colormap_entries = red_colormap_entries + 1;
/*----------------------------------------------------------------------------*/
/* Green component                                                            */
/*----------------------------------------------------------------------------*/
   green_colormap_entries = visual_info.green_mask;
   green_offset           = 0;
   while ((green_colormap_entries & 0x00000001) == 0)
   {
      green_offset           = green_offset + 1;
      green_colormap_entries = (green_colormap_entries >> 1);
   }
   green_colormap_entries = green_colormap_entries + 1;
/*----------------------------------------------------------------------------*/
/* Blue component                                                             */
/*----------------------------------------------------------------------------*/
   blue_colormap_entries = visual_info.blue_mask;
   blue_offset           = 0;
   while ((blue_colormap_entries & 0x00000001) == 0)
   {
      blue_offset           = blue_offset + 1;
      blue_colormap_entries = (blue_colormap_entries >> 1);
   }
   blue_colormap_entries = blue_colormap_entries + 1;
/******************************************************************************/
/* GET PARAMETERS                                                             */
/******************************************************************************/
/* CASE OF GRAY-SCALE                                                         */
/*============================================================================*/
   if ((argc == 4)                                                            &&
       (sscanf(argv[2],"%d",&nliin) == 1)                                     &&
       (sscanf(argv[3],"%d",&npxin) == 1))
   {
      channel_number = 1;
/*----------------------------------------------------------------------------*/
/*    Get and open input file                                                 */
/*----------------------------------------------------------------------------*/
      strcpy (file_name[0],argv[1]);
      if ((fp[0]=fopen(file_name[0],"r")) == NULL)
      {
         fprintf (stderr,"skelet : can't open \"%s\"\n",file_name[0]);
         exit (1);
      }
   } /* CASE OF GRAY-SCALE */
/*============================================================================*/
/* CASE OF RED-GREE-BLUE IMAGES                                               */
/*============================================================================*/
   else
   {
      channel_number = 3;
/*----------------------------------------------------------------------------*/
/*    Process RED file                                                        */
/*----------------------------------------------------------------------------*/
      if (argc >= 2)
         strcpy(file_name[0],argv[1]);
      else
      {
         printf("Nom du fichier image: ROUGE  : ");
         scanf ("%s",file_name[0]);
      }
      if ((fp[0]=fopen(file_name[0],"r")) == NULL)
      {
         fprintf (stderr,"skelet : can't open \"%s\"\n",file_name[0]);
         exit (1);
      }
/*----------------------------------------------------------------------------*/
/*    Process GREEN file                                                      */
/*----------------------------------------------------------------------------*/
      if (argc >= 3)
         strcpy(file_name[1],argv[2]);
      else
      {
         printf("Nom du fichier image: VERT   : ");
         scanf ("%s",file_name[1]);
      }
      if ((fp[1]=fopen(file_name[1],"r")) == NULL)
      {
         fprintf (stderr,"skelet : can't open \"%s\"\n",file_name[1]);
         exit (1);
      }
/*----------------------------------------------------------------------------*/
/*    Process BLUE file                                                       */
/*----------------------------------------------------------------------------*/
      if (argc >= 4)
         strcpy(file_name[2],argv[3]);
      else
      {
         printf("Nom du fichier image: BLEU   : ");
         scanf ("%s",file_name[2]);
      }
      if ((fp[2]=fopen(file_name[2],"r")) == NULL)
      {
         fprintf (stderr,"skelet : can't open \"%s\"\n",file_name[2]);
         exit (1);
      }
/*----------------------------------------------------------------------------*/
/*    Get size of input image                                                 */
/*----------------------------------------------------------------------------*/
      if ((argc < 5)                                                          ||
          (sscanf(argv[4],"%d",&nliin) <= 0))
      {
         printf("Nombre de lignes en entree   : ");
         scanf ("%d",&nliin);
      }
      if ((argc < 6)                                                          ||
          (sscanf(argv[5],"%d",&npxin) <= 0))
      {
         printf("Nombre de pixels en entree   : ");
         scanf ("%d",&npxin);
      }
   } /* CASE OF RED-GREE-BLUE IMAGES */
/******************************************************************************/
/* Allocate memory for the internal frame buffers                             */
/******************************************************************************/
   if (((origin_frame_buffer=(unsigned char*)malloc((npxin*nliin*bytes_per_rgb)*
         sizeof(char))) == NULL)                                              ||
       ((processed_frame_buffer=(unsigned char*)malloc((npxin*nliin*
         bytes_per_rgb)*sizeof(char))) == NULL))
   {
      fprintf (stderr,
         "skelet : Cannot allocate internal frame buffers.\n");
      exit (1);
   }
/******************************************************************************/
/* Allocate memory for the image arrays                                       */
/******************************************************************************/
   for (ichannel=0; ichannel<channel_number; ichannel++)
   {
      if (((origin_image[ichannel]=(unsigned char*)malloc((npxin*nliin)*
            sizeof(char))) == NULL)                                           ||
          ((processed_image[ichannel]=(unsigned char*)malloc((npxin*nliin)*
            sizeof(char))) == NULL))
      {
         fprintf (stderr,
            "skelet : Cannot allocate memory for image arrays.\n");
         exit (1);
      }
   }
/******************************************************************************/
/* Read input image                                                           */
/******************************************************************************/
   for (ichannel=0; ichannel<channel_number; ichannel++)
   {
      for (ili=0; ili<nliin; ili++)
      {
         if ((nread=fread(&(origin_image[ichannel][ili*npxin]),sizeof(char),
              npxin,fp[ichannel])) < npxin)
         {
            fprintf (stderr,
         "skelet : error while reading record nb. %d from \"%s\",  ",
               ili,file_name[ichannel]);
            fprintf (stderr,"(returned=%d, status=%d)\n",nread,errno);
            perror ("skelet");
            exit (1);
         }
      }
   }
/******************************************************************************/
/******************************************************************************/
/* PROCESSING SECTION                                                         */
/******************************************************************************/
/* Initialize the processed image                                             */
/******************************************************************************/

// il faut recupere le nombre total de pixel et le diviser par 255.
  
   int nbTotalPixel = 0 ;
   int[] values ;
   float distributionUniforme = 0 ;

   for (int i = 0 ; i < 256 ; i ++) {
   	values[i] = 0 ;
   }

   for (ichannel=0; ichannel<channel_number; ichannel++) {
      for (ili=0; ili<nliin; ili++) {
         for (ipx=0; ipx<npxin; ipx++) {
		processed_image[ichannel][ili*npxin+ipx] = origin_image[ichannel][ili*npxin+ipx];
		nbTotalPixel ++ ;		
     	} /* Loop on pixels */
      } /* Loop on lines */
   } /* Loop on channels */

   distributionUniforme = (float) nbTotalPixel / (float)255;
   for (int j = 0 ; j < 256 ; j ++ ){
   	values[i] = distributionUniforme ;
   }

/******************************************************************************/
/******************************************************************************/
/* Transfer image into the "origin" frame buffer                              */
/******************************************************************************/
   if (InitFrameBuffer(channel_number,origin_image,nliin,npxin,bytes_per_rgb,
                       red_colormap_entries,red_offset,
                       green_colormap_entries,green_offset,
                       blue_colormap_entries,blue_offset,
                       origin_frame_buffer) != 0)
   {
      fprintf (stderr,
         "skelet : Cannot transfer origin image in frame buffer.\n");
      return (1);
   }
/******************************************************************************/
/* Transfer image into the "processed" frame buffer                           */
/******************************************************************************/
   if (InitFrameBuffer(channel_number,processed_image,nliin,npxin,bytes_per_rgb,
                       red_colormap_entries,red_offset,
                       green_colormap_entries,green_offset,
                       blue_colormap_entries,blue_offset,
                       processed_frame_buffer) != 0)
   {
      fprintf (stderr,
      "skelet : Cannot transfer processed image in frame buffer.\n");
      return (1);
   }
/******************************************************************************/
/* Allocate memory for the XImage structures                                  */
/******************************************************************************/
   visual = visual_info.visual;
   if (((origin_ximage=XCreateImage(display,visual,required_depth,ZPixmap,0,
         (char*)origin_frame_buffer,npxin,nliin,32,0)) == NULL)               ||
       ((processed_ximage=XCreateImage(display,visual,required_depth,ZPixmap,0,
         (char*)processed_frame_buffer,npxin,nliin,32,0)) == NULL))
   {
      fprintf (stderr,
         "skelet : cannot allocate ximage structures\n");
      exit (1);
   }
/******************************************************************************/
/* Create a window                                                            */
/******************************************************************************/
/* Create and install a colormap for the selected visual                      */
/*----------------------------------------------------------------------------*/
   if ((colormap=XCreateColormap(display,RootWindow(display,screen),visual,
        AllocNone)) == None)
   {
      fprintf (stderr,"skelet : Cannot create a colormap.\n");
      exit (1);
   }
   XInstallColormap (display,colormap);
/*----------------------------------------------------------------------------*/
/* Set window attributes for the window creation                              */
/*----------------------------------------------------------------------------*/
   window_attributes.colormap     = colormap;
   window_attributes.border_pixel = 1;
/*----------------------------------------------------------------------------*/
/* Create the window                                                          */
/*----------------------------------------------------------------------------*/
   if ((window=XCreateWindow (
           display,                     /* connection to X server */
           RootWindow(display,screen),  /* parent window */
           0,0,                         /* coord.of UL corner in parent window*/
           2*npxin,nliin,               /* width/height of window */
           1,                           /* border width */
           required_depth,              /* depth of window in bits */
           InputOutput,                 /* class of this window */
           visual,                      /* visual to be used for the window */
           CWBorderPixel | CWColormap,  /* value mask within window_attributes*/
           &window_attributes)) == 0)
   {
      fprintf (stderr,"skelet : Cannot create the window \n");
      exit (1);
   }
/*----------------------------------------------------------------------------*/
/* Set window title                                                           */
/*----------------------------------------------------------------------------*/
   strcpy (window_title,"- ");
   for (ichannel=0; ichannel<channel_number; ichannel++)
   {
      strcpy (window_title,file_name[ichannel]);
      strcpy (window_title," -");
   }
   XStoreName (display,window,window_title);
/*----------------------------------------------------------------------------*/
/* Select events that will be received by window                              */
/*----------------------------------------------------------------------------*/
   XSelectInput (display,window,ExposureMask | ButtonPressMask);
/*----------------------------------------------------------------------------*/
/* Display the window                                                         */
/*----------------------------------------------------------------------------*/
   XMapWindow (display,window);
/******************************************************************************/
/* Get graphic context                                                        */
/******************************************************************************/
   gc = DefaultGC(display,screen);
   gc = XCreateGC(display,window,0L,&GC_values);
/******************************************************************************/
/* Events loop                                                                */
/******************************************************************************/
   while (True)
   {
/*----------------------------------------------------------------------------*/
/*    Get next event                                                          */
/*----------------------------------------------------------------------------*/
      XNextEvent (display,&event);
      switch (event.type) {
/*----------------------------------------------------------------------------*/
/*       Expose => send image into the window                                 */
/*----------------------------------------------------------------------------*/
         case Expose:
            XPutImage (display,window,gc,origin_ximage,
                       0,0,0,0,npxin,nliin);
            XPutImage (display,window,gc,processed_ximage,
                       0,0,npxin,0,npxin,nliin);
            break;
/*----------------------------------------------------------------------------*/
/*       ButtonPress => close display and exit application                    */
/*----------------------------------------------------------------------------*/
         case ButtonPress:
            XCloseDisplay (display);
            exit (0);
/*----------------------------------------------------------------------------*/
/*       Any other event is not processed                                     */
/*----------------------------------------------------------------------------*/
         default:
            break;
      }
   } /* event loop */
} /* Application core */



/******************************************************************************/
/* InitFrameBuffer initializes the frame buffer from the entire image provided*/
/* in input.                                                                  */
/******************************************************************************/
int InitFrameBuffer (
   int              channel_number,     /* number of channels (1 or 3) */
   unsigned char    *image_buffer[3],   /* image array: ORIGIN or PROCESSED */
   int              nliin,              /* input line number */
   int              npxin,              /* input pixel number */
   int              bytes_per_rgb,       /* bytes nb.per RGB pixel in frame bu*/
   int              red_colormap_entries, /* nb.of possible values for Red */
   int              red_offset,         /* left offset to match the Red mask*/
   int              green_colormap_entries, /* nb.of possible values for Green*/
   int              green_offset,       /* left offset to match the Green mask*/
   int              blue_colormap_entries, /* nb.of possible values for Blue */
   int              blue_offset,        /* left offset to match the Blue mask*/
   unsigned char    *frame_buffer)      /* frame buffer to be initialized */
{
/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/
   int              ili;                /* index among lines */
   int              ipx;                /* index among pixels */
   int              icolor_rgb;         /* color value for compound RGB values*/
   unsigned char    byte_order[4];      /* used to check byte order in int */
   int              int_MSB_first;      /* "Most Significant Byte first in
                                           integer representation" flag */
   int              icolor_red;         /* color value for component red */
   int              icolor_green;       /* color value for component green */
   int              icolor_blue;        /* color value for component blue */

/******************************************************************************/
/* Initialise the frame buffer                                                */
/******************************************************************************/
/* Analyze order inside an integer                                            */
/*----------------------------------------------------------------------------*/
   icolor_rgb = 0x01020304;
   memcpy (byte_order,&icolor_rgb,4);
   if (byte_order[0] == 0x01)
      int_MSB_first = True;
   else
      int_MSB_first = False;
/*============================================================================*/
/* Initialize origin frame buffer                                             */
/*============================================================================*/
/* Loop on lines                                                              */
/*----------------------------------------------------------------------------*/
   for (ili=0; ili<nliin; ili++)
   {
/*----------------------------------------------------------------------------*/
/*    Interleave Red, Green and Blue components into frame buffer             */
/*----------------------------------------------------------------------------*/
      for (ipx=0; ipx<npxin; ipx++)
      {
         if (channel_number == 3)
         {
            icolor_red   = image_buffer[0][ili*npxin+ipx];
            icolor_green = image_buffer[1][ili*npxin+ipx];
            icolor_blue  = image_buffer[2][ili*npxin+ipx];
         }
         else
         {
            icolor_red   = image_buffer[0][ili*npxin+ipx];
            icolor_green = icolor_red;
            icolor_blue  = icolor_red;
         }
/*----------------------------------------------------------------------------*/
/*       Set RGB values according to their colormap entries                   */
/*----------------------------------------------------------------------------*/
         icolor_red   = nint((float)red_colormap_entries  * icolor_red   / 256);
         if (icolor_red >= red_colormap_entries)
            icolor_red = red_colormap_entries - 1;
         icolor_green = nint((float)green_colormap_entries* icolor_green / 256);
         if (icolor_green >= green_colormap_entries)
            icolor_green = green_colormap_entries - 1;
         icolor_blue  = nint((float)blue_colormap_entries * icolor_blue  / 256);
         if (icolor_blue >= blue_colormap_entries)
            icolor_blue = blue_colormap_entries - 1;
/*----------------------------------------------------------------------------*/
/*       Combine the three components according to their masks                */
/*----------------------------------------------------------------------------*/
         icolor_rgb   = (icolor_red   << red_offset)    |
                        (icolor_green << green_offset)  |
                        (icolor_blue  << blue_offset);
/*----------------------------------------------------------------------------*/
/*       Report value in frame buffer                                         */
/*----------------------------------------------------------------------------*/
         if (int_MSB_first)
         {
            memcpy (&(frame_buffer[(ili*npxin+ipx)*bytes_per_rgb]),
                 &(((unsigned char*)(&icolor_rgb))[sizeof(int)-bytes_per_rgb]),
                 bytes_per_rgb);
         }
         else
         {
            memcpy (&(frame_buffer[(ili*npxin+ipx)*bytes_per_rgb]),
                 &icolor_rgb,bytes_per_rgb);
         }
      } /* Loop on pixels */
   } /* Loop on lines */
/******************************************************************************/
/* Return "Ok" status                                                         */
/******************************************************************************/
   return (0);
} /* InitFrameBuffer */
