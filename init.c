// #include "ucode.c"
// int console1;
// int console2;
// int console3;

// int parent()
// {
//     int pid, status;
//     while (1) {
//         printf("INIT : wait for ZOMBIE child\n");
//         pid = wait(&status);

//         if (pid == console1) {
//             // console 1
//             printf("INIT: forks a new console login\n");
//             console1 = fork();
//             if (!console1) exec("login /dev/tty0");
//             continue;
//         }
//         else if (pid == console2) {
//             // console 2
//             printf("INIT: forks a new console login\n");
//             console2 = fork();
//             if (!console2) exec("login /dev/ttyS0");
//             continue;
//         }
//         else if (pid == console3) {
//             // console 3
//             printf("INIT: forks a new console login\n");
//             console3 = fork();
//             if (!console3) exec("login /dev/ttyS1");
//             continue;
//         }
//         printf("INIT: I just buried an orphan child proc %d\n", pid);
//     }
// }

// // fork tty0, ttyS0, ttyS1
// int main()
// {
//     int in, out;

//     in = open("/dev/tty0", O_RDONLY);
//     out = open("/dev/tty0", O_WRONLY);

//     // console 1
//     printf("INIT : fork a login proc on console\n");
//     console1 = fork();
//     if (!console1) exec("login /dev/tty0");

//     // console 2
//     printf("INIT : fork a login proc on console\n");
//     console2 = fork();
//     if (!console2) exec("login /dev/ttyS0");

//     // console 3
//     printf("INIT : fork a login proc on console\n");
//     console3 = fork();
//     if (!console3) exec("login /dev/ttyS1");

//     parent();
// }

#include "ucode.c"
int console;
int parent() // P1's code
{
    int pid, status;
    while(1){
    printf("INIT : wait for ZOMBIE child\n");
    pid = wait(&status);
    if (pid==console){ // if console login process died
    printf("INIT: forks a new console login\n");
    console = fork(); // fork another one
    if (console)
    continue;
    else
    exec("login /dev/tty0"); // new console login process
    }
    printf("INIT: I just buried an orphan child proc %d\n", pid);
    }
}
main()
{
    int in, out; // file descriptors for terminal I/O
    in = open("/dev/tty0", O_RDONLY); // file descriptor 0
    out = open("/dev/tty0", O_WRONLY); // for display to console
    printf("INIT : fork a login proc on console\n");
    console = fork();
    if (console) // parent
    parent();
    else // child: exec to login on tty0
    exec("login /dev/tty0");
}