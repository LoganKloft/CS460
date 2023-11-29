#include "ucode.c"

int do_pipe(char* head, char* tail)
{
    int pid;
    int pd[2];
    pipe(pd);

    pid = fork();
    if (pid)
    {
        close(pd[1]);
        dup2(pd[0], 0);

        do_command(tail);
    }
    else
    {
        close(pd[0]);
        dup2(pd[1], 1);

        do_command(head);
    }
}

int do_redirection(char* line)
{
    // operate on copy of line for tokenization
    // but we edit line to remove redirection part
    char linecopy[128];
    char* tokens[32];
    int max_tokens = 32;
    int token_count = 0;
    int fd;

    strcpy(linecopy, line);
    token_count = tokenize(linecopy, tokens, max_tokens, " ");

    for (int i = 0; i < token_count; i++)
    {
        if ((strcmp(tokens[i], ">")) == 0)
        {
            fd = open(tokens[i + 1], O_WRONLY | O_CREAT);
            dup2(fd, 1);
            char* start = mystrstr(line, ">");
            *(start - 1) = 0; // -1 because assuming space before ">"
        }
        else if ((strcmp(tokens[i], ">>")) == 0)
        {
            fd = open(tokens[i + 1], O_WRONLY | O_CREAT | O_APPEND);
            dup2(fd, 1);
            char* start = mystrstr(line, ">>");
            *(start - 1) = 0; // -1 because assuming space before ">>"
        }
        else if ((strcmp(tokens[i], "<")) == 0)
        {
            fd = open(tokens[i + 1], O_RDONLY);
            dup2(fd, 0);
            char* start = mystrstr(line, "<");
            *(start - 1) = 0; // -1 because assuming space before "<"
        }
    }
}

int do_command(char* line)
{
    // (1) check for pipes and handle pipe in reverse order
    // we don't return from pipe
    int len = strlen(line);
    for (int i = len - 1; i >= 0; i--)
    {
        if (line[i] == '|')
        {
            char head[128] = "\0";
            char tail[128] = "\0";
            int headI = 0;
            int tailI = 0;

            for (int j = i + 2; j < len; j++, tailI++) // +2 to skip the white space after the pipe
            {
                tail[tailI] = line[j];
            }

            for (int j = 0; j <= i - 2; j++, headI++) // -2 to skip the white space after the pipe
            {
                head[headI] = line[j];
            }

            do_pipe(head, tail);
        }
    }

    // (2) handle redirections if any
    // we return from redirection
    do_redirection(line);

    // (3) execute command
    // we don't return from exec
    exec(line);
}

int main(int argc, char *argv[])
{
    signal(2, 1); // Control-C does not KILL sh
    int pid;
    int status;
    char cwd[128];
    char line[128];
    char linecopy[128];
    char* tokens[32];
    int max_tokens = 32;
    int token_count = 0;
    while (1)
    {
        // (1) prompt for command
        line[0] = 0;
        getcwd(cwd);
        printf("%s$ ", cwd);
        gets(line);

        // (2) parse command
        strcpy(linecopy, line);
        token_count = tokenize(linecopy, tokens, max_tokens, " ");
        if (!token_count) continue;

        // (3) check if trivial command
        if ((strcmp(tokens[0], "logout")) == 0)
        {
            uexit();
        }
        else if ((strcmp(tokens[0], "exit")) == 0)
        {
            uexit();
        }
        else if ((strcmp(tokens[0], "cd")) == 0)
        {
            if (token_count > 1)
            {
                chdir(tokens[1]);
            }
            else
            {
                chdir(0); // should go to home directory
            }
        }
        else
        {
            // (4) non-trivial command, child performs command while parent waits
            pid = fork();
            if (pid)
            {
                // parent
                pid = wait(&status);
            }
            else
            {
                // child
                // we don't return from do_command
                do_command(line);
            }
        }
    }
}