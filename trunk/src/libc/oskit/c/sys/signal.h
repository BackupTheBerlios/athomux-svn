/*
 * Copyright (c) 1982, 1986, 1989, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)signal.h	8.2 (Berkeley) 1/21/94
 * signal.h,v 1.2 1994/08/02 07:53:32 davidg Exp
 */

#ifndef	_OSKIT_C_SYS_SIGNAL_H_
#define	_OSKIT_C_SYS_SIGNAL_H_

#include <oskit/types.h>
#include <oskit/compiler.h>

#define NSIG	32		/* counting 0; could be 33 (mask is 1-32) */

/*
 * These signal numbers are taken from BSD.
 * Note that POSIX.1 allows <signal.h> to define any SIG[A-Z]*
 * names it wants in addition to the POSIX.1-specified signals,
 * so there is no need to conditionalize these definitions.
 */
#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt */
#define	SIGQUIT	3	/* quit */
#define	SIGILL	4	/* illegal instruction (not reset when caught) */
#define	SIGTRAP	5	/* trace trap (not reset when caught) */
#define	SIGABRT	6	/* abort() */
#define	SIGIOT	SIGABRT	/* compatibility */
#define	SIGEMT	7	/* EMT instruction */
#define	SIGFPE	8	/* floating point exception */
#define	SIGKILL	9	/* kill (cannot be caught or ignored) */
#define	SIGBUS	10	/* bus error */
#define	SIGSEGV	11	/* segmentation violation */
#define	SIGSYS	12	/* bad argument to system call */
#define	SIGPIPE	13	/* write on a pipe with no one to read it */
#define	SIGALRM	14	/* alarm clock */
#define	SIGTERM	15	/* software termination signal from kill */
#define	SIGURG	16	/* urgent condition on IO channel */
#define	SIGSTOP	17	/* sendable stop signal not from tty */
#define	SIGTSTP	18	/* stop signal from tty */
#define	SIGCONT	19	/* continue a stopped process */
#define	SIGCHLD	20	/* to parent on child stop or exit */
#define	SIGTTIN	21	/* to readers pgrp upon background tty read */
#define	SIGTTOU	22	/* like TTIN for output if (tp->t_local&LTOSTOP) */
#define	SIGIO	23	/* input/output possible signal */
#define	SIGXCPU	24	/* exceeded CPU time limit */
#define	SIGXFSZ	25	/* exceeded file size limit */
#define	SIGVTALRM 26	/* virtual time alarm */
#define	SIGPROF	27	/* profiling time alarm */
#define SIGWINCH 28	/* window size changes */
#define SIGINFO	29	/* information request */
#define SIGUSR1 30	/* user defined signal 1 */
#define SIGUSR2 31	/* user defined signal 2 */

#if defined(_ANSI_SOURCE) || defined(__cplusplus)
/*
 * Language spec sez we must list exactly one parameter, even though we
 * actually supply three.  Ugh!
 */
#define	SIG_DFL		(void (*)(int))0
#define	SIG_IGN		(void (*)(int))1
#define	SIG_ERR		(void (*)(int))-1
#else
#define	SIG_DFL		(void (*)())0
#define	SIG_IGN		(void (*)())1
#define	SIG_ERR		(void (*)())-1
#endif

#ifndef _ANSI_SOURCE

typedef unsigned int sigset_t;

/*
 * POSIX 1003.1b: Generic value to pass back to an application.
 */
union sigval
{
	int	sival_int;
	void	*sival_ptr;
};

/*
 * This structure is passed to signal handlers
 * that use the new SA_SIGINFO calling convention (see below).
 */
typedef struct
{
	int		si_signo;
	int		si_code;
	union sigval	si_value;
} siginfo_t;

/* Values for si_code, indicating the source of the signal */
#define SI_USER		0	/* sent by kill(), raise(), or abort() */
#define SI_QUEUE	1	/* sent by sigqueue() */
#define SI_TIMER	2	/* generated by an expired timer */
#define SI_ASYNCIO	3	/* generated by completion of an async i/o */
#define SI_MESGQ	4	/* generated by the arrival of a message */
#define SI_IRQ		5	/* hardware int dispatched to application */
#define SI_EXCEP	6	/* processor or kernel-generated exception */

/*
 * Signal vector "template" used in sigaction call.
 */
struct	sigaction {
	union {				/* signal handler */
		void	(*sa_u_handler)(int);
		void	(*sa_u_sigaction)(int, siginfo_t *, void *);
	} sa_u;
	sigset_t sa_mask;		/* signal mask to apply */
	int	sa_flags;		/* see signal options below */
};
#define sa_handler	sa_u.sa_u_handler
#define sa_sigaction	sa_u.sa_u_sigaction

#ifndef _POSIX_SOURCE
#define SA_ONSTACK	0x0001	/* take signal on signal stack */
#define SA_RESTART	0x0002	/* restart system on signal return */
#define	SA_DISABLE	0x0004	/* disable taking signals on alternate stack */
#endif
#define SA_NOCLDSTOP	0x0008	/* do not generate SIGCHLD on child stop */
#define SA_SIGINFO	0x0010	/* use sa_sigaction calling convention */

/*
 * Flags for sigprocmask:
 */
#define	SIG_BLOCK	1	/* block specified signal set */
#define	SIG_UNBLOCK	2	/* unblock specified signal set */
#define	SIG_SETMASK	3	/* set specified signal set */

/*
 * POSIX 1003.1b:
 * Used when requesting queued notification of an event,
 * such as a timer expiration or a message arrival.
 */
struct sigevent
{
	int		sigev_notify;
	int		sigev_signo;
	union sigval	sigev_value;
};

/* Values for sigev_notify */
#define SIGEV_NONE	0
#define SIGEV_SIGNAL	1


OSKIT_BEGIN_DECLS
struct oskit_timespec;

int kill(oskit_pid_t pid, int sig);
int raise(int sig);
int sigaction(int sig, const struct sigaction *act,
		struct sigaction *oact);
int sigprocmask(int how, const sigset_t *set, sigset_t *oset);
int sigsuspend(const sigset_t *sigmask);
int sigwait(const sigset_t *set, int *sig);
int sigwaitinfo(const sigset_t *set, siginfo_t *info);
int sigtimedwait(const sigset_t *set, siginfo_t *info,
		const struct oskit_timespec *timeout);
int sigqueue(oskit_pid_t pid, int signo, const union sigval value);
void (*signal(int sig, void (*func)(int)))(int);

#define sigaddset(set, signo)	(*(set) |= 1 << ((signo) - 1), 0)
#define sigdelset(set, signo)	(*(set) &= ~(1 << ((signo) - 1)), 0)
#define sigemptyset(set)	(*(set) = 0, 0)
#define sigfillset(set)		(*(set) = ~(sigset_t)0, 0)
#define sigismember(set, signo)	((*(set) & (1 << ((signo) - 1))) != 0)

OSKIT_END_DECLS

#endif	/* !_ANSI_SOURCE */

#endif	/* !_OSKIT_C_SYS_SIGNAL_H_ */
