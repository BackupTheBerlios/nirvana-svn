#ifndef G_LIB_H
#define G_LIB_H

#include <malloc.h>
#include "gunicode.h"
#include "gtypes.h"
#include "gerror.h"

#define GINT_TO_POINTER(i)	((gpointer)  (i))
#define GPOINTER_TO_INT(p)	((gint)   (p))
#define GUINT_TO_POINTER(u)	((gpointer)  (u))
#define GPOINTER_TO_UINT(p)	((guint)  (p))

#define G_CONVERT_ERROR 1000
typedef enum
{
    G_CONVERT_ERROR_NO_CONVERSION,
    G_CONVERT_ERROR_ILLEGAL_SEQUENCE,
    G_CONVERT_ERROR_FAILED,
    G_CONVERT_ERROR_PARTIAL_INPUT,
    G_CONVERT_ERROR_BAD_URI,
    G_CONVERT_ERROR_NOT_ABSOLUTE_PATH
} GConvertError;

struct _GStaticPrivate
{
  /*< private >*/
    guint index;
};

typedef struct _GStaticPrivate GStaticPrivate;

#define G_STATIC_PRIVATE_INIT { 0 } 

#define g_new(struct_type, n_structs) malloc (((gsize) sizeof (struct_type)) * ((gsize) (n_structs))))
#define g_new(struct_type, n_structs) malloc (((gsize) sizeof (struct_type)) * ((gsize) (n_structs))))
#define g_free(ptr) free((void *)(ptr))

#endif /* __G_LIB_H__ */
