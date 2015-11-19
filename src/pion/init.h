#ifndef PION_INIT_H
#define PION_INIT_H

#include "config.h"
#include <php.h>
#include <event.h>
#include "engine.h"

#ifdef PHP_WIN32
#	define ION_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define ION_API __attribute__ ((visibility("default")))
#else
#	define ION_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#if defined(COMPILE_DL_ION) && defined(ZTS)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

typedef struct event          ion_event;
typedef struct evconnlistener ion_evlistener;
typedef struct evbuffer       ion_evbuffer;
typedef struct bufferevent    ion_buffer;
typedef struct event_base     ion_event_base;
typedef struct event_config   ion_event_config;
typedef struct evdns_base     ion_evdns_base;

//#define ION(prop) \
//    ionBase->prop

//#define ion_loop_break() event_base_loopbreak(ION(base))

#define ION_LOOP_CB_BEGIN()
#define ION_LOOP_CB_END() ION_CHECK_LOOP()

#define ION_CHECK_LOOP()                 \
    if(EG(exception)) {                  \
        event_base_loopbreak(GION(base)); \
    }

#define SET_TIMEVAL(tval, dval)                          \
    tval.tv_usec = (int)((int)(dval*1000000) % 1000000); \
    tval.tv_sec = (int)dval;

ZEND_BEGIN_MODULE_GLOBALS(ion)
    // base
    ion_event_base   * base;    // event base
    ion_event_config * config;  // event config
    uint               flags;
    HashTable        * timers;  // array of timers

    // DNS
    ion_evdns_base * evdns;      // event dns base
    HashTable      * resolvers;  // resolve requests

    // Process
    HashTable * signals;     // registered signals
    HashTable * childs;      // spawned workers

    // FS
    int         watch_fd;    // inotify or kqueue file descriptor
    ion_event * watch_event; // watch_fd listener
    HashTable * watchers;    // list of listened filenames
ZEND_END_MODULE_GLOBALS(ion)

ZEND_EXTERN_MODULE_GLOBALS(ion);

// ION globals access
#define GION(v) ZEND_MODULE_GLOBALS_ACCESSOR(ion, v)


#endif //PION_INIT_H
