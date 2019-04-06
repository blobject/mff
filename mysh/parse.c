// file: mysh/parse.c
// by  : jooh@cuni.cz
// for : nswi015
// lic.: mit

////////////////////////////////////////////////////////////////////////
// directives && globals

#include <dirent.h>   /* DIR, {close,open}dir */
#include <regex.h>    /* reg{comp,exec,free} */
#include <sys/wait.h> /* wait */
#include "mysh.h"     /* mysh.h includes:
                       * stdio, stdlib, editline/readline, errno,
                       * histedit, string, sys/queue */

extern int errno;
extern int LINECOUNT;
extern int LASTOK;

////////////////////////////////////////////////////////////////////////
// parse & eval

/*
 * fill_words
 * - Helper to parse(), 3 levels deep in the structure-filling.
 */

int
fill_words(char* str, struct ltok* fill)
{
    struct ltok* words;
    char* w;
    char* w_cp;
    char* w_end = NULL;

    w = strtok_r(str, " \t", &w_end);
    while (w != NULL)
    {
        words = malloc(sizeof(struct ltok));
        if (words == NULL)
        {
            warnx("malloc error");
            return 1;
        }
        w_cp = malloc(strlen(w));
        if (w_cp == NULL)
        {
            warnx("malloc error");
            return 1;
        }

        strcpy(w_cp, w);
        trim(&w_cp);
        if (white(w_cp))
        {
            free(w_cp);
            w = strtok_r(NULL, " \t", &w_end);
            continue;
        }

        words->word = w_cp;
        TAILQ_INSERT_TAIL(&fill->head, words, list);

        w = strtok_r(NULL, " \t", &w_end);
    }

    return 0;
}

/*
 * fill_cmds
 * - Helper to parse(), 2 levels deep in the structure-filling.
 */

int
fill_cmds(char* str, struct lltok* fill)
{
    struct lltok* cmds;
    struct ltok* cmd;
    char* c;
    char* c_cp;
    char* c_end = NULL;

    c = strtok_r(str, "|", &c_end);
    while (c != NULL && !white(c))
    {
        cmds = malloc(sizeof(struct lltok));
        if (cmds == NULL)
        {
            warnx("malloc error");
            return 1;
        }
        c_cp = malloc(strlen(c));
        if (c_cp == NULL)
        {
            warnx("malloc error");
            return 1;
        }
        strcpy(c_cp, c);

        cmd = malloc(sizeof(struct ltok));
        if (cmd == NULL)
        {
            warnx("malloc error");
            return 1;
        }
        TAILQ_INIT(&cmd->head);

        if ((fill_words(c_cp, cmd)) != 0)
        {
            return 1;
        }

        cmds->cmd = cmd;
        TAILQ_INSERT_TAIL(&fill->head, cmds, list);

        c = strtok_r(NULL, "|", &c_end);
    }

    return 0;
}

/*
 * fill_semis
 * - Helper to parse(), 1 level deep in the structure-filling.
 */

int
fill_semis(char* str, struct llltok* fill)
{
    struct llltok* semis;
    struct lltok* semi;
    char* s;
    char* s_cp;
    char* s_end = NULL;

    s = strtok_r(str, ";", &s_end);
    while (s != NULL && !white(s))
    {
        semis = malloc(sizeof(struct llltok));
        if (semis == NULL)
        {
            warnx("malloc error");
            return 1;
        }
        s_cp = malloc(strlen(s));
        if (s_cp == NULL)
        {
            warnx("malloc error");
            return 1;
        }
        strcpy(s_cp, s);

        semi = malloc(sizeof(struct lltok));
        if (semi == NULL)
        {
            warnx("malloc error");
            return 1;
        }
        TAILQ_INIT(&semi->head);

        if ((fill_cmds(s_cp, semi)) != 0)
        {
            return 1;
        }

        semis->semi = semi;
        TAILQ_INSERT_TAIL(&fill->head, semis, list);

        s = strtok_r(NULL, ";", &s_end);
    }

    return 0;
}

/*
 * parse
 * - Split input on semicolon, then on pipe, then on whitespace.
 *   Accomplish this by using 3 nested loops. See fill_*().
 */
int
parse(char* line, struct llltok* all)
{
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
        return -1;
    }
    regfree(&re);

    /* ignore comments */
    char* p = strstr(line, "#");

    if (p != NULL)
    {
        *p = '\0';
    }

    /* nested hydration */
    if ((fill_semis(line, all)) != 0)
    {
        return 1;
    }

    return 0;
}

/*
 * eval
 * - Evaluate result of parse.
 * - Converts llltok into behavior (ie. system calls or native
 *   functionality).
 */
int
eval(const struct llltok* semis)
{
    struct llltok* semi;
    struct lltok* cmd;
    struct ltok* word;
    unsigned int size;
    unsigned int count;
    pid_t child;
    int status;

    /* handle semicolon-split commands (see parse()) */
    TAILQ_FOREACH(semi, &semis->head, list)
    {
        TAILQ_FOREACH(cmd, &semi->semi->head, list)
        {
            size = 0;
            count = 0;
            TAILQ_FOREACH(word, &cmd->cmd->head, list)
            {
                size++;
            }

            char* a[size + 1];
            TAILQ_FOREACH(word, &cmd->cmd->head, list)
            {
                a[count++] = word->word;
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
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////
// misc. evaluatory behavior

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

