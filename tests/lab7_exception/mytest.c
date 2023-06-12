#include <lib.h> 
void handler(int num){
    debugf("Reach handler1\n");
}

int main(int argc, char **argv) {
    //注册信号1，要求信号1在处理过程中屏蔽信号2
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = handler;

    debugf("The return value of sigaction(66, &act, NULL) is %d\n", sigaction(66, &act, NULL));
    debugf("The return value of sigaction(-7, &act, NULL) is %d\n", sigaction(-7, &act, NULL));
    debugf("The return value of sigprocmask(43, &set, NULL) is %d\n", sigprocmask(43, &act.sa_mask, NULL));
    debugf("The return value of kill(6499, 3) is %d, where 6499 is a invalid envid\n", kill(6499, 3));
    return 0;
}       
