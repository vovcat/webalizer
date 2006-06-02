/*
    graphs.c  - produces graphs used by the Webalizer

    Copyright (C) 1997-2001  Bradford L. Barrett (brad@mrunix.net)

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
    Quest Protein Database Center, Cold Spring Harbor Labs.  Please
    see the documentation supplied with the library for additional
    information and license terms, or visit www.boutell.com/gd/ for the
    most recent version of the library and supporting documentation.
*/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <gd.h>
#include <gdfontt.h>
#include <gdfonts.h>
#include <gdfontmb.h>

#include "webalizer.h"
#include "lang.h"
#include "graphs.h"

/* Some systems don't define this */
#ifndef PI
#define PI 3.14159265358979323846
#endif

#define COLOR1 green                       /* graph color - hits  */
#define COLOR2 blue                        /* files               */
#define COLOR3 orange                      /* sites               */
#define COLOR4 red                         /* KBytes              */
#define COLOR5 cyan                        /* Files               */
#define COLOR6 yellow                      /* Visits              */

#define CX 156                             /* center x (for pie)  */
#define CY 150                             /* center y  (chart)   */
#define XRAD 240                           /* X-axis radius       */
#define YRAD 200                           /* Y-axis radius       */

/* forward reference internal routines */

void    init_graph(char *, int, int);
struct  pie_data *calc_arc(float, float);

/* common public declarations */

char *numchar[] = { " 0"," 1"," 2"," 3"," 4"," 5"," 6"," 7"," 8"," 9","10",
                    "11","12","13","14","15","16","17","18","19","20",
                    "21","22","23","24","25","26","27","28","29","30","31"};

gdImagePtr	im;                        /* image buffer        */
FILE		*out;                      /* output file for PNG */
struct stat     out_stat;                  /* stat struct for PNG */
char		maxvaltxt[32];             /* graph values        */
float		percent;                   /* percent storage     */
u_long		julday;                    /* julday value        */

struct pie_data { int x; int y;            /* line x,y            */
                  int mx; int my; };       /* midpoint x,y        */
/* colors */
int		black, white, grey, dkgrey, red,
                blue, orange, green, cyan, yellow;

/*****************************************************************/
/*                                                               */
/* YEAR_GRAPH6x  - Year graph with six data sets                 */
/*                                                               */
/*****************************************************************/

int year_graph6x(  char *fname,            /* file name use      */
                   char *title,            /* title for graph    */
                    int fmonth,            /* begin month number */
                 u_long data1[12],         /* data1 (hits)       */
                 u_long data2[12],         /* data2 (files)      */
                 u_long data3[12],         /* data3 (sites)      */
                 double data4[12],         /* data4 (kbytes)     */
                 u_long data5[12],         /* data5 (views)      */
                 u_long data6[12])         /* data6 (visits)     */
{

   /* local variables */
   int i,j,x1,y1,x2;
   int s_mth;

   u_long maxval=1;
   double fmaxval=0.0;

   /* initalize the graph */
   init_graph(title,512,256);              /* init as 512 x 256  */

   gdImageLine(im, 305,25,305,233,black);  /* draw section lines */
   gdImageLine(im, 304,25,304,233,white);
   gdImageLine(im, 305,130,490,130,black);
   gdImageLine(im, 305,129,490,129,white);

   /* index lines? */
   if (graph_lines)
   {
      y1=210/(graph_lines+1);
      for (i=0;i<graph_lines;i++)
       gdImageLine(im,21,((i+1)*y1)+25,303,((i+1)*y1)+25,dkgrey);
      y1=105/(graph_lines+1);
      for (i=0;i<graph_lines;i++)
       gdImageLine(im,306,((i+1)*y1)+25,489,((i+1)*y1)+25,dkgrey);
      for (i=0;i<graph_lines;i++)
       gdImageLine(im,306,((i+1)*y1)+130,489,((i+1)*y1)+130,dkgrey);
   }

   /* x-axis legend */
   s_mth = fmonth;
   for (i=0;i<12;i++)
   {
      gdImageString(im,gdFontSmall,28+(i*23),238,_(s_month[s_mth-1]),black);

      s_mth++;
      if (s_mth > 12) s_mth = 1;
      if (data1[i] > maxval) maxval = data1[i];           /* get max val    */
      if (data2[i] > maxval) maxval = data2[i];
      if (data5[i] > maxval) maxval = data5[i];
   }
   if (maxval <= 0) maxval = 1;
   sprintf(maxvaltxt, "%lu", maxval);
   gdImageStringUp(im,gdFontSmall,8,26+(strlen(maxvaltxt)*6),maxvaltxt,black);

   if (graph_legend)                          /* print color coded legends? */
   {
      /* Kbytes Legend */
      i = (strlen(_("KBytes"))*6);
      gdImageString(im,gdFontSmall,491-i,239,_("KBytes"),dkgrey);
      gdImageString(im,gdFontSmall,490-i,238,_("KBytes"),COLOR4);

      /* Sites/Visits Legend */
      i = (strlen(_("Visits"))*6);
      j = (strlen(_("Sites"))*6);
      gdImageString(im,gdFontSmall,491-i-j-12,11,_("Visits"),dkgrey);
      gdImageString(im,gdFontSmall,490-i-j-12,10,_("Visits"),COLOR6);
      gdImageString(im,gdFontSmall,491-j-9,11,"/",dkgrey);
      gdImageString(im,gdFontSmall,490-j-9,10,"/",black);
      gdImageString(im,gdFontSmall,491-j,11,_("Sites"),dkgrey);
      gdImageString(im,gdFontSmall,490-j,10,_("Sites"),COLOR3);

      /* Hits/Files/Pages Legend */
      i = (strlen(_("Pages"))*6);
      j = (strlen(_("Files"))*6);
      gdImageStringUp(im,gdFontSmall,8,231,_("Pages"),dkgrey);
      gdImageStringUp(im,gdFontSmall,7,230,_("Pages"),COLOR5);
      gdImageStringUp(im,gdFontSmall,8,231-i-3,"/",dkgrey);
      gdImageStringUp(im,gdFontSmall,7,230-i-3,"/",black);
      gdImageStringUp(im,gdFontSmall,8,231-i-12,_("Files"),dkgrey);
      gdImageStringUp(im,gdFontSmall,7,230-i-12,_("Files"),COLOR2);
      gdImageStringUp(im,gdFontSmall,8,231-i-j-15,"/",dkgrey);
      gdImageStringUp(im,gdFontSmall,7,230-i-j-15,"/",black);
      gdImageStringUp(im,gdFontSmall,8,231-i-j-24,_("Hits"),dkgrey);
      gdImageStringUp(im,gdFontSmall,7,230-i-j-24,_("Hits"),COLOR1);
   }

   /* data1 */
   s_mth = fmonth;
   for (i=0; i<12; i++)
   {
      if (s_mth > 12) s_mth = 1;
      percent = ((float)data1[s_mth++ -1] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 26 + (i*23);
      x2 = x1 + 13;
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
      x1 = 29 + (i*23);
      x2 = x1 + 13;
      y1 = 232 - (percent * 203);
      gdImageFilledRectangle(im, x1, y1, x2, 232, COLOR2);
      gdImageRectangle(im, x1, y1, x2, 232, black);
   }

   /* data5 */
   s_mth = fmonth;
   for (i=0; i<12; i++)
   {
      if (s_mth > 12) s_mth = 1;
      percent = ((float)data5[s_mth++ -1] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 32 + (i*23);
      x2 = x1 + 13;
      y1 = 232 - (percent * 203);
      gdImageFilledRectangle(im, x1, y1, x2, 232, COLOR5);
      gdImageRectangle(im, x1, y1, x2, 232, black);
   }

   maxval=0;
   for (i=0; i<12; i++)
   {
       if (data3[i] > maxval) maxval = data3[i];           /* get max val    */
       if (data6[i] > maxval) maxval = data6[i];
   }
   if (maxval <= 0) maxval = 1;
   sprintf(maxvaltxt, "%lu", maxval);
   gdImageStringUp(im, gdFontSmall,493,26+(strlen(maxvaltxt)*6),
                   maxvaltxt, black);

   /* data6 */
   s_mth = fmonth;
   for (i=0; i<12; i++)
   {
      if (s_mth > 12) s_mth = 1;
      percent = ((float)data6[s_mth++ -1] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 310 + (i*15);
      x2 = x1 + 8;
      y1 = 127 - (percent * 98);
      gdImageFilledRectangle(im, x1, y1, x2, 127, COLOR6);
      gdImageRectangle(im, x1, y1, x2, 127, black);
   }

   /* data3 */
   s_mth = fmonth;
   for (i=0; i<12; i++)
   {
      if (s_mth > 12) s_mth = 1;
      percent = ((float)data3[s_mth++ -1] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 314 + (i*15);
      x2 = x1 + 7;
      y1 = 127 - (percent * 98);
      gdImageFilledRectangle(im, x1, y1, x2, 127, COLOR3);
      gdImageRectangle(im, x1, y1, x2, 127, black);
   }

   fmaxval=0.0;
   for (i=0; i<12; i++)
       if (data4[i] > fmaxval) fmaxval = data4[i];         /* get max val    */
   if (fmaxval <= 0.0) fmaxval = 1.0;
   sprintf(maxvaltxt, "%.0f", fmaxval);
   gdImageStringUp(im, gdFontSmall,493,130+(strlen(maxvaltxt)*6),
                   maxvaltxt,black);

   /* data4 */
   s_mth = fmonth;
   for (i=0; i<12; i++)
   {
      if (s_mth > 12) s_mth = 1;
      percent = ((float)data4[s_mth++ -1] / (float)fmaxval);
      if (percent <= 0.0) continue;
      x1 = 311 + (i*15);
      x2 = x1 + 9;
      y1 = 232 - (percent * 98);
      gdImageFilledRectangle(im, x1, y1, x2, 232, COLOR4);
      gdImageRectangle(im, x1, y1, x2, 232, black);
   }

   /* stat the file */
   if ( !(lstat(fname, &out_stat)) )
   {
     /* check if the file a symlink */
     if ( S_ISLNK(out_stat.st_mode) )
     {
       if (verbose)
       fprintf(stderr,"%s %s!\n",_("Error: File is a symlink"),fname);
       return;
     }
   }

   /* save png image */
   if ((out = fopen(fname, "wb")) != NULL)
   {
      gdImagePng(im, out);
      fclose(out);
   }
   /* deallocate memory */
   gdImageDestroy(im);

   return (0);
}

/*****************************************************************/
/*                                                               */
/* MONTH_GRAPH6  - Month graph with six data sets                */
/*                                                               */
/*****************************************************************/

#define YSIZE 400

int month_graph6(  char *fname,            /* filename           */
                   char *title,            /* graph title        */
                    int month,             /* graph month        */
                    int year,              /* graph year         */
                 u_long data1[31],         /* data1 (hits)       */
                 u_long data2[31],         /* data2 (files)      */
                 u_long data3[31],         /* data3 (sites)      */
                 double data4[31],         /* data4 (kbytes)     */
                 u_long data5[31],         /* data5 (views)      */
                 u_long data6[31])         /* data6 (visits)     */
{

   /* local variables */
   int i,j,s,x1,y1,x2;
   u_long maxval=0;
   double fmaxval=0.0;

   /* calc julian date for month */
   julday = (jdate(1, month,year) % 7);

   /* initalize the graph */
   init_graph(title,512,400);

   gdImageLine(im, 21, 180, 490, 180, black); /* draw section lines */
   gdImageLine(im, 21, 179, 490, 179, white);
   gdImageLine(im, 21, 280, 490, 280, black);
   gdImageLine(im, 21, 279, 490, 279, white);

   /* index lines? */
   if (graph_lines)
   {
      y1=154/(graph_lines+1);
      for (i=0;i<graph_lines;i++)
       gdImageLine(im,21,((i+1)*y1)+25,489,((i+1)*y1)+25,dkgrey);
      y1=100/(graph_lines+1);
      for (i=0;i<graph_lines;i++)
       gdImageLine(im,21,((i+1)*y1)+180,489,((i+1)*y1)+180,dkgrey);
      for (i=0;i<graph_lines;i++)
       gdImageLine(im,21,((i+1)*y1)+280,489,((i+1)*y1)+280,dkgrey);
   }

   /* x-axis legend */
   for (i=0;i<31;i++)
   {
      if ((julday % 7 == 6) || (julday % 7 == 0))
       gdImageString(im,gdFontSmall,25+(i*15),382,numchar[i+1],COLOR1);
      else
       gdImageString(im,gdFontSmall,25+(i*15),382,numchar[i+1],black);
      julday++;
   }

   /* y-axis legend */
   for (i=0; i<31; i++)
   {
       if (data1[i] > maxval) maxval = data1[i];           /* get max val    */
       if (data2[i] > maxval) maxval = data2[i];
       if (data5[i] > maxval) maxval = data5[i];
   }
   if (maxval <= 0) maxval = 1;
   sprintf(maxvaltxt, "%lu", maxval);
   gdImageStringUp(im, gdFontSmall,8,26+(strlen(maxvaltxt)*6),
                   maxvaltxt,black);

   if (graph_legend)                           /* Print color coded legends? */
   {
      /* Kbytes Legend */
      gdImageStringUp(im,gdFontSmall,494,376,_("KBytes"),dkgrey);
      gdImageStringUp(im,gdFontSmall,493,375,_("KBytes"),COLOR4);

      /* Sites/Visits Legend */
      i = (strlen(_("Sites"))*6);
      gdImageStringUp(im,gdFontSmall,494,276,_("Sites"),dkgrey);
      gdImageStringUp(im,gdFontSmall,493,275,_("Sites"),COLOR3);
      gdImageStringUp(im,gdFontSmall,494,276-i-3,"/",dkgrey);
      gdImageStringUp(im,gdFontSmall,493,275-i-3,"/",black);
      gdImageStringUp(im,gdFontSmall,494,276-i-12,_("Visits"),dkgrey);
      gdImageStringUp(im,gdFontSmall,493,275-i-12,_("Visits"),COLOR6);

      /* Pages/Files/Hits Legend */
      s = ( i=(strlen(_("Pages"))*6) )+
          ( j=(strlen(_("Files"))*6) )+
          ( strlen(_("Hits"))*6 )+ 52;
      gdImageStringUp(im,gdFontSmall,494,s,_("Pages"),dkgrey);
      gdImageStringUp(im,gdFontSmall,493,s-1,_("Pages"),COLOR5);
      gdImageStringUp(im,gdFontSmall,494,s-i-3,"/",dkgrey);
      gdImageStringUp(im,gdFontSmall,493,s-i-4,"/",black);
      gdImageStringUp(im,gdFontSmall,494,s-i-12,_("Files"),dkgrey);
      gdImageStringUp(im,gdFontSmall,493,s-i-13,_("Files"),COLOR2);
      gdImageStringUp(im,gdFontSmall,494,s-i-j-15,"/",dkgrey);
      gdImageStringUp(im,gdFontSmall,493,s-i-j-16,"/",black);
      gdImageStringUp(im,gdFontSmall,494,s-i-j-24,_("Hits"),dkgrey);
      gdImageStringUp(im,gdFontSmall,493,s-i-j-25,_("Hits"),COLOR1);
   }

   /* data1 */
   for (i=0; i<31; i++)
   {
      percent = ((float)data1[i] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 25 + (i*15);
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
      x1 = 27 + (i*15);
      x2 = x1 + 7;
      y1 = 176 - (percent * 147);
      gdImageFilledRectangle(im, x1, y1, x2, 176, COLOR2);
      gdImageRectangle(im, x1, y1, x2, 176, black);
   }

   /* data5 */
   for (i=0; i<31; i++)
   {
      if (data5[i]==0) continue;
      percent = ((float)data5[i] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 29 + (i*15);
      x2 = x1 + 7;
      y1 = 176 - (percent * 147);
      gdImageFilledRectangle(im, x1, y1, x2, 176, COLOR5);
      gdImageRectangle(im, x1, y1, x2, 176, black);
   }

   /* sites / visits */
   maxval=0;
   for (i=0; i<31; i++)
   {
      if (data3[i]>maxval) maxval = data3[i];
      if (data6[i]>maxval) maxval = data6[i];
   }
   if (maxval <= 0) maxval = 1;
   sprintf(maxvaltxt, "%lu", maxval);
   gdImageStringUp(im, gdFontSmall,8,180+(strlen(maxvaltxt)*6),
                   maxvaltxt, black);

   /* data 6 */
   for (i=0; i<31; i++)
   {
      percent = ((float)data6[i] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 25 + (i*15);
      x2 = x1 + 8;
      y1 = 276 - (percent * 92);
      gdImageFilledRectangle(im, x1, y1, x2, 276, COLOR6);
      gdImageRectangle(im, x1, y1, x2, 276, black);
   }

   /* data 3 */
   for (i=0; i<31; i++)
   {
      percent = ((float)data3[i] / (float)maxval);
      if (percent <= 0.0) continue;
      x1 = 29 + (i*15);
      x2 = x1 + 7;
      y1 = 276 - (percent * 92);
      gdImageFilledRectangle(im, x1, y1, x2, 276, COLOR3);
      gdImageRectangle(im, x1, y1, x2, 276, black);
   }

   /* data4 */
   fmaxval=0.0;
   for (i=0; i<31; i++)
      if (data4[i]>fmaxval) fmaxval = data4[i];
   if (fmaxval <= 0.0) fmaxval = 1.0;
   sprintf(maxvaltxt, "%.0f", fmaxval/1024);
   gdImageStringUp(im, gdFontSmall,8,280+(strlen(maxvaltxt)*6),
                   maxvaltxt, black);

   for (i=0; i<31; i++)
   {
      percent = data4[i] / fmaxval;
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
      gdImagePng(im, out);
      fclose(out);
   }
   /* deallocate memory */
   gdImageDestroy(im);

   return (0);
}

/*****************************************************************/
/*                                                               */
/* DAY_GRAPH3  - Day graph with three data sets                  */
/*                                                               */
/*****************************************************************/

int day_graph3(  char *fname,
                 char *title,
               u_long data1[24],
               u_long data2[24],
               u_long data3[24])
{

   /* local variables */
   int i,j,s,x1,y1,x2;
   u_long maxval=0;

   /* initalize the graph */
   init_graph(title,512,256);

   /* index lines? */
   if (graph_lines)
   {
      y1=210/(graph_lines+1);
      for (i=0;i<graph_lines;i++)
       gdImageLine(im,21,((i+1)*y1)+25,489,((i+1)*y1)+25,dkgrey);
   }

   /* x-axis legend */
   for (i=0;i<24;i++)
   {
      gdImageString(im,gdFontSmall,33+(i*19),238,numchar[i],black);
      if (data1[i] > maxval) maxval = data1[i];           /* get max val    */
      if (data2[i] > maxval) maxval = data2[i];
      if (data3[i] > maxval) maxval = data3[i];
   }
   if (maxval <= 0) maxval = 1;
   sprintf(maxvaltxt, "%lu", maxval);
   gdImageStringUp(im, gdFontSmall, 8, 26+(strlen(maxvaltxt)*6),
                   maxvaltxt, black);

   if (graph_legend)                          /* print color coded legends? */
   {
      /* Pages/Files/Hits Legend */
      s = ( i=(strlen(_("Pages"))*6) )+
          ( j=(strlen(_("Files"))*6) )+
          ( strlen(_("Hits"))*6 )+ 52;
      gdImageStringUp(im,gdFontSmall,494,s,_("Pages"),dkgrey);
      gdImageStringUp(im,gdFontSmall,493,s-1,_("Pages"),COLOR5);
      gdImageStringUp(im,gdFontSmall,494,s-i-3,"/",dkgrey);
      gdImageStringUp(im,gdFontSmall,493,s-i-4,"/",black);
      gdImageStringUp(im,gdFontSmall,494,s-i-12,_("Files"),dkgrey);
      gdImageStringUp(im,gdFontSmall,493,s-i-13,_("Files"),COLOR2);
      gdImageStringUp(im,gdFontSmall,494,s-i-j-15,"/",dkgrey);
      gdImageStringUp(im,gdFontSmall,493,s-i-j-16,"/",black);
      gdImageStringUp(im,gdFontSmall,494,s-i-j-24,_("Hits"),dkgrey);
      gdImageStringUp(im,gdFontSmall,493,s-i-j-25,_("Hits"),COLOR1);
   }

   /* data1 */
   for (i=0; i<24; i++)
   {
      percent = ((float)data1[i] / (float)maxval);  /* percent of 100% */
      if (percent <= 0.0) continue;
      x1 = 29 + (i*19);
      x2 = x1 + 10;
      y1 = 232 - (percent * 203);
      gdImageFilledRectangle(im, x1, y1, x2, 232, COLOR1);
      gdImageRectangle(im, x1, y1, x2, 232, black);
   }

   /* data2 */
   for (i=0; i<24; i++)
   {
      percent = ((float)data2[i] / (float)maxval);  /* percent of 100% */
      if (percent <= 0.0) continue;
      x1 = 32 + (i*19);
      x2 = x1 + 10;
      y1 = 232 - (percent * 203);
      gdImageFilledRectangle(im, x1, y1, x2, 232, COLOR2);
      gdImageRectangle(im, x1, y1, x2, 232, black);
   }

   /* data3 */
   for (i=0; i<24; i++)
   {
      percent = ((float)data3[i] / (float)maxval);  /* percent of 100% */
      if (percent <= 0.0) continue;
      x1 = 35 + (i*19);
      x2 = x1 + 10;
      y1 = 232 - (percent * 203);
      gdImageFilledRectangle(im, x1, y1, x2, 232, COLOR5);
      gdImageRectangle(im, x1, y1, x2, 232, black);
   }

   /* stat the file */
   if ( !(lstat(fname, &out_stat)) )
   {
     /* check if the file a symlink */
     if ( S_ISLNK(out_stat.st_mode) )
     {
       if (verbose)
       fprintf(stderr,"%s %s!\n",_("Error: File is a symlink"),fname);
       return(1);
     }
   }

   /* save as png	file */
   if ( (out = fopen(fname, "wb")) != NULL)
   {
      gdImagePng(im, out);
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
              u_long data1[], char *legend[])
{
   int i,x,percent,y=47;
   double s_arc=0.0;
   int purple, ltpurple, ltgreen, brown;
   char buffer[128];

   struct pie_data gdata;

   /* init graph and colors */
   init_graph(title,512,300);
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
         gdImageFill(im, gdata.mx, gdata.my, i+4);

         snprintf(buffer,sizeof(buffer),"%s (%d%%)",legend[i], percent);
         x=480-(strlen(buffer)*7);
         gdImageString(im,gdFontMediumBold, x+1, y+1, buffer, black);
         gdImageString(im,gdFontMediumBold, x, y, buffer, i+4);
         y+=20;
      }
   }

   if (s_arc < 1.0)                         /* anything left over?        */
   {
      gdata=*calc_arc(s_arc,1.0);

      gdImageFill(im, gdata.mx, gdata.my, white);
      snprintf(buffer,sizeof(buffer),"%s (%d%%)",
              _("Other") ,100-(int)(s_arc*100));
      x=480-(strlen(buffer)*7);
      gdImageString(im,gdFontMediumBold, x+1, y+1, buffer, black);
      gdImageString(im,gdFontMediumBold, x, y, buffer, white);
   }

   /* stat the file */
   if ( !(lstat(fname, &out_stat)) )
   {
     /* check if the file a symlink */
     if ( S_ISLNK(out_stat.st_mode) )
     {
       if (verbose)
       fprintf(stderr,"%s %s!\n",_("Error: File is a symlink"),fname);
       return;
     }
   }

   /* save png image */
   if ((out = fopen(fname, "wb")) != NULL)
   {
      gdImagePng(im, out);
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
   cyan    = gdImageColorAllocate(im, 0, 192, 255);
   yellow  = gdImageColorAllocate(im, 255, 255, 0);

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
