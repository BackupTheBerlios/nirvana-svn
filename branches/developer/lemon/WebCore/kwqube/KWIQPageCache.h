#ifndef KWIQPageCache_h
#define KWIQPageCache_h

class KWQPageState;

// interface for core class KWQPageState so component library can
// destroy the objects Actual object will be inside the core.
class KWIQPageState
{
 public:  
  virtual ~KWIQPageState() {};    
};


// interface for component library so core can obtain objects
// actually more sort of a KWIQPageCacheEntry, but
// actual object will be inside component library
class KWIQPageCache
{

public:
  virtual ~KWIQPageCache(){};
  virtual KWIQPageState* state() =0;

};


#endif
