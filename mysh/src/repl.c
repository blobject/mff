// file: mysh/src/repl.c
// by  : jooh@cuni.cz
// for : nswi015
// lic.: mit

////////////////////////////////////////////////////////////////////////
// directives & globals

#include <fcntl.h>  /* open */
#include <setjmp.h> /* sigsetjmp */
#include <signal.h> /* sig{action,emptyset} */
#include "mysh.h"   /* mysh.h includes:
                     * stdio, stdlib, editline/readline, errno,
                     * histedit, string, sys/queue */

extern int errno;
sigjmp_buf ctrlc;

////////////////////////////////////////////////////////////////////////
// repl: init

/*
 * sigint_handler
 * - Handle C-c during el_gets().
 */
void
sigint_handler(int sig)
{
    (void)sig;
    printf("^C\n");
    siglongjmp(ctrlc, 1);
}

/*
 * eh_init
 * - Initialise libedit stuff.
 */
struct eh*
eh_init(char* me)
{
    struct eh* eh = malloc(sizeof(struct eh));

    if (eh == NULL)
    {
        warnx("malloc error");
        exit(ERR_FAIL);
    }

    EditLine* ed = el_init(me, stdin, stdout, stderr);
    History* hi = history_init();
    HistEvent* ev = malloc(sizeof(HistEvent));

    if (ev == NULL)
    {
        warnx("malloc error");
        exit(ERR_FAIL);
    }

    eh->editline = ed;
    eh->history = hi;
    eh->histevent = ev;

    if (hi == 0)
    {
        warnx("history init error");
        el_end(ed);
        exit(ERR_FAIL);
    }

    el_set(ed, EL_PROMPT, &prompt);
    el_set(ed, EL_EDITOR, "emacs");
    history(hi, ev, H_SETSIZE, LIM);
    el_set(ed, EL_HIST, history, hi);

    return eh;
}

/*
 * eh_end
 * - Finalise libedit stuff.
 */
void
eh_end(struct eh* eh)
{
    history_end(eh->history);
    el_end(eh->editline);
    free(eh->histevent);
    free(eh);
}

/*
 * opt
 * - Handle mysh arguments using getopt.
 * - Currently only handles "-c".
 * - If "-c" occurs, the rest are ignored
 */
int
opt(int argc, char** argv, char** line)
{
    int c;

    while ((c = getopt(argc, argv, "c:")) != -1)
    {
        switch (c)
        {
        case 'c':
            *line = strdup(optarg);
            return 0;
        default:
            return -1;
        }
    }

    return 1;
}

////////////////////////////////////////////////////////////////////////
// repl: inner

/*
 * get
 * - Get user input from stdin.
 * - C-d or "exit" breaks. (see loop())
 * - C-c or all-whitespace continues. (see loop())
 */
int
get(const char** line, EditLine* ed, int* count)
{
    while (sigsetjmp(ctrlc, 1) != 0);

    errno = 0;
    *line = el_gets(ed, count);
    fflush(stdin);
    if (errno == EINTR)
    {
        return 1;
    }
    if (*line == NULL)
    {
        return -1;
    }
    if (white(*line))
    {
        return 1;
    }

    return 0;
}

/*
 * rinse
 * - Get ready for the next iteration of REPL.
 */
void
rinse(struct llltok* semis)
{
    struct llltok* semi;
    struct lltok* cmds;
    struct lltok* cmd;
    struct ltok* words;
    struct ltok* word;

    while (!TAILQ_EMPTY(&semis->head))
    {
        semi = TAILQ_FIRST(&semis->head);
        cmds = semi->semi;
        while (!TAILQ_EMPTY(&cmds->head))
        {
            cmd = TAILQ_FIRST(&cmds->head);
            words = cmd->cmd;
            while (!TAILQ_EMPTY(&words->head))
            {
                word = TAILQ_FIRST(&words->head);
                TAILQ_REMOVE(&word->head, word, list);
                free(word->word); // the actual string
                free(word);
            }
            free(words);
            if (cmd->red_r)
            {
                free(cmd->red_r);
            }
            if (cmd->red_w)
            {
                free(cmd->red_w);
            }
            if (cmd->red_a)
            {
                free(cmd->red_a);
            }
            TAILQ_REMOVE(&cmds->head, cmd, list);
            free(cmd);
        }
        free(cmds);
        TAILQ_REMOVE(&semis->head, semi, list);
        free(semi);
    }

    ++LINECOUNT;
}

////////////////////////////////////////////////////////////////////////
// repl: outer

/*
 * loop_body
 * - read (-> history) -> parse -> eval -> rinse
 * - Separated into its own function for stdin/arg/file variability.
 */
int
loop_body(struct eh* eh, const char* line)
{
    int ok;

    /* history */
    history(eh->history, eh->histevent, H_ENTER, line);

    /* check */
    if ((ok = check(line)) != 0)
    {
        return ok;
    }

    /* parse */
    char* line_cp = malloc(sizeof(char) * LIM);

    if (line_cp == NULL)
    {
        warnx("malloc error");
        return 1;
    }

    if (strlen(line) > LIM)
    {
        warnx("input truncated");
    }

    strcpy(line_cp, line);

    struct llltok* semis = malloc(sizeof(struct llltok));

    if (semis == NULL)
    {
        warnx("malloc error");
        return 1;
    }

    TAILQ_INIT(&semis->head);
    if ((ok = parse(line_cp, semis)) > 0)
    {
        rinse(semis);
        free(semis);
        return 1;
    }
    if (ok < 0)
    {
        rinse(semis);
        free(semis);
        return 0;
    }
    free(line_cp);

    /* eval */
    if ((ok = eval(semis)) > 0)
    {
        rinse(semis);
        free(semis);
        return 1;
    }

    /* rinse */
    rinse(semis);
    free(semis);

    return 0;
}

/*
 * loop
 * - Wrapper around loop_body() with actual looping structure.
 * - Handle stdin/arg/file variability.
 */
int
loop(struct eh* eh, enum loop_type t, const char* line_or_file)
{
    int ok;
    const char* line;
    int getcount;
    int file;
    char ch;
    char readbuf[LIM];
    int readcount = 0;
    struct sigaction sa;

    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    switch (t)
    {

    case LOOP_STDIN:
        for (;;)
        {
            if ((ok = get(&line, eh->editline, &getcount)) < 0)
            { /* C-d */
                return 0;
            }
            if (ok > 0)
            { /* empty line */
                continue;
            }
            if (getcount <= 0)
            { /* el_gets() error */
                return ERR_FAIL;
            }
            if ((ok = loop_body(eh, line)) != 0)
            {
                if (ok > 0)
                {
                    return ok;
                }
            }
        }
        break;

    case LOOP_ARG:
        if (loop_body(eh, line_or_file) != 0)
        {
            return ERR_FAIL;
        }
        break;

    case LOOP_FILE:
        if ((file = open(line_or_file, O_RDONLY)) < 0)
        {
            warnx("file read error");
            return ERR_FAIL;
        }
        while (read(file, &ch, 1) > 0)
        {
            readbuf[readcount++] = ch;
            if (ch == '\n' || ch == 0)
            {
                readbuf[readcount] = 0;
                readcount = 0;
                if ((ok = loop_body(eh, readbuf)) != 0)
                {
                    break;
                }
            }
        }
        close(file);
        break;
    }

    return 0;
}

