#ifndef _SIGNAL_H_
#define _SIGNAL_H_
#include <queue.h>
#include <trap.h>
#include <env.h>

#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

#define SIG_RUNNING 0
#define SIG_WAITING 1

#define SIGKILL 9
#define SIGSEGV 11
#define SIGTERM 15

typedef struct sigset_t {
    int sig[2]; //最多 32*2=64 种信号
} sigset_t;

struct sigaction {
    void (*sa_handler)(int);
    sigset_t sa_mask;
};

struct SigTask {
    LIST_ENTRY(SigTask) sig_link;
    int signum;
    int state;
};

struct LinkedTf {
    LIST_ENTRY(LinkedTf) tf_link;
    struct Trapframe tf;
};

LIST_HEAD(sig_list, SigTask);
LIST_HEAD(tf_list, LinkedTf);

int tf_alloc(struct LinkedTf **new);
/**
 * @brief 为当前进程设置用户态syscall_sig_return的返回地址。若信号不存在信号页面，则初始化信号页面
 * @param envid 
 * @param addr 
 */
void sig_set_return_addr(void *addr);

/**
 * @brief 为当前进程注册新信号
 * @param signum 
 * @param act 
 * @param oldact 
 * @return int 若成功，返回0。若进程不存在，返回异常码
 */
int sig_register(int signum, const struct sigaction *act, struct sigaction *oldact);

/**
 * @brief 为当前进程修改进程掩码
 * @param how 
 * @param set 
 * @param oldset 若不为NULL，则将旧sigset存在此处
 * @return int how违法则返回-1
 */
int modify_proc_mask(int how, const sigset_t *set, sigset_t *oldset);

/**
 * @brief 为envid发送信号sig
 * @param envid 
 * @param sig 
 * @return int 若进程不存在、sig不在合法区域则返回-1，否则返回0
 */
int sig_send(u_int envid, int sig);

/**
 * @brief 选取下一个信号进行执行
 * 
 */
void do_signal(int finished);
void sig_init_env(u_int envid, u_int parent_id);
void sig_free_env(u_int envid);
#endif

