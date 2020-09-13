#ifndef _X_THREAD_H_2018_12_18
#define _X_THREAD_H_2018_12_18

#include "xmtdef.h"
#include "base/tstring.h"
#ifdef XMT_WIN_32

#else
#include <pthread.h>
#endif  // XMT_WIN_32

NAMESPACE_BEGIN

//! The base class of thread. declare the base operation of thread for
//! cross-platform
class TThreadBase {
public:
    TThreadBase(void);
    virtual ~TThreadBase(void) {}

    enum THREAD_PRIORITY {
        PRIORITY_HIGHEST = 4,        //
        PRIORITY_ABOVE_NORMAL = 2,   //
        PRIORITY_NORMAL = 1,         //
        PRIORITY_BELOW_NORMAL = -2,  //
        PRIORITY_LOWEST = -4         //
    };

protected:
    //! define the callback
#if defined(XMT_WIN_32)
#define ThreadRetCode void*
    typedef void* (*ThreadCallBack)(void* args);
    typedef unsigned int typeThreadID;
#else
#define ThreadRetCode void*
    typedef void* (*ThreadCallBack)(void* args);
    typedef pthread_t typeThreadID;
#endif
    typeThreadID m_nThraedID;

    virtual void start(void);  //
};

class TThread : public TThreadBase {
public:
    TThread(void);

    TThread(ThreadCallBack Func);
};

NAMESPACE_END
#endif
