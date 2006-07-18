#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "KWQPixmap.h"
#include "KWQAssertions.h"

#include "WebCoreImageRendererFactory.h"
#include "WebCoreImageRenderer.h"


void initGdkRectangleFromSize(GdkRectangle * rect, const QSize & size) {
    ASSERT(rect);
    rect->x = rect->y = 0;
    rect->width = size.width();
    rect->height = size.height();
}

void initGdkRectangle(GdkRectangle * rect, const int w, const int h) {
    ASSERT(rect);
    rect->x = rect->y = 0;
    rect->width = w;
    rect->height = h;
}

class QPixmapPrivate
{
public:
    GdkPixmap *pixmap;
    GdkPixmap *alpha_bitmap;
    GdkPixbuf *buf;
    QPixmapPrivate();
};


QPixmapPrivate::QPixmapPrivate()
    : pixmap(0),alpha_bitmap(0),buf(0)
{
}


QPixmap *KWQLoadPixmap(const char *name)
{
    QPixmap *p = new QPixmap(WebCoreImageRendererFactory::sharedFactory()->imageRendererWithName(name));

    return p;
}

bool canRenderImageType(const QString &type)
{
    return WebCoreImageRendererFactory::sharedFactory()->supports(type.latin1());
}

QPixmap::QPixmap ()
{
    imageRenderer = WebCoreImageRendererFactory::sharedFactory()->imageRenderer();
    needCopyOnWrite = false;
}

QPixmap::QPixmap(WebCoreImageRenderer* r)
{
    ASSERT(r);
    imageRenderer = r;
    needCopyOnWrite = false;
}

QPixmap::QPixmap(const QString& MIME)
{
    imageRenderer = WebCoreImageRendererFactory::sharedFactory()->imageRendererWithMIMEType(MIME.latin1());
    MIMEType = MIME;
    needCopyOnWrite = false;
}

QPixmap::QPixmap(const QSize &sz)
{
    GdkRectangle rect;
    initGdkRectangleFromSize(&rect, sz);
    imageRenderer = WebCoreImageRendererFactory::sharedFactory()->imageRendererWithSize(&rect);
    needCopyOnWrite = false;
}

QPixmap::QPixmap(const QByteArray &bytes)
{
    imageRenderer = WebCoreImageRendererFactory::sharedFactory()->imageRendererWithBytes(bytes.data(), bytes.size());
    needCopyOnWrite = false;
}

QPixmap::QPixmap(const QByteArray &bytes, const QString &MIME)
{
    MIMEType = MIME;
    imageRenderer = WebCoreImageRendererFactory::sharedFactory()->imageRendererWithBytes(bytes.data(), bytes.size(), MIMEType.latin1());
}

QPixmap::QPixmap(int w, int h)
{
    GdkRectangle rect;
    initGdkRectangle(&rect, w, h);
    imageRenderer = WebCoreImageRendererFactory::sharedFactory()->imageRendererWithSize(&rect);
    needCopyOnWrite = false;
}

QPixmap::QPixmap(const QPixmap &copyFrom) : QPaintDevice(copyFrom)
{
    imageRenderer = copyFrom.imageRenderer;
    if (imageRenderer) { 
	imageRenderer->ref();
	copyFrom.needCopyOnWrite = true;
	needCopyOnWrite = true;
    }

    MIMEType = copyFrom.MIMEType;

}

QPixmap::~QPixmap()
{
    if (imageRenderer) imageRenderer->unref();
}

bool QPixmap::receivedData(const QByteArray &bytes, bool isComplete)
{
    if (!imageRenderer) {
	if (isComplete) {
	    imageRenderer = WebCoreImageRendererFactory::sharedFactory()->imageRendererWithBytes(bytes.data(), bytes.size(), MIMEType.latin1());
	    return imageRenderer->isNull();
	}

	imageRenderer = WebCoreImageRendererFactory::sharedFactory()->imageRendererWithMIMEType(MIMEType.utf8());
    }

    return imageRenderer->incrementalLoadWithBytes(bytes.data(), bytes.size(), isComplete);
}

bool QPixmap::mask() const
{
    return false;
}

bool QPixmap::isNull() const
{
    return !imageRenderer || imageRenderer->isNull();
}


QSize QPixmap::size () const
{
    if (!imageRenderer) return QSize(0, 0);
    GdkRectangle rect;
    imageRenderer->size(&rect);
    return QSize(&rect);
    
}

QRect QPixmap::rect () const
{
    return QRect(QPoint(0,0), size());
}

int QPixmap::width () const
{
    return size().width();
}

int QPixmap::height () const
{
    return size().height();
}

void QPixmap::resize(const QSize &sz)
{
    if (needCopyOnWrite) {
        WebCoreImageRenderer* newImageRenderer = imageRenderer->copy();
	newImageRenderer->ref();
	imageRenderer->unref();
        imageRenderer = newImageRenderer;
        needCopyOnWrite = false;
    }

    // FIXME: do copy here?
    GdkRectangle newSize;
    initGdkRectangleFromSize(&newSize, sz);
    imageRenderer->resize(&newSize);
}

void QPixmap::resize(int w, int h)
{
    resize(QSize(w,h));
}

QPixmap QPixmap::xForm(const QWMatrix &m) const {
    // This function is only called when an image needs to be scaled.
    // We can depend on render_image.cpp to call resize AFTER
    // creating a copy of the image to be scaled. So, this
    // implementation simply returns a copy of the image. Note,
    // this assumption depends on the implementation of
    // RenderImage::printObject.
    return *this;
}

QPixmap & QPixmap::operator=(const QPixmap &assignFrom)
{    
    WebCoreImageRenderer* oldImageRenderer = imageRenderer;
    imageRenderer = assignFrom.imageRenderer;
    if (imageRenderer) {
	imageRenderer->ref();
	assignFrom.needCopyOnWrite = true;
	needCopyOnWrite = true;
    }
    if (oldImageRenderer) oldImageRenderer->unref();

    MIMEType = assignFrom.MIMEType;
    return *this;
}

void QPixmap::increaseUseCount() const
{
    if (imageRenderer) imageRenderer->increaseUseCount();
}

void QPixmap::decreaseUseCount() const
{
    if (imageRenderer)  imageRenderer->decreaseUseCount();
}

void QPixmap::stopAnimations()
{
    if (imageRenderer) imageRenderer->stopAnimation();
}

void QPixmap::flushRasterCache()
{
    if (imageRenderer) imageRenderer->flushRasterCache();
}
