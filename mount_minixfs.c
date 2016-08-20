#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <sysexits.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/uio.h>
#include <sys/linker.h>

void usage(void);
void rmslashes(char *rrpin, char *rrpout);
int checkpath(const char *path, char *resolved);
void build_iovec(struct iovec **iov, int *iovlen, const char *name, void *val, size_t len);

int
main(int argc, char *argv[])
{
	struct iovec *iov;
	int iovlen;
	int mntflags;
	char *dev, *dir, mntpath[MAXPATHLEN];
	char fstype[] = "minixfs";

	iov = NULL;
	iovlen = 0;
	mntflags = 0;

	if (argc != 3)
		usage();

	dev = argv[1];
	dir = argv[2];

	mntflags |= MNT_RDONLY;
	build_iovec(&iov, &iovlen, "fstype", fstype, (size_t)-1);
	build_iovec(&iov, &iovlen, "fspath", mntpath, (size_t)-1);
	build_iovec(&iov, &iovlen, "from", dev, (size_t)-1);

	if (nmount(iov, iovlen, mntflags) < 0)
		err(1, "%s", dev);

	exit(0);
}

void
usage(void)
{
	(void)fprintf(stderr, "usage: mount_minixfs  special node\n");
	exit(EX_USAGE);
}

void
rmslashes(char *rrpin, char *rrpout)
{
	char *rrpoutstart;

	*rrpout = *rrpin;
	for (rrpoutstart = rrpout; *rrpin != '\0'; *rrpout++ = *rrpin++) {

		/* skip all double slashes */
		while (*rrpin == '/' && *(rrpin + 1) == '/')
			 rrpin++;
	}

	/* remove trailing slash if necessary */
	if (rrpout - rrpoutstart > 1 && *(rrpout - 1) == '/')
		*(rrpout - 1) = '\0';
	else
		*rrpout = '\0';
}

int
checkpath(const char *path, char *resolved)
{
	struct stat sb;

	if (realpath(path, resolved) == NULL || stat(resolved, &sb) != 0)
		return (1);
	if (!S_ISDIR(sb.st_mode)) {
		errno = ENOTDIR;
		return (1);
	}
	return (0);
}

void
build_iovec(struct iovec **iov, int *iovlen, const char *name, void *val, size_t len)
{
	int i;

	if (*iovlen < 0)
		return;
	i = *iovlen;
	*iov = realloc(*iov, sizeof(**iov) * (i + 2));
	if (*iov == NULL) {
		*iovlen = -1;
		return;
	}
	(*iov)[i].iov_base = strdup(name);
	(*iov)[i].iov_len = strlen(name) + 1;
	i++;
	(*iov)[i].iov_base = val;
	if (len == (size_t)-1) {
		if (val != NULL)
			len = strlen(val) + 1;
		else
			len = 0;
	}
	(*iov)[i].iov_len = (int)len;
	*iovlen = ++i;
}
