#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <proc/readproc.h>

#define EXIT_USAGE 2
#define EXIT_ENOEXEC 126 // "command invoked cannot execute"
#define EXIT_ENOENT 127  // "command not found"
#define EXIT_FATAL 128

#define ALARM_INTERVAL 1

// must ensures a system call returns successfully.
// Prints the system error message and exits otherwise
void must(int ret)
{
  if (ret == 0)
    return;

  perror("minit");
  exit(EXIT_FAILURE);
}

// setup_signals sets the handler for all supported signals
void setup_signals(void (*handler)(int))
{
  struct sigaction action = {0};
  for (int signum = 1; signum <= SIGUNUSED; signum++)
  {
    switch (signum)
    {
    // those signals cannot be handled, and will error if used on sigaction
    case SIGKILL:
    case SIGSTOP:
      break;

    default:;
      action.sa_handler = handler;
      action.sa_flags = 0;
      must(sigemptyset(&action.sa_mask));
      must(sigaddset(&action.sa_mask, signum));
      must(sigaction(signum, &action, NULL));
      break;
    }
  }
}

// kill_children forwards the signal to all children processes
void kill_children(int signum)
{
  pid_t ppid = getpid();
  PROCTAB *proc = openproc(PROC_FILLSTATUS);
  proc_t stats = {0};
  while (readproc(proc, &stats) != NULL)
  {
    if (stats.ppid != ppid)
      continue;

    killpg(stats.tid, signum);
  }
  closeproc(proc);
}

int usage(int status)
{
  FILE *output = status == EXIT_SUCCESS ? stdout : stderr;
  fprintf(output, "usage: minit <command> [arguments]\n");
  return status;
}

// reap consumes a terminated child process state so the kernel cleans it up
void reap()
{
  int stats;
  pid_t pid;

  switch (pid = waitpid(-1, &stats, WUNTRACED))
  {
  case -1:
    if (errno != ECHILD)
    {
      perror("reaper");
      exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
  case 0:
    fprintf(stderr, "reaper: unexpected PID 0\n");
    exit(EXIT_FAILURE);
  default:
    if (WIFSIGNALED(stats))
    {
      printf("reaper: child %d terminated with signal %d\n", pid, WTERMSIG(stats));
      return;
    }

    if (WIFEXITED(stats))
    {
      printf("reaper: child %d exited (%d)\n", pid, WEXITSTATUS(stats));
      return;
    }

    fprintf(stderr, "reaper: unexpected child %d state\n", pid);
    exit(EXIT_FATAL | EXIT_FAILURE);
  }
}

// receives and processes any signal. Most signals are forwarded to all children
// processes, with a few exceptions/side effects:
//
// * SIGCHLD is consumed to reap a terminated child processes
// * SIGTERM is forwarded, then minit itself will exit
// * SIGALRM is consumed to check if there's any children alive
void handle_signal(int signum)
{
  pid_t pid;
  int stats;

  switch (signum)
  {
  case SIGCHLD:
    reap();
    break;

  case SIGTERM:
    kill_children(SIGTERM);
    reap();
    exit(EXIT_SUCCESS);

  case SIGALRM:
    pid = waitpid(-1, &stats, __WALL | WNOHANG | WUNTRACED | WCONTINUED | WSTOPPED);
    switch (pid)
    {
    case -1:
      if (errno != ECHILD)
      {
        perror("minit");
        exit(EXIT_FAILURE);
      }
      exit(EXIT_SUCCESS);
      break;
    case 0:
      break;
    default:
      break;
    }
    alarm(ALARM_INTERVAL);
    break;

  default:
    kill_children(signum);
    break;
  }
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    fprintf(stderr, "no command provided\n");
    return usage(EXIT_USAGE);
  }

  setup_signals(handle_signal);

  prctl(PR_SET_CHILD_SUBREAPER, 1, 0, 0, 0);

  switch (vfork())
  {
  case -1:
    perror("minit");
    return EXIT_FAILURE;
  case 0:
    if (execvp(argv[1], &argv[1]) == -1)
    {
      perror("minit");
      switch (errno)
      {
      case ENOEXEC:
        _exit(EXIT_ENOEXEC);
      case EACCES:
        _exit(EXIT_ENOENT);
      default:
        _exit(EXIT_FAILURE);
      }
    }
  }

  alarm(ALARM_INTERVAL);
  for (;;)
    pause();
}
