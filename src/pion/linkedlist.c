#include "linkedlist.h"
#include <php.h>


pionLList *pionLListInit() {
    pionLList *list = emalloc(sizeof(pionLList));
    list->tail = list->head = NULL;
    return list;
}

void pionLListFree(pionLList *list) {
    efree(list);
}

void pionLListRPush(pionLList *list, void *data) {
    pionLListItem *item = emalloc(sizeof(pionLListItem));
    item->data = data;
    item->next  = NULL;
    if(list->tail) { // append item
        item->prev = list->tail;
        list->tail->next = item;
        list->tail = item;
    } else { // push first item
        item->prev = NULL;
        list->tail = list->head = item;
    }
}

void pionLListLPush(pionLList *list, void *data) {
    pionLListItem *item = emalloc(sizeof(pionLListItem));
    item->data = data;
    item->prev  = NULL;
    if(list->head) { // prepend item
        item->next = list->head;
        list->head->prev = item;
        list->head = item;
    } else { // push first item
        item->next = NULL;
        list->tail = list->head = item;
    }

}

void* pionLListLPop(pionLList *list) {
    pionLListItem *item;
    void *data = NULL;
    if(list->head == NULL) {
        return NULL;
    }
    item = list->head;
    if(item->next) { // pop item
        list->head = item->next;
        item->next->prev = NULL;
    } else { // pop last item
        list->head = list->tail = NULL;
    }
    data = item->data;
    efree(item);
    return data;
}

void* pionLListRPop(pionLList *list) {
    pionLListItem *item;
    void *data = NULL;
    if(list->tail == NULL) {
        return NULL;
    }
    item = list->tail;
    if(item->prev) { // pop item
        list->tail = item->prev;
        item->prev->next = NULL;
    } else { // pop last item
        list->head = list->tail = NULL;
    }
    data = item->data;
    efree(item);
    return data;
}