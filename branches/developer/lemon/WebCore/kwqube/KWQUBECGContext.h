#ifndef KWQUBECGContext_h
#define KWQUBECGContext_h

#include <InterfaceKit.h>
//#include <gdk/gdk.h>

class CGContext
{
public:
    //GdkDrawable* drawable;
    //GdkGC* gc;
    //GdkRegion* clip;
    BRegion *clip;
    BView *view;
    virtual ~CGContext() {};
    virtual void addClip(BRect* rect) = 0;
    virtual void saveGraphicsState() = 0;
    virtual void restoreGraphicsState() = 0;
};

typedef CGContext* CGContextRef;

enum NSCompositingOperation {
    NSCompositeClear = 0,
    NSCompositeCopy,
    NSCompositeSourceOver ,
    NSCompositeSourceIn,
    NSCompositeSourceOut, 
    NSCompositeSourceAtop, 
    NSCompositeDestinationOver,
    NSCompositeDestinationIn,
    NSCompositeDestinationOut,
    NSCompositeDestinationAtop,
    NSCompositeXOR,
    NSCompositePlusDarker,
    NSCompositeHighlight,
    NSCompositePlusLighter
};

#endif
