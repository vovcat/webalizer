#ifndef _LANG_H
#define _LANG_H

#include <libintl.h>
#include <locale.h>

#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
#define Q_(String) intl_strip_context (String)

extern char *h_msg[];

extern char *s_month[12];
extern char *l_month[12];

extern char *report_title;
extern struct response_code response[];
extern struct country_code ctry[];

#endif  /* _LANG_H */
