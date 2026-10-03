/* mutex.c - see include/proc/mutex.h.                           [Lesson 11] */
#include "proc/mutex.h"
#include "proc/thread.h"

/*
 * TODO(lesson 11). The danger is the gap between "is it free?" and "it's
 * mine now". On a single CPU, what can happen in that gap, and how do you
 * close it?
 */

void mutex_init(mutex_t *m) { (void)m; }
void mutex_lock(mutex_t *m) { (void)m; }
void mutex_unlock(mutex_t *m) { (void)m; }
