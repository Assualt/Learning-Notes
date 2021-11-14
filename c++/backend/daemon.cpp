#include <signal.h> // signal
#include <stdio.h>
#include <stdlib.h> //exit
#include <sys/param.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h> // fork
void BeComeDaemon() {

    //忽略终端I/O信号，STOP信号
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    // （1）让程序在后台执行。方法是调用fork（）产生一个子进程，然后使父进程退出。
    int pid = fork();
    if (pid > 0) { // 父进程结束
        printf("father process exit ..\n");
        exit(0);
    } else if (pid < 0) {
        printf("fork error \n");
        return;
    }
    // （2）调用setsid（）创建一个新对话期。控制终端、登录会话和进程组通常是从父进程继承下来的，守护进程要摆脱它们，不受它们的影响，方法是调用setsid（）使进程成为一个会话组长。setsid（）调用成功后，进程成为新的会话组长和进程组长，并与原来的登录会话、进程组和控制终端脱离。
    setsid();

    // （3）禁止进程重新打开控制终端。经过以上步骤，进程已经成为一个无终端的会话组长，但是它可以重新申请打开一个终端。为了避免这种情况发生，可以通过使进程不再是会话组长来实现。再一次通过fork（）创建新的子进程，使调用fork的进程退出
    pid = fork();
    if (pid > 0) {
        exit(0);
    } else if (pid < 0) {
        return;
    }

    //关闭所有从父进程继承的不再需要的文件描述符
    for (int i = 0; i < NOFILE; close(i++))
        ;

    //改变工作目录，使得进程不与任何文件系统联系
    chdir("/");

    //将文件当时创建屏蔽字设置为0
    umask(0);

    //忽略SIGCHLD信号
    signal(SIGCHLD, SIG_IGN);
}

int main(int argc, char const *argv[]) {
    /* code */

    time_t tNow;
    // BeComeDaemon();
    daemon(0, 0);
    syslog(LOG_USER | LOG_INFO, "Test Daemon Process\n");
    while (1) {
        sleep(10);
        time(&tNow);
        syslog(LOG_USER | LOG_INFO, "SystemTime: \t%s\t\t\n", ctime(&tNow));
    }
    return 0;
}
