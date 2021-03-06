#include "ion.h"
//#include "exceptions.h"


pion_cb * pion_cb_create(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr) {
    pion_cb *cb = emalloc(sizeof(pion_cb));
    cb->fci = emalloc(sizeof(zend_fcall_info));
    cb->fcc = emalloc(sizeof(zend_fcall_info_cache));

    memcpy(cb->fci, fci_ptr, sizeof(zend_fcall_info));
    memcpy(cb->fcc, fcc_ptr, sizeof(zend_fcall_info_cache));
    Z_TRY_ADDREF(cb->fci->function_name);
    cb->fci->param_count = 0;
    cb->fci->no_separation = 1;
    cb->fci->retval = NULL;

    return cb;
}

pion_cb * pion_cb_create_from_zval(zval * zcb) {
    pion_cb *cb = emalloc(sizeof(pion_cb));
    char *is_callable_error = NULL;
    memset(cb, 0, sizeof(pion_cb));
    cb->fci = emalloc(sizeof(zend_fcall_info));
    cb->fcc = emalloc(sizeof(zend_fcall_info_cache));

    *cb->fci = empty_fcall_info;
    *cb->fcc = empty_fcall_info_cache;
    cb->fci->param_count = 0;
    cb->fci->params = NULL;
    Z_TRY_ADDREF_P(zcb);

    if (zend_fcall_info_init(zcb, IS_CALLABLE_CHECK_NO_ACCESS | IS_CALLABLE_CHECK_SILENT, cb->fci, cb->fcc, NULL, &is_callable_error) == SUCCESS) {
        if (is_callable_error) {
            efree(is_callable_error);
            return NULL;
        } else {
            return cb;
        }
    } else {
        if (is_callable_error) {
            efree(is_callable_error);
        }
        return NULL;
    }
}

pion_cb * pion_cb_fetch_method_ce(zend_class_entry * ce, const char * method_name) {
    pion_cb * cb;
    zend_function * fptr;
    char * function_name_lc = zend_str_tolower_dup(method_name, (int) strlen(method_name));

    fptr = zend_hash_str_find_ptr(&ce->function_table, function_name_lc, (int) strlen(method_name));

    if(fptr == NULL) {
        efree(function_name_lc);
        return NULL;
    }
    efree(function_name_lc);
    cb = emalloc(sizeof(pion_cb));
    cb->fci = emalloc(sizeof(empty_fcall_info));
    cb->fcc = emalloc(sizeof(empty_fcall_info_cache));
    *cb->fci = empty_fcall_info;
    *cb->fcc = empty_fcall_info_cache;
    cb->fci->size = sizeof(zend_fcall_info);
    ZVAL_UNDEF(&cb->fci->function_name);
    cb->fci->object        = NULL;
    cb->fci->retval        = NULL;
    cb->fci->param_count   = 0;
    cb->fci->params        = NULL;
    cb->fci->no_separation = 1;

#ifdef HAS_FCC_INITIALIZED
    cb->fcc->initialized      = 1;
#endif
    cb->fcc->function_handler = fptr;
    cb->fcc->calling_scope    = NULL;
    cb->fcc->called_scope     = ce;
    cb->fcc->object           = NULL;

    return cb;
}

pion_cb * pion_cb_create_from_object(zend_object * object, const char * method_name) {
    pion_cb * cb = pion_cb_fetch_method_ce(object->ce, method_name);
    cb->fci->object = object;
    zend_object_addref(object);
    return cb;
}

pion_cb * pion_cb_fetch_method(const char * class_name, const char * method_name) {
    zend_class_entry *ce;
    zend_string * class_name_zs = zend_string_init(class_name, strlen(class_name), 0);

    ce = zend_lookup_class(class_name_zs);
    zend_string_free(class_name_zs);

    if (ce == NULL) {
        return NULL;
    }

    return pion_cb_fetch_method_ce(ce, method_name);
}


void pion_cb_release(pion_cb * cb) {
    pion_cb_free(cb);
}

void pion_cb_free(pion_cb *cb) {
    zval_ptr_dtor(&cb->fci->function_name);
//    if(cb->fci->object) {
//        OBJ_ADDREF(cb->fci->object);
//        obj_ptr_dtor(cb->fci->object);
//    }
    efree(cb->fcc);
    efree(cb->fci);
    efree(cb);
}

pion_cb * pion_cb_dup(pion_cb * proto) {
    pion_cb * cb = emalloc(sizeof(pion_cb));
    cb->fci = emalloc(sizeof(empty_fcall_info));
    cb->fcc = emalloc(sizeof(empty_fcall_info_cache));
    *cb->fci = empty_fcall_info;
    *cb->fcc = empty_fcall_info_cache;

    cb->fci->size = sizeof(zend_fcall_info);
    if(Z_ISUNDEF(proto->fci->function_name)) {
        ZVAL_UNDEF(&proto->fci->function_name);
    } else {
        ZVAL_COPY(&cb->fci->function_name, &proto->fci->function_name);
    }
    cb->fci->retval        = NULL;
    cb->fci->param_count   = 0;
    cb->fci->params        = NULL;
    cb->fci->no_separation = proto->fci->no_separation;

    if(proto->fci->object) {
        zend_object_addref(proto->fci->object);
        cb->fci->object = proto->fci->object;
//        zval obj_from, obj_to;
//        ZVAL_OBJ(&obj_from, cb->fci->object);
//        ZVAL_COPY(&obj_to, &obj_from);
//        cb->fci->object = Z_OBJ(obj_to);
    }

    // fcc
#ifdef HAS_FCC_INITIALIZED
    cb->fcc->initialized = proto->fcc->initialized;
#endif
    cb->fcc->function_handler = proto->fcc->function_handler;
    cb->fcc->calling_scope = proto->fcc->calling_scope;
    cb->fcc->called_scope = proto->fcc->called_scope;


    if(proto->fcc->object) {
        if(proto->fci->object == proto->fcc->object) {
            cb->fcc->object = cb->fci->object;
        } else {
            zend_object_addref(proto->fcc->object);
            cb->fcc->object = proto->fcc->object;
//            zval obj_from, obj_to;
//            ZVAL_OBJ(&obj_from, cb->fcc->object);
//            ZVAL_COPY(&obj_to, &obj_from);
//            cb->fcc->object = Z_OBJ(obj_to);
        }
    }

    return cb;
}


static zend_bool pion_parse_arg_double_weak(zval *arg) /* {{{ */
{
    if (EXPECTED(Z_TYPE_P(arg) == IS_LONG)) {
        return true;
    } else if (EXPECTED(Z_TYPE_P(arg) == IS_STRING)) {
        zend_long l;
        double d;
        int type;

        if (UNEXPECTED((type = is_numeric_string_ex(ZSTR_VAL(Z_STR_P(arg)), ZSTR_LEN(Z_STR_P(arg)), &l, &d, 0, NULL)) != IS_LONG)) {
            if (EXPECTED(type != 0)) {
                return true;
            } else {
                return false;
            }
        }
    } else if (EXPECTED(Z_TYPE_P(arg) < IS_TRUE || Z_TYPE_P(arg) == IS_TRUE)) {
        return true;
    } else {
        return false;
    }
    return true;
}

static zend_bool pion_parse_arg_long_weak(zval *arg) {
    if (EXPECTED(Z_TYPE_P(arg) == IS_DOUBLE)) {
        if (UNEXPECTED(zend_isnan(Z_DVAL_P(arg)))) {
            return false;
        }
        if (UNEXPECTED(!ZEND_DOUBLE_FITS_LONG(Z_DVAL_P(arg)))) {
            return false;
        }
    } else if (EXPECTED(Z_TYPE_P(arg) == IS_STRING)) {
        double d;
        zend_long l;
        int type;


        if (UNEXPECTED((type = is_numeric_string_ex(ZSTR_VAL(Z_STR_P(arg)), ZSTR_LEN(Z_STR_P(arg)), &l, &d, 1, NULL)) != IS_LONG)) {
            if (EXPECTED(type != 0)) {
                if (UNEXPECTED(zend_isnan(d))) {
                    return false;
                }
                if (UNEXPECTED(!ZEND_DOUBLE_FITS_LONG(d))) {
                    return false;
                }
            } else {
                return false;
            }
        }
    } else if (EXPECTED(Z_TYPE_P(arg) < IS_TRUE || Z_TYPE_P(arg) == IS_TRUE)) {
        return true;
    } else {
        return false;
    }
    return true;
}

static zend_bool pion_verify_weak_scalar_type_hint(zend_uchar type_hint, zval * arg) {
    switch (type_hint) {
        case _IS_BOOL:
            if (EXPECTED(Z_TYPE_P(arg) <= IS_STRING)) {
                return true;
            } else {
                return false;
            }
        case IS_LONG:
            return pion_parse_arg_long_weak(arg);
        case IS_DOUBLE: {
            return pion_parse_arg_double_weak(arg);
        }
        case IS_STRING: {
            if(EXPECTED(Z_TYPE_P(arg) == IS_OBJECT)) {
                if(EXPECTED(zend_hash_str_exists(&Z_OBJCE_P(arg)->function_table, "__tostring", sizeof("__tostring")-1) == false)) {
                    return false;
                }
            } else if(EXPECTED(Z_TYPE_P(arg) >= IS_ARRAY)) {
                return false;
            }
            return true;
        }
        default:
            return false;
    }
}

static zend_bool pion_verify_scalar_type_hint(zend_uchar type_hint, zval *arg, zend_bool strict) {
    if (UNEXPECTED(strict)) {
        /* SSTH Exception: IS_LONG may be accepted as IS_DOUBLE (converted) */
        if (type_hint != IS_DOUBLE || Z_TYPE_P(arg) != IS_LONG) {
            return SUCCESS;
        }
    } else if (UNEXPECTED(Z_TYPE_P(arg) == IS_NULL)) {
        /* NULL may be accepted only by nullable hints (this is already checked) */
        return SUCCESS;
    }
    return pion_verify_weak_scalar_type_hint(type_hint, arg);
}

zend_class_entry * zend_fetch_class_ex(const char * class_name, int fetch_type) {
    zend_string * key;
    zend_class_entry *ce;
    ALLOCA_FLAG(use_heap);

    ZSTR_ALLOCA_INIT(key, class_name, strlen(class_name), use_heap);
    ce = zend_fetch_class(key, fetch_type);
    ZSTR_ALLOCA_FREE(key, use_heap);
    return ce;
}

static zend_always_inline zend_bool pion_verify_arg_type_user(pion_cb * cb, uint32_t arg_num, zval * arg)
{
    zend_arg_info *cur_arg_info;
    zend_class_entry *ce;
    zend_string * class_name;
    zend_function *zf = cb->fcc->function_handler;

    if (EXPECTED(arg_num <= zf->common.num_args)) {
        cur_arg_info = &zf->common.arg_info[arg_num];
    } else if (UNEXPECTED(zf->common.fn_flags & ZEND_ACC_VARIADIC)) {
        cur_arg_info = &zf->common.arg_info[zf->common.num_args];
    } else {
        return true;
    }

    if (ION_TYPE_IS_SET(cur_arg_info)) {
        ZVAL_DEREF(arg);
        if (ION_TYPE_IS_CLASS(cur_arg_info)) {
            if (Z_TYPE_P(arg) != IS_OBJECT) {
                return false;
            }
            class_name = ION_TYPE_NAME(cur_arg_info);
//            ce = ZEND_TYPE_CE(cur_arg_info->type);
            ce = zend_fetch_class(class_name, (ZEND_FETCH_CLASS_AUTO | ZEND_FETCH_CLASS_NO_AUTOLOAD));
            if (!ce || !instanceof_function(Z_OBJCE_P(arg), ce)) {
                return false;
            }
        } else if (EXPECTED(ION_TYPE_CODE(cur_arg_info) == Z_TYPE_P(arg))) {
            /* pass */
        } else if (Z_TYPE_P(arg) != IS_NULL || !ION_TYPE_ALLOW_NULL(cur_arg_info)) {
            if (ION_TYPE_IS_CLASS(cur_arg_info)) {
                return false;
            } else if (ION_TYPE_CODE(cur_arg_info) == IS_CALLABLE) {
                if (!zend_is_callable(arg, IS_CALLABLE_CHECK_SILENT, NULL)) {
                    return false;
                }
            } else if (ION_TYPE_CODE(cur_arg_info) == _IS_BOOL &&
                       EXPECTED(Z_TYPE_P(arg) == IS_FALSE || Z_TYPE_P(arg) == IS_TRUE)) {
                /* pass */
            } else {
                return pion_verify_scalar_type_hint((zend_uchar) ION_TYPE_CODE(cur_arg_info), arg, (zend_bool) pion_cb_uses_strict_types(cb));
            }
        }
    }
    return true;
}

static zend_always_inline zend_bool pion_verify_arg_type_internal(pion_cb * cb, uint32_t arg_num, zval * arg) {
    zend_internal_arg_info * cur_arg_info;
    zend_class_entry       * ce;
    zend_function          * zf = cb->fcc->function_handler;

    if (EXPECTED(arg_num <= zf->internal_function.num_args)) {
        cur_arg_info = &zf->internal_function.arg_info[arg_num];
    } else if (zf->internal_function.fn_flags & ZEND_ACC_VARIADIC) {
        cur_arg_info = &zf->internal_function.arg_info[zf->internal_function.num_args];
    } else {
        return true;
    }
    if (ION_TYPE_IS_SET(cur_arg_info)) {
        ZVAL_DEREF(arg);
        if (ION_TYPE_IS_CLASS(cur_arg_info)) {
            if (Z_TYPE_P(arg) != IS_OBJECT) {
                return false;
            }
#ifdef IS_PHP71
            ce = zend_fetch_class_ex(ION_TYPE_NAME(cur_arg_info), (ZEND_FETCH_CLASS_AUTO | ZEND_FETCH_CLASS_NO_AUTOLOAD));
#else
            ce = zend_fetch_class(ION_TYPE_NAME(cur_arg_info), (ZEND_FETCH_CLASS_AUTO | ZEND_FETCH_CLASS_NO_AUTOLOAD));
#endif
            if (!ce || !instanceof_function(Z_OBJCE_P(arg), ce)) {
                return false;
            }
        } else if (EXPECTED(ION_TYPE_CODE(cur_arg_info) == Z_TYPE_P(arg))) {
            /* pass */
        } else if (Z_TYPE_P(arg) != IS_NULL || !ION_TYPE_ALLOW_NULL(cur_arg_info)) {
            if (ION_TYPE_IS_CLASS(cur_arg_info)) {
                return false;
            } else if (ION_TYPE_CODE(cur_arg_info) == IS_CALLABLE) {
                if (!zend_is_callable(arg, IS_CALLABLE_CHECK_SILENT, NULL)) {
                    return false;
                }
            } else if (ION_TYPE_CODE(cur_arg_info) == _IS_BOOL &&
                       EXPECTED(Z_TYPE_P(arg) == IS_FALSE || Z_TYPE_P(arg) == IS_TRUE)) {
                /* pass */
            } else {
                return pion_verify_scalar_type_hint((zend_uchar) ION_TYPE_CODE(cur_arg_info), arg, (zend_bool) ZEND_CALL_USES_STRICT_TYPES(EG(current_execute_data)));
            }
        }
    }
    return true;
}


zend_bool pion_verify_arg_type(pion_cb * cb, uint32_t arg_num, zval * arg) {
    if(cb->fcc->function_handler->type == ZEND_USER_FUNCTION) {
        return pion_verify_arg_type_user(cb, arg_num, arg);
    } else {
        ZEND_ASSERT(cb->fcc->function_handler->type == ZEND_INTERNAL_FUNCTION);
        return pion_verify_arg_type_internal(cb, arg_num, arg);
    }
}


int _pion_fcall(zval * result, zend_fcall_info * fci_ptr, zend_fcall_info_cache * fcc_ptr, int num, zval * args) {
    if (ZEND_FCI_INITIALIZED(*fci_ptr)) {
        fci_ptr->retval = result;
        fci_ptr->params = args;
        fci_ptr->no_separation = 1;
        fci_ptr->param_count = (uint32_t)num;
        return zend_call_function(fci_ptr, fcc_ptr);
    } else {
        return FAILURE;
    }
}

int _pion_fcall_void(zend_fcall_info *fci_ptr, zend_fcall_info_cache *fcc_ptr, int num, ...) {
    zval args[num];
    va_list args_list;
    zval result;
    zval * arg;

    va_start(args_list, num);
    for (int j = 0; j < num; j++) {
        arg = va_arg(args_list, zval *);
        args[j] = *arg;
    }
    va_end(args_list);
    int r =  _pion_fcall(&result, fci_ptr, fcc_ptr, num, args);

//    if(result) {
        zval_ptr_dtor(&result);
//    }
    return r;
}


int pion_cb_void(pion_cb *cb, int num, zval *args) {
    zval retval;
//    zval *pretval = NULL;
    if (ZEND_FCI_INITIALIZED(*cb->fci)) {
        cb->fci->retval = &retval;
        cb->fci->params = args;
        cb->fci->param_count = (uint32_t)num;
        if(zend_call_function(cb->fci, cb->fcc) == FAILURE) {
            return FAILURE;
        }
        zval_ptr_dtor(&retval);
        return SUCCESS;
    } else {
        return FAILURE;
    }
}

int pion_cb_void_with_1_arg(pion_cb * cb, zval* arg1) {
    zval args[1];
    args[0] = *arg1;
    return pion_cb_void(cb, 1, args);
}

int pion_cb_void_with_2_args(pion_cb *cb, zval *arg1, zval *arg2) {
    zval args[2];
    args[0] = *arg1;
    args[1] = *arg2;
    return pion_cb_void(cb, 2, args);
}

int pion_cb_void_with_3_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3) {
    zval args[3];
    args[0] = *arg1;
    args[1] = *arg2;
    args[2] = *arg3;
    return pion_cb_void(cb, 3, args);
}

//int pion_cb_void_with_4_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3, zval *arg4) {
//    zval args[4];
//    args[0] = *arg1;
//    args[1] = *arg2;
//    args[2] = *arg3;
//    args[3] = *arg4;
//    return pion_cb_void(cb, 4, args);
//}


zval pion_cb_call(pion_cb *cb, int num, zval *args) {
    zval retval;
    ZVAL_UNDEF(&retval);
    if (ZEND_FCI_INITIALIZED(*cb->fci)) {
        cb->fci->retval = &retval;
        cb->fci->params = args;
        cb->fci->param_count = (uint32_t)num;
        zend_call_function(cb->fci, cb->fcc);
        cb->fci->params = NULL;
        cb->fci->param_count = 0;
    }
    return retval;
}

zval pion_cb_call_with_1_arg(pion_cb * cb, zval* arg1) {
    zval args[1];
    args[0] = *arg1;
    return pion_cb_call(cb, 1, args);
}

zval pion_cb_call_with_2_args(pion_cb *cb, zval *arg1, zval *arg2) {
    zval args[2];
    args[0] = *arg1;
    args[1] = *arg2;
    return pion_cb_call(cb, 2, args);
}

zval pion_cb_call_with_3_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3) {
    zval args[3];
    args[0] = *arg1;
    args[1] = *arg2;
    args[2] = *arg3;
    return pion_cb_call(cb, 3, args);
}

//zval pion_cb_call_with_4_args(pion_cb *cb, zval *arg1, zval *arg2, zval *arg3, zval *arg4) {
//    zval args[4];
//    args[0] = *arg1;
//    args[1] = *arg2;
//    args[2] = *arg3;
//    args[3] = *arg4;
//    return pion_cb_call(cb, 4, args);
//}

zval pion_cb_obj_call(pion_cb *cb, zend_object * obj, int num, zval *args) {
    zval retval;
    zval object;
    ZVAL_UNDEF(&retval);
    ZVAL_OBJ(&object, obj);
    if (ZEND_FCI_INITIALIZED(*cb->fci)) {
        cb->fci->retval = &retval;
        cb->fci->params = args;
        cb->fci->param_count = (uint32_t)num;
        if(obj) {
            Z_ADDREF(object);
            cb->fci->object = obj;
            cb->fcc->object = obj;
            cb->fcc->calling_scope = obj->ce;
        }
        zend_call_function(cb->fci, cb->fcc);
        if(cb->fcc->object) {
            Z_DELREF(object);
            cb->fcc->object = NULL;
            cb->fcc->calling_scope = NULL;
            cb->fci->object = NULL;
        }
        cb->fci->param_count = 0;
        cb->fci->params = NULL;
    }
    return retval;
}

zval pion_cb_obj_call_with_1_arg(pion_cb * cb, zend_object * obj, zval* arg1) {
    zval args[1];
    args[0] = *arg1;
    return pion_cb_obj_call(cb, obj, 1, args);
}

zval pion_cb_obj_call_with_2_args(pion_cb *cb, zend_object * obj, zval *arg1, zval *arg2) {
    zval args[2];
    args[0] = *arg1;
    args[1] = *arg2;
    return pion_cb_obj_call(cb, obj, 2, args);
}

zval pion_cb_obj_call_with_3_args(pion_cb *cb, zend_object * obj, zval *arg1, zval *arg2, zval *arg3) {
    zval args[3];
    args[0] = *arg1;
    args[1] = *arg2;
    args[2] = *arg3;
    return pion_cb_obj_call(cb, obj, 3, args);
}

//zval pion_cb_obj_call_with_4_args(pion_cb *cb, zend_object * obj, zval *arg1, zval *arg2, zval *arg3, zval *arg4) {
//    zval args[4];
//    args[0] = *arg1;
//    args[1] = *arg2;
//    args[2] = *arg3;
//    args[3] = *arg4;
//    return pion_cb_obj_call(cb, obj, 4, args);
//}


int pion_call_constructor(zend_class_entry * ce, zend_object * this_ptr, int args_num, zval *args) {
    zval retval_ptr;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    if (!(ce->constructor->common.fn_flags & ZEND_ACC_PUBLIC)) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 1, "Invocation of %s's constructor failed",  ce->name);
        return FAILURE;
    }
    fci.size = sizeof(fci);
    ZVAL_UNDEF(&fci.function_name);
#ifdef IS_PHP70
    fci.symbol_table = NULL;
    fci.function_table = EG(function_table);
    fcc.calling_scope = EG(scope);
#else
    fcc.calling_scope = EG(fake_scope);
#endif
    fci.object = this_ptr;
    fci.retval = &retval_ptr;
    fci.param_count = (uint32_t)args_num;
    fci.params = args;
    fci.no_separation = 1;

#ifdef HAS_FCC_INITIALIZED
    fcc.initialized = 1;
#endif
    fcc.function_handler = ce->constructor;
    fcc.called_scope = this_ptr->ce;
    fcc.object = this_ptr;

    if (zend_call_function(&fci, &fcc) == FAILURE) {
        if(!EG(exception)) {
            zend_throw_exception_ex(spl_ce_RuntimeException, 1, "Invocation of %s's constructor failed",  ce->name);
        }
        return FAILURE;
    }
    zval_ptr_dtor(&retval_ptr);

    return SUCCESS;
}

zend_object * pion_new_object(zend_class_entry *ce, int args_num, zval * args) {
    zval object;

    object_init_ex(&object, ce);
    if(ce->constructor) {
        if(pion_call_constructor(ce, Z_OBJ(object), args_num, args) == FAILURE) {
            zval_ptr_dtor(&object);
            return NULL;
        }
    }

    return Z_OBJ(object);
}


//zend_object * _pion_new_object_arg_1(zend_class_entry *ce, zval *arg1) {
//    zval args[1];
//    args[0] = *arg1;
//    return pion_new_object(ce, 1, args);
//}
//
zend_object * pion_new_object_arg_2(zend_class_entry *ce, zval *arg1, zval *arg2) {
    zval args[2];
    args[0] = *arg1;
    args[1] = *arg2;
    return pion_new_object(ce, 2, args);
}
//
//zend_object * _pion_new_object_arg_3(zend_class_entry *ce, zval *arg1, zval *arg2, zval *arg3) {
//    zval args[3];
//    args[0] = *arg1;
//    args[1] = *arg2;
//    args[2] = *arg3;
//    return pion_new_object(ce, 3, args);
//}
