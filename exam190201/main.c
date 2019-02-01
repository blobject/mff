// file: exam190201/main.c
// by  : jooh@cuni.cz
// for : nswi015
// lic.: mit

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define LIST_MAX 100

const char* ME = NULL;

void
help(const char* me)
{
  printf("usage: %s <file1> <file2> <file3>\n", me);
}

void
follow(int* stop, char* str, int* count, const char* filename)
{
  assert(*count < LIST_MAX);

  int f = open(filename, O_RDONLY);
  if (f == -1)
  {
    errx(1, "%s", filename);
  }

  char line[PATH_MAX];
  int restcount = 0;

  // char
  read(f, line, 1);
  str[*count] = line[0];
  ++(*count);
  line[1] = '\0';

  // newline
  read(f, line, 1);
  assert(line[0] == '\n');

  // rest
  for (int i = 0; i < PATH_MAX; i++)
  {
    line[i] = '\0';
  }
  read(f, line, PATH_MAX);
  for (int i = 0; i < PATH_MAX; i++)
  {
    if (line[i] == '\0')
    {
      break;
    }
    ++restcount;
  }
  line[restcount - 1] = '\0';
  close(f);

  // stop or keep going
  if (restcount == 0 || line[0] == '\n' || line[0] == 0)
  {
    *stop = 1;
  }
  else
  {
    follow(stop, str, count, line);
  }
}

void
get(const char* f, char* str)
{
  int stop = 0;
  int count = 0;
  follow(&stop, str, &count, f);
}

int
main(int argc, char** argv)
{
  ME = argv[0];
  if (argc != 4)
  {
    help(ME);
    exit(1);
  }

  char one[PATH_MAX];
  char two[PATH_MAX];
  char out[PATH_MAX];
  pid_t pidone;
  pid_t pidtwo;
  int p[2];

  get(argv[1], one);
  get(argv[2], two);
  get(argv[3], out);

  printf("DEBUG: 1st command: %s\n", one);
  printf("DEBUG: 2nd command: %s\n", two);
  printf("DEBUG: output file: %s\n", out);

  if (pipe(p) == -1)
  {
    errx(1, "pipe");
  }

  pidone = fork();
  if (pidone == -1)
  {
    errx(1, "fork");
  }

  // child one
  if (pidone == 0)
  {
    printf("DEBUG: 1st child, executing %s\n", one);
    close(p[0]);
    dup2(p[1], 1);

    if (execlp(one, one, NULL) < 0)
    {
      errx(1, "exec (%s)", one);
    }
    exit(0);
  }

  pidtwo = fork();
  if (pidtwo == -1)
  {
    errx(1, "fork");
  }

  // child two
  if (pidtwo == 0)
  {
    printf("DEBUG: 2nd child, executing %s\n", two);

    // redirect output
    int f = open(out, O_CREAT | O_WRONLY, 0600);
    if (f == -1)
    {
      errx(1, "open for writing");
    }

    close(p[1]);
    dup2(p[0], 0);
    dup2(f, 1);
    close(f);

    if (execlp(two, two, NULL) < 0)
    {
      errx(1, "exec (%s)", two);
    }
    exit(0);
  }

  // parent
  wait(NULL);

  return 0;
}
