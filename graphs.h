/* graphs.h */

extern int  month_graph6(char *, char *, int, int, u_long *,
             u_long *, u_long *, double *, u_long *, u_long *);
extern int  day_graph3(char *, char *, u_long *, u_long *, u_long *);
extern int  year_graph6x(char *, char *, int,
             u_long *, u_long *, u_long *, double *, u_long *, u_long *);
extern int  pie_chart(char *, char *, u_long, u_long *, char **);

extern u_long jdate(int,int,int);  /* julian date routine */
