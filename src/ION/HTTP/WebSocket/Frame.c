#include "../../../pion.h"

zend_object_handlers ion_oh_ION_HTTP_WebSocket_Frame;
zend_class_entry * ion_ce_ION_HTTP_WebSocket_Frame;

zend_object * ion_http_websocket_frame_init(zend_class_entry * ce) {
    ion_http_websocket_frame * frame = ecalloc(1, sizeof(ion_http_websocket_frame));

    RETURN_INSTANCE(ION_HTTP_WebSocket_Frame, frame);
}

void ion_http_websocket_frame_free(zend_object * object) {
    zend_object_std_dtor(object);

    ion_http_websocket_frame * frame = get_object_instance(object, ion_http_websocket_frame);
    if(frame->body) {
        zend_string_release(frame->body);
    }
}

int ion_websocket_frame_header(websocket_parser * parser) {
    if(parser->data) {
        return 1;
    }
    zend_string * s = parser->data = zend_string_alloc(parser->length, 0);
    s->val[s->len] = '\000';
    return 0;
}

int ion_websocket_frame_body(websocket_parser * parser, const char *at, size_t size) {
    if(parser->flags & WS_HAS_MASK) {
        websocket_parser_copy_masked(((zend_string*)parser->data)->val, at, size, parser);
    } else {
        memcpy(((zend_string*)parser->data)->val, at, size);
    }
    return 0;
}

/** public function ION\HTTP\WebSocket\Frame::parse(string $raw) */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, parse) {
    zend_string      * raw = NULL;
    zend_object      * frame_object = NULL;
    websocket_parser * parser;
    websocket_parser_settings   settings;
    ion_http_websocket_frame  * frame = NULL;


    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(raw)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    frame_object = pion_new_object_arg_0(zend_get_called_scope(execute_data));
    RETVAL_OBJ(frame_object);
    if(!frame_object) {
        return;
    }
    frame = get_object_instance(frame_object, ion_http_websocket_frame);

    websocket_parser_settings_init(&settings);
    settings.on_frame_header = ion_websocket_frame_header;
    settings.on_frame_body = ion_websocket_frame_body;
    parser = emalloc(sizeof(websocket_parser));
    parser->data = NULL;
    websocket_parser_init(parser);
    websocket_parser_execute(parser, &settings, raw->val, raw->len);

    frame->flags = parser->flags;
    if(parser->data) {
        frame->body = parser->data;
    }
    if(frame->flags & WS_HAS_MASK) {
        frame->mask[0] = (unsigned char *)(uint32_t)parser->mask;
    }

    efree(parser);
}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, parse, 1)
    METHOD_ARG_STRING(frame, 0)
METHOD_ARGS_END();

CLASS_METHOD(ION_HTTP_WebSocket_Frame, factory) {

}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, factory, 1)
    METHOD_ARG_STRING(body, 0)
    METHOD_ARG_LONG(flags, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::withOpcode(int $opcode) : static */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, withOpcode) {
    zend_long opcode = 0;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(opcode)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    get_instance(getThis(), ion_http_websocket_frame)->flags = opcode & WS_OP_MASK;

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, withOpcode, 1)
    METHOD_ARG_LONG(opcode, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::getOpcode() : int */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, getOpcode) {
    RETURN_LONG(get_instance(getThis(), ion_http_websocket_frame)->flags & WS_OP_MASK);
}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, getOpcode);

/** public function ION\HTTP\Message::withBody(string $body) : static */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, withBody) {
    zend_string * body = NULL;
    ion_http_websocket_frame * frame = get_this_instance(ion_http_websocket_frame);

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(body)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(frame->body) {
        zend_string_release(frame->body);
    }
    frame->body = zend_string_copy(body);

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, withBody, 1)
    METHOD_ARG_STRING(body, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::getBody() : string */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, getBody) {
    ion_http_websocket_frame * frame = get_this_instance(ion_http_websocket_frame);

    if(frame->body) {
        RETURN_STR(zend_string_copy(frame->body));
    } else {
        RETURN_EMPTY_STRING();
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, getBody);

/** public function ION\HTTP\Message::withFinalFlag(bool $flag) : static */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, withFinalFlag) {
    zend_bool flag = true;
    ion_http_websocket_frame * frame = get_this_instance(ion_http_websocket_frame);

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_BOOL(flag)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(flag) {
        frame->flags |= WS_FIN;
    } else {
        frame->flags &= ~WS_FIN;
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, withFinalFlag, 1)
    METHOD_ARG_BOOL(flag, 0)
METHOD_ARGS_END();

/** public function ION\HTTP\Message::getFinalFlag() : bool */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, getFinalFlag) {
    ion_http_websocket_frame * frame = get_this_instance(ion_http_websocket_frame);

    if(frame->flags & WS_FIN) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, getFinalFlag);

/** public function ION\HTTP\Message::hasMasking() : bool */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, hasMasking) {
    ion_http_websocket_frame * frame = get_this_instance(ion_http_websocket_frame);

    if(frame->flags & WS_HAS_MASK) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, hasMasking);

/** public function ION\HTTP\Message::getMasking() : int */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, getMasking) {
    ion_http_websocket_frame * frame = get_this_instance(ion_http_websocket_frame);

    if(frame->flags & WS_HAS_MASK) {
        RETURN_LONG((zend_long)frame->mask);
    } else {
        RETURN_LONG(0);
    }
}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, getMasking);

/** public function ION\HTTP\Message::withMasking() : bool */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, withMasking) {
    zend_long mask = 0;
    ion_http_websocket_frame * frame = get_this_instance(ion_http_websocket_frame);

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(mask)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);

    if(mask) {
        frame->flags |= WS_HAS_MASK;
        frame->mask[0] = (unsigned char *)((uint32_t)mask);
    } else {
        frame->flags &= ~WS_HAS_MASK;
    }

    RETURN_THIS();
}

METHOD_ARGS_BEGIN(ION_HTTP_WebSocket_Frame, withMasking, 1)
    METHOD_ARG_LONG(mask, 0)
METHOD_ARGS_END();

void ion_websocket_parser_copy_masked(char * dst, const char * src, size_t len, char * mask) {
    size_t i = 0;
    for(; i < len; i++) {
        dst[i] = src[i] ^ mask[i % 4];
    }
}

#define COPY_DATA(dst, src, len, mask) mask ? ion_websocket_parser_copy_masked(dst, src, len, mask) : memcpy(dst, src, len)

/** public function ION\HTTP\WebSocket\Frame::parse(string $raw) */
CLASS_METHOD(ION_HTTP_WebSocket_Frame, build) {
    ion_http_websocket_frame * frame = get_this_instance(ion_http_websocket_frame);
    zend_string * raw;
    size_t        len = 2;


    if(frame->flags & WS_HAS_MASK) {
        len += 4;
    }
    if(frame->body->len > 125) {
        if(frame->body->len > 0xFFFF) {
            len += 8;
        } else {
            len += 2;
        }
    }
    len += frame->body->len;
    raw = zend_string_alloc(len, 0);

    raw->val[0] = 0;
    raw->val[1] = 0;
    if(frame->flags & WS_FIN) {
        raw->val[0] = (char) (1 << 7);
    }
    raw->val[0] |= frame->flags & WS_OP_MASK;
    if(frame->flags & WS_HAS_MASK) {
        raw->val[1] = (char) (1 << 7);
    }
    if(frame->body->len < 126) {
        raw->val[1] |= frame->body->len;
        COPY_DATA(&raw->val[2], frame->body->val, frame->body->len, frame->mask);
    } else {
        raw->val[2] = (unsigned char *) frame->body->len;
    }

}

METHOD_WITHOUT_ARGS(ION_HTTP_WebSocket_Frame, build)

CLASS_METHODS_START(ION_HTTP_WebSocket_Frame)
    METHOD(ION_HTTP_WebSocket_Frame, parse,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_WebSocket_Frame, factory, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    METHOD(ION_HTTP_WebSocket_Frame, withOpcode,    ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, getOpcode,     ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, withBody,      ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, getBody,       ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, withFinalFlag, ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, getFinalFlag,  ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, hasMasking,    ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, getMasking,    ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, withMasking,   ZEND_ACC_PUBLIC)
    METHOD(ION_HTTP_WebSocket_Frame, build,         ZEND_ACC_PUBLIC)
CLASS_METHODS_END;


PHP_MINIT_FUNCTION(ION_HTTP_WebSocket_Frame) {
    pion_register_class(ION_HTTP_WebSocket_Frame, "ION\\HTTP\\WebSocket\\Frame", ion_http_websocket_frame_init, CLASS_METHODS(ION_HTTP_WebSocket_Frame));
    pion_init_std_object_handlers(ION_HTTP_WebSocket_Frame);
    pion_set_object_handler(ION_HTTP_WebSocket_Frame, free_obj, ion_http_websocket_frame_free);

    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_CONT",  WS_OP_CONTINUE);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_TEXT",  WS_OP_TEXT);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_BIN",   WS_OP_BINARY);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_CLOSE", WS_OP_CLOSE);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_PING",  WS_OP_PING);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "OP_PONG",  WS_OP_PONG);

    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "FLAG_OPCODE",  WS_OP_MASK);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "FLAG_MASKING", WS_FIN);
    PION_CLASS_CONST_LONG(ION_HTTP_WebSocket_Frame, "FLAG_FINAL",   WS_HAS_MASK);

    return SUCCESS;
}