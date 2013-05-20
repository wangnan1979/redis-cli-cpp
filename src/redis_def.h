#ifndef __REDIS_DEF_H__
#define __REDIS_DEF_H__

#ifdef _WIN32
#include <WinDef.h>
#define NEW_ERR(a,b) MAKEWORD(a,b)
#define ERR_TYPE(w) HIBYTE(w)
#define ERR_NUM(w) LOBYTE(w)
#endif

#define ERR_REDIS_CONTEX 0x1
#define ERR_REDIS_REPLY 0x2


#endif
