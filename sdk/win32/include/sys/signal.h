/* sys/signal.h */

#ifndef _SYS_SIGNAL_H
#define _SYS_SIGNAL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

typedef unsigned long sigset_t;

#define SA_NOCLDSTOP 1  /* only value supported now for sa_flags */

typedef void (*_sig_func_ptr)(int);

struct sigaction 
{
	_sig_func_ptr sa_handler;
	sigset_t sa_mask;
	int sa_flags;
};

#define SIG_SETMASK 0	/* set mask with sigprocmask() */
#define SIG_BLOCK 1	/* set of signals to block */
#define SIG_UNBLOCK 2	/* set of signals to, well, unblock */
#define	SIGBUS	10	/* bus error */
#define	SIGPIPE	13	/* write on a pipe with no one to read it */

/* These depend upon the type of sigset_t, which right now 
   is always a long.. They're in the POSIX namespace, but
   are not ANSI. */
#define sigaddset(what,sig) (*(what) |= (1<<(sig)), 0)
#define sigdelset(what,sig) (*(what) &= ~(1<<(sig)), 0)
#define sigemptyset(what)   (*(what) = 0, 0)
#define sigfillset(what)    (*(what) = ~(0), 0)
#define sigismember(what,sig) (((*(what)) & (1<<(sig))) != 0)

int sigprocmask(int how, const sigset_t *set, sigset_t *oset);
int sigaction(int sig, const struct sigaction *act, struct sigaction *oact);

#if defined(_POSIX_THREADS)
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oset);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SIGNAL_H */
