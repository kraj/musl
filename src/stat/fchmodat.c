#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include "syscall.h"
#include "kstat.h"
#include "statx.h"

int fchmodat(int fd, const char *path, mode_t mode, int flag)
{
	if (!flag) return syscall(SYS_fchmodat, fd, path, mode, flag);

	if (flag != AT_SYMLINK_NOFOLLOW)
		return __syscall_ret(-EINVAL);

	int ret, fd2;
	char proc[15+3*sizeof(int)];

#ifdef SYS_fstatat
	struct kstat st;
	if ((ret = __syscall(SYS_fstatat, fd, path, &st, flag)))
		return __syscall_ret(ret);
	mode_t get_mode = st.st_mode;
#else
	struct statx st;
	if ((ret = __syscall(SYS_statx, fd, path, flag, STATX_TYPE, &st)))
		return __syscall_ret(ret);
	mode_t get_mode = st.stx_mode;
#endif

	if (S_ISLNK(get_mode))
		return __syscall_ret(-EOPNOTSUPP);

	if ((fd2 = __syscall(SYS_openat, fd, path, O_RDONLY|O_PATH|O_NOFOLLOW|O_NOCTTY|O_CLOEXEC)) < 0) {
		if (fd2 == -ELOOP)
			return __syscall_ret(-EOPNOTSUPP);
		return __syscall_ret(fd2);
	}

	__procfdname(proc, fd2);
#ifdef SYS_fstatat
	ret = __syscall(SYS_fstatat, AT_FDCWD, proc, &st, 0);
	get_mode = st.st_mode;
#else
	ret = __syscall(SYS_statx, AT_FDCWD, proc, 0, STATX_TYPE, &st);
	get_mode = st.stx_mode;
#endif
	if (!ret) {
		if (S_ISLNK(get_mode)) ret = -EOPNOTSUPP;
		else ret = __syscall(SYS_fchmodat, AT_FDCWD, proc, mode);
	}

	__syscall(SYS_close, fd2);
	return __syscall_ret(ret);
}
