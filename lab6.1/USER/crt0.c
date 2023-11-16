int argc; char *argv[32]; // assume at most 32 tokens in cmdline
int parseArg(char *line)
{
    char *cp = line; argc = 0;
    while (*cp != 0){
        while (*cp == ' ') *cp++ = 0; // skip over blanks
            if (*cp != 0) // token start
                argv[argc++] = cp; // pointed by argv[ ]

        while (*cp != ' ' && *cp != 0) // scan token chars
            cp++;

        if (*cp != 0)
            *cp = 0; // end of token
        else
            break; // end of line

        cp++; // continue scan
    }

    argv[argc] = 0; // argv[argc]=0
}

int main0(char *cmdline)
{
    uprintf("main0: cmdline = %s\n", cmdline);
    parseArg(cmdline);
    main(argc, argv);
}