#include "ion.h"

zend_object_handlers ion_oh_ION_Process_IPC;
zend_class_entry * ion_ce_ION_Process_IPC;
//websocket_parser_settings parser_settings;

zend_object * ion_process_ipc_init(zend_class_entry * ce) {
    ion_process_ipc * ipc = ion_alloc_object(ce, ion_process_ipc);
    ipc->parser = ecalloc(1, sizeof(websocket_parser));
    websocket_parser_init(ipc->parser);
    ipc->parser->data = ipc;
    ZVAL_NULL(&ipc->ctx);
    return ion_init_object(ION_OBJECT_ZOBJ(ipc), ce, &ion_oh_ION_Process_IPC);
}


void ion_process_ipc_free(zend_object * object) {
    ion_process_ipc * ipc = ION_ZOBJ_OBJECT(object, ion_process_ipc);
    zend_object_std_dtor(object);
    if(ipc->on_message) {
        ion_object_release(ipc->on_message);
    }
    if(ipc->on_disconnect) {
        ion_object_release(ipc->on_disconnect);
        ipc->on_disconnect = NULL;
    }
    if(ipc->parser) {
        efree(ipc->parser);
        ipc->parser = NULL;
    }
    if(ipc->buffer) {
        bufferevent_disable(ipc->buffer, EV_READ | EV_WRITE);
        bufferevent_free(ipc->buffer);
    }
//    if(ipc->flags & ION_IPC_CTX_RELEASE) {
//        PHPDBG("relase");
        zval_ptr_dtor(&ipc->ctx);
//    }
}


//int ion_process_ipc_message_begin(websocket_parser * parser) {
//    ion_process_ipc * ipc = parser->data;
//
//    if(parser->length) {
//        ipc->frame_body = zend_string_alloc(parser->length, 0);
//    }
//    return 0;
//}

void ion_process_ipc_notification(ion_buffer * bev, short what, void * ctx) {
    ION_CB_BEGIN();
    ion_process_ipc * ipc = ctx;

    if(what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        if(ipc->on_disconnect) {
            ion_promisor_done(ipc->on_disconnect, &ipc->ctx);
        }
        ipc->flags &= ~ION_IPC_CONNECTED;
    }
    ION_CB_END();
}

int ion_ipc_create(zval * one, zval * two, zval * ctx1, zval * ctx2, uint32_t flags) {
    ion_buffer * buffer_one = NULL;
    ion_buffer * buffer_two = NULL;
    ion_process_ipc * ipc1;
    ion_process_ipc * ipc2;
    if(!ion_buffer_pair(&buffer_one, &buffer_two)) {
        return FAILURE;
    }
    object_init_ex(one, ion_ce_ION_Process_IPC);
    object_init_ex(two, ion_ce_ION_Process_IPC);
    ipc1 = ION_ZVAL_OBJECT_P(one, ion_process_ipc);
    ipc2 = ION_ZVAL_OBJECT_P(two, ion_process_ipc);
    bufferevent_setcb(buffer_one, ion_process_ipc_incoming, NULL, ion_process_ipc_notification, ipc1);
    bufferevent_setcb(buffer_two, ion_process_ipc_incoming, NULL, ion_process_ipc_notification, ipc2);
    ipc1->buffer = buffer_one;
    ipc2->buffer = buffer_two;
    ipc1->flags |= ION_IPC_CONNECTED | flags;
    ipc2->flags |= ION_IPC_CONNECTED | flags;
    if(ctx1) {
        ZVAL_COPY(&ipc1->ctx, ctx1);
    }
    if(ctx2) {
        ZVAL_COPY(&ipc2->ctx, ctx2);
    }

    return SUCCESS;
}

/** public function ION\Process\IPC::create(mixed $ctx1 = null, mixed $ctx2 = null) : ION\IPC[] */
CLASS_METHOD(ION_Process_IPC, create) {
    zval one;
    zval two;
    zval * ctx1 = NULL;
    zval * ctx2 = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 2)
        Z_PARAM_OPTIONAL
        Z_PARAM_ZVAL(ctx1)
        Z_PARAM_ZVAL(ctx2)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(ion_ipc_create(&one, &two, ctx1, ctx2, ION_IPC_CTX_RELEASE) == FAILURE) {
        zend_throw_exception(ion_ce_ION_RuntimeException, ERR_ION_PROCESS_IPC_FAIL, 0);
        return;
    }
    zval_add_ref(ctx1);
    zval_add_ref(ctx2);
    array_init(return_value);
    add_next_index_zval(return_value, &one);
    add_next_index_zval(return_value, &two);
}

METHOD_WITHOUT_ARGS(ION_Process_IPC, create);

/** public function ION\Process\IPC::isConnected() : bool */
CLASS_METHOD(ION_Process_IPC, isConnected) {
    ion_process_ipc * ipc = ION_THIS_OBJECT(ion_process_ipc);

    if(ipc->flags & ION_IPC_CONNECTED) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_Process_IPC, isConnected);

/** public function ION\Process\IPC::whenIncoming() : ION\Sequence */
CLASS_METHOD(ION_Process_IPC, whenIncoming) {
    ion_process_ipc * ipc = ION_THIS_OBJECT(ion_process_ipc);

    if(!ipc->on_message) {
        ipc->on_message = ion_promisor_new(ion_ce_ION_Sequence, 0);
    }
    ion_object_addref(ipc->on_message);
    RETURN_ION_OBJ(ipc->on_message);
}

METHOD_WITHOUT_ARGS(ION_Process_IPC, whenIncoming);

/** public function ION\Process\IPC::whenDisconnected() : ION\Promise */
CLASS_METHOD(ION_Process_IPC, whenDisconnected) {
    ion_process_ipc * ipc = ION_THIS_OBJECT(ion_process_ipc);

    if(!ipc->on_disconnect) {
        ipc->on_disconnect = ion_promisor_new(ion_ce_ION_Promise, ION_PROMISOR_INTERNAL);
    }
    ion_object_addref(ipc->on_disconnect);
    RETURN_ION_OBJ(ipc->on_disconnect);
}

METHOD_WITHOUT_ARGS(ION_Process_IPC, whenDisconnected);

/** public function ION\Process\IPC::send(string $data) : self */
CLASS_METHOD(ION_Process_IPC, send) {
    ion_process_ipc * ipc = ION_THIS_OBJECT(ion_process_ipc);
    zend_string * data = NULL;
    size_t frame_size;
    char * frame;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    frame_size = websocket_calc_frame_size(ION_IPC_FIN | ION_IPC_DATA, ZSTR_LEN(data));
    frame = emalloc(frame_size);
    websocket_build_frame(frame, ION_IPC_FIN | ION_IPC_DATA, NULL, ZSTR_VAL(data), ZSTR_LEN(data));
    bufferevent_write(ipc->buffer, frame, frame_size);
    efree(frame);
    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_Process_IPC, send, 1)
    ARGUMENT(name, IS_STRING)
METHOD_ARGS_END();

/** public function ION\Process\IPC::getContext() : mixed */
CLASS_METHOD(ION_Process_IPC, getContext) {
    ion_process_ipc * ipc = ION_THIS_OBJECT(ion_process_ipc);
    if(!Z_ISUNDEF(ipc->ctx)) {
        if(!Z_ISREF(ipc->ctx)) {
            zval ref;
            ZVAL_NEW_EMPTY_REF(&ref);
            ZVAL_COPY_VALUE(Z_REFVAL(ref), &ipc->ctx);
            ZVAL_COPY_VALUE(return_value, &ref);
        } else {
            ZVAL_COPY(return_value, &ipc->ctx);
        }
    }
}

METHOD_WITHOUT_ARGS_RETURN(ION_Process_IPC, getContext, RET_REF);

METHODS_START(methods_ION_Process_IPC)
    METHOD(ION_Process_IPC, create,         ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_Process_IPC, isConnected,    ZEND_ACC_PUBLIC)
    METHOD(ION_Process_IPC, whenIncoming,       ZEND_ACC_PUBLIC)
    METHOD(ION_Process_IPC, whenDisconnected,   ZEND_ACC_PUBLIC)
    METHOD(ION_Process_IPC, send,           ZEND_ACC_PUBLIC)
    METHOD(ION_Process_IPC, getContext,     ZEND_ACC_PUBLIC)
METHODS_END;

PHP_MINIT_FUNCTION(ION_Process_IPC) {
    ion_register_class(ion_ce_ION_Process_IPC, "ION\\Process\\IPC", ion_process_ipc_init, methods_ION_Process_IPC);
    ion_init_object_handlers(ion_oh_ION_Process_IPC);
    ion_oh_ION_Process_IPC.free_obj = ion_process_ipc_free;
    ion_oh_ION_Process_IPC.clone_obj = NULL;
    ion_oh_ION_Process_IPC.offset = ion_offset(ion_process_ipc);

    return SUCCESS;
}
