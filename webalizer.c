/*
    webalizer - a web server log analysis program

    Copyright (C) 1997-1999  Bradford L. Barrett (brad@mrunix.net)

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

/*********************************************/
/* STANDARD INCLUDES                         */
/*********************************************/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>                           /* normal stuff             */
#include <ctype.h>
#include <sys/utsname.h>
#include <sys/times.h>

/* ensure getopt */
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

/* ensure sys/types */
#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif

/* some systems need this */
#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include "webalizer.h"                        /* main header              */
#include "graphs.h"                           /* graphs header            */
#include "ctry.h"                             /* ctry codes               */
#include "webalizer_lang.h"                   /* lang. support            */

/* SunOS 4.x Fix */
#ifndef CLK_TCK
#define CLK_TCK _SC_CLK_TCK
#endif

/*********************************************/
/* GLOBAL VARIABLES                          */
/*********************************************/

char    *version     = "1.30";                /* program version          */
char    *editlvl     = "04";                  /* edit level               */
char    *moddate     = "11-Jul-1999";         /* modification date        */
char    *copyright   = "Copyright 1997-1999 by Bradford L. Barrett";

FILE    *log_fp, *out_fp;                     /* global file pointers     */

char    buffer[BUFSIZE];                      /* log file record buffer   */
char    tmp_buf[BUFSIZE];                     /* used to temp save above  */

int     verbose      = 2;                     /* 2=verbose,1=err, 0=none  */
int     debug_mode   = 0;                     /* debug mode flag          */
int     time_me      = 0;                     /* timing display flag      */
int     local_time   = 1;                     /* 1=localtime 0=GMT (UTC)  */
int     ignore_hist  = 0;                     /* history flag (1=skip)    */
int     hourly_graph = 1;                     /* hourly graph display     */
int     hourly_stats = 1;                     /* hourly stats table       */
int     ctry_graph   = 1;                     /* country graph display    */
int     shade_groups = 1;                     /* Group shading 0=no 1=yes */
int     hlite_groups = 1;                     /* Group hlite 0=no 1=yes   */
int     mangle_agent = 0;                     /* mangle user agents       */
int     incremental  = 0;                     /* incremental mode 1=yes   */
int     use_https    = 0;                     /* use 'https://' on URL's  */
int     visit_timeout= 3000;                  /* visit timeout (30 min)   */
int     graph_legend = 1;                     /* graph legend (1=yes)     */
int     graph_lines  = 2;                     /* graph lines (0=none)     */
int     fold_seq_err = 0;                     /* fold seq err (0=no)      */
int     log_type     = 0;                     /* Log type (0=web,1=ftp)   */
char    *hname       = NULL;                  /* hostname for reports     */
char    *state_fname = "webalizer.current";   /* run state file name      */
char    *hist_fname  = "webalizer.hist";      /* name of history file     */
char    *html_ext    = "html";                /* HTML file prefix         */
char    *conf_fname  = NULL;                  /* name of config file      */
char    *log_fname   = NULL;                  /* log file pointer         */
char    *out_dir     = NULL;                  /* output directory         */
char    *blank_str   = "";                    /* blank string             */

int     ntop_sites   = 30;                    /* top n sites to display   */
int     ntop_sitesK  = 10;                    /* top n sites (by kbytes)  */
int     ntop_urls    = 30;                    /* top n url's to display   */
int     ntop_urlsK   = 10;                    /* top n url's (by kbytes)  */
int     ntop_entry   = 10;                    /* top n entry url's        */
int     ntop_exit    = 10;                    /* top n exit url's         */
int     ntop_refs    = 30;                    /* top n referrers ""       */
int     ntop_agents  = 15;                    /* top n user agents ""     */
int     ntop_ctrys   = 50;                    /* top n countries   ""     */
int     ntop_search  = 20;                    /* top n search strings     */

int     hist_month[12], hist_year[12];        /* arrays for monthly total */
u_long  hist_hit[12];                         /* calculations: used to    */
u_long  hist_files[12];                       /* produce index.html       */
u_long  hist_site[12];                        /* these are read and saved */
double  hist_xfer[12];                        /* in the history file      */
u_long  hist_page[12];
u_long  hist_visit[12];

int     hist_fday[12], hist_lday[12];         /* first/last day arrays    */

int     cur_year=0, cur_month=0,              /* year/month/day/hour      */
        cur_day=0, cur_hour=0,                /* tracking variables       */
        cur_min=0, cur_sec=0;

u_long  cur_tstamp=0;                         /* Timestamp...             */
u_long  rec_tstamp=0;
u_long  req_tstamp=0;
char    tmp_tstamp[16];                       /* used for ascii->float    */
u_long  epoch;                                /* used for timestamp adj.  */

int     check_dup=0;                          /* check for dup flag       */

double  t_xfer=0.0;                           /* monthly total xfer value */
u_long  t_hit=0,t_file=0,t_site=0,            /* monthly total vars       */
        t_url=0,t_ref=0,t_agent=0,
        t_page=0, t_visit;

double  tm_xfer[31];                          /* daily transfer totals    */

u_long  tm_hit[31], tm_file[31],              /* daily total arrays       */
        tm_site[31], tm_page[31],
        tm_visit[31];

u_long  dt_site;                              /* daily 'sites' total      */

u_long  ht_hit=0, mh_hit=0;                   /* hourly hits totals       */

u_long  th_hit[24], th_file[24],              /* hourly total arrays      */
        th_page[24];

double  th_xfer[24];

int     f_day,l_day;                          /* first/last day vars      */

struct  utsname system_info;                  /* system info structure    */

u_long  ul_bogus =0;                          /* Dummy counter for groups */

struct  log_struct log_rec;                   /* expanded log storage     */

time_t  now;                                  /* used by cur_time funct   */
struct  tm *tp;                               /* to generate timestamp    */
char    timestamp[32];                        /* for the reports          */

HNODEPTR sm_htab[MAXHASH], sd_htab[MAXHASH];  /* hash tables              */
UNODEPTR um_htab[MAXHASH];                    /* for hits, sites,         */
RNODEPTR rm_htab[MAXHASH];                    /* referrers and agents...  */
ANODEPTR am_htab[MAXHASH];
SNODEPTR sr_htab[MAXHASH];                    /* search string table      */

HNODEPTR *top_sites    = NULL;                /* "top" lists              */
CLISTPTR *top_ctrys    = NULL;
UNODEPTR *top_urls     = NULL;
RNODEPTR *top_refs     = NULL;
ANODEPTR *top_agents   = NULL;
SNODEPTR *top_search   = NULL;

/* Linkded list pointers */
GLISTPTR group_sites   = NULL;                /* "group" lists            */
GLISTPTR group_urls    = NULL;
GLISTPTR group_refs    = NULL;
GLISTPTR group_agents  = NULL;
NLISTPTR hidden_sites  = NULL;                /* "hidden" lists           */
NLISTPTR hidden_urls   = NULL;
NLISTPTR hidden_refs   = NULL;
NLISTPTR hidden_agents = NULL;
NLISTPTR ignored_sites = NULL;                /* "Ignored" lists          */
NLISTPTR ignored_urls  = NULL;
NLISTPTR ignored_refs  = NULL;
NLISTPTR ignored_agents= NULL;
NLISTPTR include_sites = NULL;                /* "Include" lists          */
NLISTPTR include_urls  = NULL;
NLISTPTR include_refs  = NULL;
NLISTPTR include_agents= NULL;
NLISTPTR index_alias   = NULL;                /* index. aliases           */
NLISTPTR html_pre      = NULL;                /* before anything else :)  */
NLISTPTR html_head     = NULL;                /* top HTML code            */
NLISTPTR html_body     = NULL;                /* body HTML code           */
NLISTPTR html_post     = NULL;                /* middle HTML code         */
NLISTPTR html_tail     = NULL;                /* tail HTML code           */
NLISTPTR html_end      = NULL;                /* after everything else    */
NLISTPTR page_type     = NULL;                /* page view types          */

/*********************************************/
/* MAIN - start here                         */
/*********************************************/

int main(int argc, char *argv[])
{
   int      i;                           /* generic counter             */
   char     *cp1, *cp2, *cp3, *str;      /* generic char pointers       */
   NLISTPTR lptr;                        /* generic list pointer        */

   extern char *optarg;                  /* used for command line       */
   extern int optind;                    /* parsing routine 'getopt'    */
   extern int opterr;

   time_t start_time, end_time;          /* program timers              */
   struct tms     mytms;                 /* bogus tms structure         */

   int    rec_year,rec_month=1,rec_day,rec_hour,rec_min,rec_sec;

   int    good_rec    =0;                /* 1 if we had a good record   */
   u_long total_rec   =0;                /* Total Records Processed     */
   u_long total_ignore=0;                /* Total Records Ignored       */
   u_long total_bad   =0;                /* Total Bad Records           */

   /* month names used for parsing logfile (shouldn't be lang specific) */
   char *log_month[12]={ "Jan", "Feb", "Mar",
                         "Apr", "May", "Jun",
                         "Jul", "Aug", "Sep",
                         "Oct", "Nov", "Dec"};

   /* initalize epoch */
   epoch=jdate(1,1,1993);                /* used for timestamp adj.     */

   /* add default index. alias */
   add_nlist("index.",&index_alias);

   /* check for default config file */
   if (!access("webalizer.conf",F_OK))
      get_config("webalizer.conf");
   else if (!access("/etc/webalizer.conf",F_OK))
      get_config("/etc/webalizer.conf");

   /* get command line options */
   opterr = 0;     /* disable parser errors */
   while ((i=getopt(argc,argv,"c:dhifFgLpP:qQvVYGHTa:e:l:m:n:o:r:s:t:u:x:A:C:E:I:M:R:S:U:"))!=EOF)
   {
      switch (i)
      {
        case 'd': debug_mode=1;              break;  /* Debug               */
        case 'h': print_opts(argv[0]);       break;  /* help                */
        case 'g': local_time=0;              break;  /* GMT (UTC) time      */
        case 'i': ignore_hist=1;             break;  /* Ignore history      */
        case 'p': incremental=1;             break;  /* Incremental run     */
        case 'q': verbose=1;                 break;  /* Quiet (verbose=1)   */
        case 'Q': verbose=0;                 break;  /* Really Quiet        */
        case 'V':
        case 'v': print_version();           break;  /* Version             */
        case 'G': hourly_graph=0;            break;  /* no hourly graph     */
        case 'H': hourly_stats=0;            break;  /* no hourly stats     */
        case 'M': mangle_agent=atoi(optarg); break;  /* mangle user agents  */
        case 'T': time_me=1;                 break;  /* TimeMe              */
        case 'a': add_nlist(optarg,&hidden_agents); break; /* Hide agents   */
        case 'c': get_config(optarg);        break;  /* Config file         */
        case 'n': hname=optarg;              break;  /* Hostname            */
        case 'o': out_dir=optarg;            break;  /* Output directory    */
        case 'r': add_nlist(optarg,&hidden_refs);   break; /* Hide referrer */
        case 's': add_nlist(optarg,&hidden_sites);  break; /* Hide site     */
        case 't': msg_title=optarg;          break;  /* Report title        */
        case 'u': add_nlist(optarg,&hidden_urls);   break; /* hide URL      */
        case 'x': html_ext=optarg;           break;  /* HTML file extension */
        case 'A': ntop_agents=atoi(optarg);  break;  /* Top agents          */
        case 'C': ntop_ctrys=atoi(optarg);   break;  /* Top countries       */
        case 'I': add_nlist(optarg,&index_alias); break; /* Index alias     */
        case 'R': ntop_refs=atoi(optarg);    break;  /* Top referrers       */
        case 'S': ntop_sites=atoi(optarg);   break;  /* Top sites           */
        case 'U': ntop_urls=atoi(optarg);    break;  /* Top urls            */
        case 'P': add_nlist(optarg,&page_type); break; /* page view types   */
        case 'l': graph_lines=atoi(optarg);  break;  /* Graph Lines         */
        case 'L': graph_legend=0;            break;  /* Graph Legends       */
        case 'm': visit_timeout=atoi(optarg); break; /* Visit Timeout       */
        case 'f': fold_seq_err=1;            break;  /* Fold sequence errs  */
        case 'Y': ctry_graph=0;              break;  /* Supress ctry graph  */
        case 'e': ntop_entry=atoi(optarg);   break;  /* Top entry pages     */
        case 'E': ntop_exit=atoi(optarg);    break;  /* Top exit pages      */
        case 'F': log_type=1;                break;  /* FTP Log type (1)    */
      }
   }

   if (argc - optind != 0) log_fname = argv[optind];

   /* setup our internal variables */
   init_counters();                      /* initalize main counters         */

   if (page_type==NULL)                  /* check if page types present     */
   {
      if (!log_type)
      {
         add_nlist("htm*"  ,&page_type); /* if no page types specified, we  */
         add_nlist("cgi"   ,&page_type); /* use the default ones here...    */
         if (!isinlist(page_type,html_ext)) add_nlist(html_ext,&page_type);
      }
      else add_nlist("txt" ,&page_type); /* FTP logs default to .txt        */
   }

   if (ntop_ctrys > MAX_CTRY) ntop_ctrys = MAX_CTRY;   /* force upper limit */
   if (graph_lines> 20)       graph_lines= 20;         /* keep graphs sane! */

   if (log_type) ntop_entry=ntop_exit=0; /* disable entry/exit for ftp logs */

   /* ensure entry/exits don't exceed urls */
   i=(ntop_urls>ntop_urlsK)?ntop_urls:ntop_urlsK;
   if (ntop_entry>i) ntop_entry=i;
   if (ntop_exit>i)  ntop_exit=i;

   for (i=0;i<MAXHASH;i++)
   {
      sm_htab[i]=sd_htab[i]=NULL;        /* initalize hash tables           */
      um_htab[i]=NULL;
      rm_htab[i]=NULL;
      am_htab[i]=NULL;
      sr_htab[i]=NULL;
   }

   /* Be polite and announce yourself... */
   if (verbose>1)
   {
      uname(&system_info);
      printf("Webalizer V%s-%s (%s %s) %s\n",
              version,editlvl,system_info.sysname,
              system_info.release,language);
   }

   /* open log file */
   if (log_fname)
   {
      log_fp = fopen(log_fname,"r");
      if (!log_fp)
      {
         /* Error: Can't open log file ... */
         fprintf(stderr, "%s %s\n",msg_log_err,log_fname);
         exit(1);
      }
   }

   /* Using logfile ... */
   if (verbose>1) printf("%s %s\n",msg_log_use,log_fname?log_fname:"STDIN");

   /* switch directories if needed */
   if (out_dir)
   {
      if (chdir(out_dir) != 0)
      {
         /* Error: Can't change directory to ... */
         fprintf(stderr, "%s %s\n",msg_dir_err,out_dir);
         exit(1);
      }
   }

   /* Creating output in ... */
   if (verbose>1)
      printf("%s %s\n",msg_dir_use,out_dir?out_dir:msg_cur_dir);

   /* prep hostname */
   if (!hname)
   {
      if (uname(&system_info)) hname="localhost";
      else hname=system_info.nodename;
   }

   /* Hostname for reports is ... */
   if (verbose>1) printf("%s '%s'\n",msg_hostname,hname);

   /* get past history */
   if (ignore_hist) {if (verbose>1) printf("%s\n",msg_ign_hist); }
   else get_history();

   if (incremental)                      /* incremental processing?         */
   {
      if ((i=restore_state()))           /* restore internal data structs   */
      {
         /* Error: Unable to restore run data (error num) */
         if (verbose) fprintf(stderr,"%s (%d)\n",msg_bad_data,i);
         init_counters();
         del_hlist(sd_htab);             /* if error occurs, warn the user  */
         del_ulist(um_htab);             /* and re-initalize our internal   */
         del_hlist(sm_htab);             /* data as if nothing was done...  */
         del_rlist(rm_htab);
         del_alist(am_htab);
         del_slist(sr_htab);
      }
   }

   /* Allocate memory for our TOP arrays */
   i = (ntop_sites>ntop_sitesK)?ntop_sites:ntop_sitesK;
   if (i != 0)            /* top sites */
   { if ( (top_sites=calloc(i,sizeof(HNODEPTR))) == NULL)
    /* Can't get memory, Top Sites disabled! */
    {if (verbose) fprintf(stderr,"%s\n",msg_nomem_ts); ntop_sites=0;}}

   if (ntop_ctrys  != 0)  /* top countries */
   { if ( (top_ctrys=calloc(ntop_ctrys,sizeof(CLISTPTR))) == NULL)
    /* Can't get memory, Top Countries disabled! */
    {if (verbose) fprintf(stderr,"%s\n",msg_nomem_tc); ntop_ctrys=0;}}

   i = (ntop_urls>ntop_urlsK)?ntop_urls:ntop_urlsK;
   if (i != 0)            /* top urls */
   {if ( (top_urls=calloc(i,sizeof(UNODEPTR))) == NULL)
    /* Can't get memory, Top URLs disabled! */
    {if (verbose) fprintf(stderr,"%s\n",msg_nomem_tu); ntop_urls=0;}}

   if (ntop_refs  != 0)   /* top referrers */
   { if ( (top_refs=calloc(ntop_refs,sizeof(RNODEPTR))) == NULL)
    /* Can't get memory, Top Referrers disabled! */
    {if (verbose) fprintf(stderr,"%s\n",msg_nomem_tr); ntop_refs=0;}}

   if (ntop_agents  != 0) /* top user agents */
   {if ( (top_agents=calloc(ntop_agents,sizeof(ANODEPTR))) == NULL)
    /* Can't get memory, Top User Agents disabled! */
    {if (verbose) fprintf(stderr,"%s\n",msg_nomem_ta); ntop_agents=0;}}

   if (ntop_search  !=0)  /* top search strings */
   { if ( (top_search=calloc(ntop_search,sizeof(SNODEPTR))) == NULL)
    /* Can't get memory, Top Search Strings disabled! */
    {if (verbose) fprintf(stderr,"%s\n",msg_nomem_tsr); ntop_search=0;}}

   start_time = times(&mytms);

   /*********************************************/
   /* MAIN PROCESS LOOP - read through log file */
   /*********************************************/

   while ((fgets(buffer, BUFSIZE, log_fname?log_fp:stdin)) != NULL)
   {
      total_rec++;
      if (strlen(buffer) == (BUFSIZE-1))
      {
         if (verbose)
         {
            fprintf(stderr,"%s",msg_big_rec);
            if (debug_mode) fprintf(stderr,":\n%s",buffer);
            else fprintf(stderr,"\n");
         }

         total_bad++;                     /* bump bad record counter      */

         /* get the rest of the record */
         while ( (fgets(buffer,BUFSIZE,log_fname?log_fp:stdin)) != NULL)
         {
            if (strlen(buffer) < BUFSIZE-1)
            {
               if (debug_mode && verbose) fprintf(stderr,"%s\n",buffer);
               break;
            }
            if (debug_mode && verbose) fprintf(stderr,"%s",buffer);
         }
         continue;                        /* go get next record if any    */
      }

      /* got a record... */
      strcpy(tmp_buf, buffer);            /* save buffer in case of error */
      if (parse_record())                 /* parse the record             */
      {
         /*********************************************/
         /* PASSED MINIMAL CHECKS, DO A LITTLE MORE   */
         /*********************************************/

         /* get year/month/day/hour/min/sec values    */
         for (i=0;i<12;i++)
         {
            if (strncmp(log_month[i],&log_rec.datetime[4],3)==0)
               { rec_month = i+1; break; }
         }

         rec_year=atoi(&log_rec.datetime[8]);    /* get year number (int)   */
         rec_day =atoi(&log_rec.datetime[1]);    /* get day number          */
         rec_hour=atoi(&log_rec.datetime[13]);   /* get hour number         */
         rec_min =atoi(&log_rec.datetime[16]);   /* get minute number       */
         rec_sec =atoi(&log_rec.datetime[19]);   /* get second number       */

         /* Kludge for Netscape server time (0-24?) error                   */
         if (rec_hour>23) rec_hour=0;

         /* minimal sanity check on date */
         if ((i>=12)||(rec_min>59)||(rec_sec>59)||(rec_year<1993))
         {
            total_bad++;                /* if a bad date, bump counter      */
            if (verbose)
            {
               fprintf(stderr,"%s: %s [%lu]",
                 msg_bad_date,log_rec.datetime,total_rec);
               if (debug_mode) fprintf(stderr,":\n%s\n",tmp_buf);
               else fprintf(stderr,"\n");
            }
            continue;                   /* and ignore this record           */
         }

         /*********************************************/
         /* GOOD RECORD, CHECK INCREMENTAL/TIMESTAMPS */
         /*********************************************/

         /* Flag as a good one */
         good_rec = 1;

         /* get current records timestamp (jdate-epochHHMMSS) */
         req_tstamp=cur_tstamp;
         rec_tstamp=((jdate(rec_day,rec_month,rec_year)-epoch)*1000000)+
            (rec_hour*10000) + (rec_min*100) + rec_sec;

         /* Do we need to check for duplicate records? (incremental mode)   */
         if (check_dup)
         {
            /* check if less than/equal to last record processed            */
            if ( rec_tstamp <= cur_tstamp )
            {
               /* if it is, assume we have already processed and ignore it  */
               total_ignore++;
               continue;
            }
            else
            {
               /* if it isn't.. disable any more checks this run            */
               check_dup=0;
               /* now check if it's a new month                             */
               if (cur_month != rec_month)
               {
                  clear_month();
                  cur_sec   = rec_sec;          /* set current counters     */
                  cur_min   = rec_min;
                  cur_hour  = rec_hour;
                  cur_day   = rec_day;
                  cur_month = rec_month;
                  cur_year  = rec_year;
                  cur_tstamp= rec_tstamp;
                  f_day=l_day=rec_day;          /* reset first and last day */
               }
            }
         }

         /* check for out of sequence records */
         if (rec_tstamp/10000 < cur_tstamp/10000)
         {
            if (!fold_seq_err) { total_ignore++; continue; }
            else
            {
               rec_sec   = cur_sec;             /* if folding sequence      */
               rec_min   = cur_min;             /* errors, just make it     */
               rec_hour  = cur_hour;            /* look like the last       */
               rec_day   = cur_day;             /* good records timestamp   */
               rec_month = cur_month;
               rec_year  = cur_year;
               rec_tstamp= cur_tstamp;
            }
         }
         cur_tstamp=rec_tstamp;                 /* update current timestamp */

         /*********************************************/
         /* DO SOME PRE-PROCESS FORMATTING            */
         /*********************************************/

         /* lowercase hostname */
         cp1 = log_rec.hostname;
         while (*cp1 != '\0')
         {
            if ( (*cp1>='A') && (*cp1<='Z')) *cp1 += 'a'-'A';
            cp1++;
         }

         /* fix URL field */
         cp1 = cp2 = log_rec.url;
         /* handle null '-' case here... */
         if (*++cp1 == '-') { *cp2++ = '-'; *cp2 = '\0'; }
         else
         {
            /* strip actual URL out of request */
            while  ( (*cp1 != ' ') && (*cp1 != '\0') ) cp1++;
            if (*cp1 != '\0')
            {
               /* scan to begin of actual URL field */
               while ((*cp1 == ' ') && (*cp1 != '\0')) cp1++;
               /* remove duplicate / if needed */
               if (( *cp1=='/') && (*(cp1+1)=='/')) cp1++;
               while ((*cp1 != ' ')&&(*cp1 != '"')&&(*cp1 != '\0'))
                  *cp2++ = *cp1++;
               *cp2 = '\0';
            }
         }

         /* un-escape URL */
         unescape(log_rec.url);

         /* check for service (ie: http://) and lowercase if found */
         if ( (cp2=strstr(log_rec.url,"://")) != NULL)
         {
            cp1=log_rec.url;
            while (cp1!=cp2)
            {
               if ( (*cp1>='A') && (*cp1<='Z')) *cp1 += 'a'-'A';
               cp1++;
            }
         }

         /* strip query portion of cgi scripts */
         cp1 = log_rec.url;
         while (*cp1 != '\0')
           if (!isurlchar(*cp1)) { *cp1 = '\0'; break; }
           else cp1++;
         if (log_rec.url[0]=='\0')
           { log_rec.url[0]='/'; log_rec.url[1]='\0'; }

         /* strip off index.html (or any aliases) */
         lptr=index_alias;
         while (lptr!=NULL)
         {
            if ((cp1=strstr(log_rec.url,lptr->string))!=NULL)
            {
               if ((cp1==log_rec.url)||(*(cp1-1)=='/'))
               {
                  *cp1='\0';
                  if (log_rec.url[0]=='\0')
                   { log_rec.url[0]='/'; log_rec.url[1]='\0'; }
                  break;
               }
            }
            lptr=lptr->next;
         }

         /* fix referrer field */
         cp1 = log_rec.refer;
         cp3 = cp2 = cp1++;
         if ( (*cp2 != '\0') && (*cp2 == '"') )
         {
            while ( *cp1 != '\0' ) { cp3 = cp2; *cp2++ = *cp1++; }
            *cp3 = '\0';
         }

         /* unescape referrer */
         unescape(log_rec.refer);

         /* strip query portion of cgi referrals */
         cp1 = log_rec.refer;
         if (*cp1 != '\0')
         {
            while (*cp1 != '\0')
            {
               if (!isurlchar(*cp1))
               {
                  *cp1++='\0'; /* save search string */
                  strncpy(log_rec.srchstr,cp1,MAXSRCH);
                  break;
               }
               else cp1++;
            }
            /* handle null referrer */
            if (log_rec.refer[0]=='\0')
              { log_rec.refer[0]='-'; log_rec.refer[1]='\0'; }
         }

         /* if HTTP request, lowercase http://sitename/ portion */
         cp1 = log_rec.refer;
         if ( (*cp1=='h') || (*cp1=='H'))
         {
            while ( (*cp1!='/') && (*cp1!='\0'))
            {
               if ( (*cp1>='A') && (*cp1<='Z')) *cp1 += 'a'-'A';
               cp1++;
            }
            /* now do hostname */
            if ( (*cp1=='/') && ( *(cp1+1)=='/')) {cp1++; cp1++;}
            while ( (*cp1!='/') && (*cp1!='\0'))
            {
               if ( (*cp1>='A') && (*cp1<='Z')) *cp1 += 'a'-'A';
               cp1++;
            }
         }

         /* Do we need to mangle? */
         if (mangle_agent)
         {
            str=cp2=log_rec.agent;
	    cp1=strstr(str,"ompatible"); /* check known fakers */
	    if (cp1!=NULL) {
		while (*cp1!=';'&&*cp1!='\0') cp1++;
		/* kludge for Mozilla/3.01 (compatible;) */
		if (*cp1++==';' && strcmp(cp1,")\"")) { /* success! */
		    while (*cp1 == ' ') cp1++; /* eat spaces */
		    while (*cp1!='.'&&*cp1!='\0'&&*cp1!=';') *cp2++=*cp1++;
		    if (mangle_agent<5)
		    {
			while (*cp1!='.'&&*cp1!=';'&&*cp1!='\0') *cp2++=*cp1++;
			if (*cp1!=';'&&*cp1!='\0') {
			    *cp2++=*cp1++;
			    *cp2++=*cp1++;
			}
		    }
		    if (mangle_agent<4)
			if (*cp1>='0'&&*cp1<='9') *cp2++=*cp1++;
		    if (mangle_agent<3)
			while (*cp1!=';'&&*cp1!='\0'&&*cp1!=')') *cp2++=*cp1++;
		    if (mangle_agent<2)
		    {
			/* Level 1 - try to get OS */
			cp1=strstr(str,")");
			if (cp1!=NULL)
			{
			    *cp2++=' ';
			    *cp2++='(';
			    while (*cp1!=';'&&*cp1!='('&&cp1!=str) cp1--;
			    if (cp1!=str&&*cp1!='\0') cp1++;
			    while (*cp1==' '&&*cp1!='\0') cp1++;
			    while (*cp1!=')'&&*cp1!='\0') *cp2++=*cp1++;
			    *cp2++=')';
			}
		    }
		    *cp2='\0';
		} else { /* nothing after "compatible", should we mangle? */
		    /* not for now */
		}
	    } else {
		cp1=strstr(str,"Opera");  /* Netscape flavor      */
		if (cp1!=NULL)
		{
		    while (*cp1!=' '&&*cp1!=' '&&*cp1!='\0') *cp2++=*cp1++;
		    while (*cp1!='.'&&*cp1!='\0') *cp2++=*cp1++;
		    if (mangle_agent<5)
		    {
			while (*cp1!='.'&&*cp1!='\0') *cp2++=*cp1++;
			*cp2++=*cp1++;
			*cp2++=*cp1++;
		    }
		    if (mangle_agent<4)
			if (*cp1>='0'&&*cp1<='9') *cp2++=*cp1++;
		    if (mangle_agent<3)
			while (*cp1!=' '&&*cp1!='\0'&&*cp1!=')')
			    *cp2++=*cp1++;
		    if (mangle_agent<2)
		    {
			cp1=strstr(str,"(");
			if (cp1!=NULL)
			{
			    cp1++;
			    *cp2++=' ';
			    *cp2++='(';
			    while (*cp1!=';'&&*cp1!=')'&&*cp1!='\0')
				*cp2++=*cp1++;
			    *cp2++=')';
			}
		    }
		    *cp2='\0';
		} else {
		    cp1=strstr(str,"Mozilla");  /* Netscape flavor      */
		    if (cp1!=NULL)
		    {
			while (*cp1!='/'&&*cp1!=' '&&*cp1!='\0') *cp2++=*cp1++;
			if (*cp1==' ') *cp1='/';
			while (*cp1!='.'&&*cp1!='\0') *cp2++=*cp1++;
			if (mangle_agent<5)
			{
			    while (*cp1!='.'&&*cp1!='\0') *cp2++=*cp1++;
			    *cp2++=*cp1++;
			    *cp2++=*cp1++;
			}
			if (mangle_agent<4)
			    if (*cp1>='0'&&*cp1<='9') *cp2++=*cp1++;
			if (mangle_agent<3)
			    while (*cp1!=' '&&*cp1!='\0'&&*cp1!=')')
				*cp2++=*cp1++;
			if (mangle_agent<2)
			{
			    /* Level 1 - Try to get OS */
			    cp1=strstr(str,"(");
			    if (cp1!=NULL)
			    {
				cp1++;
				*cp2++=' ';
				*cp2++='(';
				while (*cp1!=';'&&*cp1!=')'&&*cp1!='\0')
				    *cp2++=*cp1++;
				*cp2++=')';
			    }
			}
			*cp2='\0';
		    }
		}
	    }
	 }

         /* if necessary, shrink referrer to fit storage */
         if (strlen(log_rec.refer)>=MAXREFH)
         {
            if (verbose) fprintf(stderr,"%s [%lu]\n",
                msg_big_ref,total_rec);
            log_rec.refer[MAXREFH-1]='\0';
         }

         /* if necessary, shrink URL to fit storage */
         if (strlen(log_rec.url)>MAXURLH)
         {
            if (verbose) fprintf(stderr,"%s [%lu]\n",
                msg_big_req,total_rec);
            log_rec.url[MAXURLH-1]='\0';
         }


         /* fix user agent field */
         cp1 = log_rec.agent;
         cp3 = cp2 = cp1++;
         if ( (*cp2 != '\0') && ((*cp2 == '"')||(*cp2 == '(')) )
         {
            while (*cp1 |= '\0') { cp3 = cp2; *cp2++ = *cp1++; }
            *cp3 = '\0';
         }

         /********************************************/
         /* PROCESS RECORD                           */
         /********************************************/

         /* first time through? */
         if (cur_month == 0)
         {
             /* if yes, init our date vars */
             cur_month=rec_month; cur_year=rec_year;
             cur_day=rec_day; cur_hour=rec_hour;
             cur_min=rec_min; cur_sec=rec_sec;
             f_day=rec_day;
         }

         /* adjust last day processed if different */
         if (rec_day > l_day) l_day = rec_day;

         /* update min/sec stuff */
         if (cur_sec != rec_sec) cur_sec = rec_sec;
         if (cur_min != rec_min) cur_min = rec_min;

         /* check for hour change  */
         if (cur_hour != rec_hour)
         {
            /* if yes, init hourly stuff */
            if (ht_hit > mh_hit) mh_hit = ht_hit;
            ht_hit = 0;
            cur_hour = rec_hour;
         }

         /* check for day change   */
         if (cur_day != rec_day)
         {
            /* if yes, init daily stuff */
            tm_site[cur_day-1]=dt_site; dt_site=0;
            tm_visit[cur_day-1]=tot_visit(sd_htab);
            del_hlist(sd_htab);
            cur_day = rec_day;
         }

         /* check for month change */
         if (cur_month != rec_month)
         {
            /* if yes, do monthly stuff */
            t_visit=tot_visit(sm_htab);
            month_update_exit(req_tstamp);    /* process exit pages      */
            write_month_html();               /* generate HTML for month */
            clear_month();
            cur_month = rec_month;            /* update our flags        */
            cur_year  = rec_year;
            f_day=l_day=rec_day;
         }

         /* Ignore/Include check */
         if ( (isinlist(include_sites,log_rec.hostname)==NULL) &&
              (isinlist(include_urls,log_rec.url)==NULL)       &&
              (isinlist(include_refs,log_rec.refer)==NULL)     &&
              (isinlist(include_agents,log_rec.agent)==NULL)   )
         {
            if (isinlist(ignored_sites,log_rec.hostname)!=NULL)
              { total_ignore++; continue; }
            if (isinlist(ignored_urls,log_rec.url)!=NULL)
              { total_ignore++; continue; }
            if (isinlist(ignored_agents,log_rec.agent)!=NULL)
              { total_ignore++; continue; }
            if (isinlist(ignored_refs,log_rec.refer)!=NULL)
              { total_ignore++; continue; }
         }

         /* Bump response code totals */
         switch (log_rec.resp_code) {
          case RC_CONTINUE:         i=IDX_CONTINUE;         break;
          case RC_SWITCHPROTO:      i=IDX_SWITCHPROTO;      break;
          case RC_OK:               i=IDX_OK;               break;
          case RC_CREATED:          i=IDX_CREATED;          break;
          case RC_ACCEPTED:         i=IDX_ACCEPTED;         break;
          case RC_NONAUTHINFO:      i=IDX_NONAUTHINFO;      break;
          case RC_NOCONTENT:        i=IDX_NOCONTENT;        break;
          case RC_RESETCONTENT:     i=IDX_RESETCONTENT;     break;
          case RC_PARTIALCONTENT:   i=IDX_PARTIALCONTENT;   break;
          case RC_MULTIPLECHOICES:  i=IDX_MULTIPLECHOICES;  break;
          case RC_MOVEDPERM:        i=IDX_MOVEDPERM;        break;
          case RC_MOVEDTEMP:        i=IDX_MOVEDTEMP;        break;
          case RC_SEEOTHER:         i=IDX_SEEOTHER;         break;
          case RC_NOMOD:            i=IDX_NOMOD;            break;
          case RC_USEPROXY:         i=IDX_USEPROXY;         break;
	  case RC_MOVEDTEMPORARILY: i=IDX_MOVEDTEMPORARILY; break;
          case RC_BAD:              i=IDX_BAD;              break;
          case RC_UNAUTH:           i=IDX_UNAUTH;           break;
          case RC_PAYMENTREQ:       i=IDX_PAYMENTREQ;       break;
          case RC_FORBIDDEN:        i=IDX_FORBIDDEN;        break;
          case RC_NOTFOUND:         i=IDX_NOTFOUND;         break;
          case RC_METHODNOTALLOWED: i=IDX_METHODNOTALLOWED; break;
          case RC_NOTACCEPTABLE:    i=IDX_NOTACCEPTABLE;    break;
          case RC_PROXYAUTHREQ:     i=IDX_PROXYAUTHREQ;     break;
          case RC_TIMEOUT:          i=IDX_TIMEOUT;          break;
          case RC_CONFLICT:         i=IDX_CONFLICT;         break;
          case RC_GONE:             i=IDX_GONE;             break;
          case RC_LENGTHREQ:        i=IDX_LENGTHREQ;        break;
          case RC_PREFAILED:        i=IDX_PREFAILED;        break;
          case RC_REQENTTOOLARGE:   i=IDX_REQENTTOOLARGE;   break;
          case RC_REQURITOOLARGE:   i=IDX_REQURITOOLARGE;   break;
          case RC_UNSUPMEDIATYPE:   i=IDX_UNSUPMEDIATYPE;   break;
	  case RC_RNGNOTSATISFIABLE:i=IDX_RNGNOTSATISFIABLE;break;
	  case RC_EXPECTATIONFAILED:i=IDX_EXPECTATIONFAILED;break;
          case RC_SERVERERR:        i=IDX_SERVERERR;        break;
          case RC_NOTIMPLEMENTED:   i=IDX_NOTIMPLEMENTED;   break;
          case RC_BADGATEWAY:       i=IDX_BADGATEWAY;       break;
          case RC_UNAVAIL:          i=IDX_UNAVAIL;          break;
          case RC_GATEWAYTIMEOUT:   i=IDX_GATEWAYTIMEOUT;   break;
          case RC_BADHTTPVER:       i=IDX_BADHTTPVER;       break;
          default:                  i=IDX_UNDEFINED;        break;
         }
         response[i].count++;

         /* now save in the various hash tables... */
         if (log_rec.resp_code==RC_OK)
            i=1; else i=0;

         /* URL hash table */
         if ((log_rec.resp_code==RC_OK)||(log_rec.resp_code==RC_NOMOD))
            if (put_unode(log_rec.url,OBJ_REG,(u_long)1,
                log_rec.xfer_size,&t_url,(u_long)0,(u_long)0,um_htab))
            {
               if (verbose)
               /* Error adding URL node, skipping ... */
               fprintf(stderr,"%s %s\n", msg_nomem_u, log_rec.url);
            }

         /* referrer hash table */
         if (ntop_refs)
         {
            if (log_rec.refer[0]!='\0')
             if (put_rnode(log_rec.refer,OBJ_REG,(u_long)1,&t_ref,rm_htab))
             {
              if (verbose)
              fprintf(stderr,"%s %s\n", msg_nomem_r, log_rec.refer);
             }
         }

         /* hostname (site) hash table - daily */
         if (put_hnode(log_rec.hostname,OBJ_REG,
             1,(u_long)i,log_rec.xfer_size,&dt_site,
             0,rec_tstamp,"",sd_htab))
         {
            if (verbose)
            /* Error adding host node (daily), skipping .... */
            fprintf(stderr,"%s %s\n",msg_nomem_dh, log_rec.hostname);
         }

         /* hostname (site) hash table - monthly */
         if (put_hnode(log_rec.hostname,OBJ_REG,
             1,(u_long)i,log_rec.xfer_size,&t_site,
             0,rec_tstamp,"",sm_htab))
         {
            if (verbose)
            /* Error adding host node (monthly), skipping .... */
            fprintf(stderr,"%s %s\n", msg_nomem_mh, log_rec.hostname);
         }

         /* user agent hash table */
         if (ntop_agents)
         {
            if (log_rec.agent[0]!='\0')
             if (put_anode(log_rec.agent,OBJ_REG,(u_long)1,&t_agent,am_htab))
             {
              if (verbose)
              fprintf(stderr,"%s %s\n", msg_nomem_a, log_rec.agent);
             }
         }

         /* bump monthly/daily/hourly totals        */
         t_hit++; ht_hit++;                         /* daily/hourly hits    */
         t_xfer += log_rec.xfer_size;               /* total xfer size      */
         tm_xfer[rec_day-1] += log_rec.xfer_size;   /* daily xfer total     */
         tm_hit[rec_day-1]++;                       /* daily hits total     */
         th_xfer[rec_hour] += log_rec.xfer_size;    /* hourly xfer total    */
         th_hit[rec_hour]++;                        /* hourly hits total    */

         /* if RC_OK, increase file counters */
         if (log_rec.resp_code == RC_OK)
         {
            t_file++;
            tm_file[rec_day-1]++;
            th_file[rec_hour]++;
         }

         /* Pages (pageview) calculation */
         if (ispage(log_rec.url))
         {
            t_page++;
            tm_page[rec_day-1]++;
            th_page[rec_hour]++;
         }

         /*********************************************/
         /* RECORD PROCESSED - DO GROUPS HERE         */
         /*********************************************/

         /* URL Grouping */
         if ( (cp1=isinglist(group_urls,log_rec.url))!=NULL)
         {
            if (put_unode(cp1,OBJ_GRP,(u_long)1,log_rec.xfer_size,
                &ul_bogus,(u_long)0,(u_long)0,um_htab))
            {
               if (verbose)
               /* Error adding URL node, skipping ... */
               fprintf(stderr,"%s %s\n", msg_nomem_u, cp1);
            }
         }

         /* Site Grouping */
         if ( (cp1=isinglist(group_sites,log_rec.hostname))!=NULL)
         {
            if (put_hnode(cp1,OBJ_GRP,1,(u_long)(log_rec.resp_code==RC_OK)?1:0,
                          log_rec.xfer_size,&ul_bogus,
                          0,rec_tstamp,"",sm_htab))
            {
               if (verbose)
               /* Error adding Site node, skipping ... */
               fprintf(stderr,"%s %s\n", msg_nomem_mh, cp1);
            }
         }

         /* Referrer Grouping */
         if ( (cp1=isinglist(group_refs,log_rec.refer))!=NULL)
         {
            if (put_rnode(cp1,OBJ_GRP,(u_long)1,&ul_bogus,rm_htab))
            {
               if (verbose)
               /* Error adding Referrer node, skipping ... */
               fprintf(stderr,"%s %s\n", msg_nomem_r, cp1);
            }
         }

         /* User Agent Grouping */
         if ( (cp1=isinglist(group_agents,log_rec.agent))!=NULL)
         {
            if (put_anode(cp1,OBJ_GRP,(u_long)1,&ul_bogus,am_htab))
            {
               if (verbose)
               /* Error adding User Agent node, skipping ... */
               fprintf(stderr,"%s %s\n", msg_nomem_a, cp1);
            }
         }
      }

      /*********************************************/
      /* BAD RECORD                                */
      /*********************************************/

      else
      {
         /* If first record, check if stupid Netscape header stuff      */
         if ( (total_rec==1) && (strncmp(buffer,"format=",7)==0) )
         {
            /* Skipping Netscape header record */
            if (verbose>1) printf("%s\n",msg_ign_nscp);
            /* count it as ignored... */
            total_ignore++;
         }
         else
         {
            /* really bad record... */
            total_bad++;
            if (verbose)
            {
               fprintf(stderr,"%s (%lu)",msg_bad_rec,total_rec);
               if (debug_mode) fprintf(stderr,":\n%s\n",tmp_buf);
               else fprintf(stderr,"\n");
            }
         }
      }
   }

   /*********************************************/
   /* DONE READING LOG FILE - final processing  */
   /*********************************************/

   /* only close log if really a file */
   if (log_fname) fclose(log_fp);            /* close log file           */

   if (good_rec)                             /* were any good records?   */
   {
      tm_site[cur_day-1]=dt_site;            /* If yes, clean up a bit   */
      tm_visit[cur_day-1]=tot_visit(sd_htab);
      t_visit=tot_visit(sm_htab);
      if (ht_hit > mh_hit) mh_hit = ht_hit;

      if (total_rec > (total_ignore+total_bad)) /* did we process any?   */
      {
         if (incremental)
         {
            if (save_state())                /* incremental stuff        */
            {
               /* Error: Unable to save current run data */
               if (verbose) fprintf(stderr,"%s\n",msg_data_err);
               unlink(state_fname);
            }
         }
         month_update_exit(rec_tstamp);      /* calculate exit pages     */
         write_month_html();                 /* write monthly HTML file  */
         write_main_index();                 /* write main HTML file     */
         put_history();                      /* write history            */
      }

      end_time = times(&mytms);              /* display timing totals?   */
      if (time_me || (verbose>1))
      {
         printf("%lu %s ",total_rec, msg_records);
         if (total_ignore)
         {
            printf("(%lu %s",total_ignore,msg_ignored);
            if (total_bad) printf(", %lu %s) ",total_bad,msg_bad);
               else        printf(") ");
         }
         else if (total_bad) printf("(%lu %s) ",total_bad,msg_bad);

         printf("%s %.2f %s", msg_in, (float)(end_time-start_time)/CLK_TCK,
                                msg_seconds);

         /* calculate records per second */
         i=((int)((float)total_rec / ((float)(end_time-start_time)/CLK_TCK)) );
         if ( (i>0) && (i<=total_rec) ) printf(", %d/sec\n", i);
            else  printf("\n");
      }
      /* Whew, all done! Exit with completion status (0) */
      exit(0);
   }
   else
   {
      /* No valid records found... exit with error (1) */
      if (verbose) printf("%s\n",msg_no_vrec);
      exit(1);
   }
}

/*********************************************/
/* FMT_LOGREC - terminate log fields w/zeros */
/*********************************************/

void fmt_logrec()
{
   char *cp=buffer;
   int  q=0,b=0,p=0;

   while (*cp != '\0')
   {
      /* break record up, terminate fields with '\0' */
      switch (*cp)
      {
       case ' ': if (b || q || p) break; *cp='\0'; break;
       case '"': q=(q)?0:1; break;
       case '[': if (q) break; b++; break;
       case ']': if (q) break; if (b>0) b--; break;
       case '(': if (q) break; p++; break;
       case ')': if (q) break; if (p>0) p--; break;
      }
      cp++;
   }
}

/*********************************************/
/* PARSE_RECORD - uhhh, you know...          */
/*********************************************/

int parse_record()
{
   /* clear out structure */
   log_rec.hostname[0]=0;
   log_rec.datetime[0]=0;
   log_rec.url[0]=0;
   log_rec.resp_code=0;
   log_rec.xfer_size=0;
   log_rec.refer[0]=0;
   log_rec.agent[0]=0;
   log_rec.srchstr[0]=0;

   /* call appropriate handler */
   if (log_type) return parse_record_ftp();
    else         return parse_record_web();
}

/*********************************************/
/* PARSE_RECORD_FTP - ftp log handler        */
/*********************************************/

int parse_record_ftp()
{
   int size;
   int i,j;
   char *cp1, *cp2, *cpx, *cpy, *eob;

   size = strlen(buffer);                 /* get length of buffer        */
   eob = buffer+size;                     /* calculate end of buffer     */
   fmt_logrec();                          /* seperate fields with \0's   */

   /* Start out with date/time       */
   cp1=buffer;
   while (*cp1!=0 && cp1<eob) cp1++;
   while (*cp1==0) cp1++;
   cpx=cp1;       /* save month name */
   while (*cp1!=0 && cp1<eob) cp1++;
   while (*cp1==0) cp1++;
   i=atoi(cp1);   /* get day number  */
   while (*cp1!=0 && cp1<eob) cp1++;
   while (*cp1==0) cp1++;
   cpy=cp1;       /* get timestamp   */
   while (*cp1!=0 && cp1<eob) cp1++;
   while (*cp1==0) cp1++;
   j=atoi(cp1);   /* get year        */

   /* minimal sanity check */
   if (*(cpy+2)!=':' || *(cpy+5)!=':') return 0;
   if (j<1993 || j>2100) return 0;
   if (i<1 || i>31) return 0;

   /* format date/time field         */
   sprintf(log_rec.datetime,"[%02d/%s/%4d:%s -0000]",i,cpx,j,cpy);

   /* skip seconds... */
   while (*cp1!=0 && cp1<eob) cp1++;
   while (*cp1==0) cp1++;
   while (*cp1!=0 && cp1<eob) cp1++;
   while (*cp1==0) cp1++;

   /* get hostname */
   cp2=log_rec.hostname;
   strncpy(cp2,cp1,MAXHOST-1);

   /* get filesize */
   while (*cp1!=0 && cp1<eob) cp1++;
   while (*cp1==0) cp1++;
   log_rec.xfer_size = strtoul(cp1,NULL,10);

   /* URL stuff */
   while (*cp1!=0 && cp1<eob) cp1++;
   while (*cp1==0) cp1++;
   cpx=cp1;
   /* get next field for later */
   while (*cp1!=0 && cp1<eob) cp1++;
   while (*cp1==0) cp1++;
   if (strlen(cpx)>MAXURL-20) *(cpx+(MAXURL-20))=0;

   /* skip next two */
   while (*cp1!=0 && cp1<eob) cp1++;
   while (*cp1==0) cp1++;
   while (*cp1!=0 && cp1<eob) cp1++;
   while (*cp1==0) cp1++;

   /* fabricate an appropriate request string based on direction */
   if (*cp1=='i') sprintf(log_rec.url,"\"POST %s HTTP/1.0\"",cpx);
      else        sprintf(log_rec.url,"\"GET %s HTTP/1.0\"",cpx);

   /* return appropriate response code */
   log_rec.resp_code=(*(eob-2)=='i')?206:200;

   return 1;
}

/*********************************************/
/* PARSE_RECORD_WEB - web log handler        */
/*********************************************/

int parse_record_web()
{
   int size;
   char *cp1, *cp2, *cpx, *eob, *eos;

   size = strlen(buffer);                 /* get length of buffer        */
   eob = buffer+size;                     /* calculate end of buffer     */
   fmt_logrec();                          /* seperate fields with \0's   */

   /* HOSTNAME */
   cp1 = cpx = buffer; cp2=log_rec.hostname;
   eos = (cp1+MAXHOST)-1;
   if (eos >= eob) eos=eob-1;

   while ( (*cp1 != '\0') && (cp1 != eos) ) *cp2++ = *cp1++;
   *cp2 = '\0';
   if (*cp1 != '\0')
   {
      if (verbose)
      {
         fprintf(stderr,"%s",msg_big_host);
         if (debug_mode) fprintf(stderr,": %s\n",cpx);
         else fprintf(stderr,"\n");
      }
      while (*cp1 != '\0') cp1++;
   }
   if (cp1 < eob) cp1++;

   /* skip next two fields (ident and auth) */
   while ( (*cp1 != '\0') && (cp1 < eob) ) cp1++;
   if (cp1 < eob) cp1++;
   while ( (*cp1 != '\0') && (cp1 < eob) ) cp1++;
   if (cp1 < eob) cp1++;
   /* space in auth_name? */
   if (*cp1 != '[')
      while ( (*cp1 != '[') && (cp1 < eob) ) cp1++;

   if (cp1 >= eob) return 0;

   /* date/time string */
   cpx = cp1;
   cp2 = log_rec.datetime;
   eos = (cp1+28);
   if (eos >= eob) eos=eob-1;

   while ( (*cp1 != '\0') && (cp1 != eos) ) *cp2++ = *cp1++;
   *cp2 = '\0';
   if (*cp1 != '\0')
   {
      if (verbose)
      {
         fprintf(stderr,"%s",msg_big_date);
         if (debug_mode) fprintf(stderr,": %s\n",cpx);
         else fprintf(stderr,"\n");
      }
      while (*cp1 != '\0') cp1++;
   }
   if (cp1 < eob) cp1++;

   /* minimal sanity check on timestamp */
   if ( (log_rec.datetime[0] != '[') ||
        (log_rec.datetime[3] != '/') ||
        (cp1 >= eob))  return 0;

   /* HTTP request */
   cpx = cp1;
   cp2 = log_rec.url;
   eos = (cp1+MAXURL-1);
   if (eos >= eob) eos = eob-1;

   while ( (*cp1 != '\0') && (cp1 != eos) ) *cp2++ = *cp1++;
   *cp2 = '\0';
   if (*cp1 != '\0')
   {
      if (verbose)
      {
         fprintf(stderr,"%s",msg_big_req);
         if (debug_mode) fprintf(stderr,": %s\n",cpx);
         else fprintf(stderr,"\n");
      }
      while (*cp1 != '\0') cp1++;
   }
   if (cp1 < eob) cp1++;

   if ( (log_rec.url[0] != '"') ||
        (cp1 >= eob) ) return 0;

   /* response code */
   log_rec.resp_code = atoi(cp1);

   /* xfer size */
   while ( (*cp1 != '\0') && (cp1 < eob) ) cp1++;
   if (cp1 < eob) cp1++;
   if (*cp1<'0'||*cp1>'9') log_rec.xfer_size=0;
   else log_rec.xfer_size = strtoul(cp1,NULL,10);

   /* done with CLF record */
   if (cp1>=eob) return 1;

   while ( (*cp1 != '\0') && (cp1 < eob) ) cp1++;
   if (cp1 < eob) cp1++;
   /* get referrer if present */
   cpx = cp1;
   cp2 = log_rec.refer;
   eos = (cp1+MAXREF-1);
   if (eos >= eob) eos = eob-1;

   while ( (*cp1 != '\0') && (cp1 != eos) ) *cp2++ = *cp1++;
   *cp2 = '\0';
   if (*cp1 != '\0')
   {
      if (verbose)
      {
         fprintf(stderr,"%s",msg_big_ref);
         if (debug_mode) fprintf(stderr,": %s\n",cpx);
         else fprintf(stderr,"\n");
      }
      while (*cp1 != '\0') cp1++;
   }
   if (cp1 < eob) cp1++;

   cpx = cp1;
   cp2 = log_rec.agent;
   eos = cp1+(MAXAGENT-1);
   if (eos >= eob) eos = eob-1;

   while ( (*cp1 != '\0') && (cp1 != eos) ) *cp2++ = *cp1++;
   *cp2 = '\0';

   return 1;     /* maybe a valid record, return with TRUE */
}

/*********************************************/
/* CLEAR_MONTH - initalize monthly stuff     */
/*********************************************/

void clear_month()
{
   int i,j;

   init_counters();                  /* reset monthly counters  */
   del_hlist(sd_htab);               /* and clear tables        */
   del_ulist(um_htab);
   del_hlist(sm_htab);
   del_rlist(rm_htab);
   del_alist(am_htab);
   del_slist(sr_htab);
   j=(ntop_sites>ntop_sitesK)?ntop_sites:ntop_sitesK;
   if (ntop_sites!=0 ) for (i=0;i<j;i++)  top_sites[i]=NULL;
   if (ntop_ctrys!=0 ) for (i=0;i<ntop_ctrys;i++)  top_ctrys[i]=NULL;
   j=(ntop_urls>ntop_urlsK)?ntop_urls:ntop_urlsK;
   if (ntop_urls!=0 )  for (i=0;i<j;i++)   top_urls[i]=NULL;
   if (ntop_refs!=0 )  for (i=0;i<ntop_refs;i++)   top_refs[i]=NULL;
   if (ntop_agents!=0) for (i=0;i<ntop_agents;i++) top_agents[i]=NULL;
   if (ntop_search!=0) for (i=0;i<ntop_search;i++) top_search[i]=NULL;
}

/*********************************************/
/* GET_CONFIG - get configuration file info  */
/*********************************************/

void get_config(char *fname)
{
   char *kwords[]= { "Undefined",         /* 0 = undefined keyword       0  */
                     "OutputDir",         /* Output directory            1  */
                     "LogFile",           /* Log file to use for input   2  */
                     "ReportTitle",       /* Title for reports           3  */
                     "HostName",          /* Hostname to use             4  */
                     "IgnoreHist",        /* Ignore history file         5  */
                     "Quiet",             /* Run in quiet mode           6  */
                     "TimeMe",            /* Produce timing results      7  */
                     "Debug",             /* Produce debug information   8  */
                     "HourlyGraph",       /* Hourly stats graph          9  */
                     "HourlyStats",       /* Hourly stats table         10  */
                     "TopSites",          /* Top sites                  11  */
                     "TopURLs",           /* Top URL's                  12  */
                     "TopReferrers",      /* Top Referrers              13  */
                     "TopAgents",         /* Top User Agents            14  */
                     "TopCountries",      /* Top Countries              15  */
                     "HideSite",          /* Sites to hide              16  */
                     "HideURL",           /* URL's to hide              17  */
                     "HideReferrer",      /* Referrers to hide          18  */
                     "HideAgent",         /* User Agents to hide        19  */
                     "IndexAlias",        /* Aliases for index.html     20  */
                     "HTMLHead",          /* HTML Top1 code             21  */
                     "HTMLPost",          /* HTML Top2 code             22  */
                     "HTMLTail",          /* HTML Tail code             23  */
                     "MangleAgents",      /* Mangle User Agents         24  */
                     "IgnoreSite",        /* Sites to ignore            25  */
                     "IgnoreURL",         /* Url's to ignore            26  */
                     "IgnoreReferrer",    /* Referrers to ignore        27  */
                     "IgnoreAgent",       /* User Agents to ignore      28  */
                     "ReallyQuiet",       /* Dont display ANY messages  29  */
                     "GMTTime",           /* Local or UTC time?         30  */
                     "GroupURL",          /* Group URL's                31  */
                     "GroupSite",         /* Group Sites                32  */
                     "GroupReferrer",     /* Group Referrers            33  */
                     "GroupAgent",        /* Group Agents               34  */
                     "GroupShading",      /* Shade Grouped entries      35  */
                     "GroupHighlight",    /* BOLD Grouped entries       36  */
                     "Incremental",       /* Incremental runs           37  */
                     "IncrementalName",   /* Filename for state data    38  */
                     "HistoryName",       /* Filename for history data  39  */
                     "HTMLExtension",     /* HTML filename extension    40  */
                     "HTMLPre",           /* HTML code at beginning     41  */
                     "HTMLBody",          /* HTML body code             42  */
                     "HTMLEnd",           /* HTML code at end           43  */
                     "UseHTTPS",          /* Use https:// on URL's      44  */
                     "IncludeSite",       /* Sites to always include    45  */
                     "IncludeURL",        /* URL's to always include    46  */
                     "IncludeReferrer",   /* Referrers to include       47  */
                     "IncludeAgent",      /* User Agents to include     48  */
                     "PageType",          /* Page Type (pageview)       49  */
                     "VisitTimeout",      /* Visit timeout (HHMMSS)     50  */
                     "GraphLegend",       /* Graph Legends (yes/no)     51  */
                     "GraphLines",        /* Graph Lines (0=none)       52  */
                     "FoldSeqErr",        /* Fold sequence errors       53  */
                     "CountryGraph",      /* Display ctry graph (0=no)  54  */
                     "TopKSites",         /* Top sites (by KBytes)      55  */
                     "TopKURLs",          /* Top URL's (by KBytes)      56  */
                     "TopEntry",          /* Top Entry Pages            57  */
                     "TopExit",           /* Top Exit Pages             58  */
                     "TopSearch",         /* Top Search Strings         59  */
                     "LogType"            /* Log Type (clf or ftp)      60  */
                   };

   FILE *fp;

   char buffer[BUFSIZE];
   char keyword[32];
   char value[132];
   char *cp1, *cp2;
   int  i,key;
   int	num_kwords=sizeof(kwords)/sizeof(char *);

   if ( (fp=fopen(fname,"r")) == NULL)
   {
      if (verbose)
      fprintf(stderr,"%s %s\n",msg_bad_conf,fname);
      return;
   }

   while ( (fgets(buffer,BUFSIZE,fp)) != NULL)
   {
      /* skip comments and blank lines */
      if ( (buffer[0]=='#') || isspace(buffer[0]) ) continue;

      /* Get keyword */
      cp1=buffer;cp2=keyword;
      while ( isalnum((int)*cp1) ) *cp2++ = *cp1++;
      *cp2='\0';

      /* Get value */
      cp2=value;
      while ( (*cp1!='\n')&&(*cp1!='\0')&&(isspace((int)*cp1)) ) cp1++;
      while ( (*cp1!='\n')&&(*cp1!='\0') ) *cp2++ = *cp1++;
      *cp2--='\0';
      while ( (isspace((int)*cp2)) && (cp2 != value) ) *cp2--='\0';

      /* check if blank keyword/value */
      if ( (keyword[0]=='\0') || (value[0]=='\0') ) continue;

      key=0;
      for (i=0;i<num_kwords;i++)
         if (!strcmp(keyword,kwords[i])) { key=i; break; }

      if (key==0) { printf("%s '%s' (%s)\n",       /* Invalid keyword       */
                    msg_bad_key,keyword,fname);
                    continue;
                  }

      switch (key)
      {
        case 1:  out_dir=save_opt(value);          break; /* OutputDir      */
        case 2:  log_fname=save_opt(value);        break; /* LogFile        */
        case 3:  msg_title=save_opt(value);        break; /* ReportTitle    */
        case 4:  hname=save_opt(value);            break; /* HostName       */
        case 5:  ignore_hist=(value[0]=='n')?0:1;  break; /* IgnoreHist     */
        case 6:  verbose=(value[0]=='n')?2:1;      break; /* Quiet          */
        case 7:  time_me=(value[0]=='n')?0:1;      break; /* TimeMe         */
        case 8:  debug_mode=(value[0]=='n')?0:1;   break; /* Debug          */
        case 9:  hourly_graph=(value[0]=='n')?0:1; break; /* HourlyGraph    */
        case 10: hourly_stats=(value[0]=='n')?0:1; break; /* HourlyStats    */
        case 11: ntop_sites = atoi(value);         break; /* TopSites       */
        case 12: ntop_urls = atoi(value);          break; /* TopURLs        */
        case 13: ntop_refs = atoi(value);          break; /* TopRefs        */
        case 14: ntop_agents = atoi(value);        break; /* TopAgents      */
        case 15: ntop_ctrys = atoi(value);         break; /* TopCountries   */
        case 16: add_nlist(value,&hidden_sites);   break; /* HideSite       */
        case 17: add_nlist(value,&hidden_urls);    break; /* HideURL        */
        case 18: add_nlist(value,&hidden_refs);    break; /* HideReferrer   */
        case 19: add_nlist(value,&hidden_agents);  break; /* HideAgent      */
        case 20: add_nlist(value,&index_alias);    break; /* IndexAlias     */
        case 21: add_nlist(value,&html_head);      break; /* HTMLHead       */
        case 22: add_nlist(value,&html_post);      break; /* HTMLPost       */
        case 23: add_nlist(value,&html_tail);      break; /* HTMLTail       */
        case 24: mangle_agent=atoi(value);         break; /* MangleAgents   */
        case 25: add_nlist(value,&ignored_sites);  break; /* IgnoreSite     */
        case 26: add_nlist(value,&ignored_urls);   break; /* IgnoreURL      */
        case 27: add_nlist(value,&ignored_refs);   break; /* IgnoreReferrer */
        case 28: add_nlist(value,&ignored_agents); break; /* IgnoreAgent    */
        case 29: if (value[0]=='y') verbose=0;     break; /* ReallyQuiet    */
        case 30: local_time=(value[0]=='y')?0:1;   break; /* GMTTime        */
        case 31: add_glist(value,&group_urls);     break; /* GroupURL       */
        case 32: add_glist(value,&group_sites);    break; /* GroupSite      */
        case 33: add_glist(value,&group_refs);     break; /* GroupReferrer  */
        case 34: add_glist(value,&group_agents);   break; /* GroupAgent     */
        case 35: shade_groups=(value[0]=='y')?1:0; break; /* GroupShading   */
        case 36: hlite_groups=(value[0]=='y')?1:0; break; /* GroupHighlight */
        case 37: incremental=(value[0]=='y')?1:0;  break; /* Incremental    */
        case 38: state_fname=save_opt(value);      break; /* State FName    */
        case 39: hist_fname=save_opt(value);       break; /* History FName  */
        case 40: html_ext=save_opt(value);         break; /* HTML extension */
        case 41: add_nlist(value,&html_pre);       break; /* HTML Pre code  */
        case 42: add_nlist(value,&html_body);      break; /* HTML Body code */
        case 43: add_nlist(value,&html_end);       break; /* HTML End code  */
        case 44: use_https=(value[0]=='y')?1:0;    break; /* Use https://   */
        case 45: add_nlist(value,&include_sites);  break; /* IncludeSite    */
        case 46: add_nlist(value,&include_urls);   break; /* IncludeURL     */
        case 47: add_nlist(value,&include_refs);   break; /* IncludeReferrer*/
        case 48: add_nlist(value,&include_agents); break; /* IncludeAgent   */
        case 49: add_nlist(value,&page_type);      break; /* PageType       */
        case 50: visit_timeout=atoi(value);        break; /* VisitTimeout   */
        case 51: graph_legend=(value[0]=='y')?1:0; break; /* GraphLegend    */
        case 52: graph_lines = atoi(value);        break; /* GraphLines     */
        case 53: fold_seq_err=(value[0]=='y')?1:0; break; /* FoldSeqErr     */
        case 54: ctry_graph=(value[0]=='y')?1:0;   break; /* CountryGraph   */
        case 55: ntop_sitesK = atoi(value);        break; /* TopKSites (KB) */
        case 56: ntop_urlsK  = atoi(value);        break; /* TopKUrls (KB)  */
        case 57: ntop_entry  = atoi(value);        break; /* Top Entry pgs  */
        case 58: ntop_exit   = atoi(value);        break; /* Top Exit pages */
        case 59: ntop_search = atoi(value);        break; /* Top Search pgs */
        case 60: log_type=(value[0]=='f')?1:0;     break; /* LogType (1=ftp)*/
      }
   }
   fclose(fp);
}

/*********************************************/
/* SAVE_OPT - save option from config file   */
/*********************************************/

char *save_opt(char *str)
{
   int  i;
   char *cp1;

   i=strlen(str);

   if ( (cp1=malloc(i+1))==NULL) return NULL;

   strcpy(cp1,str);
   return cp1;
}

/*********************************************/
/* GET_HISTORY - load in history file        */
/*********************************************/

void get_history()
{
   int i;
   FILE *hist_fp;

   /* first initalize internal array */
   for (i=0;i<12;i++)
   {
      hist_month[i]=hist_year[i]=hist_fday[i]=hist_lday[i]=0;
      hist_hit[i]=hist_files[i]=hist_site[i]=hist_page[i]=hist_visit[i]=0;
      hist_xfer[i]=0.0;
   }

   hist_fp=fopen(hist_fname,"r");

   if (hist_fp)
   {
      if (verbose>1) printf("%s %s\n",msg_get_hist,hist_fname);
      while ((fgets(buffer,BUFSIZE,hist_fp)) != NULL)
      {
         i = atoi(buffer) -1;
         if (i>11)
         {
            if (verbose)
               fprintf(stderr,"%s (mth=%d)\n",msg_bad_hist,i+1);
            continue;
         }

         /* month# year# requests files sites xfer firstday lastday */
         sscanf(buffer,"%d %d %lu %lu %lu %lf %d %d %lu %lu",
                       &hist_month[i],
                       &hist_year[i],
                       &hist_hit[i],
                       &hist_files[i],
                       &hist_site[i],
                       &hist_xfer[i],
                       &hist_fday[i],
                       &hist_lday[i],
                       &hist_page[i],
                       &hist_visit[i]);
      }
      fclose(hist_fp);
   }
   else if (verbose>1) printf("%s\n",msg_no_hist);
}

/*********************************************/
/* PUT_HISTORY - write out history file      */
/*********************************************/

void put_history()
{
   int i;
   FILE *hist_fp;

   hist_fp = fopen(hist_fname,"w");

   if (hist_fp)
   {
      if (verbose>1) printf("%s\n",msg_put_hist);
      for (i=0;i<12;i++)
      {
         if ((hist_month[i] != 0) && (hist_hit[i] != 0))
         {
            fprintf(hist_fp,"%d %d %lu %lu %lu %.0f %d %d %lu %lu\n",
                            hist_month[i],
                            hist_year[i],
                            hist_hit[i],
                            hist_files[i],
                            hist_site[i],
                            hist_xfer[i],
                            hist_fday[i],
                            hist_lday[i],
                            hist_page[i],
                            hist_visit[i]);
         }
      }
      fclose(hist_fp);
   }
   else
      if (verbose)
      fprintf(stderr,"%s %s\n",msg_hist_err,hist_fname);
}

/*********************************************/
/* INIT_COUNTERS - prep counters for use     */
/*********************************************/

void init_counters()
{
   int i;
   for (i=0;i<TOTAL_RC;i++) response[i].count = 0;
   for (i=0;i<31;i++)  /* monthly totals      */
   {
    tm_xfer[i]=0.0;
    tm_hit[i]=tm_file[i]=tm_site[i]=tm_page[i]=tm_visit[i]=0;
   }
   for (i=0;i<24;i++)  /* hourly totals       */
   {
      th_hit[i]=th_file[i]=th_page[i]=0;
      th_xfer[i]=0.0;
   }
   for (i=0;i<MAX_CTRY;i++) /* country totals */
   {
      ctry[i].count=0;
      ctry[i].files=0;
      ctry[i].xfer=0;
   }
   t_hit=t_file=t_site=t_url=t_ref=t_agent=t_page=t_visit=0;
   t_xfer=0.0;
   mh_hit = dt_site = 0;
   f_day=l_day=1;
}

/*********************************************/
/* TOT_VISIT - calculate total visits        */
/*********************************************/

u_long tot_visit(HNODEPTR *list)
{
   HNODEPTR   hptr;
   u_long     tot=0;
   int        i;

   for (i=0;i<MAXHASH;i++)
   {
      hptr=list[i];
      while (hptr!=NULL)
      {
         if (hptr->flag!=OBJ_GRP) tot+=hptr->visit;
         hptr=hptr->next;
      }
   }
   return tot;
}

/*********************************************/
/* PRINT_OPTS - print command line options   */
/*********************************************/

void print_opts(char *pname)
{
   int i;

   printf("%s: %s %s\n",h_usage1,pname,h_usage2);
   for (i=0;i<LAST_HLP_MSG;i++) printf("%s\n",h_msg[i]);
   exit(1);
}

/*********************************************/
/* PRINT_VERSION                             */
/*********************************************/

void print_version()
{
 uname(&system_info);
 printf("Webalizer V%s-%s (%s %s) %s\n%s\n\n",
    version,editlvl,
    system_info.sysname,system_info.release,
    language,copyright);
 exit(1);
}

/*********************************************/
/* CUR_TIME - return date/time as a string   */
/*********************************************/

char *cur_time()
{
   /* get system time */
   now = time(NULL);
   /* convert to timestamp string */
   if (local_time)
      strftime(timestamp,sizeof(timestamp),"%d-%b-%Y %H:%M %Z",
            localtime(&now));
   else
      strftime(timestamp,sizeof(timestamp),"%d-%b-%Y %H:%M GMT",
            gmtime(&now));

   return timestamp;
}

/*********************************************/
/* WRITE_HTML_HEAD - output top of HTML page */
/*********************************************/

void write_html_head(char *period)
{
   NLISTPTR lptr;                          /* used for HTMLhead processing */

   /* HTMLPre code goes before all else    */
   lptr = html_pre;
   if (lptr==NULL)
   {
      /* Default 'DOCTYPE' header record if none specified */
      fprintf(out_fp,"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n\n");
   }
   else
   {
      while (lptr!=NULL)
      {
         fprintf(out_fp,"%s\n",lptr->string);
         lptr=lptr->next;
      }
   }
   /* Standard header comments */
   fprintf(out_fp,"<!-- Generated by The Webalizer  Ver. %s-%s -->\n",
      version,editlvl);
   fprintf(out_fp,"<!--                                          -->\n");
   fprintf(out_fp,"<!-- Copyright 1997-1999 Bradford L. Barrett  -->\n");
   fprintf(out_fp,"<!-- (brad@mrunix.net  http://www.mrunix.net) -->\n");
   fprintf(out_fp,"<!--                                          -->\n");
   fprintf(out_fp,"<!-- Distributed under the GNU GPL  Version 2 -->\n");
   fprintf(out_fp,"<!--        Full text may be found at:        -->\n");
   fprintf(out_fp,"<!--     http://www.mrunix.net/webalizer/     -->\n");
   fprintf(out_fp,"<!--                                          -->\n");
   fprintf(out_fp,"<!--  Give the power back to the programmers  -->\n");
   fprintf(out_fp,"<!--    Fight Micro$oft,  support the FSF!    -->\n");
   fprintf(out_fp,"<!--           (http://www.fsf.org)           -->\n");
   fprintf(out_fp,"<!--                                          -->\n");
   fprintf(out_fp,"<!-- *** Generated: %s *** -->\n\n",cur_time());

   fprintf(out_fp,"<HTML>\n<HEAD>\n");
   fprintf(out_fp," <TITLE>%s %s - %s</TITLE>\n",
                  msg_title, hname, period);
   lptr=html_head;
   while (lptr!=NULL)
   {
      fprintf(out_fp,"%s\n",lptr->string);
      lptr=lptr->next;
   }
   fprintf(out_fp,"</HEAD>\n\n");

   lptr = html_body;
   if (lptr==NULL)
      fprintf(out_fp,"<BODY BGCOLOR=\"%s\" TEXT=\"%s\" "   \
              "LINK=\"%s\" VLINK=\"%s\">\n",
              LTGREY, BLACK, BLUE, RED);
   else
   {
      while (lptr!=NULL)
      {
         fprintf(out_fp,"%s\n",lptr->string);
         lptr=lptr->next;
      }
   }
   fprintf(out_fp,"<H2>%s %s</H2>\n",msg_title, hname);
   fprintf(out_fp,"<SMALL><STRONG>\n%s: %s<BR>\n",msg_hhdr_sp,period);
   fprintf(out_fp,"%s %s<BR>\n</STRONG></SMALL>\n",msg_hhdr_gt,cur_time());
   lptr=html_post;
   while (lptr!=NULL)
   {
      fprintf(out_fp,"%s\n",lptr->string);
      lptr=lptr->next;
   }
   fprintf(out_fp,"<CENTER>\n<HR>\n<P>\n");
}

/*********************************************/
/* WRITE_HTML_TAIL - output HTML page tail   */
/*********************************************/

void write_html_tail()
{
   NLISTPTR lptr;

   fprintf(out_fp,"</CENTER>\n");
   fprintf(out_fp,"<P>\n<HR>\n");
   fprintf(out_fp,"<TABLE WIDTH=\"100%%\" CELLPADDING=0 " \
                  "CELLSPACING=0 BORDER=0>\n");
   fprintf(out_fp,"<TR>\n");
   fprintf(out_fp,"<TD ALIGN=left VALIGN=top>\n");
   fprintf(out_fp,"<SMALL>Generated by\n");
   fprintf(out_fp,"<A HREF=\"http://www.mrunix.net/webalizer/\">");
   fprintf(out_fp,"<STRONG>Webalizer Version %s</STRONG></A>\n",version);
   fprintf(out_fp,"</SMALL>\n</TD>\n");
   lptr=html_tail;
   if (lptr)
   {
      fprintf(out_fp,"<TD ALIGN=\"right\" VALIGN=\"top\">\n");
      while (lptr!=NULL)
      {
         fprintf(out_fp,"%s\n",lptr->string);
         lptr=lptr->next;
      }
      fprintf(out_fp,"</TD>\n");
   }
   fprintf(out_fp,"</TR>\n</TABLE>\n");

   /* wind up, this is the end of the file */
   fprintf(out_fp,"\n<!-- Webalizer Version %s-%s (Mod: %s) -->\n",
           version,editlvl,moddate);
   lptr = html_end;
   if (lptr)
   {
      while (lptr!=NULL)
      {
         fprintf(out_fp,"%s\n",lptr->string);
         lptr=lptr->next;
      }
   }
   else fprintf(out_fp,"\n</BODY>\n</HTML>\n");
}

/*********************************************/
/* WRITE_MONTH_HTML - does what it says...   */
/*********************************************/

int write_month_html()
{
   int i;
   char html_fname[256];           /* filename storage areas...       */
   char png1_fname[32];
   char png2_fname[32];

   char buffer[BUFSIZE];           /* scratch buffer                  */
   char dtitle[256];
   char htitle[256];

   if (verbose>1)
      printf("%s %s %d\n",msg_gen_rpt, l_month[cur_month-1], cur_year);

   /* update history */
   i=cur_month-1;
   hist_month[i] =  cur_month;
   hist_year[i]  =  cur_year;
   hist_hit[i]   =  t_hit;
   hist_files[i] =  t_file;
   hist_page[i]  =  t_page;
   hist_visit[i] =  t_visit;
   hist_site[i]  =  t_site;
   hist_xfer[i]  =  t_xfer/1024;
   hist_fday[i]  =  f_day;
   hist_lday[i]  =  l_day;

   /* fill in filenames */
   sprintf(html_fname,"usage_%04d%02d.%s",cur_year,cur_month,html_ext);
   sprintf(png1_fname,"daily_usage_%04d%02d.png",cur_year,cur_month);
   sprintf(png2_fname,"hourly_usage_%04d%02d.png",cur_year,cur_month);

   /* create PNG images for web page */
   sprintf(dtitle,"%s %s %d",msg_hmth_du,l_month[cur_month-1],cur_year);

   month_graph6 (  png1_fname,          /* filename          */
                   dtitle,              /* graph title       */
                   cur_month,           /* graph month       */
                   cur_year,            /* graph year        */
                   tm_hit,              /* data 1 (hits)     */
                   tm_file,             /* data 2 (files)    */
                   tm_site,             /* data 3 (sites)    */
                   tm_xfer,             /* data 4 (kbytes)   */
                   tm_page,             /* data 5 (pages)    */
                   tm_visit);           /* data 6 (visits)   */

   if (hourly_graph)
   {
      sprintf(htitle,"%s %s %d",msg_hmth_hu,l_month[cur_month-1],cur_year);
      day_graph3(    png2_fname,
                     htitle,
                     th_hit,
                     th_file,
                     th_page );
   }

   /* now do html stuff... */
   if ( (out_fp = fopen(html_fname,"w")) == NULL)
   {
      if (verbose)
      fprintf(stderr,"%s %s\n",msg_no_open,html_fname);
      return 1;
   }
   sprintf(buffer,"%s %d",l_month[cur_month-1],cur_year);
   write_html_head(buffer);
   month_links();
   month_total_table();
   fprintf(out_fp,"<A NAME=\"DAYSTATS\"></A>\n");
   fprintf(out_fp,"<IMG SRC=\"%s\" ALT=\"%s\" " \
                  "HEIGHT=400 WIDTH=512><P>\n",png1_fname,dtitle);
   daily_total_table();

   if (hourly_graph)                      /* hourly graph                   */
   {
      fprintf(out_fp,"<A NAME=\"HOURSTATS\"></A>\n");
      fprintf(out_fp,"<IMG SRC=\"%s\" ALT=\"%s\" "  \
                     "HEIGHT=256 WIDTH=512><P>\n",png2_fname,htitle);
   }

   if (hourly_stats) hourly_total_table();  /* hourly usage table           */

   if (ntop_urls  ) top_urls_table(0);    /* top URL's table                */
   if (ntop_urlsK ) top_urls_table(1);    /* top URL's (by kbytes)          */
   if (ntop_entry ) top_entry_table(0);   /* Top Entry Pages                */
   if (ntop_exit  ) top_entry_table(1);   /* Top Exit Pages                 */
   if (ntop_sites ) top_sites_table(0);   /* top sites table                */
   if (ntop_sitesK) top_sites_table(1);   /* top sites (by kbytes)          */
   if (ntop_refs  ) top_refs_table();     /* top referrers table            */
   if (ntop_search) top_search_table();   /* top search string table        */
   if (ntop_agents) top_agents_table();   /* top user agents table          */
   if (ntop_ctrys ) top_ctry_table();     /* top countries table            */

   write_html_tail();                     /* finish up the HTML document    */
   fclose(out_fp);                        /* close the file                 */
   return (0);                            /* done...                        */
}

/*********************************************/
/* MONTH_LINKS - links to other page parts   */
/*********************************************/

void month_links()
{
   fprintf(out_fp,"<SMALL>\n");
   fprintf(out_fp,"<A HREF=\"#DAYSTATS\">[%s]</A>\n",msg_hlnk_ds);
   if (hourly_stats || hourly_graph)
      fprintf(out_fp,"<A HREF=\"#HOURSTATS\">[%s]</A>\n",msg_hlnk_hs);
   if (ntop_urls || ntop_urlsK)
      fprintf(out_fp,"<A HREF=\"#TOPURLS\">[%s]</A>\n",msg_hlnk_u);
   if (ntop_entry)
      fprintf(out_fp,"<A HREF=\"#TOPENTRY\">[%s]</A>\n",msg_hlnk_en);
   if (ntop_entry)
      fprintf(out_fp,"<A HREF=\"#TOPEXIT\">[%s]</A>\n",msg_hlnk_ex);
   if (ntop_sites || ntop_sitesK)
      fprintf(out_fp,"<A HREF=\"#TOPSITES\">[%s]</A>\n",msg_hlnk_s);
   if (ntop_refs && t_ref)
      fprintf(out_fp,"<A HREF=\"#TOPREFS\">[%s]</A>\n",msg_hlnk_r);
   if (ntop_search && t_ref)
      fprintf(out_fp,"<A HREF=\"#TOPSEARCH\">[%s]</A>\n",msg_hlnk_sr);
   if (ntop_agents && t_agent)
      fprintf(out_fp,"<A HREF=\"#TOPAGENTS\">[%s]</A>\n",msg_hlnk_a);
   if (ntop_ctrys)
      fprintf(out_fp,"<A HREF=\"#TOPCTRYS\">[%s]</A>\n",msg_hlnk_c);
   fprintf(out_fp,"</SMALL>\n<P>\n");
}
/*********************************************/
/* MONTH_TOTAL_TABLE - monthly totals table  */
/*********************************************/

void month_total_table()
{
   int i,days_in_month;
   u_long max_files=0,max_hits=0,max_visits=0,max_pages=0;
   double max_xfer=0.0;

   days_in_month=(l_day-f_day)+1;
   for (i=0;i<31;i++)
   {  /* Get max/day values */
      if (tm_hit[i]>max_hits)     max_hits  = tm_hit[i];
      if (tm_file[i]>max_files)   max_files = tm_file[i];
      if (tm_page[i]>max_pages)   max_pages = tm_page[i];
      if (tm_visit[i]>max_visits) max_visits= tm_visit[i];
      if (tm_xfer[i]>max_xfer)    max_xfer  = tm_xfer[i];
   }

   fprintf(out_fp,"<TABLE WIDTH=510 BORDER=2 CELLSPACING=1 CELLPADDING=1>\n");
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH COLSPAN=3 ALIGN=center BGCOLOR=\"%s\">"           \
           "%s %s %d</TH></TR>\n",
           GREY,msg_mtot_ms,l_month[cur_month-1],cur_year);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   /* Total Hits */
   fprintf(out_fp,"<TR><TD WIDTH=380><FONT SIZE=\"-1\">%s</FONT></TD>\n"     \
           "<TD ALIGN=right COLSPAN=2><FONT SIZE=\"-1\"><B>%lu</B>"          \
           "</FONT></TD></TR>\n",
           msg_mtot_th,t_hit);
   /* Total Files */
   fprintf(out_fp,"<TR><TD WIDTH=380><FONT SIZE=\"-1\">%s</FONT></TD>\n"     \
           "<TD ALIGN=right COLSPAN=2><FONT SIZE=\"-1\"><B>%lu</B>"          \
           "</FONT></TD></TR>\n",
           msg_mtot_tf,t_file);
   /* Total Pages */
   fprintf(out_fp,"<TR><TD WIDTH=380><FONT SIZE=\"-1\">%s %s</FONT></TD>\n"  \
           "<TD ALIGN=right COLSPAN=2><FONT SIZE=\"-1\"><B>%lu</B>"          \
           "</FONT></TD></TR>\n",
           msg_h_total, msg_h_pages, t_page);
   /* Total Visits */
   fprintf(out_fp,"<TR><TD WIDTH=380><FONT SIZE=\"-1\">%s %s</FONT></TD>\n"  \
           "<TD ALIGN=right COLSPAN=2><FONT SIZE=\"-1\"><B>%lu</B>"          \
           "</FONT></TD></TR>\n",
           msg_h_total, msg_h_visits, t_visit);
   /* Total XFer */
   fprintf(out_fp,"<TR><TD WIDTH=380><FONT SIZE=\"-1\">%s</FONT></TD>\n"     \
           "<TD ALIGN=right COLSPAN=2><FONT SIZE=\"-1\"><B>%.0f</B>"         \
           "</FONT></TD></TR>\n",
           msg_mtot_tx,t_xfer/1024);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   /**********************************************/
   /* Unique Sites */
   fprintf(out_fp,"<TR>"                                                     \
           "<TD WIDTH=380><FONT SIZE=\"-1\">%s</FONT></TD>\n"                \
           "<TD ALIGN=right COLSPAN=2><FONT SIZE=\"-1\"><B>%lu</B>"          \
           "</FONT></TD></TR>\n",
           msg_mtot_us,t_site);
   /* Unique URL's */
   fprintf(out_fp,"<TR>"                                                     \
           "<TD WIDTH=380><FONT SIZE=\"-1\">%s</FONT></TD>\n"                \
           "<TD ALIGN=right COLSPAN=2><FONT SIZE=\"-1\"><B>%lu</B>"          \
           "</FONT></TD></TR>\n",
           msg_mtot_uu,t_url);
   /* Unique Referrers */
   if (t_ref != 0)
   fprintf(out_fp,"<TR>"                                                     \
           "<TD WIDTH=380><FONT SIZE=\"-1\">%s</FONT></TD>\n"                \
           "<TD ALIGN=right COLSPAN=2><FONT SIZE=\"-1\"><B>%lu</B>"          \
           "</FONT></TD></TR>\n",
           msg_mtot_ur,t_ref);
   /* Unique Agents */
   if (t_agent != 0)
   fprintf(out_fp,"<TR>"                                                     \
           "<TD WIDTH=380><FONT SIZE=\"-1\">%s</FONT></TD>\n"                \
           "<TD ALIGN=right COLSPAN=2><FONT SIZE=\"-1\"><B>%lu</B>"          \
           "</FONT></TD></TR>\n",
           msg_mtot_ua,t_agent);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   /**********************************************/
   /* Hourly/Daily avg/max totals */
   fprintf(out_fp,"<TR>"                                                     \
           "<TH WIDTH=380 BGCOLOR=\"%s\"><FONT SIZE=-1 COLOR=\"%s\">.</FONT></TH>\n" \
           "<TH WIDTH=65 BGCOLOR=\"%s\" ALIGN=right>"                        \
           "<FONT SIZE=-1>%s </FONT></TH>\n"                                 \
           "<TH WIDTH=65 BGCOLOR=\"%s\" ALIGN=right>"                        \
           "<FONT SIZE=-1>%s </FONT></TH></TR>\n",
           GREY,GREY,GREY,msg_h_avg,GREY,msg_h_max);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   /* Max/Avg Hits per Hour */
   fprintf(out_fp,"<TR>"                                                     \
           "<TD><FONT SIZE=\"-1\">%s</FONT></TD>\n"                          \
           "<TD ALIGN=right WIDTH=65><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
           "<TD WIDTH=65 ALIGN=right><FONT SIZE=-1><B>%lu</B>"               \
           "</FONT></TD></TR>\n",
           msg_mtot_mhh, t_hit/(24*days_in_month),mh_hit);
   /* Max/Avg Hits per Day */
   fprintf(out_fp,"<TR>"                                                     \
           "<TD><FONT SIZE=\"-1\">%s</FONT></TD>\n"                          \
           "<TD ALIGN=right WIDTH=65><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
           "<TD WIDTH=65 ALIGN=right><FONT SIZE=-1><B>%lu</B>"               \
           "</FONT></TD></TR>\n",
           msg_mtot_mhd, t_hit/days_in_month, max_hits);
   /* Max/Avg Files per Day */
   fprintf(out_fp,"<TR>"                                                     \
           "<TD><FONT SIZE=\"-1\">%s</FONT></TD>\n"                          \
           "<TD ALIGN=right WIDTH=65><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
           "<TD WIDTH=65 ALIGN=right><FONT SIZE=-1><B>%lu</B>"               \
           "</FONT></TD></TR>\n",
           msg_mtot_mfd, t_file/days_in_month,max_files);
   /* Max/Avg Pages per Day */
   fprintf(out_fp,"<TR>"                                                     \
           "<TD><FONT SIZE=\"-1\">%s</FONT></TD>\n"                          \
           "<TD ALIGN=right WIDTH=65><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
           "<TD WIDTH=65 ALIGN=right><FONT SIZE=-1><B>%lu</B>"               \
           "</FONT></TD></TR>\n",
           msg_mtot_mpd, t_page/days_in_month,max_pages);
   /* Max/Avg Visits per Day */
   fprintf(out_fp,"<TR>"                                                     \
           "<TD><FONT SIZE=\"-1\">%s</FONT></TD>\n"                          \
           "<TD ALIGN=right WIDTH=65><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
           "<TD WIDTH=65 ALIGN=right><FONT SIZE=-1><B>%lu</B>"               \
           "</FONT></TD></TR>\n",
           msg_mtot_mvd, t_visit/days_in_month,max_visits);
   /* Max/Avg KBytes per Day */
   fprintf(out_fp,"<TR>"                                                     \
           "<TD><FONT SIZE=\"-1\">%s</FONT></TD>\n"                          \
           "<TD ALIGN=right WIDTH=65><FONT SIZE=\"-1\"><B>%.0f</B></FONT></TD>\n"\
           "<TD WIDTH=65 ALIGN=right><FONT SIZE=-1><B>%.0f</B>"\
           "</FONT></TD></TR>\n",
           msg_mtot_mkd, (t_xfer/1024)/days_in_month,max_xfer/1024);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   /**********************************************/
   /* response code totals */
   fprintf(out_fp,"<TR><TH COLSPAN=3 ALIGN=center BGCOLOR=\"%s\">\n"         \
           "<FONT SIZE=\"-1\">%s</FONT></TH></TR>\n",GREY,msg_mtot_rc);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   for (i=0;i<TOTAL_RC;i++)
   {
      if (response[i].count != 0)
         fprintf(out_fp,"<TR><TD><FONT SIZE=\"-1\">%s</FONT></TD>\n"         \
            "<TD ALIGN=right COLSPAN=2><FONT SIZE=\"-1\"><B>%lu</B>"         \
            "</FONT></TD></TR>\n",
            response[i].desc, response[i].count);
   }
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   /**********************************************/

   fprintf(out_fp,"</TABLE>\n");
   fprintf(out_fp,"<P>\n");
}

/*********************************************/
/* DAILY_TOTAL_TABLE - daily totals          */
/*********************************************/

void daily_total_table()
{
   int i;

   /* Daily stats */
   fprintf(out_fp,"<TABLE WIDTH=510 BORDER=2 CELLSPACING=1 CELLPADDING=1>\n");
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   /* Daily statistics for ... */
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" COLSPAN=13 ALIGN=center>"          \
           "%s %s %d</TH></TR>\n",
           GREY,msg_dtot_ds,l_month[cur_month-1], cur_year);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH ALIGN=center BGCOLOR=\"%s\">"                     \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n"                       \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=2>"               \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n"                       \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=2>"               \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n"                       \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=2>"               \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n"                       \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=2>"               \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n"                       \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=2>"               \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n"                       \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=2>"               \
                  "<FONT SIZE=\"-1\">%s</FONT></TH></TR>\n",
                  GREY,    msg_h_day,
                  DKGREEN, msg_h_hits,
                  LTBLUE,  msg_h_files,
                  CYAN,    msg_h_pages,
                  YELLOW,  msg_h_visits,
                  ORANGE,  msg_h_sites,
                  RED,     msg_h_xfer);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");

   /* skip beginning blank days in a month */
   for (i=0;i<hist_lday[cur_month-1];i++) if (tm_hit[i]!=0) break;
   if (i==hist_lday[cur_month-1]) i=0;

   for (;i<hist_lday[cur_month-1];i++)
   {
      fprintf(out_fp,"<TR><TD ALIGN=center>"                                 \
              "<FONT SIZE=\"-1\"><B>%d</B></FONT></TD>\n", i+1);
      fprintf(out_fp,"<TD ALIGN=right>"                                      \
              "<FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n"                   \
              "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n",
              tm_hit[i],PCENT(tm_hit[i],t_hit));
      fprintf(out_fp,"<TD ALIGN=right>"                                      \
              "<FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n"                   \
              "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n",
              tm_file[i],PCENT(tm_file[i],t_file));
      fprintf(out_fp,"<TD ALIGN=right>"                                      \
              "<FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n"                   \
              "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n",
              tm_page[i],PCENT(tm_page[i],t_page));
      fprintf(out_fp,"<TD ALIGN=right>"                                      \
              "<FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n"                   \
              "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n",
              tm_visit[i],PCENT(tm_visit[i],t_visit));
      fprintf(out_fp,"<TD ALIGN=right>"                                      \
              "<FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n"                   \
              "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n",
              tm_site[i],PCENT(tm_site[i],t_site));
      fprintf(out_fp,"<TD ALIGN=right>"                                      \
              "<FONT SIZE=\"-1\"><B>%.0f</B></FONT></TD>\n"                  \
              "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD></TR>\n",
              tm_xfer[i]/1024,PCENT(tm_xfer[i],t_xfer));
   }
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"</TABLE>\n");
   fprintf(out_fp,"<P>\n");
}

/*********************************************/
/* HOURLY_TOTAL_TABLE - hourly table         */
/*********************************************/

void hourly_total_table()
{
   int i,days_in_month;
   u_long avg_file=0;
   double avg_xfer=0.0;

   days_in_month=(l_day-f_day)+1;

   /* Hourly stats */
   if (!hourly_graph)
      fprintf(out_fp,"<A NAME=\"HOURSTATS\"></A>\n");
   fprintf(out_fp,"<TABLE WIDTH=510 BORDER=2 CELLSPACING=1 CELLPADDING=1>\n");
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" COLSPAN=13 ALIGN=center>"\
           "%s %s %d</TH></TR>\n",
           GREY,msg_htot_hs,l_month[cur_month-1], cur_year);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH ALIGN=center ROWSPAN=2 BGCOLOR=\"%s\">" \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n"             \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=3>"     \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n"             \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=3>"     \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n"             \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=3>"     \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n"             \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=3>"     \
                  "<FONT SIZE=\"-1\">%s</FONT></TH></TR>\n",
                  GREY,    msg_h_hour,
                  DKGREEN, msg_h_hits,
                  LTBLUE,  msg_h_files,
                  CYAN,    msg_h_pages,
                  RED,     msg_h_xfer);
   fprintf(out_fp,"<TR><TH ALIGN=center BGCOLOR=\"%s\">"           \
                  "<FONT SIZE=\"-2\">%s</FONT></TH>\n"             \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=2>"     \
                  "<FONT SIZE=\"-2\">%s</FONT></TH>\n",
                  DKGREEN, msg_h_avg, DKGREEN, msg_h_total);
   fprintf(out_fp,"<TH ALIGN=center BGCOLOR=\"%s\">"               \
                  "<FONT SIZE=\"-2\">%s</FONT></TH>\n"             \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=2>"     \
                  "<FONT SIZE=\"-2\">%s</FONT></TH>\n",
                  LTBLUE, msg_h_avg, LTBLUE, msg_h_total);
   fprintf(out_fp,"<TH ALIGN=center BGCOLOR=\"%s\">"               \
                  "<FONT SIZE=\"-2\">%s</FONT></TH>\n"             \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=2>"     \
                  "<FONT SIZE=\"-2\">%s</FONT></TH>\n",
                  CYAN, msg_h_avg, CYAN, msg_h_total);
   fprintf(out_fp,"<TH ALIGN=center BGCOLOR=\"%s\">"               \
                  "<FONT SIZE=\"-2\">%s</FONT></TH>\n"             \
                  "<TH ALIGN=center BGCOLOR=\"%s\" COLSPAN=2>"     \
                  "<FONT SIZE=\"-2\">%s</FONT></TH></TR>\n",
                  RED, msg_h_avg, RED, msg_h_total);

   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   for (i=0;i<24;i++)
   {
      fprintf(out_fp,"<TR><TD ALIGN=center>"                          \
         "<FONT SIZE=\"-1\"><B>%d</B></FONT></TD>\n",i);
      fprintf(out_fp,
         "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
         "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
         "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n",
         th_hit[i]/days_in_month,th_hit[i],
         PCENT(th_hit[i],t_hit));
      fprintf(out_fp,
         "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
         "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
         "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n",
         th_file[i]/days_in_month,th_file[i],
         PCENT(th_file[i],t_file));
      fprintf(out_fp,
         "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
         "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
         "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n",
         th_page[i]/days_in_month,th_page[i],
         PCENT(th_page[i],t_page));
      fprintf(out_fp,
         "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%.0f</B></FONT></TD>\n" \
         "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%.0f</B></FONT></TD>\n" \
         "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD></TR>\n",
         (th_xfer[i]/days_in_month)/1024,th_xfer[i]/1024,
         PCENT(th_xfer[i],t_xfer));
      avg_file += th_file[i]/days_in_month;
      avg_xfer+= (th_xfer[i]/days_in_month)/1024;
   }
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"</TABLE>\n<P>\n");
}

/*********************************************/
/* TOP_SITES_TABLE - generate top n table    */
/*********************************************/

void top_sites_table(int flag)
{
   int i,j,x,cnt,tot_num=0;
   HNODEPTR hptr;

   cnt = (flag)?ntop_sitesK:ntop_sites;
   for (i=0;i<cnt;i++) top_sites[i]=NULL;
   for (i=0;i<MAXHASH;i++)
   {
      hptr=sm_htab[i];
      while (hptr!=NULL)
      {
         if (hptr->flag != OBJ_HIDE)
         {
            for (j=0;j<cnt;j++)
            {
               if (top_sites[j]==NULL) { top_sites[j]=hptr; break; }
               else
               {
                  if (flag)
                  {
                     if (hptr->xfer >= top_sites[j]->xfer)
                     {
                        for (x=cnt-1;x>j;x--)
                           top_sites[x] = top_sites[x-1];
                        top_sites[j]=hptr; break;
                     }
                  }
                  else
                  {
                     if (hptr->count >= top_sites[j]->count)
                     {
                        for (x=cnt-1;x>j;x--)
                           top_sites[x] = top_sites[x-1];
                        top_sites[j]=hptr; break;
                     }
                  }
               }
            }
         }
         hptr=hptr->next;
      }
   }

   for (i=0;i<cnt;i++) if (top_sites[i] != NULL) tot_num++;
   if (tot_num==0) return;
   if (tot_num > t_site) tot_num = t_site;
   if ((!flag) || (flag&&!ntop_sites))                     /* do anchor tag */
      fprintf(out_fp,"<A NAME=\"TOPSITES\"></A>\n");

   fprintf(out_fp,"<TABLE WIDTH=510 BORDER=2 CELLSPACING=1 CELLPADDING=1>\n");
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   if (flag) fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=CENTER COLSPAN=10>" \
           "%s %d %s %lu %s %s %s</TH></TR>\n",
           GREY, msg_top_top,tot_num,msg_top_of,
           t_site,msg_top_s,msg_h_by,msg_h_xfer);
   else      fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=CENTER COLSPAN=10>" \
           "%s %d %s %lu %s</TH></TR>\n",
           GREY,msg_top_top, tot_num, msg_top_of, t_site, msg_top_s);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=center>"                   \
          "<FONT SIZE=\"-1\">#</FONT></TH>\n",GREY);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"             \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",DKGREEN,msg_h_hits);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"             \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",LTBLUE,msg_h_files);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"             \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",RED,msg_h_xfer);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"             \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",YELLOW,msg_h_visits);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center>"                       \
          "<FONT SIZE=\"-1\">%s</FONT></TH></TR>\n",CYAN,msg_h_hname);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   for (i=0;i<cnt;i++)
   {
      if (top_sites[i] != NULL)
      {
         /* shade grouping? */
         if (shade_groups && (top_sites[i]->flag==OBJ_GRP))
            fprintf(out_fp,"<TR BGCOLOR=\"%s\">\n", GRPCOLOR);
         else fprintf(out_fp,"<TR>\n");

         fprintf(out_fp,
              "<TD ALIGN=center><FONT SIZE=\"-1\"><B>%d</B></FONT></TD>\n"  \
              "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n"  \
              "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"    \
              "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n"  \
              "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"    \
              "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%.0f</B></FONT></TD>\n" \
              "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"    \
              "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n"  \
              "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"    \
              "<TD ALIGN=left NOWRAP><FONT SIZE=\"-1\">",
              i+1,top_sites[i]->count,
              (t_hit==0)?0:((float)top_sites[i]->count/t_hit)*100.0,
              top_sites[i]->files,
              (t_file==0)?0:((float)top_sites[i]->files/t_file)*100.0,
              top_sites[i]->xfer/1024,
              (t_xfer==0)?0:((float)top_sites[i]->xfer/t_xfer)*100.0,
              top_sites[i]->visit,
              (t_visit==0)?0:((float)top_sites[i]->visit/t_visit)*100.0);

         if ((top_sites[i]->flag==OBJ_GRP)&&hlite_groups)
             fprintf(out_fp,"<STRONG>%s</STRONG></FONT></TD></TR>\n",
               top_sites[i]->string);
         else fprintf(out_fp,"%s</FONT></TD></TR>\n",
               top_sites[i]->string);
      }
   }
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"</TABLE>\n<P>\n");
}

/*********************************************/
/* TOP_URLS_TABLE - generate top n table     */
/*********************************************/

void top_urls_table(int flag)
{
   int i,j,x,cnt,tot_num=0;
   UNODEPTR uptr;

   cnt = (flag)?ntop_urlsK:ntop_urls;
   for (i=0;i<cnt;i++) top_urls[i]=NULL;
   for (i=0;i<MAXHASH;i++)
   {
      uptr=um_htab[i];
      while (uptr!=NULL)
      {
         if (uptr->flag != OBJ_HIDE)
         {
            for (j=0;j<cnt;j++)
            {
               if (top_urls[j]==NULL) { top_urls[j]=uptr; break; }
               else
               {
                  if (flag)
                  {
                     if (uptr->xfer >= top_urls[j]->xfer)
                     {
                        for (x=cnt-1;x>j;x--)
                           top_urls[x] = top_urls[x-1];
                        top_urls[j]=uptr; break;
                     }
                  }
                  else
                  {
                     if (uptr->count >= top_urls[j]->count)
                     {
                        for (x=cnt-1;x>j;x--)
                           top_urls[x] = top_urls[x-1];
                        top_urls[j]=uptr; break;
                     }
                  }
               }
            }
         }
         uptr=uptr->next;
      }
   }

   for (i=0;i<cnt;i++) if (top_urls[i] != NULL) tot_num++;
   if (tot_num==0) return;
   if (tot_num > t_url) tot_num = t_url;
   if ((!flag) || (flag&&!ntop_urls))                   /* do anchor tag    */
      fprintf(out_fp,"<A NAME=\"TOPURLS\"></A>\n");

   fprintf(out_fp,"<TABLE WIDTH=510 BORDER=2 CELLSPACING=1 CELLPADDING=1>\n");
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   if (flag) fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=CENTER COLSPAN=6>"  \
           "%s %d %s %lu %s %s %s</TH></TR>\n",
           GREY,msg_top_top,tot_num,msg_top_of,
           t_url,msg_top_u,msg_h_by,msg_h_xfer);
   else fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=CENTER COLSPAN=6>"   \
           "%s %d %s %lu %s</TH></TR>\n",
           GREY,msg_top_top,tot_num,msg_top_of,t_url,msg_top_u);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=center>"                  \
                  "<FONT SIZE=\"-1\">#</FONT></TH>\n",
                  GREY);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"            \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n",
                  DKGREEN,msg_h_hits);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"            \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n",
                  RED,msg_h_xfer);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center>"                      \
                  "<FONT SIZE=\"-1\">%s</FONT></TH></TR>\n",
                  CYAN,msg_h_url);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   for (i=0;i<cnt;i++)
   {
      if (top_urls[i] != NULL)
      {
         /* shade grouping? */
         if (shade_groups && (top_urls[i]->flag==OBJ_GRP))
            fprintf(out_fp,"<TR BGCOLOR=\"%s\">\n", GRPCOLOR);
         else fprintf(out_fp,"<TR>\n");

         fprintf(out_fp,
             "<TD ALIGN=center><FONT SIZE=\"-1\"><B>%d</B></FONT></TD>\n" \
             "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
             "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"   \
             "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%.0f</B></FONT></TD>\n" \
             "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"   \
             "<TD ALIGN=left NOWRAP><FONT SIZE=\"-1\">",
             i+1,top_urls[i]->count,
             (t_hit==0)?0:((float)top_urls[i]->count/t_hit)*100.0,
             top_urls[i]->xfer/1024,
             (t_xfer==0)?0:((float)top_urls[i]->xfer/t_xfer)*100.0);

         if (top_urls[i]->flag==OBJ_GRP)
         {
            if (hlite_groups)
               fprintf(out_fp,"<STRONG>%s</STRONG></FONT></TD></TR>\n",
                  top_urls[i]->string);
            else fprintf(out_fp,"%s</FONT></TD></TR>\n",top_urls[i]->string);
         }
         else
	 {
            /* check for a service prefix (ie: http://) */
            if (strstr(top_urls[i]->string,"://")!=NULL)
             fprintf(out_fp,
             "<A HREF=\"%s\">%s</A></FONT></TD></TR>\n",
              top_urls[i]->string,top_urls[i]->string);
	    else
            {
               if (log_type) /* FTP log? */
                fprintf(out_fp,"%s</FONT></TD></TR>\n",top_urls[i]->string);
               else
               {             /* Web log  */
                 if (use_https)
                  /* secure server mode, use https:// */
                  fprintf(out_fp,
                  "<A HREF=\"https://%s%s\">%s</A></FONT></TD></TR>\n",
                   hname,top_urls[i]->string,top_urls[i]->string);
                 else
                  /* otherwise use standard 'http://' */
                  fprintf(out_fp,
                  "<A HREF=\"http://%s%s\">%s</A></FONT></TD></TR>\n",
                   hname,top_urls[i]->string,top_urls[i]->string);
               }
            }
	 }
      }
   }
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"</TABLE>\n<P>\n");
}

/*********************************************/
/* TOP_ENTRY_TABLE - top n entry/exit urls   */
/*********************************************/

void top_entry_table(int flag)
{
   int i,j,x,cnt;
   int tot_cnt=0, tot_dsp=0, tot_val=0;

   UNODEPTR uptr;

   cnt = (flag)?ntop_exit:ntop_entry;
   for (i=0;i<cnt;i++) top_urls[i]=NULL;
   for (i=0;i<MAXHASH;i++)
   {
      uptr=um_htab[i];
      while (uptr!=NULL)
      {
         j=(flag)?uptr->exit:uptr->entry;
         if (j) { tot_cnt++; tot_val+=j; }
         if ( (uptr->flag != OBJ_HIDE) && (uptr->flag != OBJ_GRP) && j )
         {
            tot_dsp++;
            for (j=0;j<cnt;j++)
            {
               if (top_urls[j]==NULL) { top_urls[j]=uptr; break; }
               else
               {
                  if (flag)
                  {
                     if (uptr->exit >= top_urls[j]->exit)
                     {
                        for (x=cnt-1;x>j;x--)
                           top_urls[x] = top_urls[x-1];
                        top_urls[j]=uptr; break;
                     }
                  }
                  else
                  {
                     if (uptr->entry >= top_urls[j]->entry)
                     {
                        for (x=cnt-1;x>j;x--)
                           top_urls[x] = top_urls[x-1];
                        top_urls[j]=uptr; break;
                     }
                  }
               }
            }
         }
         uptr=uptr->next;
      }
   }

   if (!tot_dsp) return;
   if (tot_dsp>cnt) tot_dsp=cnt;
   if (tot_cnt>t_url) tot_cnt=t_url;

   if (flag) fprintf(out_fp,"<A NAME=\"TOPEXIT\"></A>\n"); /* do anchor tag */
   else      fprintf(out_fp,"<A NAME=\"TOPENTRY\"></A>\n");

   fprintf(out_fp,"<TABLE WIDTH=510 BORDER=2 CELLSPACING=1 CELLPADDING=1>\n");
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=CENTER COLSPAN=6>"        \
           "%s %d %s %d %s</TH></TR>\n",
           GREY,msg_top_top,tot_dsp,msg_top_of,
           tot_cnt,(flag)?msg_top_ex:msg_top_en);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=center>"                  \
                  "<FONT SIZE=\"-1\">#</FONT></TH>\n",
                  GREY);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"            \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n",
                  DKGREEN,msg_h_hits);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"            \
                  "<FONT SIZE=\"-1\">%s</FONT></TH>\n",
                  YELLOW,msg_h_visits);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center>"                      \
                  "<FONT SIZE=\"-1\">%s</FONT></TH></TR>\n",
                  CYAN,msg_h_url);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   for (i=0;i<tot_dsp;i++)
   {
      if (top_urls[i] != NULL)
      {
         fprintf(out_fp,"<TR>\n");
         fprintf(out_fp,
             "<TD ALIGN=center><FONT SIZE=\"-1\"><B>%d</B></FONT></TD>\n" \
             "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
             "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"   \
             "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
             "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"   \
             "<TD ALIGN=left NOWRAP><FONT SIZE=\"-1\">",
             i+1,top_urls[i]->count,
             (t_hit==0)?0:((float)top_urls[i]->count/t_hit)*100.0,
             (flag)?top_urls[i]->exit:top_urls[i]->entry,
             (flag)?((float)top_urls[i]->exit/tot_val)*100.0
                   :((float)top_urls[i]->entry/tot_val)*100.0);

         /* check for a service prefix (ie: http://) */
         if (strstr(top_urls[i]->string,"://")!=NULL)
          fprintf(out_fp,
             "<A HREF=\"%s\">%s</A></FONT></TD></TR>\n",
              top_urls[i]->string,top_urls[i]->string);
	 else
         {
            if (use_https)
            /* secure server mode, use https:// */
             fprintf(out_fp,
                "<A HREF=\"https://%s%s\">%s</A></FONT></TD></TR>\n",
                 hname,top_urls[i]->string,top_urls[i]->string);
            else
            /* otherwise use standard 'http://' */
             fprintf(out_fp,
                "<A HREF=\"http://%s%s\">%s</A></FONT></TD></TR>\n",
                 hname,top_urls[i]->string,top_urls[i]->string);
	 }
      }
   }
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"</TABLE>\n<P>\n");
}

/*********************************************/
/* TOP_REFS_TABLE - generate top n table     */
/*********************************************/

void top_refs_table()
{
   int i,j,x,tot_num=0;
   RNODEPTR rptr;

   if (t_ref == 0) return;  /* don't bother if we don't have any */

   for (i=0;i<MAXHASH;i++)
   {
      rptr=rm_htab[i];
      while (rptr!=NULL)
      {
         if (rptr->flag != OBJ_HIDE)
         {
            for (j=0;j<ntop_refs;j++)
            {
               if (top_refs[j]==NULL) { top_refs[j]=rptr; break; }
               else
               {
                  if (rptr->count >= top_refs[j]->count)
                  {
                     for (x=ntop_refs-1;x>j;x--)
                        top_refs[x] = top_refs[x-1];
                     top_refs[j]=rptr; break;
                  }
               }
            }
         }
         rptr=rptr->next;
      }
   }

   for (i=0;i<ntop_refs;i++) if (top_refs[i] != NULL) tot_num++;
   if (tot_num==0) return;
   if (tot_num > t_ref) tot_num = t_ref;
   fprintf(out_fp,"<A NAME=\"TOPREFS\"></A>\n");
   fprintf(out_fp,"<TABLE WIDTH=510 BORDER=2 CELLSPACING=1 CELLPADDING=1>\n");
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=CENTER COLSPAN=4>"         \
           "%s %d %s %lu %s</TH></TR>\n",
           GREY, msg_top_top, tot_num, msg_top_of, t_ref, msg_top_r);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=center>"                   \
          "<FONT SIZE=\"-1\">#</FONT></TH>\n",
          GREY);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"             \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",
          DKGREEN,msg_h_hits);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center>"                       \
          "<FONT SIZE=\"-1\">%s</FONT></TH></TR>\n",
          CYAN,msg_h_ref);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   for (i=0;i<ntop_refs;i++)
   {
      if (top_refs[i] != NULL)
      {
         /* shade grouping? */
         if (shade_groups && (top_refs[i]->flag==OBJ_GRP))
            fprintf(out_fp,"<TR BGCOLOR=\"%s\">\n", GRPCOLOR);
         else fprintf(out_fp,"<TR>\n");

         fprintf(out_fp,
             "<TD ALIGN=center><FONT SIZE=\"-1\"><B>%d</B></FONT></TD>\n"  \
             "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n"  \
             "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"    \
             "<TD ALIGN=left NOWRAP><FONT SIZE=\"-1\">",
             i+1,top_refs[i]->count,
             (t_hit==0)?0:((float)top_refs[i]->count/t_hit)*100.0);

         if (top_refs[i]->flag==OBJ_GRP)
         {
            if (hlite_groups)
               fprintf(out_fp,"<STRONG>%s</STRONG>",top_refs[i]->string);
            else fprintf(out_fp,"%s",top_refs[i]->string);
         }
         else
         {
            if (top_refs[i]->string[0] != '-')
            fprintf(out_fp,"<A HREF=\"%s\">%s</A>",
                top_refs[i]->string, top_refs[i]->string);
            else
            fprintf(out_fp,"%s", top_refs[i]->string);
         }
         fprintf(out_fp,"</FONT></TD></TR>\n");
      }
   }
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"</TABLE>\n<P>\n");
}

/*********************************************/
/* TOP_AGENTS_TABLE - generate top n table   */
/*********************************************/

void top_agents_table()
{
   int i,j,x,tot_num=0;
   ANODEPTR aptr;

   if (t_agent == 0) return;    /* don't bother if we don't have any */

   for (i=0;i<MAXHASH;i++)
   {
      aptr=am_htab[i];
      while (aptr!=NULL)
      {
         if (aptr->flag != OBJ_HIDE)
         {
            for (j=0;j<ntop_agents;j++)
            {
               if (top_agents[j]==NULL) { top_agents[j]=aptr; break; }
               else
               {
                  if (aptr->count >= top_agents[j]->count)
                  {
                     for (x=ntop_agents-1;x>j;x--)
                        top_agents[x] = top_agents[x-1];
                     top_agents[j]=aptr; break;
                  }
               }
            }
         }
         aptr=aptr->next;
      }
   }

   for (i=0;i<ntop_agents;i++) if (top_agents[i] != NULL) tot_num++;
   if (tot_num==0) return;
   if (tot_num > t_agent) tot_num = t_agent;
   fprintf(out_fp,"<A NAME=\"TOPAGENTS\"></A>\n");
   fprintf(out_fp,"<TABLE WIDTH=510 BORDER=2 CELLSPACING=1 CELLPADDING=1>\n");
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=CENTER COLSPAN=4>"        \
          "%s %d %s %lu %s</TH></TR>\n",
          GREY, msg_top_top, tot_num, msg_top_of, t_agent, msg_top_a);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=center>"                  \
          "<FONT SIZE=\"-1\">#</FONT></TH>\n",
          GREY);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"            \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",
          DKGREEN,msg_h_hits);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center>"                      \
          "<FONT SIZE=\"-1\">%s</FONT></TH></TR>\n",
          CYAN,msg_h_agent);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   for (i=0;i<ntop_agents;i++)
   {
      if (top_agents[i] != NULL)
      {
         /* shade grouping? */
         if (shade_groups && (top_agents[i]->flag==OBJ_GRP))
            fprintf(out_fp,"<TR BGCOLOR=\"%s\">\n", GRPCOLOR);
         else fprintf(out_fp,"<TR>\n");

         fprintf(out_fp,
             "<TD ALIGN=center><FONT SIZE=\"-1\"><B>%d</B></FONT></TD>\n" \
             "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
             "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"   \
             "<TD ALIGN=left NOWRAP><FONT SIZE=\"-1\">",
             i+1,top_agents[i]->count,
             (t_hit==0)?0:((float)top_agents[i]->count/t_hit)*100.0);

         if ((top_agents[i]->flag==OBJ_GRP)&&hlite_groups)
            fprintf(out_fp,"<STRONG>%s</STRONG></FONT></TD></TR>\n",
               top_agents[i]->string);
         else fprintf(out_fp,"%s</FONT></TD></TR>\n",
               top_agents[i]->string);
      }
   }
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"</TABLE>\n<P>\n");
}

/*********************************************/
/* TOP_SEARCH_TABLE - generate top n table   */
/*********************************************/

void top_search_table()
{
   int i,j,x,tot_num=0;
   u_long t_search=0, t_val=0;
   SNODEPTR sptr;

   if (t_ref == 0) return;    /* don't bother if we don't have any */

   for (i=0;i<MAXHASH;i++)
   {
      sptr=sr_htab[i];
      while (sptr!=NULL)
      {
         for (j=0;j<ntop_search;j++)
         {
            if (top_search[j]==NULL) { top_search[j]=sptr; break; }
            else
            {
               if (sptr->count >= top_search[j]->count)
               {
                  for (x=ntop_search-1;x>j;x--)
                      top_search[x] = top_search[x-1];
                  top_search[j]=sptr; break;
               }
            }
         }
         t_val+=sptr->count;
         t_search++;
         sptr=sptr->next;
      }
   }

   for (i=0;i<ntop_search;i++) if (top_search[i] != NULL) tot_num++;
   if (tot_num==0) return;
   if (tot_num > ntop_search) tot_num=ntop_search;
   fprintf(out_fp,"<A NAME=\"TOPSEARCH\"></A>\n");
   fprintf(out_fp,"<TABLE WIDTH=510 BORDER=2 CELLSPACING=1 CELLPADDING=1>\n");
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=CENTER COLSPAN=4>"        \
          "%s %d %s %lu %s</TH></TR>\n",
          GREY, msg_top_top, tot_num, msg_top_of, t_search, msg_top_sr);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=center>"                  \
          "<FONT SIZE=\"-1\">#</FONT></TH>\n",
          GREY);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"            \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",
          DKGREEN,msg_h_hits);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center>"                      \
          "<FONT SIZE=\"-1\">%s</FONT></TH></TR>\n",
          CYAN,msg_h_search);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   for (i=0;i<tot_num;i++)
   {
      if (top_search[i] != NULL)
      {
         fprintf(out_fp,
             "<TR>\n"                                                     \
             "<TD ALIGN=center><FONT SIZE=\"-1\"><B>%d</B></FONT></TD>\n" \
             "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
             "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"   \
             "<TD ALIGN=left NOWRAP><FONT SIZE=\"-1\">",
             i+1,top_search[i]->count,
             (t_val==0)?0:((float)top_search[i]->count/t_val)*100.0);

         fprintf(out_fp,"%s</FONT></TD></TR>\n",top_search[i]->string);
      }
   }
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"</TABLE>\n<P>\n");
}

/*********************************************/
/* TOP_CTRY_TABLE - top countries table      */
/*********************************************/

void top_ctry_table()
{
   int i,j,x,tot_num=0,tot_ctry=0;
   int ctry_fnd;
   u_long idx;
   HNODEPTR hptr;
   char *domain;
   u_long pie_data[10];
   char   *pie_legend[10];
   char   pie_title[48];
   char   pie_fname[48];

   extern int ctry_graph;  /* include external flag */

   /* scan hash table adding up domain totals */
   for (i=0;i<MAXHASH;i++)
   {
      hptr=sm_htab[i];
      while (hptr!=NULL)
      {
         if (hptr->flag != OBJ_GRP)   /* ignore group totals */
         {
            domain = hptr->string+strlen(hptr->string)-1;
            while ( (*domain!='.')&&(domain!=hptr->string)) domain--;
            if ((domain==hptr->string)||(isdigit((int)*++domain)))
            {
               ctry[0].count+=hptr->count;
               ctry[0].files+=hptr->files;
               ctry[0].xfer +=hptr->xfer;
            }
            else
            {
               ctry_fnd=0;
               idx=ctry_idx(domain);
               for (j=0;j<MAX_CTRY;j++)
               {
                  if (idx==ctry[j].idx)
                  {
                     ctry[j].count+=hptr->count;
                     ctry[j].files+=hptr->files;
                     ctry[j].xfer +=hptr->xfer;
                     ctry_fnd=1;
                     break;
                  }
               }
               if (!ctry_fnd)
               {
                  ctry[0].count+=hptr->count;
                  ctry[0].files+=hptr->files;
                  ctry[0].xfer +=hptr->xfer;
               }
            }
         }
         hptr=hptr->next;
      }
   }

   for (i=0;i<MAX_CTRY;i++)
   {
      if (ctry[i].count!=0) tot_ctry++;
      for (j=0;j<ntop_ctrys;j++)
      {
         if (top_ctrys[j]==NULL) { top_ctrys[j]=&ctry[i]; break; }
         else
         {
            if (ctry[i].count > top_ctrys[j]->count)
            {
               for (x=ntop_ctrys-1;x>j;x--)
                  top_ctrys[x]=top_ctrys[x-1];
               top_ctrys[x]=&ctry[i];
               break;
            }
         }
      }
   }

   /* put our anchor tag first... */
   fprintf(out_fp,"<A NAME=\"TOPCTRYS\"></A>\n");

   /* generate pie chart if needed */
   if (ctry_graph)
   {
      for (i=0;i<10;i++) pie_data[i]=0;             /* init data array      */
      if (ntop_ctrys<10) j=ntop_ctrys; else j=10;   /* ensure data size     */

      for (i=0;i<j;i++)
      {
         pie_data[i]=top_ctrys[i]->count;           /* load the array       */
         pie_legend[i]=top_ctrys[i]->desc;
      }
      sprintf(pie_title,"%s %s %d",msg_ctry_use,l_month[cur_month-1],cur_year);
      sprintf(pie_fname,"ctry_usage_%04d%02d.png",cur_year,cur_month);

      pie_chart(pie_fname,pie_title,t_hit,pie_data,pie_legend);  /* do it   */

      /* put the image tag in the page */
      fprintf(out_fp,"<IMG SRC=\"%s\" ALT=\"%s\" " \
                  "HEIGHT=300 WIDTH=512><P>\n",pie_fname,pie_title);
   }

   /* Now do the table */
   for (i=0;i<ntop_ctrys;i++) if (top_ctrys[i]->count!=0) tot_num++;
   fprintf(out_fp,"<TABLE WIDTH=510 BORDER=2 CELLSPACING=1 CELLPADDING=1>\n");
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=CENTER COLSPAN=8>"         \
           "%s %d %s %d %s</TH></TR>\n",
           GREY,msg_top_top,tot_num,msg_top_of,tot_ctry,msg_top_c);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" ALIGN=center>"                   \
          "<FONT SIZE=\"-1\">#</FONT></TH>\n",GREY);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"             \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",DKGREEN,msg_h_hits);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"             \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",LTBLUE,msg_h_files);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center COLSPAN=2>"             \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",RED,msg_h_xfer);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=center>"                       \
          "<FONT SIZE=\"-1\">%s</FONT></TH></TR>\n",CYAN,msg_h_ctry);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   for (i=0;i<ntop_ctrys;i++)
   {
      if (top_ctrys[i]->count!=0)
      fprintf(out_fp,"<TR>"                                                \
              "<TD ALIGN=center><FONT SIZE=\"-1\"><B>%d</B></FONT></TD>\n" \
              "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
              "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"   \
              "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%lu</B></FONT></TD>\n" \
              "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"   \
              "<TD ALIGN=right><FONT SIZE=\"-1\"><B>%.0f</B></FONT></TD>\n" \
              "<TD ALIGN=right><FONT SIZE=\"-2\">%3.02f%%</FONT></TD>\n"   \
              "<TD ALIGN=left NOWRAP><FONT SIZE=\"-1\">%s</FONT></TD></TR>\n",
              i+1,top_ctrys[i]->count,
              (t_hit==0)?0:((float)top_ctrys[i]->count/t_hit)*100.0,
              top_ctrys[i]->files,
              (t_file==0)?0:((float)top_ctrys[i]->files/t_file)*100.0,
              top_ctrys[i]->xfer/1024,
              (t_xfer==0)?0:((float)top_ctrys[i]->xfer/t_xfer)*100.0,
              top_ctrys[i]->desc);
   }
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"</TABLE>\n<P>\n");
}

/*********************************************/
/* WRITE_MAIN_INDEX - main index.html file   */
/*********************************************/

int write_main_index()
{
   /* create main index file */

   int  i,days_in_month;
   int  lyear=0;
   int	s_mth=0;
   double  gt_hit=0.0;
   double  gt_files=0.0;
   double  gt_pages=0.0;
   double  gt_xfer=0.0;
   double  gt_visits=0.0;
   char    index_fname[256];
   char    buffer[BUFSIZE];

   if (verbose>1) printf("%s\n",msg_gen_sum);

   sprintf(buffer,"%s %s",msg_main_us,hname);

   for (i=0;i<12;i++)                   /* get last month in history */
   {
      if (hist_year[i]>lyear)
       { lyear=hist_year[i]; s_mth=hist_month[i]; }
      if (hist_year[i]==lyear)
      {
         if (hist_month[i]>=s_mth)
            s_mth=hist_month[i];
      }
   }

   i=(s_mth==12)?1:s_mth+1;

   year_graph6x(   "usage.png",         /* filename          */
                   buffer,              /* graph title       */
                   i,                   /* last month        */
                   hist_hit,            /* data set 1        */
                   hist_files,          /* data set 2        */
                   hist_site,           /* data set 3        */
                   hist_xfer,           /* data set 4        */
                   hist_page,           /* data set 5        */
                   hist_visit);         /* data set 6        */

   /* now do html stuff... */
   sprintf(index_fname,"index.%s",html_ext);

   if ( (out_fp=fopen(index_fname,"w")) == NULL)
   {
      if (verbose)
      fprintf(stderr,"%s %s!\n",msg_no_open,index_fname);
      return 1;
   }
   write_html_head(msg_main_per);
   /* year graph */
   fprintf(out_fp,"<IMG SRC=\"usage.png\" ALT=\"%s\" "    \
                  "HEIGHT=256 WIDTH=512><P>\n",buffer);
   /* month table */
   fprintf(out_fp,"<TABLE WIDTH=600 BORDER=2 CELLSPACING=1 CELLPADDING=1>\n");
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH COLSPAN=11 BGCOLOR=\"%s\" ALIGN=center>",GREY);
   fprintf(out_fp,"%s</TH></TR>\n",msg_main_sum);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH ALIGN=left ROWSPAN=2 BGCOLOR=\"%s\">"          \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",GREY,msg_h_mth);
   fprintf(out_fp,"<TH ALIGN=center COLSPAN=4 BGCOLOR=\"%s\">"            \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",GREY,msg_main_da);
   fprintf(out_fp,"<TH ALIGN=center COLSPAN=6 BGCOLOR=\"%s\">"            \
          "<FONT SIZE=\"-1\">%s</FONT></TH></TR>\n",GREY,msg_main_mt);
   fprintf(out_fp,"<TR><TH ALIGN=center BGCOLOR=\"%s\">"                  \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",DKGREEN,msg_h_hits);
   fprintf(out_fp,"<TH ALIGN=center BGCOLOR=\"%s\">"                      \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",LTBLUE,msg_h_files);
   fprintf(out_fp,"<TH ALIGN=center BGCOLOR=\"%s\">"                      \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",CYAN,msg_h_pages);
   fprintf(out_fp,"<TH ALIGN=center BGCOLOR=\"%s\">"                      \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",YELLOW,msg_h_visits);
   fprintf(out_fp,"<TH ALIGN=center BGCOLOR=\"%s\">"                      \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",ORANGE,msg_h_sites);
   fprintf(out_fp,"<TH ALIGN=center BGCOLOR=\"%s\">"                      \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",RED,msg_h_xfer);
   fprintf(out_fp,"<TH ALIGN=center BGCOLOR=\"%s\">"                      \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",YELLOW,msg_h_visits);
   fprintf(out_fp,"<TH ALIGN=center BGCOLOR=\"%s\">"                      \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",CYAN,msg_h_pages);
   fprintf(out_fp,"<TH ALIGN=center BGCOLOR=\"%s\">"                      \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",LTBLUE,msg_h_files);
   fprintf(out_fp,"<TH ALIGN=center BGCOLOR=\"%s\">"                      \
          "<FONT SIZE=\"-1\">%s</FONT></TH></TR>\n",DKGREEN,msg_h_hits);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   for (i=0;i<12;i++)
   {
      if (--s_mth < 0) s_mth = 11;
      if ((hist_month[s_mth]==0) && (hist_files[s_mth]==0)) continue;
      days_in_month=(hist_lday[s_mth]-hist_fday[s_mth])+1;
      fprintf(out_fp,"<TR><TD NOWRAP><A HREF=\"usage_%04d%02d.%s\">"      \
                     "<FONT SIZE=\"-1\">%s %d</FONT></A></TD>\n",
                      hist_year[s_mth], hist_month[s_mth], html_ext,
                      s_month[hist_month[s_mth]-1], hist_year[s_mth]);
      fprintf(out_fp,"<TD ALIGN=right><FONT SIZE=\"-1\">%lu</FONT></TD>\n",
                      hist_hit[s_mth]/days_in_month);
      fprintf(out_fp,"<TD ALIGN=right><FONT SIZE=\"-1\">%lu</FONT></TD>\n",
                      hist_files[s_mth]/days_in_month);
      fprintf(out_fp,"<TD ALIGN=right><FONT SIZE=\"-1\">%lu</FONT></TD>\n",
                      hist_page[s_mth]/days_in_month);
      fprintf(out_fp,"<TD ALIGN=right><FONT SIZE=\"-1\">%lu</FONT></TD>\n",
                      hist_visit[s_mth]/days_in_month);
      fprintf(out_fp,"<TD ALIGN=right><FONT SIZE=\"-1\">%lu</FONT></TD>\n",
                      hist_site[s_mth]);
      fprintf(out_fp,"<TD ALIGN=right><FONT SIZE=\"-1\">%.0f</FONT></TD>\n",
                      hist_xfer[s_mth]);
      fprintf(out_fp,"<TD ALIGN=right><FONT SIZE=\"-1\">%lu</FONT></TD>\n",
                      hist_visit[s_mth]);
      fprintf(out_fp,"<TD ALIGN=right><FONT SIZE=\"-1\">%lu</FONT></TD>\n",
                      hist_page[s_mth]);
      fprintf(out_fp,"<TD ALIGN=right><FONT SIZE=\"-1\">%lu</FONT></TD>\n",
                      hist_files[s_mth]);
      fprintf(out_fp,"<TD ALIGN=right><FONT SIZE=\"-1\">%lu</FONT></TD></TR>\n",
                      hist_hit[s_mth]);
      gt_hit   += hist_hit[s_mth];
      gt_files += hist_files[s_mth];
      gt_pages += hist_page[s_mth];
      gt_xfer  += hist_xfer[s_mth];
      gt_visits+= hist_visit[s_mth];
   }
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"<TR><TH BGCOLOR=\"%s\" COLSPAN=6 ALIGN=left>"          \
          "<FONT SIZE=\"-1\">%s</FONT></TH>\n",GREY,msg_h_totals);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=right>"                       \
          "<FONT SIZE=\"-1\">%.0f</FONT></TH>\n",GREY,gt_xfer);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=right>"                       \
          "<FONT SIZE=\"-1\">%.0f</FONT></TH>\n",GREY,gt_visits);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=right>"                       \
          "<FONT SIZE=\"-1\">%.0f</FONT></TH>\n",GREY,gt_pages);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=right>"                       \
          "<FONT SIZE=\"-1\">%.0f</FONT></TH>\n",GREY,gt_files);
   fprintf(out_fp,"<TH BGCOLOR=\"%s\" ALIGN=right>"                       \
          "<FONT SIZE=\"-1\">%.0f</FONT></TH></TR>\n",GREY,gt_hit);
   fprintf(out_fp,"<TR><TH HEIGHT=4></TH></TR>\n");
   fprintf(out_fp,"</TABLE>\n");
   write_html_tail();
   fclose(out_fp);
   return 0;
}

/*********************************************/
/* NEW_NLIST - create new linked list node   */
/*********************************************/

NLISTPTR new_nlist(char *str)
{
   NLISTPTR newptr;

   if (sizeof(newptr->string) < strlen(str))
   {
      if (verbose)
    fprintf(stderr,"[new_nlist] %s\n",msg_big_one);
   }
   if (( newptr = malloc(sizeof(struct nlist))) != NULL)
    {strncpy(newptr->string, str, sizeof(newptr->string));newptr->next=NULL;}
   return newptr;
}

/*********************************************/
/* ADD_NLIST - add item to FIFO linked list  */
/*********************************************/

int add_nlist(char *str, NLISTPTR *list)
{
   NLISTPTR newptr,cptr,pptr;

   if ( (newptr = new_nlist(str)) != NULL)
   {
      if (*list==NULL) *list=newptr;
      else
      {
         cptr=pptr=*list;
         while(cptr!=NULL) { pptr=cptr; cptr=cptr->next; };
         pptr->next = newptr;
      }
   }
   return newptr==NULL;
}

/*********************************************/
/* DEL_NLIST - delete FIFO linked list       */
/*********************************************/

void del_nlist(NLISTPTR *list)
{
   NLISTPTR cptr,nptr;

   cptr=*list;
   while (cptr!=NULL)
   {
      nptr=cptr->next;
      free(cptr);
      cptr=nptr;
   }
}

/*********************************************/
/* NEW_GLIST - create new linked list node   */
/*********************************************/

GLISTPTR new_glist(char *str, char *name)
{
   GLISTPTR newptr;

   if (sizeof(newptr->string) < strlen(str) ||
       sizeof(newptr->name) < strlen(name))
   {
      if (verbose)
	fprintf(stderr,"[new_glist] %s\n",msg_big_one);
   }
   if (( newptr = malloc(sizeof(struct glist))) != NULL)
     {
       strncpy(newptr->string, str, sizeof(newptr->string));
       strncpy(newptr->name, name, sizeof(newptr->name));
       newptr->next=NULL;
     }
   return newptr;
}

/*********************************************/
/* ADD_GLIST - add item to FIFO linked list  */
/*********************************************/

int add_glist(char *str, GLISTPTR *list)
{
   GLISTPTR newptr,cptr,pptr;
   char *name=str;

   while (!isspace(*name)&&*name!=0) name++;
   if (*name==0) name=str;
   else
   {
      *name++=0;
      while (isspace(*name)&&*name!=0) name++;
      if (*name==0) name=str;
   }

   if ( (newptr = new_glist(str, name)) != NULL)
   {
      if (*list==NULL) *list=newptr;
      else
      {
         cptr=pptr=*list;
         while(cptr!=NULL) { pptr=cptr; cptr=cptr->next; };
         pptr->next = newptr;
      }
   }
   return newptr==NULL;
}

/*********************************************/
/* DEL_GLIST - delete FIFO linked list       */
/*********************************************/

void del_glist(GLISTPTR *list)
{
   GLISTPTR cptr,nptr;

   cptr=*list;
   while (cptr!=NULL)
   {
      nptr=cptr->next;
      free(cptr);
      cptr=nptr;
   }
}

/*********************************************/
/* NEW_HNODE - create host node              */
/*********************************************/

HNODEPTR new_hnode(char *str)
{
   HNODEPTR newptr;

   if (sizeof(newptr->string) < strlen(str))
   {
      if (verbose)
      fprintf(stderr,"[new_hnode] %s\n",msg_big_one);
   }
   if (( newptr = malloc(sizeof(struct hnode))) != NULL)
   {
      strncpy(newptr->string, str, sizeof(newptr->string));
      newptr->visit=1;
      newptr->tstamp=0;
      newptr->lasturl[0]=0;
   }
   return newptr;
}

/*********************************************/
/* PUT_HNODE - insert/update host node       */
/*********************************************/

int put_hnode( char     *str,   /* Hostname  */
               int       type,  /* obj type  */
               u_long    count, /* hit count */
               u_long    file,  /* File flag */
               u_long    xfer,  /* xfer size */
               u_long   *ctr,   /* counter   */
               u_long    visit, /* visits    */
               u_long    tstamp,/* timestamp */
               char     *lasturl, /* lasturl */
               HNODEPTR *htab)  /* ptr>next  */
{
   HNODEPTR cptr,nptr;

   /* check if hashed */
   if ( (cptr = htab[hash(str)]) == NULL)
   {
      /* not hashed */
      if ( (nptr=new_hnode(str)) != NULL)
      {
         nptr->flag  = type;
         nptr->count = count;
         nptr->files = file;
         nptr->xfer  = xfer;
         nptr->next  = NULL;
         htab[hash(str)] = nptr;
         if (type!=OBJ_GRP) (*ctr)++;

         if (visit)
         {
            nptr->visit=(visit-1);
            strncpy(nptr->lasturl,lasturl,MAXURLH);
            nptr->tstamp=tstamp;
            return 0;
         }
         else
         {
            if (ispage(log_rec.url))
            {
               if (htab==sm_htab) update_entry(log_rec.url);
               strncpy(nptr->lasturl,log_rec.url,MAXURLH);
               nptr->tstamp=tstamp;
            }
         }
      }
   }
   else
   {
      /* hashed */
      while (cptr != NULL)
      {
         if (strcmp(cptr->string,str)==0)
         {
            if ((type==cptr->flag)||((type!=OBJ_GRP)&&(cptr->flag!=OBJ_GRP)))
            {
               /* found... bump counter */
               cptr->count+=count;
               cptr->files+=file;
               cptr->xfer +=xfer;

               if (ispage(log_rec.url))
               {
                  if ((tstamp-cptr->tstamp)>=visit_timeout)
                  {
                     if (cptr->tstamp!=0) cptr->visit++;
                     if (htab==sm_htab)
                     {
                        update_exit(cptr->lasturl);
                        update_entry(log_rec.url);
                     }
                  }
                  strncpy(cptr->lasturl,log_rec.url,MAXURLH);
                  cptr->tstamp=tstamp;
                  if (ntop_search) srch_string(log_rec.srchstr);
               }
               return 0;
            }
         }
         cptr = cptr->next;
      }
      /* not found... */
      if ( (nptr = new_hnode(str)) != NULL)
      {
         nptr->flag  = type;
         nptr->count = count;
         nptr->files = file;
         nptr->xfer  = xfer;
         nptr->next  = htab[hash(str)];
         htab[hash(str)]=nptr;
         if (type!=OBJ_GRP) (*ctr)++;

         if (visit)
         {
            nptr->visit = (visit-1);
            strncpy(nptr->lasturl,lasturl,MAXURLH);
            nptr->tstamp= tstamp;
            return 0;
         }
         else
         {
            if (ispage(log_rec.url))
            {
               if (htab==sm_htab) update_entry(log_rec.url);
               strncpy(nptr->lasturl,log_rec.url,MAXURLH);
               nptr->tstamp= tstamp;
            }
         }
      }
   }

   if (nptr!=NULL)
   {
      /* set object type */
      if (type==OBJ_GRP) nptr->flag=OBJ_GRP;            /* is it a grouping? */
      else
      { if (isinlist(hidden_sites,nptr->string)!=NULL)  /* is it hidden?     */
           nptr->flag=OBJ_HIDE;
        if (ntop_search) srch_string(log_rec.srchstr);  /* Get search string */
      }
   }
   return nptr==NULL;
}

/*********************************************/
/* DEL_HLIST - delete host hash table        */
/*********************************************/

void	del_hlist(HNODEPTR *htab)
{
   /* free memory used by hash table */
   HNODEPTR aptr,temp;
   int i;

   for (i=0;i<MAXHASH;i++)
   {
      if (htab[i] != NULL)
      {
         aptr = htab[i];
         while (aptr != NULL)
         {
            temp = aptr->next;
            free (aptr);
            aptr = temp;
         }
         htab[i]=NULL;
      }
   }
}

/*********************************************/
/* NEW_UNODE - URL node creation             */
/*********************************************/

UNODEPTR new_unode(char *str)
{
   UNODEPTR newptr;

   if (sizeof(newptr->string) < strlen(str))
   {
      if (verbose)
      fprintf(stderr,"[new_unode] %s\n",msg_big_one);
   }
   if (( newptr = malloc(sizeof(struct unode))) != NULL)
   {
      strncpy(newptr->string, str, sizeof(newptr->string));
      newptr->count = 0;
      newptr->flag  = OBJ_REG;
   }
   return newptr;
}

/*********************************************/
/* PUT_UNODE - insert/update URL node        */
/*********************************************/

int put_unode(char *str, int type, u_long count, u_long xfer,
              u_long *ctr, u_long entry, u_long exit, UNODEPTR *htab)
{
   UNODEPTR cptr,nptr;

   if (str[0]=='-') return 0;

   /* check if hashed */
   if ( (cptr = htab[hash(str)]) == NULL)
   {
      /* not hashed */
      if ( (nptr=new_unode(str)) != NULL)
      {
         nptr->flag = type;
         nptr->count= count;
         nptr->xfer = xfer;
         nptr->next = NULL;
         nptr->entry= entry;
         nptr->exit = exit;
         htab[hash(str)] = nptr;
         if (type!=OBJ_GRP) (*ctr)++;
      }
   }
   else
   {
      /* hashed */
      while (cptr != NULL)
      {
         if (strcmp(cptr->string,str)==0)
         {
            if ((type==cptr->flag)||((type!=OBJ_GRP)&&(cptr->flag!=OBJ_GRP)))
            {
               /* found... bump counter */
               cptr->count+=count;
               cptr->xfer += xfer;
               return 0;
            }
         }
         cptr = cptr->next;
      }
      /* not found... */
      if ( (nptr = new_unode(str)) != NULL)
      {
         nptr->flag = type;
         nptr->count= count;
         nptr->xfer = xfer;
         nptr->next = htab[hash(str)];
         nptr->entry= entry;
         nptr->exit = exit;
         htab[hash(str)]=nptr;
         if (type!=OBJ_GRP) (*ctr)++;
      }
   }
   if (nptr!=NULL)
   {
      if (type==OBJ_GRP) nptr->flag=OBJ_GRP;
      else if (isinlist(hidden_urls,nptr->string)!=NULL)
                         nptr->flag=OBJ_HIDE;
   }
   return nptr==NULL;
}

/*********************************************/
/* DEL_ULIST - delete URL hash table         */
/*********************************************/

void	del_ulist(UNODEPTR *htab)
{
   /* free memory used by hash table */
   UNODEPTR aptr,temp;
   int i;

   for (i=0;i<MAXHASH;i++)
   {
      if (htab[i] != NULL)
      {
         aptr = htab[i];
         while (aptr != NULL)
         {
            temp = aptr->next;
            free (aptr);
            aptr = temp;
         }
         htab[i]=NULL;
      }
   }
}

/*********************************************/
/* NEW_RNODE - Referrer node creation        */
/*********************************************/

RNODEPTR new_rnode(char *str)
{
   RNODEPTR newptr;

   if (sizeof(newptr->string) < strlen(str))
   {
      if (verbose)
      fprintf(stderr,"[new_rnode] %s\n",msg_big_one);
   }
   if (( newptr = malloc(sizeof(struct rnode))) != NULL)
   {
      strncpy(newptr->string, str, sizeof(newptr->string));
      newptr->count = 1;
      newptr->flag  = OBJ_REG;
   }
   return newptr;
}

/*********************************************/
/* PUT_RNODE - insert/update referrer node   */
/*********************************************/

int put_rnode(char *str, int type, u_long count, u_long *ctr, RNODEPTR *htab)
{
   RNODEPTR cptr,nptr;

   if (str[0]=='-') strcpy(str,"- (Direct Request)");

   /* check if hashed */
   if ( (cptr = htab[hash(str)]) == NULL)
   {
      /* not hashed */
      if ( (nptr=new_rnode(str)) != NULL)
      {
         nptr->flag  = type;
         nptr->count = count;
         nptr->next  = NULL;
         htab[hash(str)] = nptr;
         if (type!=OBJ_GRP) (*ctr)++;
      }
   }
   else
   {
      /* hashed */
      while (cptr != NULL)
      {
         if (strcmp(cptr->string,str)==0)
         {
            if ((type==cptr->flag)||((type!=OBJ_GRP)&&(cptr->flag!=OBJ_GRP)))
            {
               /* found... bump counter */
               cptr->count+=count;
               return 0;
            }
         }
         cptr = cptr->next;
      }
      /* not found... */
      if ( (nptr = new_rnode(str)) != NULL)
      {
         nptr->flag  = type;
         nptr->count = count;
         nptr->next  = htab[hash(str)];
         htab[hash(str)]=nptr;
         if (type!=OBJ_GRP) (*ctr)++;
      }
   }
   if (nptr!=NULL)
   {
      if (type==OBJ_GRP) nptr->flag=OBJ_GRP;
      else if (isinlist(hidden_refs,nptr->string)!=NULL)
                         nptr->flag=OBJ_HIDE;
   }
   return nptr==NULL;
}

/*********************************************/
/* DEL_RLIST - delete referrer hash table    */
/*********************************************/

void	del_rlist(RNODEPTR *htab)
{
   /* free memory used by hash table */
   RNODEPTR aptr,temp;
   int i;

   for (i=0;i<MAXHASH;i++)
   {
      if (htab[i] != NULL)
      {
         aptr = htab[i];
         while (aptr != NULL)
         {
            temp = aptr->next;
            free (aptr);
            aptr = temp;
         }
         htab[i]=NULL;
      }
   }
}

/*********************************************/
/* NEW_ANODE - User Agent node creation      */
/*********************************************/

ANODEPTR new_anode(char *str)
{
   ANODEPTR newptr;

   if (sizeof(newptr->string) < strlen(str))
   {
      if (verbose)
      fprintf(stderr,"[new_anode] %s\n",msg_big_one);
   }
   if (( newptr = malloc(sizeof(struct anode))) != NULL)
   {
      strncpy(newptr->string, str, sizeof(newptr->string));
      newptr->count = 1;
      newptr->flag  = OBJ_REG;
   }
   return newptr;
}

/*********************************************/
/* PUT_ANODE - insert/update user agent node */
/*********************************************/

int put_anode(char *str, int type, u_long count, u_long *ctr, ANODEPTR *htab)
{
   ANODEPTR cptr,nptr;

   if (str[0]=='-') return 0;     /* skip bad user agents */

   /* check if hashed */
   if ( (cptr = htab[hash(str)]) == NULL)
   {
      /* not hashed */
      if ( (nptr=new_anode(str)) != NULL)
      {
         nptr->flag = type;
         nptr->count= count;
         nptr->next = NULL;
         htab[hash(str)] = nptr;
         if (type!=OBJ_GRP) (*ctr)++;
      }
   }
   else
   {
      /* hashed */
      while (cptr != NULL)
      {
         if (strcmp(cptr->string,str)==0)
         {
            if ((type==cptr->flag)||((type!=OBJ_GRP)&&(cptr->flag!=OBJ_GRP)))
            {
               /* found... bump counter */
               cptr->count+=count;
               return 0;
            }
         }
         cptr = cptr->next;
      }
      /* not found... */
      if ( (nptr = new_anode(str)) != NULL)
      {
         nptr->flag  = type;
         nptr->count = count;
         nptr->next  = htab[hash(str)];
         htab[hash(str)]=nptr;
         if (type!=OBJ_GRP) (*ctr)++;
      }
   }
   if (type==OBJ_GRP) nptr->flag=OBJ_GRP;
   else if (isinlist(hidden_agents,nptr->string)!=NULL)
                      nptr->flag=OBJ_HIDE;
   return nptr==NULL;
}

/*********************************************/
/* DEL_ALIST - delete user agent hash table  */
/*********************************************/

void	del_alist(ANODEPTR *htab)
{
   /* free memory used by hash table */
   ANODEPTR aptr,temp;
   int i;

   for (i=0;i<MAXHASH;i++)
   {
      if (htab[i] != NULL)
      {
         aptr = htab[i];
         while (aptr != NULL)
         {
            temp = aptr->next;
            free (aptr);
            aptr = temp;
         }
         htab[i]=NULL;
      }
   }
}

/*********************************************/
/* NEW_SNODE - Search str node creation      */
/*********************************************/

SNODEPTR new_snode(char *str)
{
   SNODEPTR newptr;

   if (sizeof(newptr->string) < strlen(str))
   {
      if (verbose)
      fprintf(stderr,"[new_snode] %s\n",msg_big_one);
   }
   if (( newptr = malloc(sizeof(struct snode))) != NULL)
   {
      strncpy(newptr->string, str, sizeof(newptr->string));
      newptr->count = 1;
   }
   return newptr;
}

/*********************************************/
/* PUT_SNODE - insert/update search str node */
/*********************************************/

int put_snode(char *str, u_long count, SNODEPTR *htab)
{
   SNODEPTR cptr,nptr;

   if (str[0]==0 || str[0]==' ') return 0;     /* skip bad search strs */

   /* check if hashed */
   if ( (cptr = htab[hash(str)]) == NULL)
   {
      /* not hashed */
      if ( (nptr=new_snode(str)) != NULL)
      {
         nptr->count = count;
         nptr->next = NULL;
         htab[hash(str)] = nptr;
      }
   }
   else
   {
      /* hashed */
      while (cptr != NULL)
      {
         if (strcmp(cptr->string,str)==0)
         {
            /* found... bump counter */
            cptr->count+=count;
            return 0;
         }
         cptr = cptr->next;
      }
      /* not found... */
      if ( (nptr = new_snode(str)) != NULL)
      {
         nptr->count = count;
         nptr->next  = htab[hash(str)];
         htab[hash(str)]=nptr;
      }
   }
   return nptr==NULL;
}

/*********************************************/
/* DEL_SLIST - delete search str hash table  */
/*********************************************/

void	del_slist(SNODEPTR *htab)
{
   /* free memory used by hash table */
   SNODEPTR aptr,temp;
   int i;

   for (i=0;i<MAXHASH;i++)
   {
      if (htab[i] != NULL)
      {
         aptr = htab[i];
         while (aptr != NULL)
         {
            temp = aptr->next;
            free (aptr);
            aptr = temp;
         }
         htab[i]=NULL;
      }
   }
}

/*********************************************/
/* HASH - return hash value for string       */
/*********************************************/

u_long hash(char *str)
{
   u_long hashval;
   for (hashval = 0; *str != '\0'; str++)
      hashval = *str + 31 * hashval;
   return hashval % MAXHASH;
}

/*********************************************/
/* ISPAGE - determine if an HTML page or not */
/*********************************************/

int ispage(char *str)
{
   char *cp1, *cp2;

   cp1=cp2=log_rec.url;
   while (*cp1!='\0') { if (*cp1=='.') cp2=cp1; cp1++; }
   if ((cp2++==log_rec.url)||(*(--cp1)=='/')) return 1;
   else return (isinlist(page_type,cp2)!=NULL);
}

/*********************************************/
/* UPDATE_ENTRY - update entry page total    */
/*********************************************/

void update_entry(char *str)
{
   UNODEPTR uptr;

   if (str[0]==0) return;
   if ( (uptr = um_htab[hash(str)]) == NULL) return;
   else
   {
      while (uptr != NULL)
      {
         if (strcmp(uptr->string,str)==0)
         {
            if (uptr->flag!=OBJ_GRP)
            {
               uptr->entry++;
               return;
            }
         }
         uptr=uptr->next;
      }
   }
}

/*********************************************/
/* UPDATE_EXIT  - update exit page total     */
/*********************************************/

void update_exit(char *str)
{
   UNODEPTR uptr;

   if (str[0]==0) return;
   if ( (uptr = um_htab[hash(str)]) == NULL) return;
   else
   {
      while (uptr != NULL)
      {
         if (strcmp(uptr->string,str)==0)
         {
            if (uptr->flag!=OBJ_GRP)
            {
               uptr->exit++;
               return;
            }
         }
         uptr=uptr->next;
      }
   }
}

/*********************************************/
/* MONTH_UPDATE_EXIT  - eom exit page update */
/*********************************************/

void month_update_exit(u_long tstamp)
{
   HNODEPTR nptr;
   int i;

   for (i=0;i<MAXHASH;i++)
   {
      nptr=sm_htab[i];
      while (nptr!=NULL)
      {
         if (nptr->flag!=OBJ_GRP)
         {
            if ((tstamp-nptr->tstamp)>=visit_timeout)
               update_exit(nptr->lasturl);
         }
         nptr=nptr->next;
      }
   }
}

/*********************************************/
/* ISINLIST - Test if string is in list      */
/*********************************************/

char *isinlist(NLISTPTR list, char *str)
{
   NLISTPTR lptr;

   lptr=list;
   while (lptr!=NULL)
   {
      if (isinstr(str,lptr->string)) return lptr->string;
      lptr=lptr->next;
   }
   return NULL;
}

/*********************************************/
/* ISINGLIST - Test if string is in list     */
/*********************************************/

char *isinglist(GLISTPTR list, char *str)
{
   GLISTPTR lptr;

   lptr=list;
   while (lptr!=NULL)
   {
      if (isinstr(str,lptr->string)) return lptr->name;
      lptr=lptr->next;
   }
   return NULL;
}

/*********************************************/
/* ISINSTR - Scan for string in string       */
/*********************************************/

int isinstr(char *str, char *cp)
{
   char *cp1,*cp2;

   cp1=(cp+strlen(cp))-1;
   if (*cp=='*')
   {
      /* if leading wildcard, start from end */
      cp2=str+strlen(str)-1;
      while ( (cp1!=cp) && (cp2!=str))
      {
         if (*cp1=='*') return 1;
         if (*cp1--!=*cp2--) return 0;
      }
      if (cp1==cp) return 1;
      else return 0;
   }
   else
   {
      /* if no leading/trailing wildcard, just strstr */
      if (*cp1!='*') return(strstr(str,cp)!=NULL);
      /* otherwise do normal forward scan */
      cp1=cp; cp2=str;
      while (*cp2!='\0')
      {
         if (*cp1=='*') return 1;
         if (*cp1++!=*cp2++) return 0;
      }
      if (*cp1=='*') return 1;
         else return 0;
   }
}

/*********************************************/
/* ISURLCHAR - checks for valid URL chars    */
/*********************************************/

int isurlchar(char ch)
{
   if (isalnum((int)ch)) return 1;           /* allow letters, numbers...   */
   return (strchr(":/\\.-+_@~",ch)!=NULL);  /* and a few 'special' chars.  */
}

/*********************************************/
/* CTRY_IDX - create unique # from domain    */
/*********************************************/

u_long ctry_idx(char *str)
{
   int i=strlen(str),j=0;
   u_long idx=0;
   char *cp1=str+i;
   for (;i>0;i--) { idx+=((*--cp1-'a'+1)<<j); j+=5; }
   return idx;
}

/*********************************************/
/* FROM_HEX - convert hex char to decimal    */
/*********************************************/

char from_hex(char c)                           /* convert hex to dec      */
{
   c = (c>='0'&&c<='9')?c-'0':                  /* 0-9?                    */
       (c>='A'&&c<='F')?c-'A'+10:               /* A-F?                    */
       c - 'a' + 10;                            /* lowercase...            */
   return (c<0||c>15)?0:c;                      /* return 0 if bad...      */
}

/*********************************************/
/* UNESCAPE - convert escape seqs to chars   */
/*********************************************/

char *unescape(char *str)
{
   char *cp1=str, *cp2=str;

   if (!str) return NULL;                       /* make sure strings valid */

   while (*cp1)
   {
      if (*cp1=='%')                            /* Found an escape?        */
      {
         cp1++;
         if ((*cp1>'1')&&(*cp1<'8'))
         {
            if (*cp1) *cp2=from_hex(*cp1++)*16; /* convert hex to an ascii */
            if (*cp1) *cp2+=from_hex(*cp1);     /* (hopefully) character   */
            if (*cp2<32||*cp2>126) *cp2='_';    /* make underscore if bad  */
            cp2++; cp1++;
         }
         else *cp2++='%';
      }
      else *cp2++ = *cp1++;                     /* if not, just continue   */
   }
   *cp2=*cp1;                                   /* don't forget terminator */
   return str;                                  /* return the string       */
}

/*********************************************/
/* SAVE_STATE - save internal data structs   */
/*********************************************/

int save_state()
{
   HNODEPTR hptr;
   UNODEPTR uptr;
   RNODEPTR rptr;
   ANODEPTR aptr;
   SNODEPTR sptr;

   FILE *fp;
   int  i;

   /* Open data file for write */
   fp=fopen(state_fname,"w");
   if (fp==NULL) return 1;

   /* Saving current run data... */
   if (verbose>1)
   {
      sprintf(buffer,"%02d/%02d/%04d %02d:%02d:%02d",
       cur_month,cur_day,cur_year,cur_hour,cur_min,cur_sec);
      printf("%s [%s]\n",msg_put_data,buffer);
   }

   /* first, save the easy stuff */
   /* Header record */
   sprintf(buffer,
     "# Webalizer V%s-%s Incremental Data - %02d/%02d/%04d %02d:%02d:%02d\n",
      version,editlvl,cur_month,cur_day,cur_year,cur_hour,cur_min,cur_sec);
   if (fputs(buffer,fp)==EOF) return 1;  /* error exit */

   /* Current date/time          */
   sprintf(buffer,"%d %d %d %d %d %d\n",
        cur_year, cur_month, cur_day, cur_hour, cur_min, cur_sec);
   if (fputs(buffer,fp)==EOF) return 1;  /* error exit */

   /* Monthly totals for sites, urls, etc... */
   sprintf(buffer,"%lu %lu %lu %lu %lu %lu %.0f %lu %lu\n",
        t_hit, t_file, t_site, t_url,
        t_ref, t_agent, t_xfer, t_page, t_visit);
   if (fputs(buffer,fp)==EOF) return 1;  /* error exit */

   /* Daily totals for sites, urls, etc... */
   sprintf(buffer,"%lu %lu %lu %d %d\n",
        dt_site, ht_hit, mh_hit, f_day, l_day);
   if (fputs(buffer,fp)==EOF) return 1;  /* error exit */

   /* Monthly (by day) total array */
   for (i=0;i<31;i++)
   {
      sprintf(buffer,"%lu %lu %.0f %lu %lu %lu\n",
        tm_hit[i],tm_file[i],tm_xfer[i],tm_site[i],tm_page[i],tm_visit[i]);
      if (fputs(buffer,fp)==EOF) return 1;  /* error exit */
   }

   /* Daily (by hour) total array */
   for (i=0;i<24;i++)
   {
      sprintf(buffer,"%lu %lu %.0f %lu\n",
        th_hit[i],th_file[i],th_xfer[i],th_page[i]);
      if (fputs(buffer,fp)==EOF) return 1;  /* error exit */
   }

   /* Response codes */
   for (i=0;i<TOTAL_RC;i++)
   {
      sprintf(buffer,"%lu\n",response[i].count);
      if (fputs(buffer,fp)==EOF) return 1;  /* error exit */
   }

   /* now we need to save our linked lists */
   /* daily hostname list */
   if (fputs("# -sites- (monthly)\n",fp)==EOF) return 1;  /* error exit */

   for (i=0;i<MAXHASH;i++)
   {
      hptr=sm_htab[i];
      while (hptr!=NULL)
      {
         sprintf(buffer,"%s\n%d %lu %lu %.0f %lu %lu\n%s\n",
              hptr->string,
              hptr->flag,
              hptr->count,
              hptr->files,
              hptr->xfer,
              hptr->visit,
              hptr->tstamp,
              (hptr->lasturl[0]==0)?"-":hptr->lasturl);
         if (fputs(buffer,fp)==EOF) return 1;  /* error exit */
         hptr=hptr->next;
      }
   }
   if (fputs("# End Of Table - sites (monthly)\n",fp)==EOF) return 1;

   /* hourly hostname list */
   if (fputs("# -sites- (daily)\n",fp)==EOF) return 1;  /* error exit */
   for (i=0;i<MAXHASH;i++)
   {
      hptr=sd_htab[i];
      while (hptr!=NULL)
      {
         sprintf(buffer,"%s\n%d %lu %lu %.0f %lu %lu\n%s\n",
              hptr->string,
              hptr->flag,
              hptr->count,
              hptr->files,
              hptr->xfer,
              hptr->visit,
              hptr->tstamp,
              (hptr->lasturl[0]==0)?"-":hptr->lasturl);
         if (fputs(buffer,fp)==EOF) return 1;
         hptr=hptr->next;
      }
   }
   if (fputs("# End Of Table - sites (daily)\n",fp)==EOF) return 1;

   /* URL list */
   if (fputs("# -urls- \n",fp)==EOF) return 1;  /* error exit */
   for (i=0;i<MAXHASH;i++)
   {
      uptr=um_htab[i];
      while (uptr!=NULL)
      {
         sprintf(buffer,"%s\n%d %lu %lu %.0f %lu %lu\n", uptr->string,
              uptr->flag, uptr->count, uptr->files, uptr->xfer,
              uptr->entry, uptr->exit);
         if (fputs(buffer,fp)==EOF) return 1;
         uptr=uptr->next;
      }
   }
   if (fputs("# End Of Table - urls\n",fp)==EOF) return 1;  /* error exit */

   /* Referrer list */
   if (fputs("# -referrers- \n",fp)==EOF) return 1;  /* error exit */
   if (t_ref != 0)
   {
      for (i=0;i<MAXHASH;i++)
      {
         rptr=rm_htab[i];
         while (rptr!=NULL)
         {
            sprintf(buffer,"%s\n%d %lu\n", rptr->string,
                 rptr->flag, rptr->count);
            if (fputs(buffer,fp)==EOF) return 1;  /* error exit */
            rptr=rptr->next;
         }
      }
   }
   if (fputs("# End Of Table - referrers\n",fp)==EOF) return 1;

   /* User agent list */
   if (fputs("# -agents- \n",fp)==EOF) return 1;  /* error exit */
   if (t_agent != 0)
   {
      for (i=0;i<MAXHASH;i++)
      {
         aptr=am_htab[i];
         while (aptr!=NULL)
         {
            sprintf(buffer,"%s\n%d %lu\n", aptr->string,
                 aptr->flag, aptr->count);
            if (fputs(buffer,fp)==EOF) return 1;  /* error exit */
            aptr=aptr->next;
         }
      }
   }
   if (fputs("# End Of Table - agents\n",fp)==EOF) return 1;

   /* Search String list */
   if (fputs("# -search strings- \n",fp)==EOF) return 1;  /* error exit */
   for (i=0;i<MAXHASH;i++)
   {
      sptr=sr_htab[i];
      while (sptr!=NULL)
      {
         sprintf(buffer,"%s\n%lu\n", sptr->string,sptr->count);
         if (fputs(buffer,fp)==EOF) return 1;  /* error exit */
         sptr=sptr->next;
      }
   }
   if (fputs("# End Of Table - search strings\n",fp)==EOF) return 1;

   fclose(fp);          /* close data file...                            */
   return 0;            /* successful, return with good return code      */
}

/*********************************************/
/* RESTORE_STATE - reload internal run data  */
/*********************************************/

int restore_state()
{
   FILE *fp;
   int  i;
   struct hnode t_hnode;
   struct unode t_unode;
   struct rnode t_rnode;
   struct anode t_anode;
   struct snode t_snode;

   fp=fopen(state_fname,"r");
   if (fp==NULL)
   {
      /* Previous run data not found... */
      if (verbose>1) printf("%s\n",msg_no_data);
      return 0;   /* return with ok code */
   }

   /* Reading previous run data... */
   if (verbose>1) printf("%s %s\n",msg_get_data,state_fname);

   /* get easy stuff */
   if ((fgets(buffer,BUFSIZE,fp)) != NULL)                 /* Header record */
     {if (strncmp(buffer,"# Webalizer V1.30",17)) return 99;} /* bad magic? */
   else return 1;   /* error exit */

   /* Get current timestamp */
   if ((fgets(buffer,BUFSIZE,fp)) != NULL)
   {
      sscanf(buffer,"%d %d %d %d %d %d",
       &cur_year, &cur_month, &cur_day,
       &cur_hour, &cur_min, &cur_sec);
   } else return 2;  /* error exit */

   /* calculate current timestamp (jdate-epochHHMMSS) */
   cur_tstamp=((jdate(cur_day,cur_month,cur_year)-epoch)*1000000)+
      (cur_hour*10000) + (cur_min*100) + cur_sec;

   /* Get monthly totals */
   if ((fgets(buffer,BUFSIZE,fp)) != NULL)
   {
      sscanf(buffer,"%lu %lu %lu %lu %lu %lu %lf %lu %lu",
       &t_hit, &t_file, &t_site, &t_url,
       &t_ref, &t_agent, &t_xfer, &t_page, &t_visit);
   } else return 3;  /* error exit */

   /* Get daily totals */
   if ((fgets(buffer,BUFSIZE,fp)) != NULL)
   {
      sscanf(buffer,"%lu %lu %lu %d %d",
       &dt_site, &ht_hit, &mh_hit, &f_day, &l_day);
   } else return 4;  /* error exit */

   /* get daily totals */
   for (i=0;i<31;i++)
   {
      if ((fgets(buffer,BUFSIZE,fp)) != NULL)
      {
         sscanf(buffer,"%lu %lu %lf %lu %lu %lu",
          &tm_hit[i],&tm_file[i],&tm_xfer[i],&tm_site[i],&tm_page[i],
          &tm_visit[i]);
      } else return 5;  /* error exit */
   }

   /* get hourly totals */
   for (i=0;i<24;i++)
   {
      if ((fgets(buffer,BUFSIZE,fp)) != NULL)
      {
         sscanf(buffer,"%lu %lu %lf %lu",
          &th_hit[i],&th_file[i],&th_xfer[i],&th_page[i]);
      } else return 6;  /* error exit */
   }

   /* get response code totals */
   for (i=0;i<TOTAL_RC;i++)
   {
      if ((fgets(buffer,BUFSIZE,fp)) != NULL)
         sscanf(buffer,"%lu",&response[i].count);
      else return 7;  /* error exit */
   }

   /* now do hash tables */

   /* monthly sites table */
   if ((fgets(buffer,BUFSIZE,fp)) != NULL)               /* Table header */
   { if (strncmp(buffer,"# -sites- ",10)) return 8; }    /* (monthly)    */
   else return 8;   /* error exit */

   while ((fgets(buffer,BUFSIZE,fp)) != NULL)
   {
      /* Check for end of table */
      if (!strncmp(buffer,"# End Of Table ",15)) break;
      strncpy(t_hnode.string,buffer,MAXURLH);
      for (i=0;i<strlen(t_hnode.string);i++)
         if (t_hnode.string[i]=='\n') t_hnode.string[i]='\0';

      if ((fgets(buffer,BUFSIZE,fp)) == NULL) return 8;  /* error exit */
      if (!isdigit((int)buffer[0])) return 8;  /* error exit */

      /* load temporary node data */
      sscanf(buffer,"%d %lu %lu %lf %lu %lu",
         &t_hnode.flag,&t_hnode.count,
         &t_hnode.files, &t_hnode.xfer,
         &t_hnode.visit, &t_hnode.tstamp);

      /* get last url */
      if ((fgets(buffer,BUFSIZE,fp)) == NULL) return 8;  /* error exit */
      strncpy(t_hnode.lasturl,buffer,MAXURLH);
      for (i=0;i<strlen(t_hnode.lasturl);i++)
         if (t_hnode.lasturl[i]=='\n') t_hnode.lasturl[i]='\0';

      /* Good record, insert into hash table */
      if (t_hnode.lasturl[0]=='-') t_hnode.lasturl[0]=0;
      if (put_hnode(t_hnode.string,t_hnode.flag,
         t_hnode.count,t_hnode.files,t_hnode.xfer,&ul_bogus,
         t_hnode.visit+1,t_hnode.tstamp,t_hnode.lasturl,sm_htab))
      {
         if (verbose)
         /* Error adding host node (monthly), skipping .... */
         fprintf(stderr,"%s %s\n",msg_nomem_mh, t_hnode.string);
      }
   }

   /* Daily sites table */
   if ((fgets(buffer,BUFSIZE,fp)) != NULL)               /* Table header */
   { if (strncmp(buffer,"# -sites- ",10)) return 9; }    /* (daily)      */
   else return 9;   /* error exit */

   while ((fgets(buffer,BUFSIZE,fp)) != NULL)
   {
      /* Check for end of table */
      if (!strncmp(buffer,"# End Of Table ",15)) break;
      strncpy(t_hnode.string,buffer,MAXURLH);
      for (i=0;i<strlen(t_hnode.string);i++)
         if (t_hnode.string[i]=='\n') t_hnode.string[i]='\0';

      if ((fgets(buffer,BUFSIZE,fp)) == NULL) return 9;  /* error exit */
      if (!isdigit((int)buffer[0])) return 9;  /* error exit */

      /* load temporary node data */
      sscanf(buffer,"%d %lu %lu %lf %lu %lu",
          &t_hnode.flag,&t_hnode.count,
          &t_hnode.files, &t_hnode.xfer,
          &t_hnode.visit, &t_hnode.tstamp);

      /* get last url */
      if ((fgets(buffer,BUFSIZE,fp)) == NULL) return 9;  /* error exit */
      strncpy(t_hnode.lasturl,buffer,MAXURLH);
      for (i=0;i<strlen(t_hnode.lasturl);i++)
         if (t_hnode.lasturl[i]=='\n') t_hnode.lasturl[i]='\0';

      /* Good record, insert into hash table */
      if (t_hnode.lasturl[0]=='-') t_hnode.lasturl[0]=0;
      if (put_hnode(t_hnode.string,t_hnode.flag,
         t_hnode.count,t_hnode.files,t_hnode.xfer,&ul_bogus,
         t_hnode.visit+1,t_hnode.tstamp,t_hnode.lasturl,sd_htab))
      {
         /* Error adding host node (daily), skipping .... */
         if (verbose) fprintf(stderr,"%s %s\n",msg_nomem_dh, t_hnode.string);
      }
   }

   /* url table */
   if ((fgets(buffer,BUFSIZE,fp)) != NULL)               /* Table header */
   { if (strncmp(buffer,"# -urls- ",9)) return 10; }     /* (url)        */
   else return 10;   /* error exit */

   while ((fgets(buffer,BUFSIZE,fp)) != NULL)
   {
      if (!strncmp(buffer,"# End Of Table ",15)) break;
      strncpy(t_unode.string,buffer,MAXURLH);
      for (i=0;i<strlen(t_unode.string);i++)
         if (t_unode.string[i]=='\n') t_unode.string[i]='\0';

      if ((fgets(buffer,BUFSIZE,fp)) == NULL) return 10;  /* error exit */
      if (!isdigit((int)buffer[0])) return 10;  /* error exit */

      /* load temporary node data */
      sscanf(buffer,"%d %lu %lu %lf %lu %lu",
         &t_unode.flag,&t_unode.count,
         &t_unode.files, &t_unode.xfer,
         &t_unode.entry, &t_unode.exit);

      /* Good record, insert into hash table */
      if (put_unode(t_unode.string,t_unode.flag,t_unode.count,
         t_unode.xfer,&ul_bogus,t_unode.entry,t_unode.exit,um_htab))
      {
         if (verbose)
         /* Error adding URL node, skipping ... */
         fprintf(stderr,"%s %s\n", msg_nomem_u, t_unode.string);
      }
   }

   /* Referrers table */
   if ((fgets(buffer,BUFSIZE,fp)) != NULL)               /* Table header */
   { if (strncmp(buffer,"# -referrers- ",14)) return 11; } /* (referrers)*/
   else return 11;   /* error exit */

   while ((fgets(buffer,BUFSIZE,fp)) != NULL)
   {
      if (!strncmp(buffer,"# End Of Table ",15)) break;
      strncpy(t_rnode.string,buffer,MAXREFH);
      for (i=0;i<strlen(t_rnode.string);i++)
         if (t_rnode.string[i]=='\n') t_rnode.string[i]='\0';

      if ((fgets(buffer,BUFSIZE,fp)) == NULL) return 11;  /* error exit */
      if (!isdigit((int)buffer[0])) return 11;  /* error exit */

      /* load temporary node data */
      sscanf(buffer,"%d %lu",&t_rnode.flag,&t_rnode.count);

      /* insert node */
      if (put_rnode(t_rnode.string,t_rnode.flag,
         t_rnode.count, &ul_bogus, rm_htab))
      {
         if (verbose) fprintf(stderr,"%s %s\n", msg_nomem_r, log_rec.refer);
      }
   }

   /* Agents table */
   if ((fgets(buffer,BUFSIZE,fp)) != NULL)               /* Table header */
   { if (strncmp(buffer,"# -agents- ",11)) return 12; } /* (agents)*/
   else return 12;   /* error exit */

   while ((fgets(buffer,BUFSIZE,fp)) != NULL)
   {
      if (!strncmp(buffer,"# End Of Table ",15)) break;
      strncpy(t_anode.string,buffer,MAXAGENT);
      for (i=0;i<strlen(t_anode.string);i++)
         if (t_anode.string[i]=='\n') t_anode.string[i]='\0';

      if ((fgets(buffer,BUFSIZE,fp)) == NULL) return 12;  /* error exit */
      if (!isdigit((int)buffer[0])) return 12;  /* error exit */

      /* load temporary node data */
      sscanf(buffer,"%d %lu",&t_anode.flag,&t_anode.count);

      /* insert node */
      if (put_anode(t_anode.string,t_anode.flag,t_anode.count,
         &ul_bogus,am_htab))
      {
         if (verbose) fprintf(stderr,"%s %s\n", msg_nomem_a, log_rec.agent);
      }
   }

   /* Search Strings table */
   if ((fgets(buffer,BUFSIZE,fp)) != NULL)               /* Table header */
   { if (strncmp(buffer,"# -search string",16)) return 13; }  /* (search)*/
   else return 13;   /* error exit */

   while ((fgets(buffer,BUFSIZE,fp)) != NULL)
   {
      if (!strncmp(buffer,"# End Of Table ",15)) break;
      strncpy(t_snode.string,buffer,MAXSRCH);
      for (i=0;i<strlen(t_snode.string);i++)
         if (t_snode.string[i]=='\n') t_snode.string[i]='\0';

      if ((fgets(buffer,BUFSIZE,fp)) == NULL) return 13;  /* error exit */
      if (!isdigit((int)buffer[0])) return 13;  /* error exit */

      /* load temporary node data */
      sscanf(buffer,"%lu",&t_snode.count);

      /* insert node */
      if (put_snode(t_snode.string,t_snode.count,sr_htab))
      {
         if (verbose) fprintf(stderr,"%s %s\n", msg_nomem_sc, t_snode.string);
      }
   }

   fclose(fp);
   check_dup = 1;              /* enable duplicate checking */
   return 0;                   /* return with ok code       */
}

/*********************************************/
/* SRCH_STRING - get search strings from ref */
/*********************************************/

void srch_string(char *ptr)
{
   /* ptr should point to unescaped query string */
   char tmpbuf[BUFSIZE];
   char srch[16]="";
   char *cp1, *cp2, *cps;

   /* horrible kludge code follows :) */
   if (strstr(log_rec.refer,"yahoo.com")!=NULL) cps="p=";       /* Yahoo     */
   else
   {
    if ( strstr(log_rec.refer,"altavista")!=NULL  ||            /* AltaVista */
         strstr(log_rec.refer,"google.com")!=NULL) cps="q=";    /* & Google  */
    else
    {
     if (strstr(log_rec.refer,"lycos.com")!=NULL) cps="query="; /* Lycos     */
     else
     {
      if (strstr(log_rec.refer,"hotbot.com")!=NULL) cps="MT=";  /* HotBot    */
      else
      {
       if (strstr(log_rec.refer,"infoseek.")!=NULL) cps="qt=";  /* InfoSeek  */
       else
       {
        if (strstr(log_rec.refer,"webcrawler")!=NULL) cps="searchText=";
        else
        {
         if (strstr(log_rec.refer,"excite")!=NULL  ||      /* Netscape/Excite */
             strstr(log_rec.refer,"netscape.com")!=NULL)
           cps="search=";
         else return;
        }
       }
      }
     }
    }
   }

   /* Try to find query variable */
   srch[0]='?'; strcpy(&srch[1],cps);              /* First, try "?..."      */
   if ((cp1=strstr(ptr,srch))==NULL)
   {
      srch[0]='&';                                 /* Next, try "&..."       */
      if ((cp1=strstr(ptr,srch))==NULL) return;    /* If not found, split... */
   }
   cp2=tmpbuf;
   while (*cp1!='=' && *cp1!=0) cp1++; if (*cp1!=0) cp1++;
   while (*cp1!='&' && *cp1!=0)
   {
      if (*cp1=='"' || *cp1==',') { cp1++; continue; } /* skip bad ones..    */
      if (*cp1=='+')
      { *cp2++=' '; cp1++; }                           /* + = space          */
      else
      { *cp2++=tolower(*cp1++); }                      /* normal character   */
   }
   *cp2=0; cp2=tmpbuf;
   if (tmpbuf[0]=='?') tmpbuf[0]=' ';                  /* google fix ?       */
   while( *cp2!=0 && isspace(*cp2) ) cp2++;            /* skip leading sps.  */
   if (*cp2==0) return;
   if (put_snode(cp2,(u_long)1,sr_htab))
   {
      if (verbose)
      /* Error adding search string node, skipping .... */
      fprintf(stderr,"%s %s\n", msg_nomem_sc, tmpbuf);
   }
   return;
}
