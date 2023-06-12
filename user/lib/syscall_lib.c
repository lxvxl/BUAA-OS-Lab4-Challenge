#include <env.h>
#include <lib.h>
#include <mmu.h>
#include <syscall.h>
#include <trap.h>
int sig_inited = 0;
void syscall_sig_return() {
	msyscall(SYS_sig_return);
}

int syscall_sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
	int ret = msyscall(SYS_sigprocmask, how, set, oldset);
	syscall_yield();
	return ret;
}
int syscall_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
	if (sig_inited == 0) {
		msyscall(SYS_sig_init, syscall_sig_return);
		sig_inited = 1;
	}
	return msyscall(SYS_sigaction, signum, act, oldact);
}
int syscall_kill(u_int envid, int sig) {
	int ret = msyscall(SYS_kill, envid, sig);
	syscall_yield();
	return ret;
}

//打印一个字符
void syscall_putchar(int ch) {
	msyscall(SYS_putchar, ch);
}

//打印n个字符
int syscall_print_cons(const void *str, u_int num) {
	return msyscall(SYS_print_cons, str, num);
}

//获得当前进程的envid
u_int syscall_getenvid(void) {
	return msyscall(SYS_getenvid);
}

//切换进程
void syscall_yield(void) {
	msyscall(SYS_yield);
}

//销毁一个进程，这个进程必须是当前进程的子进程或自身
int syscall_env_destroy(u_int envid) {
	return msyscall(SYS_env_destroy, envid);
}

//设置env的env_user_tlb_mod_entry
int syscall_set_tlb_mod_entry(u_int envid, void (*func)(struct Trapframe *)) {
	return msyscall(SYS_set_tlb_mod_entry, envid, func);
}

//为va申请物理内存
int syscall_mem_alloc(u_int envid, void *va, u_int perm) {
	return msyscall(SYS_mem_alloc, envid, va, perm);
}

//将src进程中va地址对应的物理页面共享给dst进程中的va地址
int syscall_mem_map(u_int srcid, void *srcva, u_int dstid, void *dstva, u_int perm) {
	return msyscall(SYS_mem_map, srcid, srcva, dstid, dstva, perm);
}

//解除va对应的物理页面与va的映射
int syscall_mem_unmap(u_int envid, void *va) {
	return msyscall(SYS_mem_unmap, envid, va);
}

//设置env的status并维护env的空闲与调度队列
int syscall_set_env_status(u_int envid, u_int status) {
	return msyscall(SYS_set_env_status, envid, status);
}

//修改进程的tf
int syscall_set_trapframe(u_int envid, struct Trapframe *tf) {
	return msyscall(SYS_set_trapframe, envid, tf);
}

void syscall_panic(const char *msg) {
	int r = msyscall(SYS_panic, msg);
	user_panic("SYS_panic returned %d", r);
}

//尝试向其他进程发送信息
int syscall_ipc_try_send(u_int envid, u_int value, const void *srcva, u_int perm) {
	return msyscall(SYS_ipc_try_send, envid, value, srcva, perm);
}

//接收其他进程的信息
int syscall_ipc_recv(void *dstva) {
	return msyscall(SYS_ipc_recv, dstva);
}

int syscall_cgetc() {
	return msyscall(SYS_cgetc);
}

int syscall_write_dev(void *va, u_int dev, u_int len) {
	/* Exercise 5.2: Your code here. (1/2) */
	return msyscall(SYS_write_dev, va, dev, len);
}

int syscall_read_dev(void *va, u_int dev, u_int len) {
	/* Exercise 5.2: Your code here. (2/2) */
	return msyscall(SYS_read_dev, va, dev, len);
}


