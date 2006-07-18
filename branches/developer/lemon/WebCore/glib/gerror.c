#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "glib.h"

GError*
g_error_new_literal (GQuark         domain,
                     gint           code,
                     const gchar   *message)
{
  GError* err;
  if (message == NULL) return NULL;
  if (domain == 0) return NULL;
  err = malloc (sizeof(GError));
  err->domain = domain;
  err->code = code;
  err->message = g_strdup (message);
  return err;
}

void
g_error_free (GError *error)
{
  if (error == NULL) return;
  g_free (error->message);
  g_free (error);
}

GError*
g_error_copy (const GError *error)
{
  GError *copy;
  if (error == NULL) return NULL;
  copy = malloc(sizeof(GError));
  *copy = *error;
  copy->message = g_strdup (error->message);
  return copy;
}

gboolean
g_error_matches (const GError *error,
                 GQuark        domain,
                 gint          code)
{
  return error &&
    error->domain == domain &&
    error->code == code;
}

void
g_set_error (GError      **err,
             GQuark        domain,
             gint          code,
             const gchar  *format)
{
    GError *new;
    if (err == NULL) return;
    new = g_error_new_literal (domain, code, format);
    if (*err == NULL) *err = new;
    else {
	g_error_free (*err);
	*err = new;
	//g_warning (ERROR_OVERWRITTEN_WARNING, new->message);
    }
}

void    
g_propagate_error (GError       **dest,
		   GError        *src)
{
    
  if (src == NULL) return;
  if (dest == NULL)
    {
      if (src)
        g_error_free (src);
      return;
    }
  else
    {
      if (*dest != NULL) {
        g_error_free(*dest);
        *dest = src;
	// g_warning (ERROR_OVERWRITTEN_WARNING, src->message);
      }
      else
        *dest = src;
    }
}

void
g_clear_error (GError **err)
{
  if (err && *err)
    {
      g_error_free (*err);
      *err = NULL;
    }
}

void report_g_conv_error(const char* file,int line, GError* err)
{
    if (err != NULL){
	char* errstr="";
	switch(err->code){
	case G_CONVERT_ERROR_NO_CONVERSION:
	    errstr="no conversion";break;
	case G_CONVERT_ERROR_ILLEGAL_SEQUENCE:
	    errstr="illegal seqence";break;
	case G_CONVERT_ERROR_FAILED:
	    errstr="failed";break;
	case  G_CONVERT_ERROR_PARTIAL_INPUT:
	    errstr="paritial input";break;
	case G_CONVERT_ERROR_BAD_URI:
	    errstr="bad uri";break;
	case G_CONVERT_ERROR_NOT_ABSOLUTE_PATH:
	    errstr="not absolute path";break;
	default:	   
 	    errstr="unknown error";
	    break;
	}
	fprintf(stderr, "%s:%d: Text conversion failed: %s, (code %d)\n",file,line, errstr,err->code);
	fprintf(stderr, "\t error message was: %s",err->message);
	fflush(stderr);
    }
}
