#include "../queue/queue.h"

#define BUFSIZE 1024
#define _PIPE_ "./named_pipes/worker_pipe"

int notify_pid = -1;
int listener_pipe[2];
queue q;

static void WorkerHandler(int sig);
static void ListenerHandler(int sig);
static void ManagerHandler(int sig);