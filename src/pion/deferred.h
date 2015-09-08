#ifndef PION_DEFERRED_H
#define PION_DEFERRED_H

#include <php.h>

#define ION_DEFERRED_DONE      1<<0
#define ION_DEFERRED_FAILED    1<<1
#define ION_DEFERRED_REJECTED  1<<2

#define ION_DEFERRED_FINISHED  (ION_DEFERRED_DONE | ION_DEFERRED_FAILED)

#define ION_DEFERRED_INTERNAL  1<<3
#define ION_DEFERRED_TIMED_OUT 1<<4

typedef void (*deferred_reject_callback)(zval *error, zval * zdeferred TSRMLS_DC);
typedef void (*deferred_object_dtor)(void *object, zval * zdeferred TSRMLS_DC);

zval * _ion_deferred_new(zval *z_reject_cb TSRMLS_DC);
zval * _ion_deferred_new_ex(deferred_reject_callback reject_cb TSRMLS_DC);
int    _ion_deferred_zval(zval * zvariable, void *object, deferred_object_dtor dtor, deferred_reject_callback reject_cb TSRMLS_DC);
void   _ion_deferred_store(zval * zdeferred, void *object, deferred_object_dtor dtor TSRMLS_DC);
void * _ion_deferred_store_get(zval * zdeferred TSRMLS_DC);
void   _ion_deferred_resolve(zval * zdeferred, zval * zresult, short type TSRMLS_DC);
void   _ion_deferred_done_long(zval * zdeferred, long * lval TSRMLS_DC);
void   _ion_deferred_done_bool(zval * zdeferred, zend_bool bval TSRMLS_DC);
void   _ion_deferred_done_stringl(zval * zdeferred, char * str, long length, int dup TSRMLS_DC);
void   _ion_deferred_done_empty_string(zval * zdeferred TSRMLS_DC);
void   _ion_deferred_exception(zval * zdeferred, zend_class_entry * ce, const char * message, long code TSRMLS_DC);
void   _ion_deferred_exception_eg(zval * zdeferred TSRMLS_DC);
void   _ion_deferred_exception_ex(zval * zdeferred, zend_class_entry * ce, long code TSRMLS_DC, const char * message, ...);
void   _ion_deferred_reject(zval *zdeferred, const char *message TSRMLS_DC);
void   _ion_deferred_free(zval *zdeferred TSRMLS_DC);
int    _ion_deferred_dequeue(TSRMLS_D);

#define ion_deferred_new(zcancel_cb)                        _ion_deferred_new(cancel_cb TSRMLS_CC)
#define ion_deferred_new_ex(cancel_cb)                      _ion_deferred_new_ex(cancel_cb TSRMLS_CC)
#define ion_deferred_new_void()                             _ion_deferred_new_ex(NULL TSRMLS_CC)
#define ion_deferred_zval(zvar, object, dtor, cancel_cb)    _ion_deferred_zval(zvar, object, dtor, cancel_cb TSRMLS_CC)
#define ion_deferred_store(zdeferred, object, object_dtor)  _ion_deferred_store(zdeferred, (void *) object, object_dtor TSRMLS_CC)
#define ion_deferred_store_get(zdeferred)                   _ion_deferred_store_get(zdeferred TSRMLS_CC)

#define ion_deferred_done(zdeferred, zresult)               _ion_deferred_resolve(zdeferred, zresult, ION_DEFERRED_DONE TSRMLS_CC)
#define ion_deferred_done_long(zdeferred, num)              _ion_deferred_done_long(zdeferred, num TSRMLS_CC)
#define ion_deferred_done_stringl(zdeferred, str, len, dup) _ion_deferred_done_stringl(zdeferred, str, len, dup TSRMLS_CC)
#define ion_deferred_done_empty_string(zdeferred)           _ion_deferred_done_empty_string(zdeferred TSRMLS_CC)
#define ion_deferred_done_bool(zdeferred, bval)             _ion_deferred_done_bool(zdeferred, bval TSRMLS_CC)
#define ion_deferred_done_true(zdeferred)                   ion_deferred_done_bool(zdeferred, 1)
#define ion_deferred_done_false(zdeferred)                  ion_deferred_done_bool(zdeferred, 0)
#define ion_deferred_fail(zdeferred, zexception)            _ion_deferred_resolve(zdeferred, zexception, ION_DEFERRED_FAILED TSRMLS_CC)
#define ion_deferred_exception(zdeferred, ce, message, code)          \
    _ion_deferred_exception(zdeferred, ce, message, code TSRMLS_CC)
#define ion_deferred_exception_eg(zdeferred)                _ion_deferred_exception_eg(zdeferred TSRMLS_CC)
#define ion_deferred_exception_ex(zdeferred, ce, code, message, ...)  \
    _ion_deferred_exception_ex(zdeferred, ce, code TSRMLS_CC, message ##__VA_ARGS__)

#define ion_deferred_reject(zdeferred, message)             _ion_deferred_reject(zdeferred, message TSRMLS_CC)
#define ion_deferred_free(zdeferred)                        _ion_deferred_free(zdeferred TSRMLS_CC)
#endif //PION_DEFERRED_H
