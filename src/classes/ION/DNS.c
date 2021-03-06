#include "ion.h"
#include <event2/dns.h>

zend_class_entry * ion_ce_ION_DNS;
zend_class_entry * ion_ce_ION_DNSException;

void _ion_dns_getaddrinfo_callback(int errcode, struct evutil_addrinfo * addr, void * d) {
    zval result, A, AAAA;
    ion_promisor * deferred = d;
    ion_dns_addr_request * req = Z_PTR(deferred->object);
    if (!errcode) {
        array_init(&result);
        array_init(&A);
        array_init(&AAAA);
        add_assoc_str(&result, "domain", zend_string_copy(req->domain));
        add_assoc_zval(&result, "ipv4", &A);
        add_assoc_zval(&result, "ipv6", &AAAA);
        struct evutil_addrinfo * ai;
        for (ai = addr; ai; ai = ai->ai_next) {
            char buf[128];
            const char *s = NULL;
            if (ai->ai_family == AF_INET) {
                struct sockaddr_in *sin = (struct sockaddr_in *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET, &sin->sin_addr, buf, 128);
                if(s) {
                    add_next_index_string(&A, s);
                }
            } else if (ai->ai_family == AF_INET6) {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ai->ai_addr;
                s = evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, buf, 128);
                if(s) {
                    add_next_index_string(&AAAA, s);
                }
            }
        }
        evutil_freeaddrinfo(addr);
        ion_promisor_done(deferred, &result);
    } else {
        zend_object * exception = pion_exception_new_ex(ion_ce_ION_DNSException, 0, ERR_ION_DNS_RESOLVE_FAILED, evutil_gai_strerror(errcode));
        ZVAL_OBJ(&result, exception);
        ion_promisor_fail(deferred, &result);
    }
    zval_ptr_dtor(&result);
    ion_object_release(deferred);
}

void ion_dns_request_dtor(zval * ptr) {
//    if(promisor->object) {
        ion_dns_addr_request *req = Z_PTR_P(ptr);
        if(req->request) {
            evdns_getaddrinfo_cancel(req->request);
        }
        zend_string_release(req->domain);
        if(zend_hash_del(GION(resolvers), req->domain) == FAILURE) {
            zend_error(E_ERROR, ERR_ION_DNS_RESOLVE_NOT_CLEANED);
        }
        efree(req);
//        promisor->object = NULL;
//    }
}

/** public static function ION\DNS::resolve(string $domain, int $flags = self::ADDR_IP_ANY) : Deferred */
CLASS_METHOD(ION_DNS, resolve) {
    zend_string  * domain   = NULL;
    zend_long      flags    = ION_DNS_RECORD_BASE;
    ion_promisor * deferred = NULL;
    zval           val;
    zval         * found;
    ion_dns_addr_request   * req;
    struct evutil_addrinfo   hints;

    ZEND_PARSE_PARAMETERS_START(1,2)
        Z_PARAM_STR(domain)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(flags)
    ZEND_PARSE_PARAMETERS_END_EX(PION_ZPP_THROW);
    found = zend_hash_find(GION(resolvers), domain);
    if(found) {
        RETURN_ZVAL(found, 1, 0);
    }

    deferred = ion_promisor_deferred_new_ex(NULL);
    ZVAL_OBJ(&val, ION_OBJECT_ZOBJ(deferred));
//    obj_promisor_set_dtor(deferred, ion_dns_request_dtor);
    memset(&hints, 0, sizeof(hints));
    if((flags & ION_DNS_RECORDS_A_AAAA) == ION_DNS_RECORDS_A_AAAA || (flags & ION_DNS_RECORDS_A_AAAA) == 0) {
        hints.ai_family = PF_UNSPEC;
    } else if(flags & ION_DNS_RECORD_A) {
        hints.ai_family = PF_INET;
    } else { // flags & ION_DNS_RECORD_AAAA
        hints.ai_family = PF_INET6;
    }
    hints.ai_flags = EVUTIL_AI_CANONNAME;
    /* Unless we specify a socktype, we'll get at least two entries for
     * each address: one for TCP and one for UDP. That's not what we
     * want. */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    req = emalloc(sizeof(ion_dns_addr_request));
    memset(req, 0, sizeof(ion_dns_addr_request));
    req->domain = zend_string_copy(domain);
    req->request = evdns_getaddrinfo(GION(evdns), domain->val, NULL, &hints, _ion_dns_getaddrinfo_callback, deferred);
//    ion_promisor_store(deferred, req);

    if(!zend_hash_add(GION(resolvers), domain, &val)) {
        ion_object_release(deferred);
        zend_throw_exception(ion_ce_ION_RuntimeException, ERR_ION_DNS_RESOLVE_NOT_STORED, 0);
        return;
    }
    ion_promisor_set_object_ptr(deferred, req, ion_dns_request_dtor);
    ion_object_addref(deferred);
    RETURN_ION_OBJ(deferred);
}

METHOD_ARGS_BEGIN(ION_DNS, resolve, 1)
    ARGUMENT(domain, IS_STRING)
    ARGUMENT(flags, IS_LONG)
METHOD_ARGS_END()

METHODS_START(methods_ION_DNS)
    METHOD(ION_DNS, resolve,   ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
METHODS_END;

ZEND_INI_BEGIN()
    STD_PHP_INI_BOOLEAN("ion.dns.async", "1", PHP_INI_SYSTEM, OnUpdateBool, adns_enabled, zend_ion_globals, ion_globals)
    STD_PHP_INI_ENTRY("ion.dns.resolv_conf", ION_DNS_RESOLV_CONF_DEFAULT, PHP_INI_SYSTEM, OnUpdateStringUnempty, resolv_conf, zend_ion_globals, ion_globals)
    STD_PHP_INI_ENTRY("ion.dns.hosts_file",  ION_DNS_HOSTS_FILE_DEFAULT, PHP_INI_SYSTEM, OnUpdateString, hosts_file, zend_ion_globals, ion_globals)
ZEND_INI_END()

PHP_MINIT_FUNCTION(ION_DNS) {
    REGISTER_INI_ENTRIES();
    ion_register_static_class(ion_ce_ION_DNS, "ION\\DNS", methods_ION_DNS);
    ion_class_declare_constant_long(ion_ce_ION_DNS, "RECORD_A", ION_DNS_RECORD_A);
    ion_class_declare_constant_long(ion_ce_ION_DNS, "RECORD_AAAA", ION_DNS_RECORD_AAAA);
    ion_class_declare_constant_long(ion_ce_ION_DNS, "RECORD_CNAME", ION_DNS_RECORD_CNAME);
    ion_register_exception(ion_ce_ION_DNSException, ion_ce_ION_RuntimeException, "ION\\DNSException");

    if(!GION(adns_enabled)) {
        return SUCCESS;
    }
    GION(evdns) = evdns_base_new(GION(base), 1);
    int error = evdns_base_resolv_conf_parse(GION(evdns), DNS_OPTIONS_ALL, GION(resolv_conf));
    if(error) {
        switch(error) {
            case 1:
                zend_error(E_WARNING, ERR_ION_DNS_RESOLV_NOT_FOUND, GION(resolv_conf));
                break;
            case 2:
                zend_error(E_WARNING, ERR_ION_DNS_RESOLV_CANT_STAT, GION(resolv_conf));
                break;
            case 3:
                zend_error(E_WARNING, ERR_ION_DNS_RESOLV_TOO_LARGE, GION(resolv_conf));
                break;
            case 4:
                zend_error(E_WARNING, ERR_ION_DNS_RESOLV_OUT_OF_MEMORY);
                break;
            case 5:
                zend_error(E_WARNING, ERR_ION_DNS_RESOLV_CANT_READ, GION(resolv_conf));
                break;
            case 6:
                zend_error(E_WARNING, ERR_ION_DNS_RESOLV_NO_SERVERS, GION(resolv_conf));
                break;
            default:
                zend_error(E_WARNING, ERR_ION_DNS_RESOLV_ERROR, GION(resolv_conf));
        }
        GION(evdns) = NULL;
        return FAILURE;
    }
    if(GION(hosts_file)) {
        evdns_base_load_hosts(GION(evdns), GION(hosts_file));
    }

    return SUCCESS;
}

PHP_RINIT_FUNCTION(ION_DNS) {
    ALLOC_HASHTABLE(GION(resolvers));
    zend_hash_init(GION(resolvers), 128, NULL, NULL, 0);
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ION_DNS) {
    GION(resolvers)->pDestructor = zval_ptr_dtor_wrapper;
    zend_hash_clean(GION(resolvers));
    zend_hash_destroy(GION(resolvers));
    FREE_HASHTABLE(GION(resolvers));
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ION_DNS) {
    if(GION(evdns)) {
        evdns_base_free(GION(evdns), 0);
    }
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}