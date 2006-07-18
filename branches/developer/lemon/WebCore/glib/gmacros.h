#ifndef G_MACROS_H
#define G_MACROS_H

#define G_GNUC_MALLOC
#define G_GNUC_CONST

/* Guard C code in headers, while including them from C++ */
#ifdef  __cplusplus
# define G_BEGIN_DECLS  extern "C" {
# define G_END_DECLS    }
#else
# define G_BEGIN_DECLS
# define G_END_DECLS
#endif

#define	FALSE	(0)
#define	TRUE	(!FALSE)

//#define GPOINTER_TO_SIZE(p)	((gsize) (p))
//#define GSIZE_TO_POINTER(s)	((gpointer) (gsize) (s))

#define G_STMT_START do
#define G_STMT_END while (0)

#define G_CONST_RETURN const

#endif 
