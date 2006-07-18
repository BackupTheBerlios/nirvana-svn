/*
 * Copyright (C) 2003 Apple Computer, Inc.  All rights reserved.
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

#include "KWQTextCodec.h"
#include "KWQAssertions.h"
#include "KWQCharsets.h"
#if KWQUBE
//#include "KWIQMacros.h"
#include "glib.h"
#endif    

const UniChar BOM = 0xFEFF;

/** FIXME: remove the code here, pick up apples KWQTextCodec.mm and modify that
 * this is rotten code 
 */

class FallbackDecoder: public QTextDecoder {
public:
     ~FallbackDecoder();
     QString toUnicode(const char *, int, bool flush = false);
};

class UTF8Decoder: public QTextDecoder {
    FallbackDecoder* fallback;
public:
    UTF8Decoder() : fallback(0) {};
    
    ~UTF8Decoder();    
    QString toUnicode(const char *, int, bool flush = false);
};

class GConvDecoder: public QTextDecoder{
    FallbackDecoder* fallback;
    const char *_encodingString;
    KWQEncodingFlags _flags;
public:
    GConvDecoder(CFStringEncoding encoding, KWQEncodingFlags f);
    
    ~GConvDecoder();    
    QString toUnicode(const char *, int, bool flush = false);
};


static void encodingToCodecValueDestroyFunc(gpointer data){
    QTextCodec *c = static_cast<QTextCodec*>(data);
    delete c;
}

static void encodingToCodecKeyDestroyFunc(gpointer data){
    gint *c = static_cast<int*>(data);
    delete c;
}

struct staticinit{
    GHashTable *encodingToCodec;
    staticinit(){
	encodingToCodec = g_hash_table_new_full( g_int_hash, g_int_equal,
	    encodingToCodecKeyDestroyFunc,encodingToCodecValueDestroyFunc);
    }
    ~staticinit(){
	g_hash_table_destroy(encodingToCodec);
    }
};

static staticinit staticcodechashtable;
    
static QTextCodec *codecForCFStringEncoding(CFStringEncoding encoding, KWQEncodingFlags flags)
{
    if (encoding == kCFStringEncodingInvalidId) {
        return 0;
    }    
    static GHashTable* encodingToCodec = staticcodechashtable.encodingToCodec;
    
    QTextCodec *key  = new QTextCodec(encoding, flags);	
    gint *hash = new gint;
    *hash = key->hash();
    
    QTextCodec *codec = static_cast<QTextCodec*>(g_hash_table_lookup(encodingToCodec, hash));
    if (!codec){
	g_hash_table_insert(encodingToCodec, hash , key);
	return key;	
    }else{
	delete key;
	delete hash;
	return codec;
    }
}

QTextCodec::QTextCodec(CFStringEncoding e, KWQEncodingFlags f)
    
{
    _encoding = e; _flags=f;
    _encodingString = KWQCFStringEncodingToIANACharsetName(_encoding);
    if (!_encodingString)
	_encodingString = "latin1";
}

QTextCodec *QTextCodec::codecForName(const char *name)
{
    KWQEncodingFlags flags;
    CFStringEncoding encoding = KWQCFStringEncodingFromIANACharsetName(name, &flags);
    return codecForCFStringEncoding(encoding, flags);
}


QTextCodec *QTextCodec::codecForNameEightBitOnly(const char *name)
{
    KWQEncodingFlags flags;
    CFStringEncoding encoding = KWQCFStringEncodingFromIANACharsetName(name, &flags);
    switch (encoding) {
    case kCFStringEncodingUnicode:
	encoding = kCFStringEncodingUTF8;
	break;
    default:
        break;
    }
    return codecForCFStringEncoding(encoding, flags);
}

QTextCodec *QTextCodec::codecForLocale()
{
    const gchar* name;
    g_get_charset(&name);
    return codecForName(name);
}

const char *QTextCodec::name() const
{
   
    return KWQCFStringEncodingToIANACharsetName(_encoding);
}

QTextDecoder *QTextCodec::makeDecoder() const
{
    QTextDecoder *d; 
    switch (_encoding){
    case (kCFStringEncodingASCII):
    case (kCFStringEncodingISOLatin1):
    case (kCFStringEncodingDOSLatin1):
	d = new FallbackDecoder();
	break;
    case (kCFStringEncodingUTF8):
	d = new UTF8Decoder();
	break;
    default:
	d = new GConvDecoder(_encoding,_flags);
	break;
    };
    return d;
}

inline CFStringEncoding effectiveEncoding(CFStringEncoding e)
{
    switch (e) {
        case kCFStringEncodingISOLatin1:
        case kCFStringEncodingASCII:
            e = kCFStringEncodingWindowsLatin1;
            break;
    }
    return e;
}

QCString QTextCodec::fromUnicode(const QString &qcs) const
{
    if (qcs.length() <= 0) {
	//g_warning("QTextCodec::fromUnicode empty string");
        return QCString();
    }
    const QChar* data = qcs.unicode();
    int len = qcs.length();
//     if (qcs.isAllASCII()){
// 	return QCString(qcs.ascii(),len+1);
//     }

    // FIXME: Since there's no "force ASCII range" mode in CFString, we change the backslash into a yen sign.
    // Encoding will change the yen sign back into a backslash.
#if 0 //FIXME:KWIQ: backslash as currency symbol block commented out    
    QString copy;
    QChar currencySymbol = backslashAsCurrencySymbol();
    if (currencySymbol != '\\' && qcs.find('\\') != -1) {
        copy = qcs;
	copy.replace('\\', currencySymbol);
	data = copy.unicode();
	len = copy.length();
    }    

#endif
    // Handle UCS-2 -> UCS-2
    if (_encoding == kCFStringEncodingUnicode && _flags == NoEncodingFlags)
	return QCString(reinterpret_cast<const char*>(data),2*len);

    GError* err=NULL;
    gchar *dest=0,*utf8=0;
    glong r_utf8,w_utf8;
    gsize r_dest,w_dest;
    
    // UCS-2 -> UTF-8
    utf8 = g_utf16_to_utf8(reinterpret_cast<const gunichar2*>(data),len,
	&r_utf8,&w_utf8,&err);
    
    if (_encoding == kCFStringEncodingUTF8 && _flags == NoEncodingFlags){
	// If UTF-8 is what we wanted
	dest=utf8;
	utf8=NULL;
	w_dest=w_utf8;
    } else{
	// Handle UTF-8 -> destination charset
	dest = g_convert(reinterpret_cast<const gchar*>(utf8), w_utf8+1,
	    _encodingString, "UTF-8", &r_dest, &w_dest, &err);
	g_free(utf8);
    }
    
    
    if (err!=NULL){
	REPORT_G_CONV_ERROR(err);
	g_error_free(err);
	return QCString();
    }
    
    QCString str(dest, w_dest+1);
    g_free(dest);
    return str;
}


QString QTextCodec::toUnicode(const char *chs, int len) const
{
    QTextDecoder* d = makeDecoder();
    QString res = d->toUnicode(chs, len, true);
    delete d;
    return res;
}

QString QTextCodec::toUnicode(const QByteArray &qba, int len) const
{
    return toUnicode(qba.data(),len);
}
    
QChar QTextCodec::backslashAsCurrencySymbol() const
{
    // FIXME: We should put this information into KWQCharsetData instead of having a switch here.
    switch (_encoding) {
        case kCFStringEncodingShiftJIS_X0213_00:
        case kCFStringEncodingEUC_JP:
            return 0x00A5; // yen sign
        default:
            return '\\';
    }
}

bool operator==(const QTextCodec &a, const QTextCodec &b)
{
    return a._encoding == b._encoding && a._flags == b._flags;
}

unsigned QTextCodec::hash() const
{
    unsigned h = _encoding;

    h += (h << 10);
    h ^= (h << 6);
    
    h ^= _flags;

    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);
    
    return h;
}
#if 0 //KWIQ: Codec comparision not used at the moment
static boolean QTextCodecsEqual(const void *a, const void *b)
{
    return *static_cast<const QTextCodec *>(a) == *static_cast<const QTextCodec *>(b);
}

static CFHashCode QTextCodecHash(const void *value)
{
    return static_cast<const QTextCodec *>(value)->hash();
}
#endif

// ================

QTextDecoder::~QTextDecoder()
{
}


/**
 * FallbackDecoder 
 */
FallbackDecoder::~FallbackDecoder(){

};

QString FallbackDecoder::toUnicode(const char *data, int len, bool flush)
{
    
    //LOG(KwiqLog,"data:%x, len:%d,flush:%d",data,len,flush);	
    ASSERT_ARG(len, len >= 0);
    if (!data || len <= 0) {
        return QString();
    }
    return QString::fromLatin1(data,len);
}

/**
 * UTF8Decoder 
 */

UTF8Decoder::~UTF8Decoder()
{
    if (fallback) delete fallback;
}

QString UTF8Decoder::toUnicode(const char *data, int len, bool flush)
{
    //LOG(KwiqLog,"data:%x, len:%d,flush:%d",data,len,flush);
    ASSERT_ARG(len, len >= 0);
    if (!data || len <= 0) {
        return QString();
    }
    
    if (fallback)
	return fallback->toUnicode(data,len,flush);
    
    GError* err=NULL;
    gunichar2 *dest=0;
//    const gchar *utf8=0;
//    gsize r_utf8=0, w_utf8=0;
    glong r_dest,w_dest;

    dest = g_utf8_to_utf16(reinterpret_cast<const gchar*>(data), len, &r_dest, &w_dest,&err);
    if (err != 0){
	if (err->code == G_CONVERT_ERROR_ILLEGAL_SEQUENCE &&
	    r_dest<len &&
	    r_dest+2 >= len)
	{
	    //g_warning("Got partial multibyte string.");
	    dest = g_utf8_to_utf16(reinterpret_cast<const gchar*>(data),r_dest,
		&r_dest,&w_dest,NULL);
	}
	else{
	    //g_warning(" --> Too much data, weird: r_dest:%d, len:%d, falling back ",r_dest,len);
	    REPORT_G_CONV_ERROR(err);
	}
	g_error_free(err);
    }
    
    if (dest){	
	QString str(reinterpret_cast<QChar*>(dest), w_dest);
	g_free(dest);
	return str;    
    }
    fallback = new FallbackDecoder();
    return fallback->toUnicode(data,len,flush);

}

/**
 * GConvDecoder
 */
GConvDecoder::GConvDecoder(CFStringEncoding encoding, KWQEncodingFlags f)
{
    fallback=0;
    _encodingString = KWQCFStringEncodingToIANACharsetName(encoding);
    _flags = f;
    
    if (!_encodingString)
	_encodingString = "latin1";
    
    //LOG(KwiqLog,"charset:%s ",_encodingString);	      
}

GConvDecoder::~GConvDecoder()
{
    if (fallback) delete fallback;
}

#define MAXBUFSIZE 4000

QString GConvDecoder::toUnicode(const char *data, int len, bool flush)
{
    //LOG(KwiqLog,"charset:%s data %x, len: %d, flush: %d",_encodingString,data,len,flush);
    ASSERT_ARG(len, len >= 0);
    if (!data || len <= 0) {
        return QString();
    }

    if (fallback){
	return fallback->toUnicode(data,len,flush);
    }

    GError* err=NULL;
    gunichar2 *dest=0;
    gchar *utf8=0;
    gsize r_utf8=0, w_utf8=0;
    glong r_dest,w_dest;

    // source -> UTF-8
    err = NULL;
    utf8 = g_convert(reinterpret_cast<const gchar*>(data), len,
	"UTF-8", _encodingString, &r_utf8, &w_utf8, &err);
	
    if (err!=NULL){
	if (err->code == G_CONVERT_ERROR_ILLEGAL_SEQUENCE &&            
	    r_utf8 < (unsigned int) len &&  // cast is safe because len > 0 for the ASSERT_ARG above.
	    r_utf8+3 < (unsigned int) len)
	{
	    //LOG(KwiqLog,"Partial multibyte, doing conversion again utf8->%s",_encodingString);
	    utf8 = g_convert(reinterpret_cast<const gchar*>(data), r_utf8,
		"UTF-8", _encodingString, &r_utf8, &w_utf8, NULL);
	}
	else {
	    REPORT_G_CONV_ERROR(err);
	}
	g_error_free(err);
    }

    if (utf8){
	dest = g_utf8_to_utf16(utf8,w_utf8, &r_dest,&w_dest,NULL);
	g_free(utf8);
	if (dest){
	    QString str(reinterpret_cast<QChar*>(dest), w_dest+1);
	    g_free(dest);
	    return str;
	}		
    }
    
    fallback = new FallbackDecoder();
    return fallback->toUnicode(data,len,flush);    
}
