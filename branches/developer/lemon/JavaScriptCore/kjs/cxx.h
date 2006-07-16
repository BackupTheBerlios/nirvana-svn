#ifndef CXX_H
#define CXX_H

#include "KWIQMacros.h"

namespace std{

inline int min(int a, int b)
{ 
    return a < b? a:b;
}

inline int max(int a, int b)
{ 
    return a > b ? a : b;
}

template<class T>
inline void swap(T &a, T &b){
    T tmp = b;
    b=a;
    a=tmp;
}

}

#endif

