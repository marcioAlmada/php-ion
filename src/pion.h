
#ifndef ION_FRAMEWORK_H
#define ION_FRAMEWORK_H

#include "pion/exceptions.h"
#include "pion/debug.h"
#include "pion/callback.h"
#include "pion/engine.h"

/** main structure (class ION) */
typedef struct _ion_base {
    struct event_base *base;     // event base
    struct evdns_base *evdns;    // event DNS base
    struct event_config *config; // event config
//    zval *dns;                   // DNS instance
    long  i;                     // internal counter of timers
    HashTable *signals;          // array of listening signals
    HashTable *timers;           // array of timers
    HashTable *execs;            // array of process childs
    short has_fatals;            // flag, fatal error occured
    struct event *sigsegv;
//    pionLList *queue;                // queue of defers object
#ifdef ZTS
    TSRMLS_D;
#endif
} IONBase;

#define ION(prop) \
    ionBase->prop

/* Fetch FD from ZVAL resource */
int pionGetFdFromStream(zval* zfd TSRMLS_DC);

/**
 * For debug
 */

#endif //ION_FRAMEWORK_H
