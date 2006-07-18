#include <string.h> 		// memcpy
#include "glib.h"

#include "KWQAssertions.h"

#include "WebCoreTextRenderer.h"

WebCoreTextRun::WebCoreTextRun(const UniChar* chars, unsigned int len, int afrom, int ato)
    : characters(chars), length(len), from(afrom), to(ato)
{
    ASSERT(length>=0);
    ASSERT(from>=0);
    ASSERT(to>=0);
    ASSERT(length>= (unsigned int) to);
}


WebCoreTextRun::~WebCoreTextRun()
{
}
