/* webalizer.h */

#define PCENT(val,max) ((val)?((double)val/(double)max)*100.0 : 0.0)

#define BUFSIZE  4096                  /* Max buffer size for log record   */
#define MAXHOST  64                    /* Max hostname buffer size         */
#define MAXURL   128                   /* Max HTTP request/URL field size  */
#define MAXREF   1024                  /* Max referrer field size          */
#define MAXREFH  256                   /* Max referrer field size in htab  */
#define MAXAGENT 64                    /* Max user agent field size        */
#define MAXCTRY  64                    /* Max country name size            */
#define MAXHASH  2048                  /* Size of our hash tables          */

/* define structure to hold expanded log record */

struct  log_struct  {  char   hostname[MAXHOST];
                       char   datetime[29];
                       char   url[MAXURL];
                       int    resp_code;
                       u_long xfer_size;
                       char   refer[MAXREF];
                       char   agent[MAXAGENT]; };

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

/* Response code defines as per RFC2068 (HTTP/1.1) */
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
#define IDX_BAD                16
#define IDX_UNAUTH             17
#define IDX_PAYMENTREQ         18
#define IDX_FORBIDDEN          19
#define IDX_NOTFOUND           20
#define IDX_METHODNOTALLOWED   21
#define IDX_NOTACCEPTABLE      22
#define IDX_PROXYAUTHREQ       23
#define IDX_TIMEOUT            24
#define IDX_CONFLICT           25
#define IDX_GONE               26
#define IDX_LENGTHREQ          27
#define IDX_PREFAILED          28
#define IDX_REQENTTOOLARGE     29
#define IDX_REQURITOOLARGE     30
#define IDX_UNSUPMEDIATYPE     31
#define IDX_SERVERERR          32
#define IDX_NOTIMPLEMENTED     33
#define IDX_BADGATEWAY         34
#define IDX_UNAVAIL            35
#define IDX_GATEWAYTIMEOUT     36
#define IDX_BADHTTPVER         37
#define TOTAL_RC               37

struct response_code {     char    *desc;         /* response code struct  */
                         u_long    count; };

/* node definitions */
typedef struct hnode *HNODEPTR;            /* site node (host) pointer     */
typedef struct unode *UNODEPTR;            /* url node pointer             */
typedef struct rnode *RNODEPTR;            /* referrer node                */
typedef struct anode *ANODEPTR;            /* user agent node pointer      */
typedef struct nlist *NLISTPTR;            /* HIDE list item pointer       */

/* Object flags */
#define OBJ_REG  0                         /* Regular object               */
#define OBJ_HIDE 1                         /* Hidden object                */
#define OBJ_GRP  2                         /* Grouped object               */

struct hnode {  char string[MAXHOST];      /* host hash table structure    */
                 int flag;
              u_long count;
              u_long files;
              u_long xfer;
              struct hnode *next; };

struct unode {  char string[MAXURL];       /* url hash table structure     */
                 int flag;
              u_long count;
              u_long files;
              u_long xfer;
              struct unode *next; };

struct rnode {  char string[MAXREFH];      /* referrer hash table struct   */
                 int flag;
              u_long count;
              struct rnode *next; };

struct anode {  char string[MAXAGENT];     /* user agent hash table struct */
                 int flag;
              u_long count;
              struct anode *next; };

struct nlist {  char string[80];           /* list struct for HIDE items   */
              struct nlist *next; };

/***********************/
/* function prototypes */
/***********************/

HNODEPTR new_hnode(char *);                         /* new host node       */
UNODEPTR new_unode(char *);                         /* new url node        */
RNODEPTR new_rnode(char *);                         /* new referrer node   */
ANODEPTR new_anode(char *);                         /* new user agent node */
/* add/update node routines */
int      put_hnode(char *, int, u_long, u_long, u_long, u_long *, HNODEPTR *);
int      put_unode(char *, int, u_long, u_long, u_long *, UNODEPTR *);
int      put_rnode(char *, int, u_long, u_long *, RNODEPTR *);
int      put_anode(char *, int, u_long, u_long *, ANODEPTR *);
void     del_hlist(HNODEPTR *);                     /* delete host htab    */
void     del_ulist(UNODEPTR *);                     /* delete url htab     */
void     del_rlist(RNODEPTR *);                     /* delete refer htab   */
void     del_alist(ANODEPTR *);                     /* delete u-agent htab */

NLISTPTR new_nlist(char *);                         /* new hide list node  */
int      add_nlist(char *, NLISTPTR *);             /* add hide list item  */
void     del_nlist(NLISTPTR *);                     /* del hide list       */

char    *unescape(char *);                          /* unescape URL's      */
char    from_hex(char);                             /* convert hex to dec  */
void    print_opts(char *);                         /* print options       */
void    print_version();                            /* duhh...             */
void    get_history();                              /* load history file   */
void    put_history();                              /* save history file   */
void    fmt_logrec();                               /* format log record   */
int     parse_record();                             /* log record parser   */
int	write_main_index();                         /* produce main HTML   */
int	write_month_html();                         /* monthy HTML page    */
void	write_html_head(char *);                    /* head of html page   */
void	write_html_tail();                          /* tail of html page   */
char    *cur_time();                                /* return current time */
u_long  hash(char *);                               /* hash function       */
int     isinstr(char *, char *);                    /* isinstr function    */
char    *isinlist(NLISTPTR, char *);                /* isinlist function   */
void	save_state();                               /* save run state      */
void    restore_state();                            /* restore run state   */
void    get_config(char *);                         /* read configuration  */
void    init_counters();                            /* initalize stuff     */
void    clear_month();                              /* clear month totals  */
void    month_links();                              /* Page links          */
void	month_total_table();                        /* monthly total table */
void	daily_total_table();                        /* daily total table   */
void    hourly_total_table();                       /* hourly total table  */
void	top_sites_table();                          /* top n sites table   */
void	top_urls_table();                           /* top n URL's table   */
void    top_refs_table();                           /* top n referrers ""  */
void    top_agents_table();                         /* top n u-agents  ""  */
void	top_ctry_table();                           /* top n countries ""  */
static  char *save_opt(char *);                     /* save conf option    */
u_long  ctry_idx(char *);                           /* index domain name   */
