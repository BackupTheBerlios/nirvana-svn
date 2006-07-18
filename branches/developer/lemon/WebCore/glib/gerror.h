#ifndef G_ERROR_H
#define G_ERROR_H

#include "gmacros.h"
#include "glib.h"

G_BEGIN_DECLS

typedef struct _GError GError;
struct _GError
{
  GQuark       domain;
  gint         code;
  gchar       *message;
};

GError*  g_error_new           (GQuark         domain,
                                gint           code,
                                const gchar   *format);
GError*  g_error_new_literal   (GQuark         domain,
                                gint           code,
                                const gchar   *message);
void     g_error_free          (GError        *error);
GError*  g_error_copy          (const GError  *error);
gboolean g_error_matches       (const GError  *error,
                                GQuark         domain,
                                gint           code);
void     g_set_error           (GError       **err,
                                GQuark         domain,
                                gint           code,
                                const gchar   *format);
void     g_propagate_error     (GError       **dest,
				GError        *src);
void     g_clear_error         (GError       **err);
void report_g_conv_error(const char* file,int line, GError* err);

G_END_DECLS

#endif /* G_ERROR_H */
