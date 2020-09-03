#define _BSD_SOURCE
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include "syscall.h"
#include "kstat.h"
#include "statx.h"

const char unsigned *__map_file(const char *pathname, size_t *size)
{
	const unsigned char *map = MAP_FAILED;
	int fd = sys_open(pathname, O_RDONLY|O_CLOEXEC|O_NONBLOCK);
	if (fd < 0) return 0;
#ifdef SYS_fstat
	struct kstat st;
	int r = syscall(SYS_fstat, fd, &st);
	size_t fsize = st.st_size;
#else
	struct statx st;
	int r = syscall(SYS_statx, fd, "", AT_EMPTY_PATH, STATX_SIZE, &st);
	size_t fsize = st.stx_size;
#endif
	if (!r) {
		map = __mmap(0, fsize, PROT_READ, MAP_SHARED, fd, 0);
		*size = fsize;
	}
	__syscall(SYS_close, fd);
	return map == MAP_FAILED ? 0 : map;
}
