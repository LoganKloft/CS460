#include "ucode.c"
int in, out, err; char name[128], password[128];

int main(int argc, char *argv[])
{
    // (1) close file descriptors 0 and 1 inherited from INIT
    close(0);
    close(1);

    // (2) open argv[1] 3 times as in(0), out(1), err(2)
    in = open(argv[1], O_RDONLY);
    out = open(argv[1], O_WRONLY);
    err = open(argv[1], O_WRONLY);

    // (3) setty(argv[1]);
    setty(argv[1]);

    // (4) open /etc/passwd for read

    // (5) parse /etc/password

    // (5) ask for username and password

    // (6) search /etc/passwd for matching credentials

    // (7) update process owner to that of matching credentials
}