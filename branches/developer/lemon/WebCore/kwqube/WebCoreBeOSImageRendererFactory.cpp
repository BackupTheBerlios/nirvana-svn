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
#include <assert.h>
#include <gtk/gtkversion.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <string.h>

#include "ImageRendererFactory.h"
#include "ImageRenderer.h"
#include "GLibHelpers.h"

void ImageRendererFactory::useAsSharedFactory()
{
    static ImageRendererFactory single;
    m_sharedFactory = &single;
}


ImageRendererFactory::ImageRendererFactory()
    :mimeTypes(0)
{


    GSList *formats = gdk_pixbuf_get_formats();
    GSList *iter = formats;

    while (iter) {
	gchar** gdkTypes = gdkTypes = gdk_pixbuf_format_get_mime_types((GdkPixbufFormat*)iter->data);
	int i =0;
	while (gdkTypes[i]) {
	    mimeTypes = g_list_append(mimeTypes, gdkTypes[i]); // own the strings
	    ++i;
	}

	if (gdkTypes) g_free(gdkTypes); // free just the shell

	iter = g_slist_next(iter);
    }
    g_slist_free(formats);
}

extern "C" {
void mimeTypes_destroy(gpointer data, gpointer user_data)
{
    g_free(data);
}
};

ImageRendererFactory::~ImageRendererFactory()
{
    g_list_foreach(mimeTypes, mimeTypes_destroy, this);
    g_list_free(mimeTypes);
}

WebCoreImageRenderer* ImageRendererFactory::imageRenderer()
{
    WebCoreImageRenderer* r = new ImageRenderer;
    r->ref();
    return r;
}

WebCoreImageRenderer* ImageRendererFactory::imageRendererWithMIMEType(const gchar* MIMEType)
{
    GError *err = NULL;
    GdkPixbufLoader* ldr = 0;

    if (!isEmptyString(MIMEType)) {
#if (GTK_MAJOR_VERSION >= 2) && (GTK_MINOR_VERSION >=4)
	ldr = gdk_pixbuf_loader_new_with_mime_type(MIMEType, &err);
#else
	const gchar* p = strchr(MIMEType,'/');
	if (p)
	    ldr = gdk_pixbuf_loader_new_with_type(p+1, &err);
#endif
	assert((!err && ldr) || (err && !ldr));
    }

    if (!ldr) 
	ldr = gdk_pixbuf_loader_new();

    ImageRenderer* r = new ImageRenderer(ldr);
    r->ref();
    return r;
}

WebCoreImageRenderer* ImageRendererFactory::imageRendererWithBytes(const gchar* bytes, unsigned length)
{
    GdkPixbufLoader* ldr = gdk_pixbuf_loader_new();
    if (!ldr) {
	ImageRenderer* r =  new ImageRenderer();
	r->ref();
	return r;
    }

    return imageRendererWithBytesAndLoader(bytes, length, ldr);
}

WebCoreImageRenderer* ImageRendererFactory::imageRendererWithBytes(const gchar* bytes, unsigned length, const gchar* MIMEType)
{
    GError *err = NULL;
#if (GTK_MAJOR_VERSION >= 2) && (GTK_MINOR_VERSION >=4)
    GdkPixbufLoader* ldr = gdk_pixbuf_loader_new_with_mime_type(MIMEType, &err);
#else
    GdkPixbufLoader* ldr;
    const gchar* p = strchr(MIMEType, '/');
    if (!p)
	ldr = gdk_pixbuf_loader_new();
    else {
	ldr = gdk_pixbuf_loader_new_with_type(p+1, &err);
    }
#endif

    if (err != NULL) {
        ImageRenderer *r = new ImageRenderer();
	r->ref();
	return r;
    }
    return imageRendererWithBytesAndLoader(bytes, length, ldr);
}

WebCoreImageRenderer* ImageRendererFactory::imageRendererWithBytesAndLoader(const gchar* bytes, unsigned length, GdkPixbufLoader* ldr)
{
    GError *err = NULL;
    if (!gdk_pixbuf_loader_write(ldr, (const guchar*) bytes, length, &err)) {
	ImageRenderer* r = new ImageRenderer();
	r->ref();
	return r;
    }
	
    
    GdkPixbufAnimation *anim = gdk_pixbuf_loader_get_animation(ldr);
    if (!anim) {
	ImageRenderer* r= new ImageRenderer();
	r->ref();
	return r;
    }

    ImageRenderer* r = new ImageRenderer(anim);
    r->ref();
    return r;
}

WebCoreImageRenderer* ImageRendererFactory::imageRendererWithSize(GdkRectangle* size)
{
    ImageRenderer *ir;
    GdkPixbufLoader* ldr = gdk_pixbuf_loader_new();
    if (!ldr)
	ir = new ImageRenderer();
    else   
	ir =  new ImageRenderer(ldr);

    ir->ref();
    ir->resize(size);
    return ir;
}

extern "C"{
gint compare_strings_func(gconstpointer a, gconstpointer b)
{
    return strcmp((const gchar*)a, (const gchar*)b);
}
};

WebCoreImageRenderer* ImageRendererFactory::imageRendererWithName(const gchar* name)
{
    if (strcmp(name, "missing_image")) {
	ImageRenderer* r =  new ImageRenderer();
	//load here the missing image 
	r->ref();
	return r;
    }

    ImageRenderer *r = new ImageRenderer();
    r->ref();
    return r;
}

bool ImageRendererFactory::supports(const gchar* mime)
{
    GList* item = g_list_find_custom(mimeTypes, mime, compare_strings_func);
    return (item!=0);
}
GList* ImageRendererFactory::supportedMIMETypes()
{
    return mimeTypes;
}

int ImageRendererFactory::CGCompositeOperationInContext(CGContextRef context)
{
#if DEBUG
    g_printerr("NotYetImplemented: %s, this:%x, context:%x", __PRETTY_FUNCTION__, (int)this,(int)context);
#endif
    return 0;
}

void ImageRendererFactory::setCGCompositeOperation(CGContextRef context, int op )
{
#if DEBUG
    g_printerr("NotYetImplemented: %s, this:%x, context:%x, op:%d", __PRETTY_FUNCTION__, (int)this,(int)context, op);
#endif
}

void ImageRendererFactory::setCGCompositeOperationFromString(CGContextRef context, const gchar* op)
{
#if DEBUG
    g_printerr("NotYetImplemented: %s, this:%x, context:%x, op:%s", __PRETTY_FUNCTION__, (int)this,(int)context, op);
#endif
}
