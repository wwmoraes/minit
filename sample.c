#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    printf("creates a sleep daemon (orphan process)\n");
    printf("usage: sample <seconds>\n");
    exit(2);
  }

  printf("sample: forking\n");
  switch (vfork())
  {
  case -1:
    perror("sample");
    exit(EXIT_FAILURE);
  case 0:
    printf("sample: executing sleep %s\n", argv[1]);
    if (execlp("sleep", "sleep", argv[1], NULL) == -1)
    {
      perror("sample");
      _exit(EXIT_FAILURE);
    }
  default:
    printf("sample: fork done, exiting\n");
    exit(EXIT_SUCCESS);
  }
  exit(EXIT_SUCCESS);
}