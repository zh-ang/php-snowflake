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

#ifndef PHP_SNOWFLAKE_H
#define PHP_SNOWFLAKE_H

extern zend_module_entry snowflake_module_entry;
#define phpext_snowflake_ptr &snowflake_module_entry

#ifdef PHP_WIN32
#   define PHP_SNOWFLAKE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_SNOWFLAKE_API __attribute__ ((visibility("default")))
#else
#   define PHP_SNOWFLAKE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(snowflake);
PHP_MSHUTDOWN_FUNCTION(snowflake);
PHP_RINIT_FUNCTION(snowflake);
PHP_RSHUTDOWN_FUNCTION(snowflake);
PHP_MINFO_FUNCTION(snowflake);

PHP_FUNCTION(snowflake_next_id);
PHP_FUNCTION(snowflake_explain);

ZEND_BEGIN_MODULE_GLOBALS(snowflake)
    int enabled;
    long epoch;
    long node_id;
ZEND_END_MODULE_GLOBALS(snowflake)

/* In every utility function you add that needs to use variables 
   in php_snowflake_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as SNOWFLAKE_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define SNOWFLAKE_G(v) TSRMG(snowflake_globals_id, zend_snowflake_globals *, v)
#else
#define SNOWFLAKE_G(v) (snowflake_globals.v)
#endif


#endif  /* PHP_SNOWFLAKE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: et sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
