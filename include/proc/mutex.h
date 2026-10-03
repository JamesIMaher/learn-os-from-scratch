/*
 * mutex.h - a sleeping lock for threads.                       [Lesson 11]
 *
 * A thread that can't get the lock must not spin: it should block and let
 * other threads run until the lock is released.
 */
#pragma once

struct thread;

typedef struct mutex {
    /* TODO(lesson 11): your fields go here. */
    int todo_replace_me;
} mutex_t;

void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);   /* only the owner may unlock */
