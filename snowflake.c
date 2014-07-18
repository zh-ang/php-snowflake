/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Zhang Zhenyu <zhangzhenyu@php.net>                           |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_snowflake.h"
#include "idgen.h"

ZEND_DECLARE_MODULE_GLOBALS(snowflake)

/* True global resources - no need for thread safety here */
static int le_snowflake;

/* {{{ snowflake_functions[]
 *
 * Every user visible function must have an entry in snowflake_functions[].
 */
const zend_function_entry snowflake_functions[] = {
    PHP_FE(snowflake_next_id,    NULL)
    PHP_FE(snowflake_explain,    NULL)
    PHP_FE_END    /* Must be the last line in snowflake_functions[] */
};
/* }}} */

/* {{{ snowflake_module_entry
 */
zend_module_entry snowflake_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "snowflake",
    snowflake_functions,
    PHP_MINIT(snowflake),
    PHP_MSHUTDOWN(snowflake),
    NULL,
    NULL,
    PHP_MINFO(snowflake),
#if ZEND_MODULE_API_NO >= 20010901
    "0.1", /* Replace with version number for your extension */
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

/* {{{ ZEND_GET_MODULE(snowflake) */
#ifdef COMPILE_DL_SNOWFLAKE
ZEND_GET_MODULE(snowflake)
#endif
/* }}} */

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("snowflake.epoch",    "0", ZEND_INI_SYSTEM, OnUpdateLong, epoch, zend_snowflake_globals, snowflake_globals)
    STD_PHP_INI_ENTRY("snowflake.node_id",  "0", ZEND_INI_SYSTEM, OnUpdateLong, node_id, zend_snowflake_globals, snowflake_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_snowflake_init_globals
 */
static void php_snowflake_init_globals(zend_snowflake_globals *snowflake_globals)
{
    snowflake_globals->enabled  = 0;
    snowflake_globals->epoch    = 0;
    snowflake_globals->node_id  = 0;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(snowflake)
{
    TSRMLS_FETCH();
    int ret = SUCCESS;

    REGISTER_INI_ENTRIES();
    if (sf_init() == FAILURE) {
        ret = FAILURE;
    }

    if (SNOWFLAKE_G(node_id) <= 0) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "\"snowflake.node_id\" must be set greater than zero");
        ret = FAILURE;
    } else if (SNOWFLAKE_G(node_id) >= (1<<NODE_ID_WIDTH)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "\"snowflake.node_id\" must be set smaller than %d", (1<<NODE_ID_WIDTH));
        ret = FAILURE;
    }

    if (SNOWFLAKE_G(epoch) < 0) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "\"snowflake.epoch\" must be set greater than zero");
        return FAILURE;
    } else if (SNOWFLAKE_G(epoch) / 1000 > time (NULL)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "\"snowflake.epoch\" must be set earlier before now");
        return FAILURE;
    }

    SNOWFLAKE_G(enabled) = (ret == SUCCESS);

    return ret;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(snowflake)
{
    TSRMLS_FETCH();
    SNOWFLAKE_G(enabled) = 0;
    UNREGISTER_INI_ENTRIES();
    sf_close();
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(snowflake)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(snowflake)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(snowflake)
{
    TSRMLS_FETCH();
    php_info_print_table_start();
    php_info_print_table_header(2, "snowflake support", 
                SNOWFLAKE_G(enabled) ? "enabled" : "disabled");
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */


/* Every user-visible function in PHP should document itself in the source */

/* {{{ proto int snowflake_next_id() */
PHP_FUNCTION(snowflake_next_id)
{
    TSRMLS_FETCH();

    if (SNOWFLAKE_G(enabled) == 0) {
        RETURN_BOOL(0);
    }


    int64_t ret = sf_gen(SNOWFLAKE_G(node_id), SNOWFLAKE_G(epoch));
    if (ret) {
        RETURN_LONG(ret);
    } else {
        RETURN_BOOL(0);
    }
}
/* }}} */

/* {{{ proto int snowflake_explain(int id) */
PHP_FUNCTION(snowflake_explain)
{
    long id;
    snowflake_t tmp;
    double timestamp;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &id) == FAILURE) {
        return;
    }

    tmp.id = id;
    timestamp = ((tmp.id >> TIMESTAMP_OFFSET) + SNOWFLAKE_G(epoch) + 0.0) / 1000;

    array_init(return_value);
    add_assoc_double(return_value, "timestamp", timestamp);
    add_assoc_long(return_value, "node_id", tmp.s.node_bits);
    add_assoc_long(return_value, "sequence", tmp.s.seq_bits);
}
/* }}} */

/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: et sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
