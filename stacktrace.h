#ifndef __STACKTRACE_H__
#define __STACKTRACE_H__


/*
 * Initialize stack trace library.
 * DESCRIPTION
 *  call init_stacktrace register crash signal handler.
 *  if process crash, this library outputs the stack calls to stderr.
 *
 * RETURN VALUE
 * - On success, init_stacktrace return 0.
 * - On error, -1 is returned.
 */
int init_stacktrace();


#endif /* __STACKTRACE_H__ */
