使用以下命令可以运行测试点，共计九个测试点

- make test lab=7_sample1 && make run

  指导书样例1

- make test lab=7_sample2 && make run

  指导书样例2

- make test lab=7_sample3 && make run

  指导书样例3

- make test lab=7_sigset_basic && make run

  sigset相关函数功能测试

- make test lab=7_recursion && make run

  递归式地调用信号，信号重入测试

- make test lab=7_bigdata && make run

  压力测试

- make test lab=7_fork && make run

  进程间互发信号测试

- make test lab=7_sa_mask && make run

  信号运行过程中的掩码测试

- make test lab=7_exception && make run

  信号注册/发射/修改掩码函数异常返回值测试。
