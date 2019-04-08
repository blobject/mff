// file: mysh/main.c
// by  : jooh@cuni.cz
// for : nswi015
// lic.: mit

////////////////////////////////////////////////////////////////////////
// mysh, a linux shell

#include "mysh.h"

int LINECOUNT;
int LASTOK;

/*
 * main
 * - Initialise, then decide which variation of the repl to run, then
 *   run the repl, finally returning proper exit codes.
 */
int
main(int argc, char** argv)
{
    LINECOUNT = 1;
    LASTOK = ERR_SUCC;
    int ok;
    char* line;
    struct eh* eh = eh_init(argv[0]);

    /* stdin */
    if (argc == 1)
    {
        printf("%s\n\n", motd());
        loop(eh, LOOP_STDIN, NULL); /* return value unused */
    }

    /* -c */
    else if ((ok = opt(argc, argv, &line)) == 0)
    {
        loop(eh, LOOP_ARG, line); /* return value unused */
    }
    else if (ok < 0)
    {
        LASTOK = ERR_ARG; /* getopt() prints its own err msg */
    }

    /* file */
    else
    {
        if (argc > 2)
        {
            warnx("too many arguments");
            LASTOK = ERR_ARG;
        }
        if(access(argv[1], F_OK) == -1)
        {
            warnx("file nonexistent");
            LASTOK = ERR_FNE;
        }
        loop(eh, LOOP_FILE, argv[1]); /* return value unused */
    }

    eh_end(eh);
    return LASTOK;
}

