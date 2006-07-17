#ifndef LIBCHARSET_H
#define LIBCHARSET_H

#ifdef __cplusplus
extern "C" {
#endif

/* Determine the current locale's character encoding, and canonicalize it
   into one of the canonical names listed in config.charset.
   The result must not be freed; it is statically allocated.
   If the canonical name cannot be determined, the result is a non-canonical
   name.  */
extern const char * _g_locale_charset_raw (void);
extern const char * _g_locale_charset_unalias (const char *codeset);
extern const char * _g_locale_get_charset_aliases (void);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCHARSET_H */
