#ifndef _GRAPHS_H
#define _GRAPHS_H

#include "webalizer.h" // HISTSIZE

int month_graph6(char  *fname,        /* filename           */
                 char  *title,        /* graph title        */
                 int   month,         /* graph month        */
                 int   year,          /* graph year         */
            u_int64_t  data1[31],     /* data1 (hits)       */
            u_int64_t  data2[31],     /* data2 (files)      */
            u_int64_t  data3[31],     /* data3 (sites)      */
            double     data4[31],     /* data4 (kbytes)     */
            double     data5[31],     /* data4 (kbytes)     */
            double     data6[31],     /* data4 (kbytes)     */
            u_int64_t  data7[31],     /* data5 (views)      */
            u_int64_t  data8[31]);    /* data6 (visits)     */

int year_graph6x(char *fname, char *title, struct hist_rec data[HISTSIZE]);
int day_graph3(char *fname, char *title, u_int64_t data1[24], u_int64_t data2[24], u_int64_t data3[24]);
int pie_chart(char *fname, char *title, u_int64_t t_val, u_int64_t data1[], char *legend[]);

#endif  /* _GRAPHS_H */
