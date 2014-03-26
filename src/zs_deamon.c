
#include <zs_core.h>

/* This function is followed the deamon function in UNP */

int_t
zs_deamon() 
{
    pid_t pid;

    if ((pid = fork()) < 0) {
        zs_err("Process cann't be forked 1st.\n");
        return ZS_ERR; 

    } else if (pid){
        _exit(0); 
    }

    if (setsid() < 0) { 
        zs_err("Cann't set sid.\n");
        return ZS_ERR; 
    }

    //signal(SIGHUP, SIG_IGN);
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGHUP, &sa, 0);

    if ((pid = fork()) < 0) {
        zs_err("Cann't forked 2nd.\n");
        return ZS_ERR; 

    } else if (pid) {
        _exit(0); 
    }

   // chdir("/");
    
    umask(0);

    int fd = open("/dev/null", O_RDWR);
    if (dup2(fd, STDIN_FILENO) == -1) {
        zs_err("deamon error.\n"); 
    }

    if (dup2(fd, STDOUT_FILENO) == -1) {
        zs_err("deamon error.\n"); 
    }

    if (fd > STDERR_FILENO) {
        close(fd); 
    }
    
    return ZS_OK;
}
