/* implementation of buffer pool to reuse big buffers allocated by malloc()
 * inspired from http://stackoverflow.com/questions/28511541/libuv-allocated-memory-buffers-re-use-techniques
 * modified with linked list
 */

#include "rum.h"

extern bufpool_t *pool;

void
bufpool_print_stats (uv_timer_t * handle)
{
    uv_timer_stop (handle);
    uv_timer_start (handle, bufpool_print_stats, 10000, 10000);
    fprintf (stderr, "pool->used: %d\npool->available: %d\n",
             pool->used, pool->available);

}

void
bufpool_enqueue (bufpool_t * pool, void *ptr)
{
    if (pool->first) {
        bufbase (ptr)->next = pool->first;
    } else {
        bufbase (ptr)->next = NULL;
    }
    pool->first = ptr;
    pool->used--;
    pool->available++;
}

void *
bufpool_dequeue (bufpool_t * pool)
{
    void *ptr;
    if (pool->first) {
        ptr = pool->first;
        pool->first = bufbase (ptr)->next;
        pool->used++;
        pool->available--;
        bufbase (ptr)->next = NULL;
        return ptr;
    } else {
        if (pool->available + pool->used <= BUFPOOL_CAPACITY) {
            return bufpool_alloc (pool, pool->alloc_size);
        } else {
            return NULL;
        }
    }
}

void
bufpool_init (bufpool_t * pool, int size)
{
    pool->alloc_size = size;
    pool->available = 0;
    pool->used = 0;
    pool->first = NULL;
}

void *
bufpool_acquire (bufpool_t * pool, int *len)
{
    int size = *len;
    void *buf = bufpool_dequeue (pool);
    if (!buf)
        buf = bufpool_alloc (0, size);
    *len = buf ? buflen (buf) : 0;
    return buf;
}

void *
bufpool_alloc (bufpool_t * pool, int len)
{
    bufbase_t *base = malloc (sizeof (bufbase_t) + len);
    if (!base)
        return 0;
    base->pool = pool;
    base->len = len;
    base->next = NULL;
    if (pool) {
        pool->used++;
    }
    return (char *) base + sizeof (bufbase_t);
}

void
bufpool_release (void *ptr)
{
    if (!ptr)
        return;
    if (bufbase (ptr)->pool) {
        bufpool_enqueue (bufbase (ptr)->pool, ptr);
    } else {
        free (bufbase (ptr));
    }
}

void
bufpool_done (bufpool_t * pool)
{
    void *ptr = pool->first;
    void *next;

    if (pool->used) {
        fprintf (stderr, "warning: %d buffers are still used\n", pool->used);
    }

    while (ptr) {
        next = bufbase (ptr)->next;
        free (bufbase (ptr));
        ptr = next;
    }
}
