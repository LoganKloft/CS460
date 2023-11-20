#include "ucode.c"
int console1;
int console2;
int console3;

int parent()
{
    int pid, status;
    while (1) {
        printf("INIT : wait for ZOMBIE child\n");
        pid = wait(&status);

        if (pid == console1) {
            // console 1
            printf("INIT: forks a new console login\n");
            console1 = fork();
            if (!console1) exec("login /dev/tty0");
            continue;
        }
        else if (pid == console2) {
            // console 2
            printf("INIT: forks a new console login\n");
            console2 = fork();
            if (!console2) exec("login /dev/ttyS0");
            continue;
        }
        else if (pid == console3) {
            // console 3
            printf("INIT: forks a new console login\n");
            console3 = fork();
            if (!console3) exec("login /dev/ttyS1");
            continue;
        }
        printf("INIT: I just buried an orphan child proc %d\n", pid);
    }
}

// fork tty0, ttyS0, ttyS1
int main()
{
    int in, out;

    in = open("/dev/tty0", O_RDONLY);
    out = open("/dev/tty0", O_WRONLY);

    // console 1
    printf("INIT : fork a login proc on /dev/tty0\n");
    console1 = fork();
    if (!console1) exec("login /dev/tty0");

    // console 2
    printf("INIT : fork a login proc on /dev/ttyS0\n");
    console2 = fork();
    if (!console2) exec("login /dev/ttyS0");

    // console 3
    printf("INIT : fork a login proc on /dev/ttyS1\n");
    console3 = fork();
    if (!console3) exec("login /dev/ttyS1");

    parent();
}