// file: mysh/mysh.h
// by  : jooh@cuni.cz
// for : nswi015
// lic.: mit

////////////////////////////////////////////////////////////////////////
// directives

#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h> /* "libedit" (probably unnecessary) */
#include <err.h>       /* warnx */
#include <histedit.h>  /* "libedit" */
#include <string.h>    /* strcmp, strcpy, strlen, strstr, strtok_r */
#include <sys/queue.h> /* TAILQ_* */
#include <unistd.h>    /* access, chdir, execvp, fork, getcwd, getopt */

#define LIM 1024
#define ERR_SUCC   0
#define ERR_FAIL   1
#define ERR_ARG   10
#define ERR_FNE   11
#define ERR_EXEC 127
#define ERR_SYN  254

////////////////////////////////////////////////////////////////////////
// structs
// - Currently, "ltok" and "lltok" are rather superfluous, as they serve
//   no other purpose than to store split strings.

/*
 * ltok
 * - List of tokens, where token is a string.
 */
struct ltok
{
    char* value; /* mysh understands only strings */
    TAILQ_ENTRY(ltok) list;
    TAILQ_HEAD(, ltok) head;
};

/*
 * lltok
 * - List of lists of tokens.
 */
struct lltok
{
    struct ltok* token;
    TAILQ_ENTRY(lltok) list;
    TAILQ_HEAD(, lltok) head;
};

/*
 * eh
 * - Wrapper around libedit structs.
 */
struct eh
{
    EditLine* editline;
    History* history;
    HistEvent* histevent;
};

////////////////////////////////////////////////////////////////////////
// for testing
// - See: "mysh.c" for implementation; "test.c" for unit tests

/* not yet unit-tested */
int trim(char** s);
int get(const char** l, EditLine* e, int* c);
void eval(const struct lltok* t);
void rinse(struct lltok* t);
int loop_body(struct eh* e, const char* l);
struct eh* eh_init(char* s);
void eh_end(struct eh* e);

/* unit-tested in "test.c" */
int white(const char *s);
char* motd(void);
char* prompt(void);
char* bye(int b);
int cd(char** a, int n);
int parse(char* l, struct lltok* t);
void loop(struct eh* e, int t, const char* l);
char* opt(int c, char** a);

