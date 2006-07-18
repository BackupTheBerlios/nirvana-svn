#ifndef KWIQMacros_h
#define KWIQMacros_h

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef floor
#define floor(a) ((int)(a))
#endif

#ifndef ceil
#define ceil(a) ((int)(a)+0.5)
#endif

#define NATIVE_ENCODING "UTF-16"

#if KWQUBE

#ifdef NDEBUG

#define REPORT_G_CONV_ERROR //

#else

typedef struct _GError GError;
void report_g_conv_error(const char* file,int line, GError* err);
#define REPORT_G_CONV_ERROR(err) report_g_conv_error(__FILE__,__LINE__,err)

#endif //NDEBUG

#endif //KWIQ

#endif

