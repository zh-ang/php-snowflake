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

#ifndef IDGEN_H
#define IDGEN_H

#include <stdint.h>

int sf_init() ;
int64_t sf_gen(long node_id, uint64_t epoch) ;
int sf_close() ;
int sf_explain(int64_t id, long node_id, uint64_t epoch) ;

#define TIMESTAMP_WIDTH 41
#define TIMESTAMP_MASK ((1 << (TIME_WIDTH + 1)) - 1)
#define TIMESTAMP_OFFSET (SEQUENCE_WIDTH + NODE_ID_WIDTH)
#define NODE_ID_WIDTH 10
#define NODE_ID_MASK ((1 << (NODE_ID_WIDTH + 1)) - 1)
#define NODE_ID_OFFSET (SEQUENCE_WIDTH)
#define SEQUENCE_WIDTH 12
#define SEQUENCE_MASK ((1 << (SEQUENCE_WIDTH + 1)) - 1)
#define SEQUENCE_OFFSET 0

typedef union _snowflake_t {
    struct {
        unsigned seq_bits:12;
        unsigned node_bits:10;
        unsigned time_low_bits:10;
        unsigned time_high_bits:31;
        unsigned sign_bit:1;
    } s;
    int64_t id;
} snowflake_t;


#endif  /* IDGEN_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: et sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
