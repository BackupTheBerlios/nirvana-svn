#ifndef G_LIB_H
#define G_LIB_H

#include <malloc.h>

#include "gtypes.h"
#include "gerror.h"
#include "gstring.h"
#include "ghash.h"
#include "gunicode.h"
#include "libcharset.h"

typedef enum {
    G_CONVERT_ERROR_NO_CONVERSION,
    G_CONVERT_ERROR_ILLEGAL_SEQUENCE,
    G_CONVERT_ERROR_FAILED,
    G_CONVERT_ERROR_PARTIAL_INPUT,
    G_CONVERT_ERROR_BAD_URI,
    G_CONVERT_ERROR_NOT_ABSOLUTE_PATH
} GConvertError;

struct _GStaticPrivate { guint index; };
typedef struct _GStaticPrivate GStaticPrivate;

#define G_CONVERT_ERROR 1000
#define G_STATIC_PRIVATE_INIT { 0 } 
#define GINT_TO_POINTER(i)	((gpointer)  (i))
#define GPOINTER_TO_INT(p)	((gint)   (p))
#define GUINT_TO_POINTER(u)	((gpointer)  (u))
#define GPOINTER_TO_UINT(p)	((guint)  (p))

#define g_new(struct_type, n_structs) malloc (((gsize) sizeof (struct_type)) * ((gsize) (n_structs))))
#define g_new(struct_type, n_structs) malloc (((gsize) sizeof (struct_type)) * ((gsize) (n_structs))))
#define g_free(ptr) free((void *)(ptr))

/*
#ifdef  __cplusplus
# define G_BEGIN_DECLS  extern "C" {
# define G_END_DECLS    }
#else
# define G_BEGIN_DECLS
# define G_END_DECLS
#endif

#define G_GNUC_CONST const
#define G_GNUC_CONST_RETURN const
#define G_GNUC_MALLOC
#define G_STMT_START do
#define G_STMT_END while (0)
#define FALSE   (0)
#define TRUE    (!FALSE)
*/


#endif 
