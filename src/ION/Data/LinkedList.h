#ifndef ION_LINKEDLIST_H
#define ION_LINKEDLIST_H

#include <php.h>
#include <zend.h>
#include "../../pion.h"
#include "../../pion/linkedlist.h"

// LinkedList PHP object
typedef struct _php_linked_list {
    zend_object std;
    pion_llist *list;
    long        count;
    long        key;
    pion_llist_item *current;
} ion_linked_list;


PHP_MINIT_FUNCTION(ION_Data_LinkedList);
DEFINE_CLASS(ION_Data_LinkedList);

PHP_METHOD(ION_Data_LinkedList, rPush);
PHP_METHOD(ION_Data_LinkedList, lPush);
PHP_METHOD(ION_Data_LinkedList, rPop);
PHP_METHOD(ION_Data_LinkedList, lPop);
PHP_METHOD(ION_Data_LinkedList, count);
PHP_METHOD(ION_Data_LinkedList, rewind);
PHP_METHOD(ION_Data_LinkedList, current);
PHP_METHOD(ION_Data_LinkedList, key);
PHP_METHOD(ION_Data_LinkedList, next);
PHP_METHOD(ION_Data_LinkedList, valid);

#endif //ION_LINKEDLIST_H
