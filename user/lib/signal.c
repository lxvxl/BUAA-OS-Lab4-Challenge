#include <lib.h>
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
	return syscall_sigaction(signum, act, oldact);
}
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    int r = syscall_sigprocmask(how, set, oldset);
    syscall_yield();
	return r;
}
void sigemptyset(sigset_t *set) {
    set->sig[0] = 0;
    set->sig[1] = 0;
}
void sigfillset(sigset_t *set) {
    set->sig[0] = 0xFFFFFFFF;
    set->sig[1] = 0xFFFFFFFF;
}
void sigaddset(sigset_t *set, int signum) {
    int i = (signum - 1) / 32;
    set->sig[i] |= 1 << ((signum - 1) % 32);
}
void sigdelset(sigset_t *set, int signum) {
    int i = (signum - 1) / 32;
    set->sig[i] &= ~(1 << ((signum - 1) % 32));
}
int sigismember(const sigset_t *set, int signum) {
    int i = (signum - 1) / 32;
    return (set->sig[i] & (1 << ((signum - 1) % 32))) > 0 ? 1 : 0;
}
int kill(u_int envid, int sig) {
    return syscall_kill(envid, sig);
}

