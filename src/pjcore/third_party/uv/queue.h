// Modification of queue.h
// From https://github.com/joyent/libuv/blob/master/src/queue.h
// Original contains the following copyright, notice and disclaimer:
//
/* Copyright (c) 2013, Ben Noordhuis <info@bnoordhuis.nl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef PJCORE_THIRD_PARTY_UV_QUEUE_H_
#define PJCORE_THIRD_PARTY_UV_QUEUE_H_

typedef void *PJCORE_QUEUE[2];

/* Private macros. */
#define PJCORE_QUEUE_NEXT(q) (*(PJCORE_QUEUE **)&((*(q))[0]))
#define PJCORE_QUEUE_PREV(q) (*(PJCORE_QUEUE **)&((*(q))[1]))
#define PJCORE_QUEUE_PREV_NEXT(q) (PJCORE_QUEUE_NEXT(PJCORE_QUEUE_PREV(q)))
#define PJCORE_QUEUE_NEXT_PREV(q) (PJCORE_QUEUE_PREV(PJCORE_QUEUE_NEXT(q)))

/* Public macros. */
#define PJCORE_QUEUE_DATA(ptr, type, field) \
  ((type *)((char *)(ptr) - ((char *)&((type *)0)->field)))

#define PJCORE_QUEUE_FOREACH(q, h) \
  for ((q) = PJCORE_QUEUE_NEXT(h); (q) != (h); (q) = PJCORE_QUEUE_NEXT(q))

#define PJCORE_QUEUE_EMPTY(q) \
  ((const PJCORE_QUEUE *)(q) == (const PJCORE_QUEUE *)PJCORE_QUEUE_NEXT(q))

#define PJCORE_QUEUE_HEAD(q) (PJCORE_QUEUE_NEXT(q))

#define PJCORE_QUEUE_INIT(q)    \
  do {                          \
    PJCORE_QUEUE_NEXT(q) = (q); \
    PJCORE_QUEUE_PREV(q) = (q); \
  } while (0)

#define PJCORE_QUEUE_ADD(h, n)                        \
  do {                                                \
    PJCORE_QUEUE_PREV_NEXT(h) = PJCORE_QUEUE_NEXT(n); \
    PJCORE_QUEUE_NEXT_PREV(n) = PJCORE_QUEUE_PREV(h); \
    PJCORE_QUEUE_PREV(h) = PJCORE_QUEUE_PREV(n);      \
    PJCORE_QUEUE_PREV_NEXT(h) = (h);                  \
  } while (0)

#define PJCORE_QUEUE_SPLIT(h, q, n)              \
  do {                                           \
    PJCORE_QUEUE_PREV(n) = PJCORE_QUEUE_PREV(h); \
    PJCORE_QUEUE_PREV_NEXT(n) = (n);             \
    PJCORE_QUEUE_NEXT(n) = (q);                  \
    PJCORE_QUEUE_PREV(h) = PJCORE_QUEUE_PREV(q); \
    PJCORE_QUEUE_PREV_NEXT(h) = (h);             \
    PJCORE_QUEUE_PREV(q) = (n);                  \
  } while (0)

#define PJCORE_QUEUE_INSERT_HEAD(h, q)           \
  do {                                           \
    PJCORE_QUEUE_NEXT(q) = PJCORE_QUEUE_NEXT(h); \
    PJCORE_QUEUE_PREV(q) = (h);                  \
    PJCORE_QUEUE_NEXT_PREV(q) = (q);             \
    PJCORE_QUEUE_NEXT(h) = (q);                  \
  } while (0)

#define PJCORE_QUEUE_INSERT_TAIL(h, q)           \
  do {                                           \
    PJCORE_QUEUE_NEXT(q) = (h);                  \
    PJCORE_QUEUE_PREV(q) = PJCORE_QUEUE_PREV(h); \
    PJCORE_QUEUE_PREV_NEXT(q) = (q);             \
    PJCORE_QUEUE_PREV(h) = (q);                  \
  } while (0)

#define PJCORE_QUEUE_REMOVE(q)                        \
  do {                                                \
    PJCORE_QUEUE_PREV_NEXT(q) = PJCORE_QUEUE_NEXT(q); \
    PJCORE_QUEUE_NEXT_PREV(q) = PJCORE_QUEUE_PREV(q); \
  } while (0)

#endif /* PJCORE_THIRD_PARTY_UV_QUEUE_H_ */
