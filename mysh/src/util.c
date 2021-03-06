// file: mysh/src/util.c
// by  : jooh@cuni.cz
// for : nswi015
// lic.: mit

////////////////////////////////////////////////////////////////////////
// directives & globals

#include <ctype.h>  /* isspace */
#include "mysh.h"   /* mysh.h includes:
                     * stdio, stdlib, editline/readline, errno,
                     * histedit, string, sys/queue */

extern int errno;

////////////////////////////////////////////////////////////////////////
// util: string management

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
        ++s;
    }
    if (*s == 0)
    {
        return 0;
    }
    end = *s + strlen(*s) - 1;
    while (end > *s && isspace(*end))
    {
        --end;
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
        ++s;
    }
    return 1;
}

/*
 * unred
 * - Return the substring until a redirection character, if one exists,
 *   or a copy of the entire string if it does not.
 */
char*
unred(const char* s)
{
    unsigned int n = 0;

    for (unsigned long i = 0; i < strlen(s); i++)
    {
        if (s[i] == '<' || s[i] == '>')
        {
            n = i;
            break;
        }
    }

    if (n > 0)
    {
        return strndup(s, n);
    }

    char* cp = malloc(sizeof(char) * LIM);

    if (cp == NULL)
    {
        warnx("malloc error");
        return NULL;
    }

    if (strlen(s) > LIM)
    {
        warnx("input truncated");
    }

    strcpy(cp, s);

    return cp;
}

////////////////////////////////////////////////////////////////////////
// util: convenience printing

/*
 * motd
 * - First thing to print in stdin-mode of REPL.
 */
char*
motd(void)
{
    return "mysh v0.2\n(\"exit\" or C-d to exit. C-c to cancel.)";
}

/*
 * prompt
 * - String to print each time user input is being waited on.
 */
char*
prompt(void)
{
    static char ret[LIM + 8];
    char cwd[LIM];

    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        cwd[0] = 0;
    }

    sprintf(ret, "(mysh)%s> ", cwd);

    return ret;
}

