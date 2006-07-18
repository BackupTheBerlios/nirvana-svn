#include <iconv.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "glib.h"
#include "gmacros.h"

static gboolean
try_conversion (const char *to_codeset,
		const char *from_codeset,
		iconv_t    *cd)
{
  *cd = iconv_open (to_codeset, from_codeset);

  if (*cd == (iconv_t)-1 && errno == EINVAL)
    return FALSE;
  else
    return TRUE;
}

static gboolean
try_to_aliases (const char **to_aliases,
		const char  *from_codeset,
		iconv_t     *cd)
{
  if (to_aliases)
    {
      const char **p = to_aliases;
      while (*p)
	{
	  if (try_conversion (*p, from_codeset, cd))
	    return TRUE;

	  p++;
	}
    }

  return FALSE;
}

extern const char **_g_charset_get_aliases (const char *canonical_name);

/**
 * g_iconv_open:
 * @to_codeset: destination codeset
 * @from_codeset: source codeset
 * 
 * Same as the standard UNIX routine iconv_open(), but
 * may be implemented via libiconv on UNIX flavors that lack
 * a native implementation.
 * 
 * GLib provides g_convert() and g_locale_to_utf8() which are likely
 * more convenient than the raw iconv wrappers.
 * 
 * Return value: a "conversion descriptor", or (GIConv)-1 if
 *  opening the converter failed.
 **/
GIConv
g_iconv_open (const gchar  *to_codeset,
	      const gchar  *from_codeset)
{
  iconv_t cd;
  
  if (!try_conversion (to_codeset, from_codeset, &cd))
    {
      const char **to_aliases = _g_charset_get_aliases (to_codeset);
      const char **from_aliases = _g_charset_get_aliases (from_codeset);

      if (from_aliases)
	{
	  const char **p = from_aliases;
	  while (*p)
	    {
	      if (try_conversion (to_codeset, *p, &cd))
		goto out;

	      if (try_to_aliases (to_aliases, *p, &cd))
		goto out;

	      p++;
	    }
	}

      if (try_to_aliases (to_aliases, from_codeset, &cd))
	goto out;
    }

 out:
  return (cd == (iconv_t)-1) ? (GIConv)-1 : (GIConv)cd;
}

/**
 * g_iconv:
 * @converter: conversion descriptor from g_iconv_open()
 * @inbuf: bytes to convert
 * @inbytes_left: inout parameter, bytes remaining to convert in @inbuf
 * @outbuf: converted output bytes
 * @outbytes_left: inout parameter, bytes available to fill in @outbuf
 * 
 * Same as the standard UNIX routine iconv(), but
 * may be implemented via libiconv on UNIX flavors that lack
 * a native implementation.
 *
 * GLib provides g_convert() and g_locale_to_utf8() which are likely
 * more convenient than the raw iconv wrappers.
 * 
 * Return value: count of non-reversible conversions, or -1 on error
 **/
size_t 
g_iconv (GIConv   converter,
	 gchar  **inbuf,
	 gsize   *inbytes_left,
	 gchar  **outbuf,
	 gsize   *outbytes_left)
{
  iconv_t cd = (iconv_t)converter;

  return iconv (cd, inbuf, inbytes_left, outbuf, outbytes_left);
}

/**
 * g_iconv_close:
 * @converter: a conversion descriptor from g_iconv_open()
 *
 * Same as the standard UNIX routine iconv_close(), but
 * may be implemented via libiconv on UNIX flavors that lack
 * a native implementation. Should be called to clean up
 * the conversion descriptor from g_iconv_open() when
 * you are done converting things.
 *
 * GLib provides g_convert() and g_locale_to_utf8() which are likely
 * more convenient than the raw iconv wrappers.
 * 
 * Return value: -1 on error, 0 on success
 **/
gint
g_iconv_close (GIConv converter)
{
  iconv_t cd = (iconv_t)converter;

  return iconv_close (cd);
}


#ifdef NEED_ICONV_CACHE

#define ICONV_CACHE_SIZE   (16)

struct _iconv_cache_bucket {
  gchar *key;
  guint32 refcount;
  gboolean used;
  GIConv cd;
};

static GList *iconv_cache_list;
static GHashTable *iconv_cache;
static GHashTable *iconv_open_hash;
static guint iconv_cache_size = 0;
G_LOCK_DEFINE_STATIC (iconv_cache_lock);

/* caller *must* hold the iconv_cache_lock */
static void
iconv_cache_init (void)
{
  static gboolean initialized = FALSE;
  
  if (initialized)
    return;
  
  iconv_cache_list = NULL;
  iconv_cache = g_hash_table_new (g_str_hash, g_str_equal);
  iconv_open_hash = g_hash_table_new (g_direct_hash, g_direct_equal);
  
  initialized = TRUE;
}


/**
 * iconv_cache_bucket_new:
 * @key: cache key
 * @cd: iconv descriptor
 *
 * Creates a new cache bucket, inserts it into the cache and
 * increments the cache size.
 *
 * Returns a pointer to the newly allocated cache bucket.
 **/
static struct _iconv_cache_bucket *
iconv_cache_bucket_new (const gchar *key, GIConv cd)
{
  struct _iconv_cache_bucket *bucket;
  
  bucket = g_new (struct _iconv_cache_bucket, 1);
  bucket->key = g_strdup (key);
  bucket->refcount = 1;
  bucket->used = TRUE;
  bucket->cd = cd;
  
  g_hash_table_insert (iconv_cache, bucket->key, bucket);
  
  /* FIXME: if we sorted the list so items with few refcounts were
     first, then we could expire them faster in iconv_cache_expire_unused () */
  iconv_cache_list = g_list_prepend (iconv_cache_list, bucket);
  
  iconv_cache_size++;
  
  return bucket;
}


/**
 * iconv_cache_bucket_expire:
 * @node: cache bucket's node
 * @bucket: cache bucket
 *
 * Expires a single cache bucket @bucket. This should only ever be
 * called on a bucket that currently has no used iconv descriptors
 * open.
 *
 * @node is not a required argument. If @node is not supplied, we
 * search for it ourselves.
 **/
static void
iconv_cache_bucket_expire (GList *node, struct _iconv_cache_bucket *bucket)
{
  g_hash_table_remove (iconv_cache, bucket->key);
  
  if (node == NULL)
    node = g_list_find (iconv_cache_list, bucket);
  
  g_assert (node != NULL);
  
  if (node->prev)
    {
      node->prev->next = node->next;
      if (node->next)
        node->next->prev = node->prev;
    }
  else
    {
      iconv_cache_list = node->next;
      if (node->next)
        node->next->prev = NULL;
    }
  
  g_list_free_1 (node);
  
  g_free (bucket->key);
  g_iconv_close (bucket->cd);
  g_free (bucket);
  
  iconv_cache_size--;
}


/**
 * iconv_cache_expire_unused:
 *
 * Expires as many unused cache buckets as it needs to in order to get
 * the total number of buckets < ICONV_CACHE_SIZE.
 **/
static void
iconv_cache_expire_unused (void)
{
  struct _iconv_cache_bucket *bucket;
  GList *node, *next;
  
  node = iconv_cache_list;
  while (node && iconv_cache_size >= ICONV_CACHE_SIZE)
    {
      next = node->next;
      
      bucket = node->data;
      if (bucket->refcount == 0)
        iconv_cache_bucket_expire (node, bucket);
      
      node = next;
    }
}

static GIConv
open_converter (const gchar *to_codeset,
		const gchar *from_codeset,
		GError     **error)
{
  struct _iconv_cache_bucket *bucket;
  gchar *key;
  GIConv cd;
  
  /* create our key */
  key = g_alloca (strlen (from_codeset) + strlen (to_codeset) + 2);
  _g_sprintf (key, "%s:%s", from_codeset, to_codeset);
  
  G_LOCK (iconv_cache_lock);
  
  /* make sure the cache has been initialized */
  iconv_cache_init ();
  
  bucket = g_hash_table_lookup (iconv_cache, key);
  if (bucket)
    {
      if (bucket->used)
        {
          cd = g_iconv_open (to_codeset, from_codeset);
          if (cd == (GIConv) -1)
            goto error;
        }
      else
        {
	  /* Apparently iconv on Solaris <= 7 segfaults if you pass in
	   * NULL for anything but inbuf; work around that. (NULL outbuf
	   * or NULL *outbuf is allowed by Unix98.)
	   */
	  gsize inbytes_left = 0;
	  gchar *outbuf = NULL;
	  gsize outbytes_left = 0;
		
          cd = bucket->cd;
          bucket->used = TRUE;
          
          /* reset the descriptor */
          g_iconv (cd, NULL, &inbytes_left, &outbuf, &outbytes_left);
        }
      
      bucket->refcount++;
    }
  else
    {
      cd = g_iconv_open (to_codeset, from_codeset);
      if (cd == (GIConv) -1)
        goto error;
      
      iconv_cache_expire_unused ();
      
      bucket = iconv_cache_bucket_new (key, cd);
    }
  
  g_hash_table_insert (iconv_open_hash, cd, bucket->key);
  
  G_UNLOCK (iconv_cache_lock);
  
  return cd;
  
 error:
  
  G_UNLOCK (iconv_cache_lock);
  
  /* Something went wrong.  */
  if (error)
    {
      if (errno == EINVAL)
	g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_NO_CONVERSION,
		     _("Conversion from character set '%s' to '%s' is not supported"),
		     from_codeset, to_codeset);
      else
	g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_FAILED,
		     _("Could not open converter from '%s' to '%s'"),
		     from_codeset, to_codeset);
    }
  
  return cd;
}

static int
close_converter (GIConv converter)
{
  struct _iconv_cache_bucket *bucket;
  const gchar *key;
  GIConv cd;
  
  cd = converter;
  
  if (cd == (GIConv) -1)
    return 0;
  
  G_LOCK (iconv_cache_lock);
  
  key = g_hash_table_lookup (iconv_open_hash, cd);
  if (key)
    {
      g_hash_table_remove (iconv_open_hash, cd);
      
      bucket = g_hash_table_lookup (iconv_cache, key);
      g_assert (bucket);
      
      bucket->refcount--;
      
      if (cd == bucket->cd)
        bucket->used = FALSE;
      else
        g_iconv_close (cd);
      
      if (!bucket->refcount && iconv_cache_size > ICONV_CACHE_SIZE)
        {
          /* expire this cache bucket */
          iconv_cache_bucket_expire (NULL, bucket);
        }
    }
  else
    {
      G_UNLOCK (iconv_cache_lock);
      
      g_warning ("This iconv context wasn't opened using open_converter");
      
      return g_iconv_close (converter);
    }
  
  G_UNLOCK (iconv_cache_lock);
  
  return 0;
}

#else  /* !NEED_ICONV_CACHE */

static GIConv
open_converter (const gchar *to_codeset,
		const gchar *from_codeset,
		GError     **error)
{
  GIConv cd;
  char *string;
  cd = g_iconv_open (to_codeset, from_codeset);

  if (cd == (GIConv) -1)
    {
      /* Something went wrong.  */
      if (error)
	{
	  if (errno == EINVAL) {
	    sprintf(string,"Conversion from character set '%s' to '%s' is not supported", from_codeset, to_codeset);
	    g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_NO_CONVERSION, string);
	  }
	  else {
	    sprintf(string,"Could not open converter from '%s' to '%s'", from_codeset, to_codeset);
	    g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_FAILED, string);
	  }
	  g_free(string);
	}
    }
  
  return cd;
}

static int
close_converter (GIConv cd)
{
  if (cd == (GIConv) -1)
    return 0;
  
  return g_iconv_close (cd);  
}

#endif /* NEED_ICONV_CACHE */

/**
 * g_convert_with_iconv:
 * @str:           the string to convert
 * @len:           the length of the string, or -1 if the string is 
 *                 nul-terminated<footnoteref linkend="nul-unsafe"/>. 
 * @converter:     conversion descriptor from g_iconv_open()
 * @bytes_read:    location to store the number of bytes in the
 *                 input string that were successfully converted, or %NULL.
 *                 Even if the conversion was successful, this may be 
 *                 less than @len if there were partial characters
 *                 at the end of the input. If the error
 *                 #G_CONVERT_ERROR_ILLEGAL_SEQUENCE occurs, the value
 *                 stored will the byte offset after the last valid
 *                 input sequence.
 * @bytes_written: the number of bytes stored in the output buffer (not 
 *                 including the terminating nul).
 * @error:         location to store the error occuring, or %NULL to ignore
 *                 errors. Any of the errors in #GConvertError may occur.
 *
 * Converts a string from one character set to another. 
 * 
 * Note that you should use g_iconv() for streaming 
 * conversions<footnote id="streaming-state">
 *  <para>
 * Despite the fact that @byes_read can return information about partial 
 * characters, the <literal>g_convert_...</literal> functions
 * are not generally suitable for streaming. If the underlying converter 
 * being used maintains internal state, then this won't be preserved 
 * across successive calls to g_convert(), g_convert_with_iconv() or 
 * g_convert_with_fallback(). (An example of this is the GNU C converter 
 * for CP1255 which does not emit a base character until it knows that 
 * the next character is not a mark that could combine with the base 
 * character.)
 *  </para>
 * </footnote>. 
 *
 * Return value: If the conversion was successful, a newly allocated
 *               nul-terminated string, which must be freed with
 *               g_free(). Otherwise %NULL and @error will be set.
 **/
gchar*
g_convert_with_iconv (const gchar *str,
		      gssize       len,
		      GIConv       converter,
		      gsize       *bytes_read, 
		      gsize       *bytes_written, 
		      GError     **error)
{
  gchar *dest;
  gchar *outp;
  const gchar *p;
  const gchar *shift_p = NULL;
  gsize inbytes_remaining;
  gsize outbytes_remaining;
  gsize err;
  gsize outbuf_size;
  gboolean have_error = FALSE;
  gboolean done = FALSE;
  
  char *err_string;
  
  if (converter == (GIConv) -1) return NULL;
     
  if (len < 0)
    len = strlen (str);

  p = str;
  inbytes_remaining = len;
  outbuf_size = len + 1; /* + 1 for nul in case len == 1 */
  
  outbytes_remaining = outbuf_size - 1; /* -1 for nul */
  outp = dest = /*g_*/malloc (outbuf_size);

  while (!done && !have_error)
    {
      err = g_iconv (converter, (char **)&p, &inbytes_remaining, &outp, &outbytes_remaining);

      if (err == (size_t) -1)
	{
	  switch (errno)
	    {
	    case EINVAL:
	      /* Incomplete text, do not report an error */
	      done = TRUE;
	      break;
	    case E2BIG:
	      {
		size_t used = outp - dest;
		
		outbuf_size *= 2;
		dest = /*g_*/ realloc (dest, outbuf_size);
		
		outp = dest + used;
		outbytes_remaining = outbuf_size - used - 1; /* -1 for nul */
	      }
	      break;
	    case EILSEQ:
	      if (error) {
		g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE,
			     ("Invalid byte sequence in conversion input"));
		}
	      have_error = TRUE;
	      break;
	    default:
	      if (error) {
	        sprintf(err_string, "Error during conversion (EERROR): %i", errno);
		g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_FAILED, err_string);
		g_free(err_string);
	      }
	      have_error = TRUE;
	      break;
	    }
	}
      else 
	{
	  if (!shift_p)
	    {
	      /* call g_iconv with NULL inbuf to cleanup shift state */
	      shift_p = p;
	      p = NULL;
	      inbytes_remaining = 0;
	    }
	  else
	    done = TRUE;
	}
    }

  if (shift_p)
    p = shift_p;

  *outp = '\0';
  
  if (bytes_read)
    *bytes_read = p - str;
  else
    {
      if ((p - str) != len) 
	{
          if (!have_error)
            {
	      if (error)
		g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_PARTIAL_INPUT,
			     ("Partial character sequence at end of input"));
              have_error = TRUE;
            }
	}
    }

  if (bytes_written)
    *bytes_written = outp - dest;	/* Doesn't include '\0' */

  if (have_error)
    {
      g_free (dest);
      return NULL;
    }
  else
    return dest;
}

/**
 * g_convert:
 * @str:           the string to convert
 * @len:           the length of the string, or -1 if the string is 
 *                 nul-terminated<footnote id="nul-unsafe">
                     <para>
                       Note that some encodings may allow nul bytes to 
                       occur inside strings. In that case, using -1 for 
                       the @len parameter is unsafe.
                     </para>
                   </footnote>. 
 * @to_codeset:    name of character set into which to convert @str
 * @from_codeset:  character set of @str.
 * @bytes_read:    location to store the number of bytes in the
 *                 input string that were successfully converted, or %NULL.
 *                 Even if the conversion was successful, this may be 
 *                 less than @len if there were partial characters
 *                 at the end of the input. If the error
 *                 #G_CONVERT_ERROR_ILLEGAL_SEQUENCE occurs, the value
 *                 stored will the byte offset after the last valid
 *                 input sequence.
 * @bytes_written: the number of bytes stored in the output buffer (not 
 *                 including the terminating nul).
 * @error:         location to store the error occuring, or %NULL to ignore
 *                 errors. Any of the errors in #GConvertError may occur.
 *
 * Converts a string from one character set to another.
 *
 * Note that you should use g_iconv() for streaming 
 * conversions<footnoteref linkend="streaming-state"/>.
 *
 * Return value: If the conversion was successful, a newly allocated
 *               nul-terminated string, which must be freed with
 *               g_free(). Otherwise %NULL and @error will be set.
 **/
gchar*
g_convert (const gchar *str,
           gssize       len,  
           const gchar *to_codeset,
           const gchar *from_codeset,
           gsize       *bytes_read, 
	   gsize       *bytes_written, 
	   GError     **error)
{
  gchar *res;
  GIConv cd;

  if  (str == NULL) return NULL;
  if (to_codeset == NULL) return NULL;
  if  (from_codeset == NULL) return NULL;
  
  cd = open_converter (to_codeset, from_codeset, error);

  if (cd == (GIConv) -1)
    {
      if (bytes_read)
        *bytes_read = 0;
      
      if (bytes_written)
        *bytes_written = 0;
      
      return NULL;
    }

  res = g_convert_with_iconv (str, len, cd,
			      bytes_read, bytes_written,
			      error);

  close_converter (cd);

  return res;
}

/**
 * g_convert_with_fallback:
 * @str:          the string to convert
 * @len:          the length of the string, or -1 if the string is 
 *                nul-terminated<footnoteref linkend="nul-unsafe"/>. 
 * @to_codeset:   name of character set into which to convert @str
 * @from_codeset: character set of @str.
 * @fallback:     UTF-8 string to use in place of character not
 *                present in the target encoding. (The string must be
 *                representable in the target encoding). 
                  If %NULL, characters not in the target encoding will 
                  be represented as Unicode escapes \uxxxx or \Uxxxxyyyy.
 * @bytes_read:   location to store the number of bytes in the
 *                input string that were successfully converted, or %NULL.
 *                Even if the conversion was successful, this may be 
 *                less than @len if there were partial characters
 *                at the end of the input.
 * @bytes_written: the number of bytes stored in the output buffer (not 
 *                including the terminating nul).
 * @error:        location to store the error occuring, or %NULL to ignore
 *                errors. Any of the errors in #GConvertError may occur.
 *
 * Converts a string from one character set to another, possibly
 * including fallback sequences for characters not representable
 * in the output. Note that it is not guaranteed that the specification
 * for the fallback sequences in @fallback will be honored. Some
 * systems may do a approximate conversion from @from_codeset
 * to @to_codeset in their iconv() functions, 
 * in which case GLib will simply return that approximate conversion.
 *
 * Note that you should use g_iconv() for streaming 
 * conversions<footnoteref linkend="streaming-state"/>.
 *
 * Return value: If the conversion was successful, a newly allocated
 *               nul-terminated string, which must be freed with
 *               g_free(). Otherwise %NULL and @error will be set.
 **/
gchar*
g_convert_with_fallback (const gchar *str,
			 gssize       len,    
			 const gchar *to_codeset,
			 const gchar *from_codeset,
			 gchar       *fallback,
			 gsize       *bytes_read,
			 gsize       *bytes_written,
			 GError     **error)
{
  gchar *utf8;
  gchar *dest;
  gchar *outp;
  const gchar *insert_str = NULL;
  const gchar *p;
  gsize inbytes_remaining;   
  const gchar *save_p = NULL;
  gsize save_inbytes = 0;
  gsize outbytes_remaining; 
  gsize err;
  GIConv cd;
  gsize outbuf_size;
  gboolean have_error = FALSE;
  gboolean done = FALSE;
  char *err_string;

  GError *local_error = NULL;
  
  if (str == NULL) return NULL;
  if (to_codeset == NULL) return NULL;
  if (from_codeset == NULL) return NULL;
     
  if (len < 0)
    len = strlen (str);
  
  /* Try an exact conversion; we only proceed if this fails
   * due to an illegal sequence in the input string.
   */
  dest = g_convert (str, len, to_codeset, from_codeset, 
		    bytes_read, bytes_written, &local_error);
  if (!local_error)
    return dest;

  if (!g_error_matches (local_error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE))
    {
      g_propagate_error (error, local_error);
      return NULL;
    }
  else
    g_error_free (local_error);

  local_error = NULL;
  
  /* No go; to proceed, we need a converter from "UTF-8" to
   * to_codeset, and the string as UTF-8.
   */
  cd = open_converter (to_codeset, "UTF-8", error);
  if (cd == (GIConv) -1)
    {
      if (bytes_read)
        *bytes_read = 0;
      
      if (bytes_written)
        *bytes_written = 0;
      
      return NULL;
    }

  utf8 = g_convert (str, len, "UTF-8", from_codeset, 
		    bytes_read, &inbytes_remaining, error);
  if (!utf8)
    {
      close_converter (cd);
      if (bytes_written)
        *bytes_written = 0;
      return NULL;
    }

  /* Now the heart of the code. We loop through the UTF-8 string, and
   * whenever we hit an offending character, we form fallback, convert
   * the fallback to the target codeset, and then go back to
   * converting the original string after finishing with the fallback.
   *
   * The variables save_p and save_inbytes store the input state
   * for the original string while we are converting the fallback
   */
  p = utf8;

  outbuf_size = len + 1; /* + 1 for nul in case len == 1 */
  outbytes_remaining = outbuf_size - 1; /* -1 for nul */
  outp = dest = /*g_*/malloc (outbuf_size);

  while (!done && !have_error)
    {
      size_t inbytes_tmp = inbytes_remaining;
      err = g_iconv (cd, (char **)&p, &inbytes_tmp, &outp, &outbytes_remaining);
      inbytes_remaining = inbytes_tmp;

      if (err == (size_t) -1)
	{
	  switch (errno)
	    {
	    case EINVAL:
	      g_assert_not_reached();
	      break;
	    case E2BIG:
	      {
		size_t used = outp - dest;

		outbuf_size *= 2;
		dest = /*g_*/realloc (dest, outbuf_size);
		
		outp = dest + used;
		outbytes_remaining = outbuf_size - used - 1; /* -1 for nul */
		
		break;
	      }
	    case EILSEQ:
	      if (save_p)
		{
		  /* Error converting fallback string - fatal
		   */
		   sprintf(err_string, "Cannot convert fallback '%s' to codeset '%s'", insert_str, to_codeset);
		  g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE, err_string);
		  g_free(err_string);
		  have_error = TRUE;
		  break;
		}
	      else if (p)
		{
		  if (!fallback)
		    { 
		      gunichar ch = g_utf8_get_char (p);
		      sprintf(insert_str, ch < 0x10000 ? "\\u%04x" : "\\U%08x", ch);
		      //insert_str = g_strdup_printf (ch < 0x10000 ? "\\u%04x" : "\\U%08x", ch);
		    }
		  else
		    insert_str = fallback;
		  
		  save_p = g_utf8_next_char (p);
		  save_inbytes = inbytes_remaining - (save_p - p);
		  p = insert_str;
		  inbytes_remaining = strlen (p);
		  break;
		}
	      /* fall thru if p is NULL */
	    default:
	      sprintf(err_string, "Error during conversion (EERROR): %i", errno);
	      g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_FAILED, err_string);
	      g_free(err_string);
	      have_error = TRUE;
	      break;
	    }
	}
      else
	{
	  if (save_p)
	    {
	      if (!fallback)
		g_free ((gchar *)insert_str);
	      p = save_p;
	      inbytes_remaining = save_inbytes;
	      save_p = NULL;
	    }
	  else if (p)
	    {
	      /* call g_iconv with NULL inbuf to cleanup shift state */
	      p = NULL;
	      inbytes_remaining = 0;
	    }
	  else
	    done = TRUE;
	}
    }

  /* Cleanup
   */
  *outp = '\0';
  
  close_converter (cd);

  if (bytes_written)
    *bytes_written = outp - dest;	/* Doesn't include '\0' */

  g_free (utf8);

  if (have_error)
    {
      if (save_p && !fallback)
	g_free ((gchar *)insert_str);
      g_free (dest);
      return NULL;
    }
  else
    return dest;
}

/*
 * g_locale_to_utf8
 *
 * 
 */

static gchar *
strdup_len (const gchar *string,
	    gssize       len,
	    gsize       *bytes_written,
	    gsize       *bytes_read,
	    GError      **error)
	 
{
  gsize real_len;

  if (!g_utf8_validate (string, len, NULL))
    {
      if (bytes_read)
	*bytes_read = 0;
      if (bytes_written)
	*bytes_written = 0;

      g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_ILLEGAL_SEQUENCE,
		   ("Invalid byte sequence in conversion input"));
      return NULL;
    }
  
  if (len < 0)
    real_len = strlen (string);
  else
    {
      real_len = 0;
      
      while (real_len < len && string[real_len])
	real_len++;
    }
  
  if (bytes_read)
    *bytes_read = real_len;
  if (bytes_written)
    *bytes_written = real_len;

  return g_strndup (string, real_len);
}

/**
 * g_locale_to_utf8:
 * @opsysstring:   a string in the encoding of the current locale. On Windows
 *                 this means the system codepage.
 * @len:           the length of the string, or -1 if the string is
 *                 nul-terminated<footnoteref linkend="nul-unsafe"/>. 
 * @bytes_read:    location to store the number of bytes in the
 *                 input string that were successfully converted, or %NULL.
 *                 Even if the conversion was successful, this may be 
 *                 less than @len if there were partial characters
 *                 at the end of the input. If the error
 *                 #G_CONVERT_ERROR_ILLEGAL_SEQUENCE occurs, the value
 *                 stored will the byte offset after the last valid
 *                 input sequence.
 * @bytes_written: the number of bytes stored in the output buffer (not 
 *                 including the terminating nul).
 * @error:         location to store the error occuring, or %NULL to ignore
 *                 errors. Any of the errors in #GConvertError may occur.
 * 
 * Converts a string which is in the encoding used for strings by
 * the C runtime (usually the same as that used by the operating
 * system) in the current locale into a UTF-8 string.
 * 
 * Return value: The converted string, or %NULL on an error.
 **/
gchar *
g_locale_to_utf8 (const gchar  *opsysstring,
		  gssize        len,            
		  gsize        *bytes_read,    
		  gsize        *bytes_written,
		  GError      **error)
{
  const char *charset;

  if (g_get_charset (&charset))
    return strdup_len (opsysstring, len, bytes_read, bytes_written, error);
  else
    return g_convert (opsysstring, len, 
		      "UTF-8", charset, bytes_read, bytes_written, error);
}

/**
 * g_locale_from_utf8:
 * @utf8string:    a UTF-8 encoded string 
 * @len:           the length of the string, or -1 if the string is
 *                 nul-terminated<footnoteref linkend="nul-unsafe"/>. 
 * @bytes_read:    location to store the number of bytes in the
 *                 input string that were successfully converted, or %NULL.
 *                 Even if the conversion was successful, this may be 
 *                 less than @len if there were partial characters
 *                 at the end of the input. If the error
 *                 #G_CONVERT_ERROR_ILLEGAL_SEQUENCE occurs, the value
 *                 stored will the byte offset after the last valid
 *                 input sequence.
 * @bytes_written: the number of bytes stored in the output buffer (not 
 *                 including the terminating nul).
 * @error:         location to store the error occuring, or %NULL to ignore
 *                 errors. Any of the errors in #GConvertError may occur.
 * 
 * Converts a string from UTF-8 to the encoding used for strings by
 * the C runtime (usually the same as that used by the operating
 * system) in the current locale.
 * 
 * Return value: The converted string, or %NULL on an error.
 **/
gchar *
g_locale_from_utf8 (const gchar *utf8string,
		    gssize       len,            
		    gsize       *bytes_read,    
		    gsize       *bytes_written,
		    GError     **error)
{
  const gchar *charset;

  if (g_get_charset (&charset))
    return strdup_len (utf8string, len, bytes_read, bytes_written, error);
  else
    return g_convert (utf8string, len,
		      charset, "UTF-8", bytes_read, bytes_written, error);
}

#define G_CONVERT_C
