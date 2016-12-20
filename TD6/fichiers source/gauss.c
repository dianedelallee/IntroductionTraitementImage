/******************************************************************************/
/* NAME                                                                       */
/* coeff_gauss computes the coefficients of a Gauss convolution matrix.       */
/******************************************************************************/
/* ADMINISTRATION                                                             */
/* Serge RIAZANOFF  | 05.12.06 | v00.01 | Creation of the SW component        */
/* Serge RIAZANOFF  | 19.01.07 | v00.02 | Empirical value of (size / 3.5)     */
/*                  |          |        | from residual analysis              */
/******************************************************************************/

/******************************************************************************/
/* Standard inclusion files                                                   */
/******************************************************************************/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <errno.h>
#include  <memory.h>
#include  <math.h>

/******************************************************************************/
/* Constant definitions                                                       */
/******************************************************************************/
#define MAX_SIZE    11                  /* maximum size of convolution */
#define SIGMA       (size / 3.5)        /* Gaussian standard deviation */

/******************************************************************************/
/* Macro definitions                                                          */
/******************************************************************************/
#define nint(float_value)  (((float_value)-(int)(float_value) > 0.5)?          \
                            (int)(float_value)+1 : (int)(float_value))


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
   double           coeff[MAX_SIZE * MAX_SIZE]; /* matrix coefficients */
   int              coeff_int[MAX_SIZE * MAX_SIZE]; /* integer values of coeff*/
   int              size;               /* size of current matrix */
   int              k;                  /* index anmong lines in matrix */
   int              l;                  /* index anmong columns in matrix */
   double           d2;                 /* square of distance of cell to centr*/
   double           gain;               /* multiplicative factor */
   int              sum;                /* sum of integer coefficients */
   double           residual;           /* nint approximation residual */

/******************************************************************************/
/* Loop on convolution sizes                                                  */
/******************************************************************************/
   for (size=3; size<=MAX_SIZE; size=size+2)
   {
      printf ("\n\nCONVOLUTION SIZE = %d\n",size);
/*============================================================================*/
/*    Compute convolution values                                              */
/*============================================================================*/
      for (k=-size/2; k<=size/2; k++)
      {
         for (l=-size/2; l<=size/2; l++)
         {
            d2 = k*k + l*l;
            coeff[(k+size/2)*size+(l+size/2)] = 1./(SIGMA * sqrt(2.0 * M_PI)) *
               exp(-d2 / (2.*SIGMA*SIGMA));
printf("%lf ",coeff[(k+size/2)*size+(l+size/2)]);
         }
printf("\n");
      }

/*============================================================================*/
/*    Apply gain to reach value 1 at the four corners                         */
/*============================================================================*/
      gain     = 1. / coeff[0];
      sum      = 0;
      residual = 0.0;
printf("\n");
      for (k=-size/2; k<=size/2; k++)
      {
         for (l=-size/2; l<=size/2; l++)
         {
            coeff_int[(k+size/2)*size+(l+size/2)] =
               nint(gain * coeff[(k+size/2)*size+(l+size/2)]);
            sum = sum + coeff_int[(k+size/2)*size+(l+size/2)];
            residual = residual + fabs(coeff_int[(k+size/2)*size+(l+size/2)] -
               gain * coeff[(k+size/2)*size+(l+size/2)]);
printf("%5d ",coeff_int[(k+size/2)*size+(l+size/2)]);
         }
printf("\n");
      }
printf("GAIN     = 1 / %d\n",sum);
printf("SIGMA    = %lf \n",SIGMA);
printf("residual = %lf / ( %d x %d ) = %lf \n",residual,size,size,
   residual/(size*size));

   } /* Loop on convolution sizes */
   exit (0);
}
