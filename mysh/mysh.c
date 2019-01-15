// file: mysh/mysh.c
// by  : jooh@cuni.cz
// for : nswi015
// lic.: mit

/*
 * directives
 */

#include <ctype.h>    /* isspace */
#include <dirent.h>   /* DIR, {close,open}dir */
#include <errno.h>    /* errno */
#include <fcntl.h>    /* open */
#include <regex.h>    /* reg{comp,exec,free} */
#include <signal.h>   /* sig{action,emptyset} */
#include <sys/wait.h> /* wait */
#include "mysh.h"
/*
 * mysh.h includes:
 * stdio, stdlib, editline/readline, histedit, string, sys/queue
 */

////////////////////////////////////////////////////////////////////////
// globals

extern int errno;
int LINECOUNT = 1;
int LASTOK = ERR_SUCC;

////////////////////////////////////////////////////////////////////////
// helpers: string management

/*
 * trim
 * - Remove any leading or trailing whitespace.
 */
int
trim(char** s)
{
    char* end;
    while (isspace(**s))
    {
        s++;
    }
    if (*s == 0)
    {
        return 0;
    }
    end = *s + strlen(*s) - 1;
    while (end > *s && isspace(*end))
    {
        end--;
    }
    end[1] = '\0';
    return 0;
}

/*
 * white
 * - Determine whether string consists only of whitespace.
 */
int
white(const char *s)
{
    while (*s != '\0')
    {
        if (!isspace(*s))
        {
            return 0;
        }
        s++;
    }
    return 1;
}

////////////////////////////////////////////////////////////////////////
// helpers: convenience printing

/*
 * motd
 * - First thing to print in stdin-mode of REPL.
 */
char*
motd(void)
{
    return "mysh v0.1\n(\"exit\", or C-d to exit. C-c C-c to cancel line.)";
}

/*
 * prompt
 * - String to print each time user input is being waited on.
 */
char*
prompt(void)
{
    static char ret[LIM + 7];
    char cwd[LIM];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        cwd[0] = 0;
    }
    sprintf(ret, "mysh:%s> ", cwd);
    return ret;
}

/*
 * bye
 * - String to print before REPL quits.
 */
char*
bye(int nl)
{
    static char ret[6];
    char* n = "";
    if (nl)
    {
        n = "\n";
    }
    sprintf(ret, "%sbye!", n);
    return ret;
}

////////////////////////////////////////////////////////////////////////
// helpers: misc

/*
 * cd
 * - Implementation of shell-standard "cd".
 * - No arg goes home; "-" arg goes to prev; "FOO" arg goes to FOO.
 */
int
cd(char** args, int size)
{
    if (size > 2)
    {
        warnx("cd: too many arguments");
        return 1;
    }

    char* dst;
    if (size == 1)
    {
        dst = getenv("HOME");
    }
    else
    {
        dst = args[1];
    }
    if (strcmp("-", dst) == 0)
    {
        dst = getenv("OLDPWD");
    }

    DIR* dir = opendir(dst);
    if (dir)
    {
        closedir(dir);
    }
    else if (errno == ENOENT)
    {
        warnx("cd: no such directory");
        return 1;
    }
    else
    {
        warnx("cd: opendir() error");
        return 1;
    }

    setenv("OLDPWD", getenv("PWD"), 1);
    chdir(dst);
    char abs_dst[LIM];
    getcwd(abs_dst, sizeof(abs_dst)); /* assume no error */
    setenv("PWD", abs_dst, 1);
    return 0;
}

/*
 * sigint_handler
 * - Handle C-c.
 * - TODO: need to press C-c *twice* for desired functionality
 */
void
sigint_handler(int sig)
{
    (void)sig;
}

////////////////////////////////////////////////////////////////////////
// helpers: repl inner

/*
 * get
 * - Get user input from stdin.
 * - C-d or "exit" breaks. (see loop())
 * - C-c or all-whitespace continues. (see loop())
 */
int
get(const char** line, EditLine* ed, int* count)
{
    errno = 0;
    *line = el_gets(ed, count);
    fflush(stdin);
    if (errno == EINTR)
    {
        printf("^C\n");
        return 1;
    }
    if (*line == NULL)
    {
        printf("%s\n", bye(1));
        return -1;
    }
    if (white(*line))
    {
        return 1;
    }
    return 0;
}

/*
 * parse
 * - Split input on (semicolon first, then) whitespace.
 * - Converts string into lltok (ie. list of lists of tokens).
 */
int
parse(char* line, struct lltok* tts)
{
    const char* delim = " \t";
    struct lltok* tt;
    struct ltok* ts;
    struct ltok* t;
    char* phrasecopy;
    char* word;
    char* wordcopy;

    /* line pattern validity */
    regex_t re;
    if ((regcomp(&re, ";[ \t]*;", 0)) != 0)
    {
        warnx("regex compilation error");
        return 1;
    }
    if ((regexec(&re, line, 0, NULL, 0)) == 0)
    {
        LASTOK = ERR_SYN;
        regfree(&re);
        warnx("%d: syntax error", LINECOUNT);
        return 1;
    }
    if ((regcomp(&re, "[ \t][ \t]*[|<>][ \t][ \t]*", 0)) != 0)
    {
        warnx("regex compilation error");
        return 1;
    }
    if ((regexec(&re, line, 0, NULL, 0)) == 0)
    {
        LASTOK = ERR_SYN;
        regfree(&re);
        warnx("%d: syntax error", LINECOUNT);
        return 1;
    }
    if ((regcomp(&re, "[ \t][ \t]*>>[ \t][ \t]*", 0)) != 0)
    {
        warnx("regex compilation error");
        return 1;
    }
    if ((regexec(&re, line, 0, NULL, 0)) == 0)
    {
        LASTOK = ERR_SYN;
        regfree(&re);
        warnx("%d: syntax error", LINECOUNT);
        return 1;
    }
    regfree(&re);

    /* ignore comments */
    char* p = strstr(line, "#");
    if (p != NULL)
    {
        *p = '\0';
    }

    /* split on semicolons */
    char* phrase_end;
    char* word_end;
    char* phrase = strtok_r(line, ";", &phrase_end);
    while (phrase != NULL && !white(phrase))
    {
        tt = malloc(sizeof(struct lltok));
        if (tt == NULL)
        {
            warnx("malloc error");
            return 1;
        }
        phrasecopy = malloc(strlen(phrase));
        if (phrasecopy == NULL)
        {
            warnx("malloc error");
            return 1;
        }
        strcpy(phrasecopy, phrase);

        /* split on whitespace */
        ts = malloc(sizeof(struct ltok));
        if (ts == NULL)
        {
            warnx("malloc error");
            return 1;
        }
        TAILQ_INIT(&ts->head);
        word_end = NULL;
        word = strtok_r(phrasecopy, delim, &word_end);
        while (word != NULL)
        {
            t = malloc(sizeof(struct ltok));
            if (t == NULL)
            {
                warnx("malloc error");
                return 1;
            }
            wordcopy = malloc(strlen(word));
            if (wordcopy == NULL)
            {
                warnx("malloc error");
                return 1;
            }
            strcpy(wordcopy, word);
            trim(&wordcopy);
            if (white(wordcopy))
            {
                free(wordcopy);
                word = strtok_r(NULL, delim, &word_end);
                continue;
            }
            t->value = wordcopy;
            TAILQ_INSERT_TAIL(&ts->head, t, list);
            word = strtok_r(NULL, delim, &word_end);
        }

        /* add whitespace-split tokens into token of tokens */
        tt->token = ts;
        TAILQ_INSERT_TAIL(&tts->head, tt, list);

        /* rinse, ready to repeat */
        phrase = strtok_r(NULL, ";", &phrase_end);
    }

    return 0;
}

/*
 * eval
 * - Evaluate parsed tokens.
 * - Converts list of lists of tokens into behavior
 *   (ie. system calls or native functionality).
 */
int
eval(const struct lltok* tts)
{
    struct lltok* tt;
    struct ltok* t;
    unsigned int size;
    unsigned int count;
    pid_t child;
    int status;

    /* handle semicolon-split commands (see parse()) */
    TAILQ_FOREACH(tt, &tts->head, list)
    {
        size = 0;
        count = 0;
        TAILQ_FOREACH(t, &tt->token->head, list)
        {
            size++;
        }

        /* hydrate whitespace-split words (see parse()) */
        char* a[size + 1];
        TAILQ_FOREACH(t, &tt->token->head, list)
        {
            a[count++] = t->value;
        }
        a[count] = NULL;

        /* cd */
        if (strcmp("cd", a[0]) == 0)
        {
            cd(a, count);
            continue;
        }

        /* exit */
        if (strcmp("exit", a[0]) == 0)
        {
            printf("%s\n", bye(0));
            return 1;
        }

        /* exec */
        if ((child = fork()) < 0)
        {
            warnx("fork error");
            return -1;
        }
        else if (child == 0)
        {
            if (execvp(a[0], a) < 0)
            {
                warnx("%s: command not found", a[0]);
                exit(ERR_EXEC); /* exit forked child */
            }
            LASTOK = errno;
        }
        else
        {
            while (wait(&status) != child);
        }

        /* register child's exit status */
        if (WIFEXITED(status))
        {
            LASTOK = WEXITSTATUS(status);
        }
    }

    return 0;
}

/*
 * rinse
 * - Get ready for the next iteration of REPL.
 */
void
rinse(struct lltok* tts)
{
    struct lltok* tt;
    struct ltok* ts;
    struct ltok* t;
    while (!TAILQ_EMPTY(&tts->head))
    {
        tt = TAILQ_FIRST(&tts->head);
        ts = tt->token;
        while (!TAILQ_EMPTY(&ts->head))
        {
            t = TAILQ_FIRST(&ts->head);
            TAILQ_REMOVE(&ts->head, t, list);
            free(t);
        }
        TAILQ_REMOVE(&tts->head, tt, list);
        free(ts);
        free(tt);
    }
    free(tts);

    LINECOUNT++;
}

////////////////////////////////////////////////////////////////////////
// helpers: REPL outer

/*
 * loop_body
 * - read (-> history) -> parse -> eval -> rinse
 * - Separated into its own function for stdin/arg/file variability.
 */
int
loop_body(struct eh* eh, const char* line)
{
    int ok;
    char* linecopy;
    struct lltok* tts;

    /* history */
    history(eh->history, eh->histevent, H_ENTER, line);

    /* parse */
    linecopy = malloc(strlen(line));
    if (linecopy == NULL)
    {
        warnx("malloc error");
        return 1;
    }
    strcpy(linecopy, line);
    tts = malloc(sizeof(struct lltok));
    if (tts == NULL)
    {
        warnx("malloc error");
        return 1;
    }
    TAILQ_INIT(&tts->head);
    if ((ok = parse(linecopy, tts)) > 0)
    {
        return 1;
    }
    free(linecopy);

    /* eval */
    if ((ok = eval(tts)) > 0)
    {
        return 1;
    }

    /* rinse */
    rinse(tts);

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
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    int ok;
    const char* line;
    int getcount;
    int file;
    char ch;
    char readbuf[LIM];
    int readcount = 0;

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
                return ok;
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

////////////////////////////////////////////////////////////////////////
// helpers: init

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
 * eh_init
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

