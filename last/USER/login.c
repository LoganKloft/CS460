#include "ucode.c"
int in, out, err; char username[128], password[128];

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
    settty(argv[1]);

    // (4) open /etc/passwd for read
    int passwd = open("/etc/passwd", O_RDONLY);
    char buf[BLKSIZE];
    read(passwd, buf, BLKSIZE);
    char* lines[10];
    int linecount = 10;
    lines[0] = strtok(buf, "\n");
    printf("LINES[0]: %s\n", lines[0]);
    for (int i = 1; i < linecount; i++)
    {
        lines[i] = strtok(0, "\n");
        if (lines[i] == 0) break;
        printf("LINES[%d]: %s\n", i, lines[i]);
    }

    while(1)
    {
        // (5) ask for username and password
        printf("login: "); gets(username);
        printf("password: "); gets(password);

        // (6) search /etc/passwd for matching credentials
        for (int i = 0; i < linecount; i++)
        {
            if (lines[i] == 0) break;

            char line[128];
            strcpy(line, lines[i]);
            printf("line: %s\n", line);
            char* user = strtok(line, ":");
            char* pass = strtok(0, ":");

            printf("USERNAME: %s\n", user);
            printf("PASSWORD: %s\n", pass);

            if ((strcmp(user, username)) == 0 && (strcmp(pass, password)) == 0)
            {
                // (7) update process owner to that of matching credentials
                char* uid = strtok(0, ":");
                char* guid = strtok(0, ":");
                chuid(atoi(uid), atoi(guid));
                strtok(0, ":");
                char* homedir = strtok(0, ":");
                chdir(homedir);
                close(passwd);

                // (8) exec user program
                char* program = strtok(0, ":");
                exec(program);
            }
        }
    }
}