/*
 * Copyright (C) 2001, 2002 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "KWQString.h"
#include "KWQLogging.h"

#include "WebCoreUnicode.h"

#include <glib.h> // gunichar
bool QChar::isDigitNonASCII(UniChar c)
{
    return g_unichar_isdigit((gunichar)c);
}

bool QChar::isLetterNonASCII(UniChar c)
{
    return g_unichar_isalpha((gunichar)c);
}

bool QChar::isNumberNonASCII(UniChar c)
{
    return isLetterOrNumberNonASCII(c) && !isLetterNonASCII(c);
}

bool QChar::isLetterOrNumberNonASCII(UniChar c)
{
    return g_unichar_isalnum((gunichar)c);
}


bool QChar::isPunct() const
{
    return g_unichar_ispunct(c);
}

UniChar QChar::lowerNonASCII(UniChar c)
{
    return WebCoreUnicodeLowerFunction(c);
}

UniChar QChar::upperNonASCII(UniChar c)
{
    return WebCoreUnicodeUpperFunction(c);
}

bool QChar::mirrored() const
{
    return WebCoreUnicodeMirroredFunction(c);
}

QChar QChar::mirroredChar() const
{
    return QChar((UniChar)WebCoreUnicodeMirroredCharFunction(c));
}

int QChar::digitValueNonASCII(UniChar)
{
    // FIXME: This isn't right. Need Unicode-savvy version of this that matches isDigitNonASCII.
    return -1;
}

