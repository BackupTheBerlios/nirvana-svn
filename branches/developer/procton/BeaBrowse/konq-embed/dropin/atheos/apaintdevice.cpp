
#include <stdio.h>

#include "apaintdevice.h"

#include <kdebug.h>

QPaintDevice::~QPaintDevice()
{
	kdDebug(10) << "QPaintDevice::" << __FUNCTION__ << "()" << endl;
  printf( "Warning: QPaintDevice::~QPaintDevice() not implemented\n" );
}
