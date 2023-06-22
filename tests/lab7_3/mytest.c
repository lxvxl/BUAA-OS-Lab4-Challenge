/**
 * Fork 强测
 * 产生多个进程，其父子进程形成一条链式结构，最末端的子进程给根进程发送一个信号11代表所有进程已经生成完毕。
 * 之后，根进程给直接子进程发送两次信号10，每个子进程都会在收到两次信号10之后给自己下一个直接子进程发送两次信号10，并销毁自己。
 */
#include <lib.h> 
int max = 10;
int father = 0;
int child = 0;
int cnt = 0;
int root;

void handler(int num){
      cnt++;
      debugf("cnt:%d HANDLER:%x %d\n",cnt,syscall_getenvid(),num);
}
struct sigaction act;
int main() {
      root = syscall_getenvid();
      act.sa_handler = handler;
      sigaction(10,&act,NULL);

      while (child == 0 && max != 0) {
            father = syscall_getenvid();
            child = fork();
            max--;
      }
      if(child == 0) {
            kill(root, 10);
            kill(root, 10);
      }
      while (cnt != 2);
      if (child != 0) {
            kill(child, 10);
            kill(child, 10);
      }
      return 0;
}