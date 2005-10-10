#include <libintl.h>
#include <locale.h>

#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

extern char *h_msg[];

extern char *s_month[12];
extern char *l_month[12];

extern struct response_code response[];

extern struct  country_code ctry[];
