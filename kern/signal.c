#include <env.h>
#include <pmap.h>

#define bool_try(expr)                                                                                  \
	do {                                                                                       \
		if ((expr) != 0)                                                                        \
			return -1;                                                                  \
	} while (0)
extern struct Env *curenv;

struct sig_list free_sig_list;
struct tf_list free_tf_list;

int sig_alloc(struct SigTask **new) {
    if (LIST_EMPTY(&free_sig_list)) {
        struct Page *p;
        try(page_alloc(&p));
	    p->pp_ref++;
        for (struct SigTask *s = page2kva(p); s + 1 < page2kva(p) + BY2PG; s++) {
            LIST_INSERT_HEAD(&free_sig_list, s, sig_link);
        }
    }
    struct SigTask *s = LIST_FIRST(&free_sig_list);
    LIST_REMOVE(s, sig_link);
    *new = s;
    return 0;
}

int tf_alloc(struct LinkedTf **new) {
    if (LIST_EMPTY(&free_tf_list)) {
        struct Page *p;
        try(page_alloc(&p));
        p->pp_ref++;
	for (struct LinkedTf *t = page2kva(p); t + 1 < page2kva(p) + BY2PG; t++) {
            LIST_INSERT_HEAD(&free_tf_list, t, tf_link);
        }
    }
    struct LinkedTf *t = LIST_FIRST(&free_tf_list);
    LIST_REMOVE(t, tf_link);
    *new = t;
    return 0;
}

void sig_set_return_addr(void *addr) {
    curenv->sig_return_addr=addr;
}

int sig_register(int signum, const struct sigaction *act, struct sigaction *oldact) {
    if (signum < 1 || signum > 64) {
        return -1;
    }
    if (oldact) {
        *oldact=curenv->actions[signum-1];
    }
    curenv->actions[signum-1]=*act;
    return 0;
}

int modify_proc_mask(int how, const sigset_t *set, sigset_t *oldset) {
    if (oldset) {
        *oldset = curenv->proc_mask;
    }
    switch (how) {
    case SIG_BLOCK:
        curenv->proc_mask.sig[0] |= set->sig[0];
        curenv->proc_mask.sig[1] |= set->sig[1];
        break;
    case SIG_UNBLOCK:
        curenv->proc_mask.sig[0] &= ~set->sig[0];
        curenv->proc_mask.sig[1] &= ~set->sig[1];
        break;
    case SIG_SETMASK:
        curenv->proc_mask.sig[0] = set->sig[0];
        curenv->proc_mask.sig[1] = set->sig[1];
        break;    
    default:
        return -1;
    }
    return 0;
}

int sig_send(u_int envid, int sig) {
    struct SigTask *s;
    struct Env *e;
    bool_try(envid2env(envid, &e, 0));
    bool_try(sig_alloc(&s));
    s->signum = sig;
    s->state = SIG_WAITING;
    LIST_INSERT_HEAD(&e->sig_waiting_list, s, sig_link);
    return 0;
}

extern void env_pop_tf(struct Trapframe *tf, u_int asid) __attribute__((noreturn));

/**
 * 若刚刚运行完一个信号返回到这个函数，则将刚刚运行完毕的信号移出
 * 寻找下一个可以执行的信号。可执行<=>正在运行||非屏蔽
 * 若这个信号已经执行了一半，或没有信号可以执行，则还原一份环境
 * 否则，执行这个信号
 */
void do_signal(int finished) {
    struct SigTask *s;
    struct LinkedTf *tf = LIST_FIRST(&curenv->tf_stack);
    if (finished) {
	    Restart:    
	    s = curenv->running_task;
        LIST_REMOVE(s, sig_link);
        LIST_INSERT_HEAD(&free_sig_list, s, sig_link);    
    }
    //取出下一个可以执行的信号
    //可以执行<==>正在运行或者未被屏蔽
    sigset_t mask = curenv->proc_mask;
    LIST_FOREACH(s, &curenv->sig_waiting_list, sig_link) {
        if (s->state == SIG_RUNNING) {
            mask.sig[0] |= curenv->actions[s->signum-1].sa_mask.sig[0];
            mask.sig[1] |= curenv->actions[s->signum-1].sa_mask.sig[1];
        }
    }

    LIST_FOREACH(s, &curenv->sig_waiting_list, sig_link) {
        if (s->state == SIG_RUNNING) {
            break;
        }
        int i = (s->signum - 1) / 32;
        if ((mask.sig[i] & (1 << ((s->signum - 1) % 32))) == 0 || s->signum == 9) {
            s->state = SIG_RUNNING;
            curenv->running_task = s;
            void (*sa_handler)(int) = curenv->actions[s->signum-1].sa_handler;
            if (sa_handler == NULL) {
                goto Restart;
            } else if (sa_handler >= 0x80000000) {
                sa_handler(s->signum);
                goto Restart;
            }
            struct Trapframe newTf = tf->tf;
            newTf.cp0_epc = curenv->actions[s->signum-1].sa_handler;
            newTf.regs[31] = curenv->sig_return_addr;
            newTf.regs[4] = s->signum;
            env_pop_tf(&newTf, curenv->env_asid);
        }
    }    
    //若已经没有信号可以运行，或者这个信号正在运行，则取出一份环境进行运行
    LIST_REMOVE(tf, tf_link);
    LIST_INSERT_HEAD(&free_tf_list, tf, tf_link);
    curenv->running_task = s;
    env_pop_tf(&tf->tf, curenv->env_asid);
}


void sig_kill(int signum) {
    env_destroy(curenv);
}

void sig_segv(int signum) {
    env_destroy(curenv);
}

void sig_term(int signum) {
    env_destroy(curenv);
}
extern struct Env envs[NENV] __attribute__((aligned(BY2PG)));
void sig_init_env(u_int envid, u_int parent_id) {
    struct Env *e;
    if (envid == 0) {
	    e = curenv;
    } else {
	    e = envs + ENVX(envid);
    }
    if (parent_id == 0) {
        memset(e->actions, 0, 64*sizeof(struct sigaction)+sizeof(void*)+sizeof(sigset_t)+sizeof(struct SigTask*)+sizeof(struct tf_list)+sizeof(struct sig_list));
    } else {
        struct Env *p;
        envid2env(parent_id, &p, 0);
        memcpy(e->actions, p->actions, 64*sizeof(struct sigaction)+sizeof(void*)+sizeof(sigset_t));
    }
    e->actions[SIGKILL - 1].sa_handler = sig_kill;
    e->actions[SIGSEGV - 1].sa_handler = sig_segv;
    e->actions[SIGTERM - 1].sa_handler = sig_term;
}

void sig_free_env(u_int envid) {
    struct Env *e;
    if (envid == 0) {
	e = curenv;
    } else {
	e = envs + ENVX(envid);
    }
    struct LinkedTf *tf;
    struct SigTask *s;
    while(!LIST_EMPTY(&e->sig_waiting_list)) {
	s = LIST_FIRST(&e->sig_waiting_list);
        LIST_REMOVE(s, sig_link);
        LIST_INSERT_HEAD(&free_sig_list, s, sig_link);
    }
    while(!LIST_EMPTY(&e->tf_stack)) {
        tf = LIST_FIRST(&e->tf_stack);
        LIST_REMOVE(tf, tf_link);
        LIST_INSERT_HEAD(&free_tf_list, tf, tf_link);
    }
}



