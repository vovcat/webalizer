/*
    graphs.c  - produces graphs used by the Webalizer

    This is part of the Webalizer, a web server log file analysis
    program.

    Copyright (C) 1997, 1998  Bradford L. Barrett (brad@mrunix.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version, and provided that the above
    copyright and permission notice is included with all distributed
    copies of this or derived software.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

    This software uses the gd graphics library, which is copyright by
    Quest Protein Database Center, Cold Spring Harbor Labs. The following
    applies only to the gd graphics library software:

    gd 1.2 is copyright 1994, 1995, Quest Protein Database Center,
    Cold Spring Harbor Labs. Permission granted to copy and distribute
    this work provided that this notice remains intact. Credit
    for the library must be given to the Quest Protein Database Center,
    Cold Spring Harbor Labs, in all derived works. This does not
    affect your ownership of the derived work itself, and the intent
    is to assure proper credit for Quest, not to interfere with your
    use of gd. If you have questions, ask. ("Derived works"
    includes all programs that utilize the library. Credit must
    be given in user-visible documentation.)

    gd 1.2 was written by Thomas Boutell and is currently
    distributed by boutell.com, Inc.

    If you wish to release modifications to gd,
    please clear them first by sending email to
    boutell@boutell.com; if this is not done, any modified version of the gd
    library must be clearly labeled as such.

    The Quest Protein Database Center is funded under Grant P41-RR02188 by
    the National Institutes of Health.

    Written by Thomas Boutell  2/94 - 8/95.
    (http://sunsite.unc.edu/boutell/index.html)

    The GIF compression code is based on that found in the pbmplus
    utilities, which in turn is based on GIFENCOD by David Rowley. See the
    notice below:

    ** Based on GIFENCOD by David Rowley <mgardi@watdscu.waterloo.edu>.A
    ** Lempel-Zim compression based on "compress".
    **
    ** Modified by Marcel Wijkstra <wijkstra@fwi.uva.nl>
    **
    ** Copyright (C) 1989 by Jef Poskanzer.
    **
    ** Permission to use, copy, modify, and distribute this software and its
    ** documentation for any purpose and without fee is hereby granted, provided
    ** that the above copyright notice appear in all copies and that both the
    ** copyright notice and this permission notice appear in supporting
    ** documentation.  This software is provided "as is" without express or
    ** implied warranty.
    **
    ** The Graphics Interchange Format(c) is the Copyright property of
    ** CompuServe Incorporated.  GIF(sm) is a Service Mark property of
    ** CompuServe Incorporated.

    The GIF decompression is based on that found in the pbmplus
    utilities, which in turn is based on GIFDECOD by David Koblas. See the
    notice below:

    +-------------------------------------------------------------------+
    | Copyright 1990, 1991, 1993, David Koblas.  (koblas@netcom.com)    |
    |   Permission to use, copy, modify, and distribute this software   |
    |   and its documentation for any purpose and without fee is hereby |
    |   granted, provided that the above copyright notice appear in all |
    |   copies and that both that copyright notice and this permission  |
    |   notice appear in supporting documentation.  This software is    |
    |   provided "as is" without express or implied warranty.           |
    +-------------------------------------------------------------------+
*/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <gd.h>
#include <gdfontt.h>
#include <gdfonts.h>
#include <gdfontmb.h>

/* Some systems don't define this */
#ifndef PI
#define PI 3.14159265358979323846
#endif

#define COLOR1 green                       /* common colors for   */
#define COLOR2 blue                        /* bar charts (hits,   */
#define COLOR3 orange                      /* files, sites and    */
#define COLOR4 red                         /* Kbytes...           */
#define CX 156                             /* center x (for pie)  */
#define CY 150                             /* center y  (chart)   */
#define XRAD 240                           /* X-axis radius       */
#define YRAD 200                           /* Y-axis radius       */

/* forward reference internal routines */

u_long  jdate(int, int, int);
void    init_graph(char *, int, int);
struct  pie_data *calc_arc(float, float);

/* common public declarations */

char *numchar[] = { " 0"," 1"," 2"," 3"," 4"," 5"," 6"," 7"," 8"," 9","10",
                    "11","12","13","14","15","16","17","18","19","20",
                    "21","22","23","24","25","26","27","28","29","30","31"};

gdImagePtr	im;                        /* image buffer        */
FILE		*out;                      /* output file for GIF */
char		maxvaltxt[32];             /* graph values        */
float		percent;                   /* percent storage     */
u_long		julday;                    /* julday value        */

struct pie_data { int x; int y;            /* line x,y            */
                  int mx; int my; };       /* midpoint x,y        */
/* colors */
int		black, white, grey, dkgrey, red, blue, orange, green;

/*****************************************************************/
/*                                                               */
/* YEAR_GRAPH4x  - Year graph with four data sets                */
/*                                                               */
/*****************************************************************/

int year_graph4x(  char *fname,            /* file name use       */
                   char *title,            /* title for graph     */
                    int fmonth,            /* begin month number  */
                 u_long data1[12],         /* data1 (hits)        */
                 u_long data2[12],         /* data2 (files)       */
                 u_long data3[12],         /* data3 (sites)       */
                 double data4[12])         /* data4 (kbytes)      */
{
   /* declare external language specific strings                  */
   extern char *msg_h_hits;                /* "Hits" string       */
   extern char *msg_h_files;               /* "Files"             */
   extern char *msg_h_sites;               /* "Sites"             */
   extern char *msg_h_xfer;                /* "KBytes"            */
   extern char *s_month[12];               /* 3 char month names  */

   /* local variables */
   int i,j,x1,y1,x2;
   int s_mth;

   u_long maxval=1;
   double fmaxval=0.0;

   /* initalize the graph */
   init_graph(title,512,256);      /* init as 512 x 256   */

   gdImageLine(im, 305,25,305,233,black);  /* draw section lines  */
   gdImageLine(im, 304,25,304,233,white);
   gdImageLine(im, 305,130,490,130,black);
   gdImageLine(im, 305,129,490,129,white);

   /* x-axis legend */
   s_mth = fmonth;
   for (i=0;i<12;i++)
   {
      gdImageString(im,gdFontSmall,28+(i*23),             /* use language   */
                    238,s_month[s_mth-1],black);          /* specific array */
      s_mth++;
      if (s_mth > 12) s_mth = 1;
      if (data1[i] > maxval) maxval = data1[i];           /* get max val    */
      if (data2[i] > maxval) maxval = data2[i];
   }
   if (maxval <= 0) maxval = 1;

   /* Now do the 'Hits Files Sites KBytes' legend */
   i = strlen(msg_h_hits) + strlen(msg_h_files) +
       strlen(msg_h_sites)+ strlen(msg_h_xfer) + 3;
   j = 400 - ((i/2)*6);
   gdImageString(im,gdFontSmall,j,238,msg_h_hits,green);
   j+= ( (strlen(msg_h_hits) +1) *6 );
   gdImageString(im,gdFontSmall,j,238,msg_h_files,blue);
   j+= ( (strlen(msg_h_files)+1) *6 );
   gdImageString(im,gdFontSmall,j,238,msg_h_sites,orange);
   j+= ( (strlen(msg_h_sites)+1) *6 );
   gdImageString(im,gdFontSmall,j,238,msg_h_xfer,red);

   sprintf(maxvaltxt, "%lu", maxval);
   gdImageStringUp(im,gdFontSmall,8,26+(strlen(maxvaltxt)*6),
                   maxvaltxt,black);

   /* data1 */
   s_mth = fmonth;
   for (i=0; i<12; i++)
   {
      if (s_mth > 12) s_mth = 1;
      percent = ((float)data1[s_mth++ -1] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 27 + (i*23);
      x2 = x1 + 14;
      y1 = 232 - (percent * 203);
      gdImageFilledRectangle(im, x1, y1, x2, 232, COLOR1);
      gdImageRectangle(im, x1, y1, x2, 232, black);
   }

   /* data2 */
   s_mth = fmonth;
   for (i=0; i<12; i++)
   {
      if (s_mth > 12) s_mth = 1;
      percent = ((float)data2[s_mth++ -1] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 32 + (i*23);
      x2 = x1 + 14;
      y1 = 232 - (percent * 203);
      gdImageFilledRectangle(im, x1, y1, x2, 232, COLOR2);
      gdImageRectangle(im, x1, y1, x2, 232, black);
   }

   /* data3 */
   maxval=0;
   for (i=0; i<12; i++)
       if (data3[i] > maxval) maxval = data3[i];           /* get max val    */
   if (maxval <= 0) maxval = 1;
   sprintf(maxvaltxt, "%lu", maxval);
   gdImageStringUp(im, gdFontSmall,493,26+(strlen(maxvaltxt)*6),
                   maxvaltxt, black);

   s_mth = fmonth;
   for (i=0; i<12; i++)
   {
      if (s_mth > 12) s_mth = 1;
      percent = ((float)data3[s_mth++ -1] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 311 + (i*15);
      x2 = x1 + 10;
      y1 = 127 - (percent * 98);
      gdImageFilledRectangle(im, x1, y1, x2, 127, COLOR3);
      gdImageRectangle(im, x1, y1, x2, 127, black);
   }

   /* data4 */
   fmaxval=0.0;
   for (i=0; i<12; i++)
       if (data4[i] > fmaxval) fmaxval = data4[i];         /* get max val    */
   if (fmaxval <= 0.0) fmaxval = 1.0;
   sprintf(maxvaltxt, "%.0f", fmaxval);
   gdImageStringUp(im, gdFontSmall,493,130+(strlen(maxvaltxt)*6),
                   maxvaltxt,black);

   s_mth = fmonth;
   for (i=0; i<12; i++)
   {
      if (s_mth > 12) s_mth = 1;
      percent = ((float)data4[s_mth++ -1] / (float)fmaxval);
      if (percent <= 0.0) continue;
      x1 = 311 + (i*15);
      x2 = x1 + 10;
      y1 = 232 - (percent * 98);
      gdImageFilledRectangle(im, x1, y1, x2, 232, COLOR4);
      gdImageRectangle(im, x1, y1, x2, 232, black);
   }

   /* save gif image */
   if ((out = fopen(fname, "wb")) != NULL)
   {
      gdImageGif(im, out);
      fclose(out);
   }
   /* deallocate memory */
   gdImageDestroy(im);

   return (0);
}

/*****************************************************************/
/*                                                               */
/* MONTH_GRAPH4  - Month graph with four data sets               */
/*                                                               */
/*****************************************************************/

#define YSIZE 400

int month_graph4(  char *fname,            /* filename           */
                   char *title,            /* graph title        */
                    int month,             /* graph month        */
                    int year,              /* graph year         */
                 u_long data1[31],         /* data1 (hits)       */
                 u_long data2[31],         /* data2 (files)      */
                 u_long data3[31],         /* data3 (sites)      */
                 u_long data4[31])         /* data4 (kbytes)     */
{
   /* local variables */
   int i,x1,y1,x2;
   u_long maxval=0;

   /* calc julian date for month */
   julday = (jdate(1, month,year) % 7);

   /* initalize the graph */
   init_graph(title,512,400);

   gdImageLine(im, 21, 180, 490, 180, black); /* draw section lines */
   gdImageLine(im, 21, 179, 490, 179, white);
   gdImageLine(im, 21, 280, 490, 280, black);
   gdImageLine(im, 21, 279, 490, 279, white);

   /* x-axis legend */
   for (i=0;i<31;i++)
   {
      if ((julday % 7 == 6) || (julday % 7 == 0))
       gdImageString(im,gdFontSmall,25+(i*15),382,numchar[i+1],green);
      else
       gdImageString(im,gdFontSmall,25+(i*15),382,numchar[i+1],black);
      julday++;
   }

   /* y-axis legend */
   for (i=0; i<31; i++)
   {
       if (data1[i] > maxval) maxval = data1[i];           /* get max val    */
       if (data2[i] > maxval) maxval = data2[i];
   }
   if (maxval <= 0) maxval = 1;
   sprintf(maxvaltxt, "%lu", maxval);
   gdImageStringUp(im, gdFontSmall,8,26+(strlen(maxvaltxt)*6),
                   maxvaltxt,black);

   /* data1 */
   for (i=0; i<31; i++)
   {
      percent = ((float)data1[i] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 26 + (i*15);
      x2 = x1 + 7;
      y1 = 176 - (percent * 147);
      gdImageFilledRectangle(im, x1, y1, x2, 176, COLOR1);
      gdImageRectangle(im, x1, y1, x2, 176, black);
   }

   /* data2 */
   for (i=0; i<31; i++)
   {
      percent = ((float)data2[i] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 29 + (i*15);
      x2 = x1 + 7;
      y1 = 176 - (percent * 147);
      gdImageFilledRectangle(im, x1, y1, x2, 176, COLOR2);
      gdImageRectangle(im, x1, y1, x2, 176, black);
   }

   /* data3 */
   maxval=0;
   for (i=0; i<31; i++)
      if (data3[i]>maxval) maxval = data3[i];
   if (maxval <= 0) maxval = 1;
   sprintf(maxvaltxt, "%lu", maxval);
   gdImageStringUp(im, gdFontSmall,8,180+(strlen(maxvaltxt)*6),
                   maxvaltxt, black);

   for (i=0; i<31; i++)
   {
      percent = ((float)data3[i] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 26 + (i*15);
      x2 = x1 + 10;
      y1 = 276 - (percent * 92);
      gdImageFilledRectangle(im, x1, y1, x2, 276, COLOR3);
      gdImageRectangle(im, x1, y1, x2, 276, black);
   }

   /* data4 */
   maxval=0;
   for (i=0; i<31; i++)
      if (data4[i]>maxval) maxval = data4[i];
   if (maxval <= 0) maxval = 1;
   sprintf(maxvaltxt, "%lu", maxval/1024);
   gdImageStringUp(im, gdFontSmall,8,280+(strlen(maxvaltxt)*6),
                   maxvaltxt, black);

   for (i=0; i<31; i++)
   {
      percent = (float)(data4[i] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 26 + (i*15);
      x2 = x1 + 10;
      y1 = 375 - ( percent * 91 );
      gdImageFilledRectangle(im, x1, y1, x2, 375, COLOR4);
      gdImageRectangle(im, x1, y1, x2, 375, black);
   }

   /* open file for writing */
   if ((out = fopen(fname, "wb")) != NULL)
   {
      gdImageGif(im, out);
      fclose(out);
   }
   /* deallocate memory */
   gdImageDestroy(im);

   return (0);
}

/*****************************************************************/
/*                                                               */
/* DAY_GRAPH2  - Day graph with two data sets                    */
/*                                                               */
/*****************************************************************/

int day_graph2(  char *fname,
                 char *title,
               u_long data1[24],
               u_long data2[24])
{
   /* local variables */
   int i,x1,y1,x2;
   u_long maxval=0;

   /* initalize the graph */
   init_graph(title,512,256);

   /* x-axis legend */
   for (i=0;i<24;i++)
   {
      gdImageString(im,gdFontSmall,33+(i*19),238,numchar[i],black);
      if (data1[i] > maxval) maxval = data1[i];           /* get max val    */
      if (data2[i] > maxval) maxval = data2[i];
   }
   if (maxval <= 0) maxval = 1;
   sprintf(maxvaltxt, "%lu", maxval);
   gdImageStringUp(im, gdFontSmall, 8, 26+(strlen(maxvaltxt)*6),
                   maxvaltxt, black);

   /* data1 */
   for (i=0; i<24; i++)
   {
      percent = ((float)data1[i] / (float)maxval);  /* percent of 100% */
      if (percent <= 0.0) continue;
      x1 = 30 + (i*19);
      x2 = x1 + 11;
      y1 = 232 - (percent * 203);
      gdImageFilledRectangle(im, x1, y1, x2, 232, COLOR1);
      gdImageRectangle(im, x1, y1, x2, 232, black);
   }

   /* data2 */
   for (i=0; i<24; i++)
   {
      percent = ((float)data2[i] / (float)maxval);  /* percent of 100% */
      if (percent <= 0.0) continue;
      x1 = 34 + (i*19);
      x2 = x1 + 11;
      y1 = 232 - (percent * 203);
      gdImageFilledRectangle(im, x1, y1, x2, 232, COLOR2);
      gdImageRectangle(im, x1, y1, x2, 232, black);
   }

   /* save as gif file */
   if ( (out = fopen(fname, "wb")) != NULL)
   {
      gdImageGif(im, out);
      fclose(out);
   }
   /* deallocate memory */
   gdImageDestroy(im);

   return (0);
}

/*****************************************************************/
/*                                                               */
/* PIE_CHART  - draw a pie chart (10 data items max)             */
/*                                                               */
/*****************************************************************/

int pie_chart(char *fname, char *title, u_long t_val,
              u_long data1[10], char *legend[10])
{
   int i,x,percent,y=47;
   double s_arc=0.0;
   int cyan, yellow, purple, ltpurple, ltgreen, brown;
   char buffer[128];

   struct pie_data gdata;

   /* In webalizer_lang.h */
   extern char *msg_h_other;

   /* init graph and colors */
   init_graph(title,512,300);
   cyan    = gdImageColorAllocate(im, 0, 255, 255);
   yellow  = gdImageColorAllocate(im, 255, 255, 0);
   purple  = gdImageColorAllocate(im, 128, 0, 128);
   ltgreen = gdImageColorAllocate(im, 128, 255, 192);
   ltpurple= gdImageColorAllocate(im, 255, 0, 255);
   brown   = gdImageColorAllocate(im, 255, 196, 128);

   /* do the circle... */
   gdImageArc(im, CX, CY, XRAD, YRAD, 0, 360, black);
   gdImageArc(im, CX, CY+10, XRAD-2, YRAD-2, 2, 178, black);
   gdImageFillToBorder(im, CX, CY+(YRAD/2)+1, black, black);

   /* slice the pie */
   gdata=*calc_arc(0.0,0.0);
   gdImageLine(im,CX,CY,gdata.x,gdata.y,black);  /* inital line           */
   for (i=0;i<10;i++)                      /* run through data array      */
   {
      if ((data1[i]!=0)&&(s_arc<1.0))      /* make sure valid slice       */
      {
         percent=(((double)data1[i]/t_val)+0.005)*100.0;
         if (percent<1) break;

         if (s_arc+((double)percent/100.0)>=1.0)
         {
            gdata=*calc_arc(s_arc,1.0);
            s_arc=1.0;
         }
         else
         {
            gdata=*calc_arc(s_arc,s_arc+((double)percent/100.0));
            s_arc+=(double)percent/100.0;
         }

         gdImageLine(im, CX, CY, gdata.x, gdata.y, black);
         gdImageFillToBorder(im, gdata.mx, gdata.my, black, i+4);

         sprintf(buffer,"%s (%d%%)",legend[i], percent);
         x=480-(strlen(buffer)*7);
         gdImageString(im,gdFontMediumBold, x+1, y+1, buffer, black);
         gdImageString(im,gdFontMediumBold, x, y, buffer, i+4);
         y+=20;
      }
   }

   if (s_arc < 1.0)                         /* anything left over?        */
   {
      gdata=*calc_arc(s_arc,1.0);

      gdImageFillToBorder(im, gdata.mx, gdata.my, black, white);
      sprintf(buffer,"%s (%d%%)",msg_h_other,100-(int)(s_arc*100));
      x=480-(strlen(buffer)*7);
      gdImageString(im,gdFontMediumBold, x+1, y+1, buffer, black);
      gdImageString(im,gdFontMediumBold, x, y, buffer, white);
   }

   /* save gif image */
   if ((out = fopen(fname, "wb")) != NULL)
   {
      gdImageGif(im, out);
      fclose(out);
   }
   /* deallocate memory */
   gdImageDestroy(im);

   return (0);
}

/*****************************************************************/
/*                                                               */
/* CALC_ARC  - generate x,y coordinates for pie chart            */
/*                                                               */
/*****************************************************************/

struct pie_data *calc_arc(float min, float max)
{
   static struct pie_data data;
   double d;

   /* Calculate max line */
   d=max;
   data.x=cos(d*(2*PI))*((XRAD-2)/2)+CX;
   data.y=sin(d*(2*PI))*((YRAD-2)/2)+CY;
   /* Now get mid-point  */
   d=((min+max)/2);
   data.mx=cos(d*(2*PI))*(XRAD/3)+CX;
   data.my=sin(d*(2*PI))*(YRAD/3)+CY;
   return &data;
}

/*****************************************************************/
/*                                                               */
/* INIT_GRAPH  - initalize graph and draw borders                */
/*                                                               */
/*****************************************************************/

void init_graph(char *title, int xsize, int ysize)
{
   int i;

   im = gdImageCreate(xsize,ysize);

   /* allocate color maps, background color first (grey) */
   grey    = gdImageColorAllocate(im, 192, 192, 192);
   dkgrey  = gdImageColorAllocate(im, 128, 128, 128);
   black   = gdImageColorAllocate(im, 0, 0, 0);
   white   = gdImageColorAllocate(im, 255, 255, 255);
   green   = gdImageColorAllocate(im, 0, 128, 92);
   orange  = gdImageColorAllocate(im, 255, 128, 0);
   blue    = gdImageColorAllocate(im, 0, 0, 255);
   red     = gdImageColorAllocate(im, 255, 0, 0);

   /* make borders */

   for (i=0; i<5 ;i++)          /* do shadow effect */
   {
      gdImageLine(im, i, i, xsize-i, i, white);
      gdImageLine(im, i, i, i, ysize-i, white);
      gdImageLine(im, i, ysize-i-1, xsize-i-1, ysize-i-1, dkgrey);
      gdImageLine(im, xsize-i-1, i, xsize-i-1, ysize-i-1, dkgrey);
   }

   gdImageRectangle(im, 20, 25, xsize-21, ysize-21, black);
   gdImageRectangle(im, 19, 24, xsize-22, ysize-22, white);
   gdImageRectangle(im, 0, 0, xsize-1, ysize-1, black);

   /* display the graph title */
   gdImageString(im, gdFontMediumBold, 20, 8, title, blue);

   return;
}

/*****************************************************************/
/*                                                               */
/* JDATE  - Julian date calculator                               */
/*                                                               */
/* Calculates the number of days since Jan 1, 0000.              */
/*                                                               */
/* Originally written by Bradford L. Barrett (03/17/1988)        */
/* Returns an unsigned long value representing the number of     */
/* days since January 1, 0000.                                   */
/*                                                               */
/* Note: Due to the changes made by Pope Gregory XIII in the     */
/*       16th Centyry (Feb 24, 1582), dates before 1583 will     */
/*       not return a truely accurate number (will be at least   */
/*       10 days off).  Somehow, I don't think this will         */
/*       present much of a problem for most situations :)        */
/*                                                               */
/* Usage: days = jdate(day, month, year)                         */
/*                                                               */
/* The number returned is adjusted by 5 to facilitate day of     */
/* week calculations.  The mod of the returned value gives the   */
/* day of the week the date is.  (ie: dow = days % 7 ) where     */
/* dow will return 0=Sunday, 1=Monday, 2=Tuesday, etc...         */
/*                                                               */
/*****************************************************************/

u_long jdate( int day, int month, int year )
{
   u_long days;                      /* value returned */
   int mtable[] = {0,31,59,90,120,151,181,212,243,273,304,334};

   /* First, calculate base number including leap and Centenial year stuff */

   days=(((u_long)year*365)+day+mtable[month-1]+
           ((year+4)/4) - ((year/100)-(year/400)));

   /* now adjust for leap year before March 1st */

   if ((year % 4 == 0) && !((year % 100 == 0) &&
       (year % 400 != 0)) && (month < 3))
   --days;

   /* done, return with calculated value */

   return(days+5);
}
