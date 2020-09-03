#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "syscall.h"
#include "kstat.h"
#include "statx.h"

#define MAXTRIES 100

char *tmpnam(char *buf)
{
	static char internal[L_tmpnam];
	char s[] = "/tmp/tmpnam_XXXXXX";
	int try;
	int r;
	for (try=0; try<MAXTRIES; try++) {
		__randname(s+12);
#if defined(SYS_lstat)
		r = __syscall(SYS_lstat, s, &(struct kstat){0});
#elif defined(SYS_fstatat)
		r = __syscall(SYS_fstatat, AT_FDCWD, s,
			&(struct kstat){0}, AT_SYMLINK_NOFOLLOW);
#else
		r = __syscall(SYS_statx, AT_FDCWD, s, AT_SYMLINK_NOFOLLOW, 0,
			&(struct statx){0});
#endif
		if (r == -ENOENT) return strcpy(buf ? buf : internal, s);
	}
	return 0;
}
