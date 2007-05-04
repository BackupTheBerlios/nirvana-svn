/*
 * Copyright (c) 2004 Nokia. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of Nokia nor the names of its contributors may be
 * used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <glib.h>

#include "WebCoreBeOSUnicode.h"

extern "C" 
{

int UnicodeImplDigitValueFunction(UChar32 c)
{
    return g_unichar_digit_value((gunichar) c);    

}

WebCoreUnicodeDirection UnicodeImplDirectionFunction(UChar32 c)
{
    GUnicodeType type = g_unichar_type((gunichar)(c));
    switch (type){
    case G_UNICODE_CONTROL:
	// this tells the direction?
    case G_UNICODE_FORMAT:
    case G_UNICODE_UNASSIGNED:
    case G_UNICODE_PRIVATE_USE:
    case G_UNICODE_SURROGATE:
    case G_UNICODE_LOWERCASE_LETTER:
    case G_UNICODE_MODIFIER_LETTER:
    case G_UNICODE_OTHER_LETTER:
    case G_UNICODE_TITLECASE_LETTER:
    case G_UNICODE_UPPERCASE_LETTER:
    case G_UNICODE_COMBINING_MARK:
    case G_UNICODE_ENCLOSING_MARK:
	return DirectionL; //### FIXME: Control characters for direction(?)

    case G_UNICODE_NON_SPACING_MARK:
	return DirectionNSM;
    case G_UNICODE_DECIMAL_NUMBER:
    case G_UNICODE_LETTER_NUMBER:
    case G_UNICODE_OTHER_NUMBER:
    case G_UNICODE_CONNECT_PUNCTUATION:
    case G_UNICODE_DASH_PUNCTUATION:
    case G_UNICODE_CLOSE_PUNCTUATION:
    case G_UNICODE_FINAL_PUNCTUATION:
    case G_UNICODE_INITIAL_PUNCTUATION:
    case G_UNICODE_OTHER_PUNCTUATION:
    case G_UNICODE_OPEN_PUNCTUATION:
    case G_UNICODE_CURRENCY_SYMBOL:
    case G_UNICODE_MODIFIER_SYMBOL:
    case G_UNICODE_MATH_SYMBOL:
    case G_UNICODE_OTHER_SYMBOL:
	return DirectionL; // FIXME: Control characters for direction

    case G_UNICODE_LINE_SEPARATOR:
	return DirectionCS;
    case G_UNICODE_PARAGRAPH_SEPARATOR:
	return DirectionB;
    case G_UNICODE_SPACE_SEPARATOR:
	return DirectionWS;
    }
    
    // NOT REACHED
    return DirectionL;
}

bool UnicodeImplMirroredFunction(UChar32 c)
{
#if ((GLIB_MAJOR_VERSION>=2) && (GLIB_MINOR_VERSION>=4))
    // glib spec says:
    // 'Returns : TRUE if ch has a mirrored character and mirrored_ch is filled in, FALSE otherwise'
    gunichar mirrored_ch;
    return g_unichar_get_mirror_char((gunichar)c, &mirrored_ch);
#else
    // XXX: Mirrored char support disabled if glib < 2.4
    return false;
#endif
}

UChar32 UnicodeImplMirroredCharFunction(UChar32 c)
{
#if ( (GLIB_MAJOR_VERSION>=2) && (GLIB_MINOR_VERSION>=4))
    gunichar mirrored_ch;
    g_unichar_get_mirror_char((gunichar)c, &mirrored_ch);
    return (UChar32)mirrored_ch;
#else
    // XXX: Mirrored char support disabled if glib < 2.4
    return 0;
#endif
}

UChar32 UnicodeImplLowerFunction(UChar32 c)
{
    return g_unichar_tolower((gunichar)c);
}

UChar32 UnicodeImplUpperFunction(UChar32 c)
{
    return g_unichar_toupper((gunichar)c);
}

} // extern "C"
