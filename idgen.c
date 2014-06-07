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
#include "TSRM/tsrm_config_common.h"
#include "php_snowflake.h"

#define pause() __asm__ __volatile__("pause;")

uint64_t current_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((uint64_t)tv.tv_sec) * 1000 + ((uint64_t)tv.tv_usec) / 1000;
}

void wait_till_next_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    usleep(1000-(tv.tv_usec%1000));
}

key_t _get_shm_key(int node_id) {

    char filename[MAXPATHLEN];
    char resolved[MAXPATHLEN];

    snprintf(filename, MAXPATHLEN, "/proc/%d/exe", getpid());
    if (VCWD_REALPATH(filename, resolved)) {
        return ftok(resolved, node_id);
    }

    return 0x19881111; // in case of fail to get current executable path

}

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

typedef struct _shm_data_t {
    snowflake_t last_id;
} shm_data_t;

extern ZEND_DECLARE_MODULE_GLOBALS(snowflake);

int shm_id;
shm_data_t *shm_data;
key_t shm_key;

/* {{{ static void seat_lock()  * /
static void seat_lock() {

    if (seat->locker_pid == my_pid) {
        // no one knows why we get privilege before we ask
        // TODO log it.
        return;
    }
    while () {
        int i, j;
        for (i=0; i<8; i++) {
            if (__sync_bool_compare_and_swap(seat->locker_pid, 0, my_pid)) {
                seat->lock_time = current_time_ms();
                return;
            }
            for (j=0; j<i; j++) {
                pause();
            }
        }
        do {
            char filename[MAXPATHLEN]={0};
            char my_resolved[MAXPATHLEN]={0};
            char locker_resolved[MAXPATHLEN]={0};
            int locker_pid = seat->locker_pid;
            snprintf(filename, MAXPATHLEN, "/proc/%d", locker_pid);
            if (VCWD_ACCESS(filename, F_OK) != 0) {
                // process is not exist
                if (__sync_bool_compare_and_swap(seat->locker_pid, locker_pid, my_pid)) {
                    seat->lock_time = current_time_ms();
                    return;
                }
            }

            snprintf(filename, MAXPATHLEN, "/proc/%d/exe", locker_pid);
            VCWD_REALPATH(filename, locker_resolved);
            snprintf(filename, MAXPATHLEN, "/proc/%d/exe", my_pid);
            VCWD_REALPATH(filename, my_resolved);

            if (strcmp(my_resolved, locker_resolved) != 0) {
                // process is not our same kind
                if (__sync_bool_compare_and_swap(seat->locker_pid, locker_pid, my_pid)) {
                    seat->lock_time = current_time_ms();
                    return;
                }
            }

            sched_yield();

        } while (0);

    }

}

void seat_unlock() {
    seat->locker_pid = 0;
}
/ * }}} */

int sf_init() {
    TSRMLS_FETCH();

    shm_key = _get_shm_key(SNOWFLAKE_G(node_id));
    shm_id = shmget(shm_key, sizeof(shm_data_t), IPC_CREAT | 0600);
    if (shm_id < 0) {
        // trigger error
        return FAILURE;
    }
    shm_data = (shm_data_t *) shmat(shm_id, NULL, 0);

    /* looking for a seat * 
    my_seat = 0;
    do {
        int n;
        for (n=1; n<MAX_SEAT; n++) {
            if (seat->pos[n] == 0) {
                // empty seat
                if (__sync_bool_compare_and_swap(shm_data->seat[n], 0, my_pid)) {
                    my_seat = n;
                    break;
                }
            }
        }
    } while (0);

    if (my_seat == 0) {
        // no seat left (you may started too much php-worker)
        // TODO find a seat
    }
    */

    SNOWFLAKE_G(enabled) = 1;
    return SUCCESS;
}

int64_t sf_gen() {
    TSRMLS_FETCH();

    if (SNOWFLAKE_G(enabled) == 0) {
        return 0;
    }

    while(1) {
        snowflake_t new, old;
        uint64_t new_time, old_time;
        new_time = current_time_ms() - SNOWFLAKE_G(epoch);
        old = shm_data->last_id;
        old_time = old.id >> TIMESTAMP_OFFSET;
        if (new_time == old_time && old.s.node_bits == SNOWFLAKE_G(node_id)) {
            new.id = old.id;
            new.s.seq_bits ++;
            if (new.s.seq_bits == 0) {
                // overflow
                wait_till_next_ms();
                continue;
            }
        } else {
            new.id = 0;
            new.id |= new_time << TIMESTAMP_OFFSET;
            new.s.node_bits = SNOWFLAKE_G(node_id);
            new.s.seq_bits = 0;
        }
        if (__sync_bool_compare_and_swap(&(shm_data->last_id.id), old.id, new.id)) {
            return new.id;
        }
    }

}

int sf_close() {
    TSRMLS_FETCH();
    shmdt(shm_data);
    SNOWFLAKE_G(enabled) = 0;
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
