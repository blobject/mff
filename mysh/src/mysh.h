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
#include <errno.h>     /* errno */
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
// globals

extern int LINECOUNT;
extern int LASTOK;

////////////////////////////////////////////////////////////////////////
// enums

enum loop_type
{
    LOOP_STDIN,
    LOOP_ARG,
    LOOP_FILE
};

////////////////////////////////////////////////////////////////////////
// structs
// - l{1,3}tok represent the nested structure of the input line:
//   - token = "word" (= string)
//   - "ltok" = "list of words" = "cmd" (delim by space)
//   - "lltok" = "list of cmds" = "semi" (delim by pipe)
//   - "llltok" = "list of semis" = "semis" (delim by semicolon)
//   - example:
//     cat /etc/passwd > txt | head ; echo foo
//     ^-^ ^---------^         ^--^   ^--^ ^-^ word
//     ^---------------****^   ^--^   ^------^ cmd
//     ^---------------------*----^   ^------^ semi
//     ^----------------------------*--------^ line

/*
 * ltok
 * - A "cmd". List of tokens, where token is a string.
 */
struct ltok
{
    char* word; /* mysh understands only strings */
    TAILQ_ENTRY(ltok) list;
    TAILQ_HEAD(, ltok) head;
};

/*
 * lltok
 * - A "semi". List of lists of tokens.
 */
struct lltok
{
    struct ltok* cmd;
    char* red_r;
    char* red_w;
    char* red_a;
    TAILQ_ENTRY(lltok) list;
    TAILQ_HEAD(, lltok) head;
};

/*
 * llltok
 * - A "list of semis". List of lists of lists of tokens.
 */
struct llltok
{
    struct lltok* semi;
    TAILQ_ENTRY(llltok) list;
    TAILQ_HEAD(, llltok) head;
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
// - See: "test.c" for unit tests

/* not yet unit-tested */
int trim(char** s);
int get(const char** l, EditLine* e, int* c);
int eval(const struct llltok* t);
void rinse(struct llltok* t);
int loop_body(struct eh* e, const char* l);
struct eh* eh_init(char* s);
void eh_end(struct eh* e);
int check(const char* l);
int redir(char t, const char* l, char** s);
char* unred(const char* l);

/* unit-tested in "test.c" */
int white(const char *s);
char* motd(void);
char* prompt(void);
char* bye(int b);
int cd(char** a, int n);
int parse(char* l, struct llltok* t);
int loop(struct eh* e, enum loop_type t, const char* l);
int opt(int c, char** a, char** l);

