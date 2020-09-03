#include <sys/wait.h>
#include "syscall.h"

pid_t waitpid(pid_t pid, int *status, int options)
{
	return __wait4(pid, status, options, 0, 1);
}
