// file: mysh/src/mysh.h
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

#define LIM 4096
#define ERR_SUCC   0
#define ERR_FAIL   1
#define ERR_ARG   10
#define ERR_FNE   11
#define ERR_EXEC 127
#define ERR_SIG  128
#define ERR_SYN  254

////////////////////////////////////////////////////////////////////////
// globals

extern int LINECOUNT;
extern int LASTOK;
enum loop_type
{
    LOOP_STDIN,
    LOOP_ARG,
    LOOP_FILE
};

////////////////////////////////////////////////////////////////////////
// structs
// - l{1,3}tok represent the nested syntactic structure of the input:
//   - token = "word" (= string)
//   - "ltok" = "list of words" = "cmd" (delim by space)
//   - "lltok" = "list of cmds" = "semi" (delim by pipe)
//   - "llltok" = "list of semis" = "semis" (delim by semicolon)
//   - example:
//     mysh> cat /etc/passwd > txt | head ; echo foo
//           ^-^ ^---------^         ^--^   ^--^ ^-^ word
//           ^---------------+---^   ^--^   ^------^ cmd
//           ^---------------------+----^   ^------^ semi
//           ^----------------------------+--------^ line
//     where "+"s refer to special lexemes pertinent to parsing.

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
    char* red_r; /* per-cmd redirection */
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
// - See: "src/test.c" for unit tests

/* not yet unit-tested */
int trim(char**);
int get(const char**, EditLine*, int*);
int eval(const struct llltok*);
void rinse(struct llltok*);
int loop_body(struct eh*, const char*);
struct eh* eh_init(char*);
void eh_end(struct eh*);
int check(const char*);
int redir(char, const char*, char**);
char* unred(const char*);
void sigint_handler(int);

/* unit-tested in "test.c" */
int white(const char*);
char* motd(void);
char* prompt(void);
char* bye(int);
int cd(char**, int);
int parse(char*, struct llltok*);
int loop(struct eh*, enum loop_type, const char*);
int opt(int, char**, char**);

