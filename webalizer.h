/* webalizer.h */

#define PCENT(val,max) ((val)?((double)val/(double)max)*100.0 : 0.0)

#define BUFSIZE  4096                  /* Max buffer size for log record   */
#define MAXHOST  64                    /* Max hostname buffer size         */
#define MAXURL   256                   /* Max HTTP request/URL field size  */
#define MAXURLH  96                    /* Max URL field size in htab       */
#define MAXREF   1024                  /* Max referrer field size          */
#define MAXREFH  128                   /* Max referrer field size in htab  */
#define MAXAGENT 64                    /* Max user agent field size        */
#define MAXCTRY  48                    /* Max country name size            */
#define MAXHASH  2048                  /* Size of our hash tables          */
#define MAXSRCH  64                    /* Max size of search string buffer */

/* define structure to hold expanded log record */

struct  log_struct  {  char   hostname[MAXHOST];   /* hostname             */
                       char   datetime[29];        /* raw timestamp        */
                       char   url[MAXURL];         /* raw request field    */
                       int    resp_code;           /* response code        */
                       u_long xfer_size;           /* xfer size in bytes   */
                       char   refer[MAXREF];       /* referrer             */
                       char   agent[MAXAGENT];     /* user agent (browser) */
                       char   srchstr[MAXSRCH]; }; /* search string        */

/* define some colors for HTML */
#define WHITE       "#FFFFFF"
#define BLACK       "#000000"
#define RED         "#FF0000"
#define ORANGE      "#FF8000"
#define LTBLUE      "#0080FF"
#define BLUE        "#0000FF"
#define GREEN       "#00FF00"
#define DKGREEN     "#008040"
#define GREY        "#C0C0C0"
#define LTGREY      "#E8E8E8"
#define YELLOW      "#FFFF00"
#define PURPLE      "#FF00FF"
#define CYAN        "#00E0FF"
#define GRPCOLOR    "#D0D0E0"

/* Response code defines as per draft ietf HTTP/1.1 rev 6 */
#define RC_CONTINUE           100
#define RC_SWITCHPROTO        101
#define RC_OK                 200
#define RC_CREATED            201
#define RC_ACCEPTED           202
#define RC_NONAUTHINFO        203
#define RC_NOCONTENT          204
#define RC_RESETCONTENT       205
#define RC_PARTIALCONTENT     206
#define RC_MULTIPLECHOICES    300
#define RC_MOVEDPERM          301
#define RC_MOVEDTEMP          302
#define RC_SEEOTHER           303
#define RC_NOMOD              304
#define RC_USEPROXY           305
#define RC_MOVEDTEMPORARILY   307
#define RC_BAD                400
#define RC_UNAUTH             401
#define RC_PAYMENTREQ         402
#define RC_FORBIDDEN          403
#define RC_NOTFOUND           404
#define RC_METHODNOTALLOWED   405
#define RC_NOTACCEPTABLE      406
#define RC_PROXYAUTHREQ       407
#define RC_TIMEOUT            408
#define RC_CONFLICT           409
#define RC_GONE               410
#define RC_LENGTHREQ          411
#define RC_PREFAILED          412
#define RC_REQENTTOOLARGE     413
#define RC_REQURITOOLARGE     414
#define RC_UNSUPMEDIATYPE     415
#define RC_RNGNOTSATISFIABLE  416
#define RC_EXPECTATIONFAILED  417
#define RC_SERVERERR          500
#define RC_NOTIMPLEMENTED     501
#define RC_BADGATEWAY         502
#define RC_UNAVAIL            503
#define RC_GATEWAYTIMEOUT     504
#define RC_BADHTTPVER         505

/* Index defines for RC codes */
#define IDX_UNDEFINED          0
#define IDX_CONTINUE           1
#define IDX_SWITCHPROTO        2
#define IDX_OK                 3
#define IDX_CREATED            4
#define IDX_ACCEPTED           5
#define IDX_NONAUTHINFO        6
#define IDX_NOCONTENT          7
#define IDX_RESETCONTENT       8
#define IDX_PARTIALCONTENT     9
#define IDX_MULTIPLECHOICES    10
#define IDX_MOVEDPERM          11
#define IDX_MOVEDTEMP          12
#define IDX_SEEOTHER           13
#define IDX_NOMOD              14
#define IDX_USEPROXY           15
#define IDX_MOVEDTEMPORARILY   16
#define IDX_BAD                17
#define IDX_UNAUTH             18
#define IDX_PAYMENTREQ         19
#define IDX_FORBIDDEN          20
#define IDX_NOTFOUND           21
#define IDX_METHODNOTALLOWED   22
#define IDX_NOTACCEPTABLE      23
#define IDX_PROXYAUTHREQ       24
#define IDX_TIMEOUT            25
#define IDX_CONFLICT           26
#define IDX_GONE               27
#define IDX_LENGTHREQ          28
#define IDX_PREFAILED          29
#define IDX_REQENTTOOLARGE     30
#define IDX_REQURITOOLARGE     31
#define IDX_UNSUPMEDIATYPE     32
#define IDX_RNGNOTSATISFIABLE  33
#define IDX_EXPECTATIONFAILED  34
#define IDX_SERVERERR          35
#define IDX_NOTIMPLEMENTED     36
#define IDX_BADGATEWAY         37
#define IDX_UNAVAIL            38
#define IDX_GATEWAYTIMEOUT     39
#define IDX_BADHTTPVER         40
#define TOTAL_RC               40

/* Response code structure */
struct response_code {     char    *desc;         /* response code struct  */
                         u_long    count; };

/* node definitions */
typedef struct hnode *HNODEPTR;            /* site node (host) pointer     */
typedef struct unode *UNODEPTR;            /* url node pointer             */
typedef struct rnode *RNODEPTR;            /* referrer node                */
typedef struct anode *ANODEPTR;            /* user agent node pointer      */
typedef struct nlist *NLISTPTR;            /* HIDE list item pointer       */
typedef struct glist *GLISTPTR;            /* GROUP list item pointer      */
typedef struct snode *SNODEPTR;            /* Search string node pointer   */

/* Object flags */
#define OBJ_REG  0                         /* Regular object               */
#define OBJ_HIDE 1                         /* Hidden object                */
#define OBJ_GRP  2                         /* Grouped object               */

struct hnode {  char string[MAXHOST];      /* host hash table structure    */
                 int flag;
              u_long count;
              u_long files;
              u_long visit;                /* visit information            */
              u_long tstamp;
                char lasturl[MAXURLH];
              double xfer;
              struct hnode *next; };

struct unode {  char string[MAXURLH];      /* url hash table structure     */
                 int flag;                 /* Object type (REG, HIDE, GRP) */
              u_long count;                /* requests counter             */
              u_long files;                /* files counter                */
              u_long entry;                /* entry page counter           */
              u_long exit;                 /* exit page counter            */
              double xfer;                 /* xfer size in bytes           */
              struct unode *next; };       /* pointer to next node         */

struct rnode {  char string[MAXREFH];      /* referrer hash table struct   */
                 int flag;
              u_long count;
              struct rnode *next; };

struct anode {  char string[MAXAGENT];     /* user agent hash table struct */
                 int flag;
              u_long count;
              struct anode *next; };

struct snode {  char string[MAXSRCH];      /* search string table struct   */
              u_long count;
              struct snode *next; };

struct nlist {  char string[80];           /* list struct for HIDE items   */
              struct nlist *next; };

struct glist {  char string[80];           /* list struct for GROUP items  */
                char name[80];
              struct glist *next; };

/***********************/
/* function prototypes */
/***********************/

HNODEPTR new_hnode(char *);                         /* new host node       */
UNODEPTR new_unode(char *);                         /* new url node        */
RNODEPTR new_rnode(char *);                         /* new referrer node   */
ANODEPTR new_anode(char *);                         /* new user agent node */
SNODEPTR new_snode(char *);                         /* new search string.. */
/* add/update node routines */
int      put_hnode(char *, int, u_long, u_long, u_long, u_long *, u_long, u_long, char *, HNODEPTR *);
int      put_unode(char *, int, u_long, u_long, u_long *, u_long, u_long, UNODEPTR *);
int      put_rnode(char *, int, u_long, u_long *, RNODEPTR *);
int      put_anode(char *, int, u_long, u_long *, ANODEPTR *);
int      put_snode(char *, u_long, SNODEPTR *);
void     del_hlist(HNODEPTR *);                     /* delete host htab    */
void     del_ulist(UNODEPTR *);                     /* delete url htab     */
void     del_rlist(RNODEPTR *);                     /* delete refer htab   */
void     del_alist(ANODEPTR *);                     /* delete u-agent htab */
void     del_slist(SNODEPTR *);                     /* delete search htab  */

NLISTPTR new_nlist(char *);                         /* new hide list node  */
int      add_nlist(char *, NLISTPTR *);             /* add hide list item  */
void     del_nlist(NLISTPTR *);                     /* del hide list       */

GLISTPTR new_glist(char *, char *);                 /* new group list node */
int      add_glist(char *, GLISTPTR *);             /* add group list item */
void     del_glist(GLISTPTR *);                     /* del group list      */

char    *unescape(char *);                          /* unescape URL's      */
char    from_hex(char);                             /* convert hex to dec  */
void    print_opts(char *);                         /* print options       */
void    print_version();                            /* duhh...             */
void    get_history();                              /* load history file   */
void    put_history();                              /* save history file   */
void    fmt_logrec();                               /* format log record   */
int     parse_record();                             /* log record parser   */
int     parse_record_web();                         /* web log handler     */
int     parse_record_ftp();                         /* xferlog handler     */
int     write_main_index();                         /* produce main HTML   */
int     write_month_html();                         /* monthy HTML page    */
void    write_html_head(char *);                    /* head of html page   */
void    write_html_tail();                          /* tail of html page   */
char    *cur_time();                                /* return current time */
u_long  hash(char *);                               /* hash function       */
int     isurlchar(char);                            /* valid URL char fnc. */
int     isinstr(char *, char *);                    /* isinstr function    */
char    *isinlist(NLISTPTR, char *);                /* isinlist function   */
char    *isinglist(GLISTPTR, char *);                /* isinlist function   */
int     save_state();                               /* save run state      */
int     restore_state();                            /* restore run state   */
void    get_config(char *);                         /* read configuration  */
void    init_counters();                            /* initalize stuff     */
void    clear_month();                              /* clear month totals  */
void    month_links();                              /* Page links          */
void    month_total_table();                        /* monthly total table */
void    daily_total_table();                        /* daily total table   */
void    hourly_total_table();                       /* hourly total table  */
void    top_sites_table(int);                       /* top n sites table   */
void    top_urls_table(int);                        /* top n URL's table   */
void    top_entry_table(int);                       /* top n entry/exits   */
void    top_refs_table();                           /* top n referrers ""  */
void    top_agents_table();                         /* top n u-agents  ""  */
void    top_ctry_table();                           /* top n countries ""  */
void    top_search_table();                         /* top n search strs   */
static  char *save_opt(char *);                     /* save conf option    */
u_long  ctry_idx(char *);                           /* index domain name   */
u_long  tot_visit(HNODEPTR *);                      /* calc total visits   */
int     ispage(char *);                             /* check for HTML type */
void    update_entry(char *);                       /* update entry total  */
void    update_exit(char *);                        /* update exit total   */
void    month_update_exit(u_long);                  /* eom exit update     */
void    srch_string(char *);                        /* srch str analysis   */
