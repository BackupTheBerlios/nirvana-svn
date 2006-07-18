#ifndef G_CONVERT_H
#define G_CONVERT_H

#include <stddef.h>      /* For size_t */
#include <iconv.h>
#include "glib.h"

G_BEGIN_DECLS

typedef enum {
  G_CONVERT_ERROR_NO_CONVERSION,
  G_CONVERT_ERROR_ILLEGAL_SEQUENCE,
  G_CONVERT_ERROR_FAILED,
  G_CONVERT_ERROR_PARTIAL_INPUT,
  G_CONVERT_ERROR_BAD_URI,
  G_CONVERT_ERROR_NOT_ABSOLUTE_PATH
} GConvertError;

typedef iconv_t GIConv;

GIConv g_iconv_open   (const gchar  *to_codeset, const gchar  *from_codeset);
size_t g_iconv        (GIConv        converter,
		       gchar       **inbuf,
		       gsize        *inbytes_left,
		       gchar       **outbuf,
		       gsize        *outbytes_left);
gint   g_iconv_close  (GIConv        converter);
gchar* g_convert               (const gchar  *str,
				gssize        len,            
				const gchar  *to_codeset,
				const gchar  *from_codeset,
				gsize        *bytes_read,     
				gsize        *bytes_written,  
				GError      **error) G_GNUC_MALLOC;
gchar* g_convert_with_iconv    (const gchar  *str,
				gssize        len,
				GIConv        converter,
				gsize        *bytes_read,     
				gsize        *bytes_written,  
				GError      **error) G_GNUC_MALLOC;
gchar* g_convert_with_fallback (const gchar  *str,
				gssize        len,            
				const gchar  *to_codeset,
				const gchar  *from_codeset,
				gchar        *fallback,
				gsize        *bytes_read,     
				gsize        *bytes_written,  
				GError      **error) G_GNUC_MALLOC;

// Convert between libc's idea of strings and UTF-8.
gchar* g_locale_to_utf8   (const gchar  *opsysstring,
			   gssize        len,            
			   gsize        *bytes_read,     
			   gsize        *bytes_written,  
			   GError      **error) G_GNUC_MALLOC;
gchar* g_locale_from_utf8 (const gchar  *utf8string,
			   gssize        len,            
			   gsize        *bytes_read,     
			   gsize        *bytes_written,  
			   GError      **error) G_GNUC_MALLOC;

G_END_DECLS

#endif /* __G_CONVERT_H__ */
