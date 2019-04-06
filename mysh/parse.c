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
 * parse
 * - Split input on semicolon, then on pipe, then on whitespace.
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
        return -1;
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
        return -1;
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
        return -1;
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

