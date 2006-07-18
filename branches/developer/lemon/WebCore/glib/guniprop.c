/*
 * Copyright (C) 1999 Tom Tromey
 * Copyright (C) 2000 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

//#include "config.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <locale.h>

#include "glib.h"
#include "gerror.h"
#include "gunichartables.h"
//#include "gmirroringtable.h"
//#include "gunibreak.h"
#include "gtypes.h"
#include "gunicode.h"
//#include "gstring.h"

//#include "gunicodeprivate.h"
//#include "galias.h"

#define ATTR_TABLE(Page) (((Page) <= G_UNICODE_LAST_PAGE_PART1) \
                          ? attr_table_part1[Page] \
                          : attr_table_part2[(Page) - 0xe00])

#define ATTTABLE(Page, Char) \
  ((ATTR_TABLE(Page) == G_UNICODE_MAX_TABLE_INDEX) ? 0 : (attr_data[ATTR_TABLE(Page)][Char]))

#define TTYPE_PART1(Page, Char) \
  ((type_table_part1[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part1[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part1[Page]][Char]))

#define TTYPE_PART2(Page, Char) \
  ((type_table_part2[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part2[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part2[Page]][Char]))

#define TYPE(Char) \
  (((Char) <= G_UNICODE_LAST_CHAR_PART1) \
   ? TTYPE_PART1 ((Char) >> 8, (Char) & 0xff) \
   : (((Char) >= 0xe0000 && (Char) <= G_UNICODE_LAST_CHAR) \
      ? TTYPE_PART2 (((Char) - 0xe0000) >> 8, (Char) & 0xff) \
      : G_UNICODE_UNASSIGNED))


#define IS(Type, Class)	(((guint)1 << (Type)) & (Class))
#define OR(Type, Rest)	(((guint)1 << (Type)) | (Rest))



#define ISDIGIT(Type)	IS ((Type),				\
			    OR (G_UNICODE_DECIMAL_NUMBER,	\
			    OR (G_UNICODE_LETTER_NUMBER,	\
			    OR (G_UNICODE_OTHER_NUMBER,		0))))

#define ISALPHA(Type)	IS ((Type),				\
			    OR (G_UNICODE_LOWERCASE_LETTER,	\
			    OR (G_UNICODE_UPPERCASE_LETTER,	\
			    OR (G_UNICODE_TITLECASE_LETTER,	\
			    OR (G_UNICODE_MODIFIER_LETTER,	\
			    OR (G_UNICODE_OTHER_LETTER,		0))))))

#define ISALDIGIT(Type)	IS ((Type),				\
			    OR (G_UNICODE_DECIMAL_NUMBER,	\
			    OR (G_UNICODE_LETTER_NUMBER,	\
			    OR (G_UNICODE_OTHER_NUMBER,		\
			    OR (G_UNICODE_LOWERCASE_LETTER,	\
			    OR (G_UNICODE_UPPERCASE_LETTER,	\
			    OR (G_UNICODE_TITLECASE_LETTER,	\
			    OR (G_UNICODE_MODIFIER_LETTER,	\
			    OR (G_UNICODE_OTHER_LETTER,		0)))))))))

#define ISMARK(Type)	IS ((Type),				\
			    OR (G_UNICODE_NON_SPACING_MARK,	\
			    OR (G_UNICODE_COMBINING_MARK,	\
			    OR (G_UNICODE_ENCLOSING_MARK,	0))))

/**
 * g_unichar_isalnum:
 * @c: a Unicode character
 * 
 * Determines whether a character is alphanumeric.
 * Given some UTF-8 text, obtain a character value
 * with g_utf8_get_char().
 * 
 * Return value: %TRUE if @c is an alphanumeric character
 **/
gboolean
g_unichar_isalnum (gunichar c)
{
  return ISALDIGIT (TYPE (c)) ? TRUE : FALSE;
}

/**
 * g_unichar_isalpha:
 * @c: a Unicode character
 * 
 * Determines whether a character is alphabetic (i.e. a letter).
 * Given some UTF-8 text, obtain a character value with
 * g_utf8_get_char().
 * 
 * Return value: %TRUE if @c is an alphabetic character
 **/
gboolean
g_unichar_isalpha (gunichar c)
{
  return ISALPHA (TYPE (c)) ? TRUE : FALSE;
}


/**
 * g_unichar_iscntrl:
 * @c: a Unicode character
 * 
 * Determines whether a character is a control character.
 * Given some UTF-8 text, obtain a character value with
 * g_utf8_get_char().
 * 
 * Return value: %TRUE if @c is a control character
 **/
gboolean
g_unichar_iscntrl (gunichar c)
{
  return TYPE (c) == G_UNICODE_CONTROL;
}

/**
 * g_unichar_isdigit:
 * @c: a Unicode character
 * 
 * Determines whether a character is numeric (i.e. a digit).  This
 * covers ASCII 0-9 and also digits in other languages/scripts.  Given
 * some UTF-8 text, obtain a character value with g_utf8_get_char().
 * 
 * Return value: %TRUE if @c is a digit
 **/
gboolean
g_unichar_isdigit (gunichar c)
{
  return TYPE (c) == G_UNICODE_DECIMAL_NUMBER;
}


/**
 * g_unichar_isgraph:
 * @c: a Unicode character
 * 
 * Determines whether a character is printable and not a space
 * (returns %FALSE for control characters, format characters, and
 * spaces). g_unichar_isprint() is similar, but returns %TRUE for
 * spaces. Given some UTF-8 text, obtain a character value with
 * g_utf8_get_char().
 * 
 * Return value: %TRUE if @c is printable unless it's a space
 **/
gboolean
g_unichar_isgraph (gunichar c)
{
  return !IS (TYPE(c),
	      OR (G_UNICODE_CONTROL,
	      OR (G_UNICODE_FORMAT,
	      OR (G_UNICODE_UNASSIGNED,
	      OR (G_UNICODE_PRIVATE_USE,
	      OR (G_UNICODE_SURROGATE,
	      OR (G_UNICODE_SPACE_SEPARATOR,
	     0)))))));
}

/**
 * g_unichar_islower:
 * @c: a Unicode character
 * 
 * Determines whether a character is a lowercase letter.
 * Given some UTF-8 text, obtain a character value with
 * g_utf8_get_char().
 * 
 * Return value: %TRUE if @c is a lowercase letter
 **/
gboolean
g_unichar_islower (gunichar c)
{
  return TYPE (c) == G_UNICODE_LOWERCASE_LETTER;
}


/**
 * g_unichar_isprint:
 * @c: a Unicode character
 * 
 * Determines whether a character is printable.
 * Unlike g_unichar_isgraph(), returns %TRUE for spaces.
 * Given some UTF-8 text, obtain a character value with
 * g_utf8_get_char().
 * 
 * Return value: %TRUE if @c is printable
 **/
gboolean
g_unichar_isprint (gunichar c)
{
  return !IS (TYPE(c),
	      OR (G_UNICODE_CONTROL,
	      OR (G_UNICODE_FORMAT,
	      OR (G_UNICODE_UNASSIGNED,
	      OR (G_UNICODE_PRIVATE_USE,
	      OR (G_UNICODE_SURROGATE,
	     0))))));
}

/**
 * g_unichar_ispunct:
 * @c: a Unicode character
 * 
 * Determines whether a character is punctuation or a symbol.
 * Given some UTF-8 text, obtain a character value with
 * g_utf8_get_char().
 * 
 * Return value: %TRUE if @c is a punctuation or symbol character
 **/
gboolean
g_unichar_ispunct (gunichar c)
{
  return IS (TYPE(c),
	     OR (G_UNICODE_CONNECT_PUNCTUATION,
	     OR (G_UNICODE_DASH_PUNCTUATION,
	     OR (G_UNICODE_CLOSE_PUNCTUATION,
	     OR (G_UNICODE_FINAL_PUNCTUATION,
	     OR (G_UNICODE_INITIAL_PUNCTUATION,
	     OR (G_UNICODE_OTHER_PUNCTUATION,
	     OR (G_UNICODE_OPEN_PUNCTUATION,
	     OR (G_UNICODE_CURRENCY_SYMBOL,
	     OR (G_UNICODE_MODIFIER_SYMBOL,
	     OR (G_UNICODE_MATH_SYMBOL,
	     OR (G_UNICODE_OTHER_SYMBOL,
	    0)))))))))))) ? TRUE : FALSE;
}

/**
 * g_unichar_isspace:
 * @c: a Unicode character
 * 
 * Determines whether a character is a space, tab, or line separator
 * (newline, carriage return, etc.).  Given some UTF-8 text, obtain a
 * character value with g_utf8_get_char().
 *
 * (Note: don't use this to do word breaking; you have to use
 * Pango or equivalent to get word breaking right, the algorithm
 * is fairly complex.)
 *  
 * Return value: %TRUE if @c is a space character
 **/
gboolean
g_unichar_isspace (gunichar c)
{
  switch (c)
    {
      /* special-case these since Unicode thinks they are not spaces */
    case '\t':
    case '\n':
    case '\r':
    case '\f':
      return TRUE;
      break;
      
    default:
      {
	return IS (TYPE(c),
	           OR (G_UNICODE_SPACE_SEPARATOR,
	           OR (G_UNICODE_LINE_SEPARATOR,
                   OR (G_UNICODE_PARAGRAPH_SEPARATOR,
		  0)))) ? TRUE : FALSE;
      }
      break;
    }
}

/**
 * g_unichar_isupper:
 * @c: a Unicode character
 * 
 * Determines if a character is uppercase.
 * 
 * Return value: %TRUE if @c is an uppercase character
 **/
gboolean
g_unichar_isupper (gunichar c)
{
  return TYPE (c) == G_UNICODE_UPPERCASE_LETTER;
}

/**
 * g_unichar_istitle:
 * @c: a Unicode character
 * 
 * Determines if a character is titlecase. Some characters in
 * Unicode which are composites, such as the DZ digraph
 * have three case variants instead of just two. The titlecase
 * form is used at the beginning of a word where only the
 * first letter is capitalized. The titlecase form of the DZ
 * digraph is U+01F2 LATIN CAPITAL LETTTER D WITH SMALL LETTER Z.
 * 
 * Return value: %TRUE if the character is titlecase
 **/
gboolean
g_unichar_istitle (gunichar c)
{
  unsigned int i;
  for (i = 0; i < G_N_ELEMENTS (title_table); ++i)
    if (title_table[i][0] == c)
      return 1;
  return 0;
}

/**
 * g_unichar_isxdigit:
 * @c: a Unicode character.
 * 
 * Determines if a character is a hexidecimal digit.
 * 
 * Return value: %TRUE if the character is a hexadecimal digit
 **/
gboolean
g_unichar_isxdigit (gunichar c)
{
  return ((c >= 'a' && c <= 'f')
	  || (c >= 'A' && c <= 'F')
	  || ISDIGIT (TYPE (c)));
}

/**
 * g_unichar_isdefined:
 * @c: a Unicode character
 * 
 * Determines if a given character is assigned in the Unicode
 * standard.
 *
 * Return value: %TRUE if the character has an assigned value
 **/
gboolean
g_unichar_isdefined (gunichar c)
{
  return TYPE (c) != G_UNICODE_UNASSIGNED;
}

/**
 * g_unichar_iswide:
 * @c: a Unicode character
 * 
 * Determines if a character is typically rendered in a double-width
 * cell.
 * 
 * Return value: %TRUE if the character is wide
 **/
/* This function stolen from Markus Kuhn <Markus.Kuhn@cl.cam.ac.uk>.  */
gboolean
g_unichar_iswide (gunichar c)
{
  if (c < 0x1100)
    return FALSE;

  return (c <= 0x115f  /* Hangul Jamo init. consonants */ 
          || c == 0x2329 || c == 0x232a     /* angle brackets */
          || (c >= 0x2e80 && c <= 0xa4cf && (c < 0x302a || c > 0x302f) 
              && c != 0x303f && c != 0x3099 && c!= 0x309a) /* CJK ... Yi */
          || (c >= 0xac00 && c <= 0xd7a3)   /* Hangul Syllables */
          || (c >= 0xf900 && c <= 0xfaff)   /* CJK Compatibility Ideographs */
          || (c >= 0xfe30 && c <= 0xfe6f)   /* CJK Compatibility Forms */
          || (c >= 0xff00 && c <= 0xff60)   /* Fullwidth Forms */
          || (c >= 0xffe0 && c <= 0xffe6)   /* Fullwidth Forms */
          || (c >= 0x20000 && c <= 0x2fffd) /* CJK extra stuff */
          || (c >= 0x30000 && c <= 0x3fffd));
}


struct Interval
{
  gunichar start, end;
};

static int
interval_compare (const void *key, const void *elt)
{
  gunichar c = GPOINTER_TO_UINT (key);
  struct Interval *interval = elt;

  if (c < interval->start)
    return -1;
  if (c > interval->end)
    return +1;

  return 0;
}

/**
 * g_unichar_iswide_cjk:
 * @c: a Unicode character
 * 
 * Determines if a character is typically rendered in a double-width
 * cell under legacy East Asian locales.  If a character is wide according to
 * g_unichar_iswide(), then it is also reported wide with this function, but
 * the converse is not necessarily true.  See the
 * <ulink url="http://www.unicode.org/reports/tr11/">Unicode Standard
 * Annex #11</ulink> for details.
 * 
 * Return value: %TRUE if the character is wide in legacy East Asian locales
 *
 * Since: 2.12
 */
/* This function stolen from Markus Kuhn <Markus.Kuhn@cl.cam.ac.uk>.  */
gboolean
g_unichar_iswide_cjk (gunichar c)
{
  /* sorted list of non-overlapping intervals of East Asian Ambiguous
   * characters, generated by "uniset +WIDTH-A -cat=Me -cat=Mn -cat=Cf c" */
  static const struct Interval ambiguous[] = {
    { 0x00A1, 0x00A1 }, { 0x00A4, 0x00A4 }, { 0x00A7, 0x00A8 },
    { 0x00AA, 0x00AA }, { 0x00AE, 0x00AE }, { 0x00B0, 0x00B4 },
    { 0x00B6, 0x00BA }, { 0x00BC, 0x00BF }, { 0x00C6, 0x00C6 },
    { 0x00D0, 0x00D0 }, { 0x00D7, 0x00D8 }, { 0x00DE, 0x00E1 },
    { 0x00E6, 0x00E6 }, { 0x00E8, 0x00EA }, { 0x00EC, 0x00ED },
    { 0x00F0, 0x00F0 }, { 0x00F2, 0x00F3 }, { 0x00F7, 0x00FA },
    { 0x00FC, 0x00FC }, { 0x00FE, 0x00FE }, { 0x0101, 0x0101 },
    { 0x0111, 0x0111 }, { 0x0113, 0x0113 }, { 0x011B, 0x011B },
    { 0x0126, 0x0127 }, { 0x012B, 0x012B }, { 0x0131, 0x0133 },
    { 0x0138, 0x0138 }, { 0x013F, 0x0142 }, { 0x0144, 0x0144 },
    { 0x0148, 0x014B }, { 0x014D, 0x014D }, { 0x0152, 0x0153 },
    { 0x0166, 0x0167 }, { 0x016B, 0x016B }, { 0x01CE, 0x01CE },
    { 0x01D0, 0x01D0 }, { 0x01D2, 0x01D2 }, { 0x01D4, 0x01D4 },
    { 0x01D6, 0x01D6 }, { 0x01D8, 0x01D8 }, { 0x01DA, 0x01DA },
    { 0x01DC, 0x01DC }, { 0x0251, 0x0251 }, { 0x0261, 0x0261 },
    { 0x02C4, 0x02C4 }, { 0x02C7, 0x02C7 }, { 0x02C9, 0x02CB },
    { 0x02CD, 0x02CD }, { 0x02D0, 0x02D0 }, { 0x02D8, 0x02DB },
    { 0x02DD, 0x02DD }, { 0x02DF, 0x02DF }, { 0x0391, 0x03A1 },
    { 0x03A3, 0x03A9 }, { 0x03B1, 0x03C1 }, { 0x03C3, 0x03C9 },
    { 0x0401, 0x0401 }, { 0x0410, 0x044F }, { 0x0451, 0x0451 },
    { 0x2010, 0x2010 }, { 0x2013, 0x2016 }, { 0x2018, 0x2019 },
    { 0x201C, 0x201D }, { 0x2020, 0x2022 }, { 0x2024, 0x2027 },
    { 0x2030, 0x2030 }, { 0x2032, 0x2033 }, { 0x2035, 0x2035 },
    { 0x203B, 0x203B }, { 0x203E, 0x203E }, { 0x2074, 0x2074 },
    { 0x207F, 0x207F }, { 0x2081, 0x2084 }, { 0x20AC, 0x20AC },
    { 0x2103, 0x2103 }, { 0x2105, 0x2105 }, { 0x2109, 0x2109 },
    { 0x2113, 0x2113 }, { 0x2116, 0x2116 }, { 0x2121, 0x2122 },
    { 0x2126, 0x2126 }, { 0x212B, 0x212B }, { 0x2153, 0x2154 },
    { 0x215B, 0x215E }, { 0x2160, 0x216B }, { 0x2170, 0x2179 },
    { 0x2190, 0x2199 }, { 0x21B8, 0x21B9 }, { 0x21D2, 0x21D2 },
    { 0x21D4, 0x21D4 }, { 0x21E7, 0x21E7 }, { 0x2200, 0x2200 },
    { 0x2202, 0x2203 }, { 0x2207, 0x2208 }, { 0x220B, 0x220B },
    { 0x220F, 0x220F }, { 0x2211, 0x2211 }, { 0x2215, 0x2215 },
    { 0x221A, 0x221A }, { 0x221D, 0x2220 }, { 0x2223, 0x2223 },
    { 0x2225, 0x2225 }, { 0x2227, 0x222C }, { 0x222E, 0x222E },
    { 0x2234, 0x2237 }, { 0x223C, 0x223D }, { 0x2248, 0x2248 },
    { 0x224C, 0x224C }, { 0x2252, 0x2252 }, { 0x2260, 0x2261 },
    { 0x2264, 0x2267 }, { 0x226A, 0x226B }, { 0x226E, 0x226F },
    { 0x2282, 0x2283 }, { 0x2286, 0x2287 }, { 0x2295, 0x2295 },
    { 0x2299, 0x2299 }, { 0x22A5, 0x22A5 }, { 0x22BF, 0x22BF },
    { 0x2312, 0x2312 }, { 0x2460, 0x24E9 }, { 0x24EB, 0x254B },
    { 0x2550, 0x2573 }, { 0x2580, 0x258F }, { 0x2592, 0x2595 },
    { 0x25A0, 0x25A1 }, { 0x25A3, 0x25A9 }, { 0x25B2, 0x25B3 },
    { 0x25B6, 0x25B7 }, { 0x25BC, 0x25BD }, { 0x25C0, 0x25C1 },
    { 0x25C6, 0x25C8 }, { 0x25CB, 0x25CB }, { 0x25CE, 0x25D1 },
    { 0x25E2, 0x25E5 }, { 0x25EF, 0x25EF }, { 0x2605, 0x2606 },
    { 0x2609, 0x2609 }, { 0x260E, 0x260F }, { 0x2614, 0x2615 },
    { 0x261C, 0x261C }, { 0x261E, 0x261E }, { 0x2640, 0x2640 },
    { 0x2642, 0x2642 }, { 0x2660, 0x2661 }, { 0x2663, 0x2665 },
    { 0x2667, 0x266A }, { 0x266C, 0x266D }, { 0x266F, 0x266F },
    { 0x273D, 0x273D }, { 0x2776, 0x277F }, { 0xE000, 0xF8FF },
    { 0xFFFD, 0xFFFD }, { 0xF0000, 0xFFFFD }, { 0x100000, 0x10FFFD }
  };

  if (g_unichar_iswide (c))
    return TRUE;

  if (bsearch (GUINT_TO_POINTER (c), ambiguous, G_N_ELEMENTS (ambiguous), sizeof ambiguous[0],
	       interval_compare))
    return TRUE;

  return FALSE;
}


/**
 * g_unichar_toupper:
 * @c: a Unicode character
 * 
 * Converts a character to uppercase.
 * 
 * Return value: the result of converting @c to uppercase.
 *               If @c is not an lowercase or titlecase character,
 *               or has no upper case equivalent @c is returned unchanged.
 **/
gunichar
g_unichar_toupper (gunichar c)
{
  int t = TYPE (c);
  if (t == G_UNICODE_LOWERCASE_LETTER)
    {
      gunichar val = ATTTABLE (c >> 8, c & 0xff);
      if (val >= 0x1000000)
	{
	  const gchar *p = special_case_table + val - 0x1000000;
	  return g_utf8_get_char (p);
	}
      else
	return val ? val : c;
    }
  else if (t == G_UNICODE_TITLECASE_LETTER)
    {
      unsigned int i;
      for (i = 0; i < G_N_ELEMENTS (title_table); ++i)
	{
	  if (title_table[i][0] == c)
	    return title_table[i][1];
	}
    }
  return c;
}

/**
 * g_unichar_tolower:
 * @c: a Unicode character.
 * 
 * Converts a character to lower case.
 * 
 * Return value: the result of converting @c to lower case.
 *               If @c is not an upperlower or titlecase character,
 *               or has no lowercase equivalent @c is returned unchanged.
 **/
gunichar
g_unichar_tolower (gunichar c)
{
  int t = TYPE (c);
  if (t == G_UNICODE_UPPERCASE_LETTER)
    {
      gunichar val = ATTTABLE (c >> 8, c & 0xff);
      if (val >= 0x1000000)
	{
	  const gchar *p = special_case_table + val - 0x1000000;
	  return g_utf8_get_char (p);
	}
      else
	return val ? val : c;
    }
  else if (t == G_UNICODE_TITLECASE_LETTER)
    {
      unsigned int i;
      for (i = 0; i < G_N_ELEMENTS (title_table); ++i)
	{
	  if (title_table[i][0] == c)
	    return title_table[i][2];
	}
    }
  return c;
}

/**
 * g_unichar_totitle:
 * @c: a Unicode character
 * 
 * Converts a character to the titlecase.
 * 
 * Return value: the result of converting @c to titlecase.
 *               If @c is not an uppercase or lowercase character,
 *               @c is returned unchanged.
 **/
gunichar
g_unichar_totitle (gunichar c)
{
  unsigned int i;
  for (i = 0; i < G_N_ELEMENTS (title_table); ++i)
    {
      if (title_table[i][0] == c || title_table[i][1] == c
	  || title_table[i][2] == c)
	return title_table[i][0];
    }
  return (TYPE (c) == G_UNICODE_LOWERCASE_LETTER
	  ? ATTTABLE (c >> 8, c & 0xff)
	  : c);
}

/**
 * g_unichar_digit_value:
 * @c: a Unicode character
 *
 * Determines the numeric value of a character as a decimal
 * digit.
 *
 * Return value: If @c is a decimal digit (according to
 * g_unichar_isdigit()), its numeric value. Otherwise, -1.
 **/
int
g_unichar_digit_value (gunichar c)
{
  if (TYPE (c) == G_UNICODE_DECIMAL_NUMBER)
    return ATTTABLE (c >> 8, c & 0xff);
  return -1;
}

/**
 * g_unichar_xdigit_value:
 * @c: a Unicode character
 *
 * Determines the numeric value of a character as a hexidecimal
 * digit.
 *
 * Return value: If @c is a hex digit (according to
 * g_unichar_isxdigit()), its numeric value. Otherwise, -1.
 **/
int
g_unichar_xdigit_value (gunichar c)
{
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (TYPE (c) == G_UNICODE_DECIMAL_NUMBER)
    return ATTTABLE (c >> 8, c & 0xff);
  return -1;
}

/**
 * g_unichar_type:
 * @c: a Unicode character
 * 
 * Classifies a Unicode character by type.
 * 
 * Return value: the type of the character.
 **/
GUnicodeType
g_unichar_type (gunichar c)
{
  return TYPE (c);
}

/*
 * Case mapping functions
 */

typedef enum {
  LOCALE_NORMAL,
  LOCALE_TURKIC,
  LOCALE_LITHUANIAN
} LocaleType;

static LocaleType
get_locale_type (void)
{
#ifdef G_OS_WIN32
  char *tem = g_win32_getlocale ();
  char locale[2];

  locale[0] = tem[0];
  locale[1] = tem[1];
  g_free (tem);
#else
  const char *locale = setlocale (LC_CTYPE, NULL);
#endif

  switch (locale[0])
    {
   case 'a':
      if (locale[1] == 'z')
	return LOCALE_TURKIC;
      break;
    case 'l':
      if (locale[1] == 't')
	return LOCALE_LITHUANIAN;
      break;
    case 't':
      if (locale[1] == 'r')
	return LOCALE_TURKIC;
      break;
    }

  return LOCALE_NORMAL;
}

static gint
output_marks (const char **p_inout,
	      char        *out_buffer,
	      gboolean     remove_dot)
{
  const char *p = *p_inout;
  gint len = 0;
  
  while (*p)
    {
      gunichar c = g_utf8_get_char (p);
      
      if (ISMARK (TYPE (c)))
	{
	  if (!remove_dot || c != 0x307 /* COMBINING DOT ABOVE */)
	    len += g_unichar_to_utf8 (c, out_buffer ? out_buffer + len : NULL);
	  p = g_utf8_next_char (p);
	}
      else
	break;
    }

  *p_inout = p;
  return len;
}

static gint
output_special_case (gchar *out_buffer,
		     int    offset,
		     int    type,
		     int    which)
{
  const gchar *p = special_case_table + offset;
  gint len;

  if (type != G_UNICODE_TITLECASE_LETTER)
    p = g_utf8_next_char (p);

  if (which == 1)
    p += strlen (p) + 1;

  len = strlen (p);
  if (out_buffer)
    memcpy (out_buffer, p, len);

  return len;
}

#define __G_UNIPROP_C__
//#include "galiasdef.c"
