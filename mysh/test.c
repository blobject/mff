// file: mysh/test.c
// by  : jooh@cuni.cz
// for : nswi015
// lic.: mit

#include "mysh.h"

////////////////////////////////////////////////////////////////////////
// unit test "framework"
//
// copied basically verbatim from MinUnit:
// http://www.jera.com/techinfo/jtns/jtn002.html

#define test_assert(msg, t) do { if (!(t)) return msg; } while (0)
#define test_run(t) do { \
    char* msg = t(); test_count++; if (msg) return msg; } while (0)
int test_count = 0;

////////////////////////////////////////////////////////////////////////
// tests

/* TODO: feeble coverage */

static char*
test_white1()
{
    test_assert("failed white()+ spaces", white("  ") == 1);
    return 0;
}

static char*
test_white2()
{
    test_assert("failed white()+ tabs", white("\t\t") == 1);
    return 0;
}

static char*
test_white3()
{
    test_assert("failed white()- alpha", white("foo") == 0);
    return 0;
}

static char*
test_motd()
{
    test_assert("failed motd()+",
                0 == strcmp(motd(), "mysh v0.1\n(\"exit\" or C-d to exit. C-c to cancel.)"));
    return 0;
}

static char*
test_prompt()
{
    char s[LIM + 7];
    char cwd[LIM];
    getcwd(cwd, sizeof(cwd));
    sprintf(s, "mysh:%s> ", cwd);
    test_assert("failed prompt()+", strcmp(prompt(), s) == 0);
    return 0;
}

static char*
test_cd1()
{
    char* a[1];
    a[0] = "cd";
    cd(a, 1);
    test_assert("failed cd()+ home",
                strcmp(getenv("PWD"), getenv("HOME")) == 0);
    return 0;
}

static char*
test_cd2()
{
    char* a[2];
    a[0] = "cd";
    a[1] = "/";
    cd(a, 2);
    test_assert("failed cd()+ root", strcmp(getenv("PWD"), "/") == 0);
    return 0;
}

static char*
test_parse1()
{
    char* s = "foo";
    struct lltok* tt = malloc(sizeof(struct lltok));
    TAILQ_INIT(&tt->head);
    parse(s, tt);
    char* word = TAILQ_FIRST(&(TAILQ_FIRST(&tt->head)->cmd)->head)->word;
    test_assert("failed parse()+ \"foo\"", strcmp(word, s) == 0);
    return 0;
}

static char*
test_parse2()
{
    struct lltok* tt = malloc(sizeof(struct lltok));
    TAILQ_INIT(&tt->head);
    parse("foo bar", tt);
    struct ltok* t;
    t = TAILQ_FIRST(&(TAILQ_FIRST(&tt->head)->cmd)->head);
    char* word1 = t->word;
    char* word2 = TAILQ_NEXT(t, list)->word;
    test_assert("failed parse()+ \"foo bar\"",
                (strcmp(word1, "foo") == 0)
                && (strcmp(word2, "bar") == 0));
    return 0;
}

static char*
test_opt()
{
    char* l;
    char* a[3];
    a[0] = "./mysh";
    a[1] = "-c";
    a[2] = "foo";
    opt(3, a, &l);
    test_assert("failed opt()+ \"-c foo\"",
                strcmp(l, "foo") == 0);
    return 0;
}

static char*
test_loopc()
{
    struct eh* eh = eh_init("mysh");
    loop(eh, 1, "cd");
    test_assert("failed loopc()+", 1);
    return 0;
}

////////////////////////////////////////////////////////////////////////
// run

static char*
test_all()
{
    test_run(test_white1); /*  1 */
    test_run(test_white2); /*  2 */
    test_run(test_white3); /*  3 */
    test_run(test_motd);   /*  4 */
    test_run(test_prompt); /*  5 */
    test_run(test_cd1);    /*  8 */
    test_run(test_cd2);    /*  9 */
    test_run(test_parse1); /* 10 */
    test_run(test_parse2); /* 11 */
    test_run(test_opt);    /* 12 */
    test_run(test_loopc);  /* 13 */
    return 0;
}

int
main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    char* res = test_all();
    if (res != 0)
    {
        printf("%s\n", res);
    }
    else
    {
        printf("ALL TESTS PASSED\n");
    }
    printf("%d tests run\n", test_count);
}

