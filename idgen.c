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

#include <sys/shm.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include "php.h"
#include "idgen.h"

typedef struct _shm_data_t {
    snowflake_t last_id;
} shm_data_t;

static inline uint64_t current_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((uint64_t)tv.tv_sec) * 1000 + ((uint64_t)tv.tv_usec) / 1000;
}

static inline void wait_till_next_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    usleep(1000-(tv.tv_usec%1000));
}

PHPAPI extern char *php_ini_opened_path;

static shm_data_t *shm_data;

int sf_init() {

    int shm_id;

    shm_id = shmget(IPC_PRIVATE, sizeof(shm_data_t), IPC_CREAT | 0600);

    if (shm_id < 0) {
        // trigger error
        return FAILURE;
    }
    shm_data = (shm_data_t *) shmat(shm_id, NULL, 0);

    return SUCCESS;
}

int64_t sf_gen(long node_id, uint64_t epoch) {
    uint64_t now = current_time_ms(); // too slow: ~1us
    while(1) {
        snowflake_t new, old;
        uint64_t new_time, old_time;
        new_time = now - epoch;
        old = shm_data->last_id;
        old_time = old.id >> TIMESTAMP_OFFSET;
        if (old_time > new_time) {
            new_time = old_time;
        }
        if (new_time == old_time) {
            new.id = old.id;
            new.s.seq_bits ++;
            if (new.s.seq_bits == 0) {
                // overflow
                wait_till_next_ms();
                now = current_time_ms();
                continue;
            }
        } else {
            new.id = 0;
            new.id |= new_time << TIMESTAMP_OFFSET;
            new.s.node_bits = node_id;
            new.s.seq_bits = 0;
        }
        if (__sync_bool_compare_and_swap(&(shm_data->last_id.id), old.id, new.id)) {
            return new.id;
        }
    }
}

int sf_close() {
    shmdt(shm_data);
    return SUCCESS;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: et sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
