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
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "ImageRenderer.h"
#include "CGContextProvider.h"
#include "GdkXftContext.h"

#include "GdkHelpers.h"

extern "C"{
static void size_prepared(GdkPixbufLoader *loader, gint width, gint height, gpointer user_data);
static void area_prepared(GdkPixbufLoader *loader, gpointer user_data);
static void area_updated(GdkPixbufLoader *loader,
			 gint arg1,
			 gint arg2,
			 gint arg3,
			 gint arg4,
			 gpointer user_data);
static void closed(GdkPixbufLoader *loader,gpointer user_data);
static gboolean anim_timeout(gpointer user_data);
#if 0 // never used
static gboolean update_timeout(gpointer user_data);
#endif
}


const int ImageRenderer::updateTimeoutMS = 100;

/** ImageRenderer class for loading images and  rendering images
 *
 * invariants: 
 * pixbuf: valid pixbuf if 
 *                 image should be regarded as still image
 *              or image is still being loaded
 *         0 if image is animation and loaded
 * anim:   animation data if 
 *                 image is still being loaded
 *              or image is valid animation
 *         0 if image is still image and loaded
 *
 * iter:   valid animation iterator if image is animation and loaded
 *         0 image is still picture, object owns pixbuf
 *
 * loader: valid GdkPixbufLoader if stuff is being still loaded 
 *         0 if data is not being loaded
 *
 * timeout_moment: valid only if animate_id != 0
 * 
 */ 
ImageRenderer::ImageRenderer()
   :refCnt(0)
   ,loader(0)
   ,anim(0)
   ,iter(0)
   ,pixbuf(0)
   ,pixmap(0)
   ,alpha(0)
   ,animate_id(0)
   ,cached(false)
   ,tileCached(false)
   ,offset(0)
{
    fillGdkRectangle(&wantedSize, 0, 0, -1,-1);
}

ImageRenderer::ImageRenderer(const ImageRenderer& ir)
   :refCnt(0)
   ,loader(0)
   ,anim(0)
   ,iter(0)
   ,pixbuf(0)
   ,pixmap(0)
   ,alpha(0)
   ,animate_id(0)
   ,cached(false)
   ,tileCached(false)
   ,offset(0)
{
    wantedSize = ir.wantedSize;

    if (ir.loader) {
	// still loading, treat as still picture but may have animation data 
	// FIXME: Loader is not "copied", so what this means is that from copied ImageRenderers
	// where the loading is still on we will have only still images!!
	// we should only connect to signals but not "own" the loader.
	assert(!ir.iter);

	assert(ir.pixbuf);
	pixbuf = ir.pixbuf;
	g_object_ref(pixbuf);
    } else {
	// not loading 
	if (ir.anim) {
	    // is an animation => get own pixbuf, owned by iterator
	    anim = ir.anim;
	    g_object_ref(anim);
	    assert(ir.iter);
	    iter = gdk_pixbuf_animation_get_iter(anim, NULL);
	    installAnimateTimeout();
	} else {
	    // not an animation
	    assert(!ir.iter);
	    pixbuf = ir.pixbuf;
	    g_object_ref(pixbuf);
	}
    }
}

ImageRenderer::ImageRenderer(GdkPixbufAnimation* a)
   :refCnt(0)
   ,loader(0)
   ,anim(a)
   ,iter(0)
   ,pixbuf(0)
   ,pixmap(0)
   ,alpha(0)
   ,animate_id(0)
   ,cached(false)
   ,tileCached(false)
   ,offset(0)
{
    assert(anim);
    fillGdkRectangle(&wantedSize, 0,0, -1,-1);

    // is an animation
    g_object_ref(anim);
    iter = gdk_pixbuf_animation_get_iter(anim, NULL);
    realSize(&wantedSize);
    installAnimateTimeout();
}

ImageRenderer::ImageRenderer(GdkPixbufLoader* ldr)
   :refCnt(0)
   ,loader(ldr)
   ,anim(0)
   ,iter(0)
   ,pixbuf(0)
   ,pixmap(0)
   ,alpha(0)
   ,animate_id(0)
   ,cached(false)
   ,tileCached(false)
   ,offset(0)
{
    assert(ldr);
    fillGdkRectangle(&wantedSize,0,0,-1,-1);

    connectHandlers();
}


void ImageRenderer::connectHandlers()
{
    assert(loader);
    loader.connect("area-prepared", ::area_prepared, this);
    loader.connect("size-prepared", ::size_prepared, this);
    loader.connect("area-updated", ::area_updated, this);
    loader.connect("closed", ::closed, this);
}

ImageRenderer::~ImageRenderer()
{
    if (loader) {
	gdk_pixbuf_loader_close(loader, NULL);
    }

    removeAnimateTimeout();
    invalidate();
    if (iter) {
	g_object_unref(iter);
	g_object_unref(anim);
    } else {
	assert(!iter);
	assert(!anim);
	if (pixbuf)
	    g_object_unref(pixbuf);
    }
}

bool ImageRenderer::incrementalLoadWithBytes(const void *bytes, unsigned length, bool isComplete)
{
    if (!loader) { 
#if DEBUG
	g_printerr("called with null/closed loader");
#endif
	assert(0);	
    }

    GError* err = NULL;

    if ((length - offset) > 0) {
	bool succ = gdk_pixbuf_loader_write(loader,
					   ((guchar*) bytes) + offset,
					   length - offset,
					   &err);
	if (succ)
	    offset += length - offset;
	else { 
#if DEBUG
          g_printerr("error loading image incrementally. this:%x bytes %d, len:%d, isComplete: %d, offset:%d", (int) this, (int) bytes,length, (int) isComplete, offset);
#endif
	}
    }
    
    if (isComplete) { 
	err = NULL;
	gdk_pixbuf_loader_close(loader, &err);
	loader = 0;
    }

    return anim != NULL || pixbuf != NULL;
}

void ImageRenderer::realSize(GdkRectangle* rect)
{
    if (iter) {
	fillGdkRectangle(rect, 0, 0, 
			 gdk_pixbuf_animation_get_width(anim), gdk_pixbuf_animation_get_height(anim));
    } else {    
	fillGdkRectangle(rect, 0, 0, 
			 gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
    }
}

void  ImageRenderer::size(GdkRectangle* outSize)
{
    assert(outSize);
    *outSize = wantedSize;
}

void ImageRenderer::resize(GdkRectangle* s)
{
    assert(s);
    assert(s->x == 0);
    assert(s->y == 0);
    assert(s->width > 0);
    assert(s->height > 0);
    invalidate();
    wantedSize = *s;
}

// Scales image or animation frame if resize() has been called.
// Sets ImageRenderer::pixmap and ImageRenderer::alpha accordingly, so the image/frame can be rendered
void ImageRenderer::cache()
{
    assert(!cached);
    GdkPixbuf* buf;

    cached = true;

    if (iter) 
	buf = gdk_pixbuf_animation_iter_get_pixbuf(iter);
    else 
	buf = pixbuf;

    
    GdkRectangle realsz;
    realSize(&realsz);
    if (wantedSize.width != realsz.width || wantedSize.height != realsz.height) {
	GdkPixbuf* scaledbuf = gdk_pixbuf_scale_simple(buf, 
						       wantedSize.width, 
						       wantedSize.height, 
						       GDK_INTERP_BILINEAR);

	gdk_pixbuf_render_pixmap_and_mask(scaledbuf, &pixmap, &alpha, 100);
	if (iter) {
            // animation -> discard scaled frame contents, only preserve the serverside representations
	    g_object_unref(scaledbuf); 
	}
	else {
            // still image -> scaled image becomes the representation of this image
	    g_object_unref(pixbuf); 
	    pixbuf = scaledbuf;
	}
	return;
    }


    gdk_pixbuf_render_pixmap_and_mask(buf, &pixmap, &alpha, 100);
    assert(gdk_pixbuf_get_has_alpha(buf) == (alpha!=NULL));
}

void ImageRenderer::tileCache()
{
    if (!cached) 
	cache();

    // tilecache not implemented yet
    tileCached = true;
}

void ImageRenderer::invalidate()
{
    if (pixmap) {
	g_object_unref(pixmap);
	pixmap = 0;
    }
    if (alpha){
	g_object_unref(alpha);
	alpha = 0;
    }
    cached = false;

    // invalidate tileCache
    tileCached = false;
}

void ImageRenderer::drawImageInRect(GdkRectangle* inRect, GdkRectangle* fromRect, NSCompositingOperation compositeOperator, CGContextRef context)
{
    // scaling not supported
    assert(inRect->width == fromRect->width);
    assert(inRect->height == fromRect->height);

    if (isNull()) return;

    if (!cached)
	cache();

    if (alpha) {
	gdk_gc_set_clip_mask(context->gc, alpha);
        gdk_gc_set_clip_origin(context->gc, inRect->x - fromRect->x, inRect->y - fromRect->y);
    }

    int x, y, sw, sh, sx, sy;

    if (!context->clip  || !alpha) {
	// no clip or clip but not alpha --> draw directly
	x = inRect->x;
	y = inRect->y;
	sx = fromRect->x;
	sy = fromRect->y;
	sw = fromRect->width;
	sh = fromRect->height;    
	gdk_draw_drawable(context->drawable, context->gc, pixmap, sx, sy, x, y, sw, sh);
    } else {
	// gdk clipping doesn't work when alpha is set --> manual clipping

	GdkRegion* dst_region = gdk_region_rectangle(inRect);
	gdk_region_intersect(dst_region, context->clip);
	GdkRectangle* areas = 0;
	gint n_areas = 0;
	gdk_region_get_rectangles(dst_region, &areas, &n_areas);

	if (n_areas>0) {
	    int i;
	    for (i=0;i<n_areas;i++) {
		x = areas[i].x;
		y = areas[i].y;
	    
		sx = fromRect->x + (x - inRect->x); // source offsets of the image rendering 
		sy = fromRect->y + (y - inRect->y); // ie. how many pixels were skipped because of clip
		sw = areas[i].width; 
		sh = areas[i].height; 
		gdk_draw_drawable(context->drawable, context->gc, pixmap, sx, sy, x, y, sw, sh);
	    }
	}
	
	if (areas) 
	    g_free(areas);  // Gdk API doesn't say whether areas allocated regardless of the result or not

	gdk_region_destroy(dst_region);
    }

    if (alpha) {
        gdk_gc_set_clip_mask(context->gc, NULL);
        gdk_gc_set_clip_origin(context->gc, 0, 0);
	gdk_gc_set_clip_region(context->gc, context->clip);
    }

    if (animate_id) {
	static_cast<GdkXftContext*>(context)->regionExpiresAt(&timeout_moment, inRect);
    }
}


void ImageRenderer::stopAnimation()
{
    removeAnimateTimeout();
}

void ImageRenderer::tileInRect(GdkRectangle* r, int sx, int sy, CGContextRef context)
{
    assert(r);
    assert(context);

    if (isNull()) return;

    int x, y, w,h;
    
    int sw = wantedSize.width;
    int sh = wantedSize.height;

    x = r->x;
    y = r->y;
    w = r->width;
    h = r->height;

    if (!tileCached)
	tileCache();

    if (!alpha) {
        // special case without alpha, we can use tiling acceleration 
        // (don't know if this makes a difference) 
	// assume gdk clipping works.
        gdk_gc_set_tile(context->gc, pixmap);
        gdk_gc_set_fill(context->gc, GDK_TILED);        

        // start copy  from (sx,sy), destination (x,y) 
        // tile origin is relative to dest. drawable         
        gdk_gc_set_ts_origin(context->gc, x - sx, y - sy);

        gdk_draw_rectangle(context->drawable, context->gc, TRUE, x, y, w, h);
        gdk_gc_set_fill(context->gc, GDK_SOLID);
    } else {
	// with alpha, do tiling by hand

        gdk_gc_set_clip_mask(context->gc, alpha);

        // remaining width, height 
        int rw,rh;
        // current x, .. y.  current source x, .. y, .. width, .. height 
        int cx, cy, csx, csy, csw,csh;
        
        cy = y;
        rh = h;
        csy = sy;        
        csh = MIN(sh - sy, h);
        while ( csh > 0 ) {
            cx = x;
            csx = sx;
            csw = MIN(sw - sx, w);

            // draw row with height of csh pixels, width of w 
            rw = w;        
            while (csw > 0) {
                // draw csw x csh pixels 
                gdk_gc_set_clip_origin(context->gc, cx - csx, cy - csy);
                gdk_draw_drawable(context->drawable,
                                  context->gc,
                                  pixmap,
                                  csx,
                                  csy,
                                  cx,
                                  cy,
                                  csw,
                                  csh);
                csx = 0;
                cx += csw;
                rw -= csw;
                csw = MIN(rw, sw);
            }
            csy = 0;
            cy += csh;
            rh -= csh;
            csh = MIN(rh, sh);
        }

	//restore old clip region        
        gdk_gc_set_clip_mask(context->gc, NULL);
	gdk_gc_set_clip_origin(context->gc, 0, 0); // assume that clip origin is always 0,0
	gdk_gc_set_clip_region(context->gc, context->clip);
    }
    if (animate_id) {
	static_cast<GdkXftContext*>(context)->regionExpiresAt(&timeout_moment, r);
    }
}

bool ImageRenderer::isNull()
{
    if (!pixbuf && !anim) return true;
    return false;
}

void ImageRenderer::increaseUseCount()
{
}

void ImageRenderer::decreaseUseCount()
{
}

void ImageRenderer::flushRasterCache()
{
    invalidate();
}

void ImageRenderer::animate()
{
    bool need_update = gdk_pixbuf_animation_iter_advance(iter, NULL);    
    if (need_update) {
	invalidate();
	pixbuf = gdk_pixbuf_animation_iter_get_pixbuf(iter);
    }
}

void ImageRenderer::installAnimateTimeout()
{
    removeAnimateTimeout();

    gint timeout = gdk_pixbuf_animation_iter_get_delay_time(iter); // milliseconds
    if (timeout) {
	animate_id = g_timeout_add(timeout,
				   (GSourceFunc)::anim_timeout,
				   this);
	g_get_current_time(&timeout_moment);
	g_time_val_add(&timeout_moment, (timeout*1000)); // microseconds
    }
    
}

void ImageRenderer::removeAnimateTimeout()
{
    if (animate_id != 0) g_source_remove(animate_id);
    animate_id = 0;
}


void ImageRenderer::loaderClosed()
{
    if (!anim && !pixbuf)
	return;

    if (gdk_pixbuf_animation_is_static_image(anim)) {
	g_object_unref(anim);
	anim = 0;
	return;
    }
    
    iter = gdk_pixbuf_animation_get_iter(anim, NULL);

    if (gdk_pixbuf_animation_iter_get_delay_time(iter) <= 0) {
	g_object_unref(iter);
	g_object_unref(anim);
	anim = 0;
	iter = 0;
	return;
    }

    invalidate();
    g_object_unref(pixbuf);
    pixbuf = 0;
    installAnimateTimeout();
}

void ImageRenderer::sizePrepared(int width, int height)
{
    if (wantedSize.width == -1 && wantedSize.height == -1) {
	fillGdkRectangle(&wantedSize, 0, 0, width, height);
	return;
    }
    if (wantedSize.width == width && wantedSize.height == height)
	return;

    gdk_pixbuf_loader_set_size(loader, wantedSize.width, wantedSize.height);
}

void ImageRenderer::areaPrepared()
{
    if (!anim) {
	anim = gdk_pixbuf_loader_get_animation(loader);
	if (anim) 
	    g_object_ref(anim);

	if (pixbuf) 
	    g_object_unref(pixbuf);

	pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
	if (pixbuf)
	    g_object_ref(pixbuf);
    }
}

void ImageRenderer::areaUpdated(int x, int y, int w, int h)
{
    invalidate();
			      
}

ImageRenderer* ImageRenderer::copy()
{
    ImageRenderer* r = new ImageRenderer(*this);

    return r;
}

void ImageRenderer::animateTimeout()
{
    animate_id = 0;
    animate();
    installAnimateTimeout();
}


GdkPixbuf* ImageRenderer::handle() const
{
    if (iter) 
	return gdk_pixbuf_animation_iter_get_pixbuf(iter);
    return pixbuf;
}

extern "C"{
static
void
area_prepared(GdkPixbufLoader *loader, gpointer user_data)
{
    ImageRenderer* d = static_cast<ImageRenderer*>(user_data);
    d->areaPrepared();

}

static
void
area_updated(GdkPixbufLoader *loader,
	     gint arg1,
	     gint arg2,
	     gint arg3,
	     gint arg4,
	     gpointer user_data)
{
    ImageRenderer* d = static_cast<ImageRenderer*>(user_data);
    d->areaUpdated(arg1,arg2,arg3,arg4);
}
static
void
closed(GdkPixbufLoader *loader,gpointer user_data)
{
    ImageRenderer* d = static_cast<ImageRenderer*>(user_data);
    d->loaderClosed();
}

static
void
size_prepared(GdkPixbufLoader *loader,
	      gint width,
	      gint height,
	      gpointer user_data)
{
    ImageRenderer* d = static_cast<ImageRenderer*>(user_data);
    d->sizePrepared(width, height);
}


static
gboolean
anim_timeout(gpointer user_data)
{
    ImageRenderer* d = static_cast<ImageRenderer*>(user_data);
    d->animateTimeout();
    return FALSE;
}
} // extern "C"
