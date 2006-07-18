
#ifndef G_STRING_H
#define G_STRING_H

#include "glib.h"

G_BEGIN_DECLS

gchar *               g_strstr_len     (const gchar  *haystack,
					gssize        haystack_len,
					const gchar  *needle);
gchar *               g_strrstr_len    (const gchar  *haystack,
					gssize        haystack_len,
					const gchar  *needle);
gchar*	              g_strdup	       (const gchar *str) G_GNUC_MALLOC;
gpointer              g_memdup	       (gconstpointer mem,
					guint	       byte_size) G_GNUC_MALLOC;
gchar*                g_stpcpy         (gchar        *dest,
                                        const char   *src);
gchar *               g_strrstr        (const gchar  *haystack,
					const gchar  *needle);
guint		g_str_hash (gconstpointer v) ;

G_END_DECLS

#endif
