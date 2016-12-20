#!/bin/sh -f
################################################################################
# compi : Procedure de compilation de programmes PASCAL utilisant la librairie #
#         graphique libMlv.a                                                   #
################################################################################

if [ `/bin/uname -s` = 'HP-UX' ]; then
   MLV_XWINDOW_INCLUDE="/usr/include/X11R5"
   MLV_XWINDOW_LIBRARY="/usr/lib/X11R5"
   MLV_MOTIF_INCLUDE="/usr/include/Motif1.2"
   MLV_MOTIF_LIBRARY="/usr/lib/Motif1.2"
   FLAGS="-Aa -D_HPUX_SOURCE -g "
   CC=cc
else if [ `/bin/uname -s` = 'Linux' ]; then
   MLV_XWINDOW_INCLUDE="/usr/include/X11"
   MLV_XWINDOW_LIBRARY="/usr/X11R6/lib"
   MLV_MOTIF_INCLUDE="/usr/local/LessTif/Motif1.2/include"
   MLV_MOTIF_LIBRARY="/usr/local/LessTif/Motif1.2/lib"
   FLAGS="-Wall"
   CC=gcc
else if [ `hostname` = gael ]; then
   MLV_XWINDOW_INCLUDE="/users2/telimago/openwin/include"
   MLV_XWINDOW_LIBRARY="/usr/lib/X11R5"
   MLV_MOTIF_INCLUDE="/usr/MOTIFv1.1/usr/include"
   MLV_MOTIF_LIBRARY="/usr/MOTIFv1.1/usr/lib"
   FLAGS="-D_NO_PROTO -DNO_REGEX -DR4_INTRINSICS -DSTRINGS_ALIGNED"
   CC=acc
else
   MLV_XWINDOW_INCLUDE="/usr/include/X11R5"
   MLV_XWINDOW_LIBRARY="/usr/lib/X11R5"
   MLV_MOTIF_INCLUDE="/usr/include/Motif1.2"
   MLV_MOTIF_LIBRARY="/usr/lib/Motif1.2"
   FLAGS="-Aa -D_HPUX_SOURCE"
   CC=cc
fi
fi
fi

for f in $*
do
   p=`echo $f | cut -f1 -d"."`
   $CC -I$MLV_XWINDOW_INCLUDE -I$MLV_MOTIF_INCLUDE $FLAGS   $p.c -o $p         \
       -L$MLV_MOTIF_LIBRARY -L$MLV_XWINDOW_LIBRARY -lXt -lX11 -lm
#       -L$MLV_MOTIF_LIBRARY -L$MLV_XWINDOW_LIBRARY -lXm -lXt -lX11 -lm
done
