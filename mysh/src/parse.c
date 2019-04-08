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
    if ((regcomp(&re, "^[ \t]*|", 0)) != 0)
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
    if ((regcomp(&re, "|[ \t\n]*$", 0)) != 0)
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
 * spawn
 * - Fork and pipe-handle a given command.
 */
int
spawn(char* const* cmd, int in, int first, int last)
{
    int fd[2];
    pipe(fd);
    int status = 0;
    pid_t child = fork();

    if (child < 0)
    {
        warnx("fork error");
        return -1;
    }
    if (child == 0)
    {
        if (in != -1)
        {
            if (first == 1)
            {
                dup2(fd[1], 1);
            }
            else if (first == 0 && last == 0)
            {
                dup2(in, 0);
                dup2(fd[1], 1);
                close(in);
            }
            else
            {
                close(fd[0]);
                dup2(in, 0);
            }
        }
        if (execvp(cmd[0], cmd) < 0)
        {
            warnx("%s: command not found", cmd[0]);
            exit(ERR_EXEC); /* exit forked child */
        }
        LASTOK = errno;
    }
    else
    {
        while (wait(&status) != child);
    }
    if (WIFEXITED(status))
    {
        LASTOK = WEXITSTATUS(status);
        if (LASTOK == ERR_EXEC)
        {
            return -1;
        }
    }

    if (in != 1)
    {
        close(fd[1]);
    }

    return fd[0];
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

    int cmd_count;
    int spawn_count;
    int count;
    int in = 0;
    int first = 1;

    TAILQ_FOREACH(semi, &semis->head, list)
    {

        cmd_count = 0;
        TAILQ_FOREACH(cmd, &semi->semi->head, list)
        {
            ++cmd_count;
        }

        spawn_count = 0;
        in = 0;
        first = 1;
        TAILQ_FOREACH(cmd, &semi->semi->head, list)
        {
            // build cmd as an array
            count = 0;
            TAILQ_FOREACH(word, &cmd->cmd->head, list)
            {
                ++count;
            }

            char* c[count + 1];
            count = 0;
            TAILQ_FOREACH(word, &cmd->cmd->head, list)
            {
                c[count++] = word->word;
            }
            c[count] = NULL;

            /* cd */
            if (strcmp("cd", c[0]) == 0)
            {
                cd(c, count);
                continue;
            }

            /* exit */
            if (strcmp("exit", c[0]) == 0)
            {
                return 1;
            }

            /* exec */
            if (cmd_count < 2)
            {
                if (spawn(c, -1, -1, -1) == -1)
                {
                    return 1;
                }
            }
            else
            {
                ++spawn_count;
                if (spawn_count < cmd_count)
                {
                    if ((in = spawn(c, in, first, 0)) == -1)
                    {
                        return 1;
                    }
                    first = 0;
                }
                else // last
                {
                    if ((in = spawn(c, in, 0, 1)) == -1)
                    {
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////
// native behavior

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

