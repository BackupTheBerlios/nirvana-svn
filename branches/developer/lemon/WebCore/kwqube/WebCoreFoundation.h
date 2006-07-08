#ifndef WebCoreFoundation_h
#define WebCoreFoundation_h

class WebCoreNSObject
{
public:
    WebCoreNSObject():refCnt(0) {}

    virtual ~WebCoreNSObject() {}

    virtual WebCoreNSObject* retain()
    {
	++refCnt;
	return this;
    }
    
    virtual void release()
    {
      if (--refCnt == 0) 
	  delete this;
    }
protected:
    int refCnt;
};
  

#endif
