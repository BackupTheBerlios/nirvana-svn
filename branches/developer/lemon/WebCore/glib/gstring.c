
#include "gtypes.h"
#include "gstring.h"
#include <string.h>
#include <malloc.h>

gboolean
g_str_equal (gconstpointer v1, gconstpointer v2)
{
    const gchar *string1 = v1;
    const gchar *string2 = v2;
    return strcmp (string1, string2) == 0;
} 
		   
guint
g_str_hash (gconstpointer v)
{
    const signed char *p = v;
    guint32 h = *p;
    if (h) for (p += 1; *p != '\0'; p++) h = (h << 5) - h + *p;
    return h;
}

gchar*
g_strdup (const gchar *str)
{
  gchar *new_str;
  gsize length;

  if (str)
    {
      length = strlen (str) + 1;
      new_str = malloc(sizeof(char)*length);//g_new (char, length);
      memcpy (new_str, str, length);
    }
  else
    new_str = NULL;

  return new_str;
}

gpointer
g_memdup (gconstpointer mem,
	  guint         byte_size)
{
  gpointer new_mem;

  if (mem)
    {
      new_mem = malloc(byte_size);//g_malloc (byte_size);
      memcpy (new_mem, mem, byte_size);
    }
  else
    new_mem = NULL;

  return new_mem;
}

gchar*
g_strndup (const gchar *str,
	   gsize        n)    
{
  gchar *new_str;

  if (str)
    {
      new_str = malloc(sizeof(gchar)*(n+1));//g_new (gchar, n + 1);
      strncpy (new_str, str, n);
      new_str[n] = '\0';
    }
  else
    new_str = NULL;

  return new_str;
}

gchar *
g_strstr_len (const gchar *haystack,
	      gssize       haystack_len,
	      const gchar *needle)
{
  if (haystack == NULL) return NULL;
  if (needle == NULL) return NULL;
  
  if (haystack_len < 0)
    return strstr (haystack, needle);
  else
    {
      const gchar *p = haystack;
      gsize needle_len = strlen (needle);
      const gchar *end;
      gsize i;

      if (needle_len == 0)
	return (gchar *)haystack;

      if (haystack_len < needle_len)
	return NULL;
      
      end = haystack + haystack_len - needle_len;
      
      while (*p && p <= end)
	{
	  for (i = 0; i < needle_len; i++)
	    if (p[i] != needle[i])
	      goto next;
	  
	  return (gchar *)p;
	  
	next:
	  p++;
	}
      
      return NULL;
    }
}

gchar *
g_strrstr_len (const gchar *haystack,
	       gssize        haystack_len,
	       const gchar *needle)
{
  if (haystack == NULL) return NULL;
  if (needle == NULL) return  NULL;
  
  if (haystack_len < 0)
    return g_strrstr (haystack, needle);
  else
    {
      gsize needle_len = strlen (needle);
      const gchar *haystack_max = haystack + haystack_len;
      const gchar *p = haystack;
      gsize i;

      while (p < haystack_max && *p)
	p++;

      if (p < haystack + needle_len)
	return NULL;
	
      p -= needle_len;

      while (p >= haystack)
	{
	  for (i = 0; i < needle_len; i++)
	    if (p[i] != needle[i])
	      goto next;
	  
	  return (gchar *)p;
	  
	next:
	  p--;
	}

      return NULL;
    }
}

gchar *
g_strrstr (const gchar *haystack,
	   const gchar *needle)
{
  gsize i;
  gsize needle_len;
  gsize haystack_len;
  const gchar *p;
      
  if (haystack == NULL) return NULL;
  if (needle == NULL) return NULL;

  needle_len = strlen (needle);
  haystack_len = strlen (haystack);

  if (needle_len == 0)
    return (gchar *)haystack;

  if (haystack_len < needle_len)
    return NULL;
  
  p = haystack + haystack_len - needle_len;

  while (p >= haystack)
    {
      for (i = 0; i < needle_len; i++)
	if (p[i] != needle[i])
	  goto next;
      
      return (gchar *)p;
      
    next:
      p--;
    }
  
  return NULL;
}
