#define _GNU_SOURCE
#include <execinfo.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>

#include "symbol_table.h"


// The max number of levels in the stack trace
#define STACK_TRACE_MAX_LEVELS  100
#define BUFFER_LENGTH           4096


typedef void (*signal_handler_t)(int signo, siginfo_t *info, void *ctx);

static void register_crash_handlers();
static int backtrace_symbol_write(int fd, const char *text, void *addr);


// store process full path.
static char program_path[PATH_MAX];
// the current program binary symbol table.
static symbol_table_t symtab;


// initialize stacktrace library.
int init_stacktrace()
{
    int n;

    n = readlink("/proc/self/exe", program_path, PATH_MAX);
    if (n < 0 || n >= PATH_MAX) {
        return -1;
    }

    program_path[n] = '\0';

    if (symbol_table_build(program_path, &symtab) != 0) {
        return -2;
    }

    register_crash_handlers();

    return 0;
}


static void
stack_trace_dump()
{
    int             i, btl;
    char          **strings;
    void           *stack[STACK_TRACE_MAX_LEVELS + 1];
    const char     *msg = " - STACK TRACE: \n";

    if (write(STDERR_FILENO, program_path, strlen(program_path)) == -1) {
        return;
    }

    if (write(STDERR_FILENO, msg, strlen(msg)) == -1) {
        return;
    }

    memset(stack, 0, sizeof(stack));

    if ((btl = backtrace(stack, STACK_TRACE_MAX_LEVELS)) > 2) {
        strings = backtrace_symbols(stack, btl);
        if (strings != NULL) {
            for (i = 2; i < btl; i++) {
                backtrace_symbol_write(STDERR_FILENO, strings[i], stack[i]);
            }

            free(strings);

        } else {
            backtrace_symbols_fd(stack + 2, btl - 2, STDERR_FILENO);
        }
    }
}


// Reset a signal handler to the default handler.
static void
signal_reset_default(int signo)
{
    struct sigaction act;

    act.sa_handler = SIG_DFL;
    act.sa_flags   = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;
    sigemptyset(&(act.sa_mask));

    assert(sigaction(signo, &act, NULL) == 0);
}


static void
signal_crash_handler(int signo, siginfo_t *siginfo, void *data)
{
    stack_trace_dump();

    signal_reset_default(signo);
    // throw signal to default handler.
    raise(signo);
}


static void
set_signal(int signo, signal_handler_t handler)
{
    struct sigaction act;

    act.sa_handler   = NULL;
    act.sa_sigaction = handler;
    act.sa_flags     = SA_SIGINFO;
    sigemptyset(&(act.sa_mask));

    assert(sigaction(signo, &act, NULL) == 0);
}


static void
register_crash_handlers()
{
    set_signal(SIGBUS,  signal_crash_handler);
    set_signal(SIGSEGV, signal_crash_handler);
    set_signal(SIGILL,  signal_crash_handler);
    set_signal(SIGTRAP, signal_crash_handler);
    set_signal(SIGFPE,  signal_crash_handler);
    set_signal(SIGABRT, signal_crash_handler);
}


static int
backtrace_symbol_format(char *buf, size_t len, const char *prefix, frame_record_t fr)
{
    int     n;
    char   *p = buf;

    // file name
    if (fr.filename != NULL) {
        n = snprintf(p, len, "%s %s", prefix, fr.filename);

    } else {
        n = snprintf(p, len, "%s ??", prefix);
    }

    p += n;
    len -= n;

    // function name
    if (fr.functionname != NULL && *fr.functionname != '\0') {
        n = snprintf(p, len, " %s()", fr.functionname);

    } else {
        n = snprintf(p, len, " ??");
    }

    p += n;
    len -= n;

    // line
    if (fr.line != 0) {
        n = snprintf(p, len, ":%u", fr.line);

        p += n;
        len -= n;
    }

    // discriminator
    if (fr.discriminator != 0) {
        n = snprintf(p, len, " (discriminator %u)\n", fr.discriminator);

    } else {
        n = snprintf(p, len, "\n");
    }

    p += n;
    len -= n;

    return p - buf;
}


static int
backtrace_symbol_write(int fd, const char *text, void *addr)
{
    frame_record_t  fr;
    int             n;
    char            buf[BUFFER_LENGTH + 1];

    if (symbol_table_find(&symtab, addr, &fr)) {
        n = backtrace_symbol_format(buf, BUFFER_LENGTH, text, fr);

    } else {
        n = snprintf(buf, BUFFER_LENGTH, "%s\n", text);
    }

    buf[n] = '\0';

    if (write(fd, buf, strlen(buf)) == -1) {
        return -1;
    }

    return 0;
}

