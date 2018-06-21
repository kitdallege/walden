#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>

#include "files.h"

static uid_t user = 33;
static gid_t group = 33;

static mode_t perms = DEFFILEMODE; 
static mode_t dir_perms = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

char *read_file(const char *filename)
{
	char *data;
	struct stat st;
	int fd;
	fd = open(filename, O_RDONLY);
	fstat(fd, &st);
	data = malloc(st.st_size + 1);
	memset(data, 0, st.st_size + 1);
	read(fd, data, st.st_size);
	close(fd);
	return data;
}

/* each positional argument is assumed to be a path component which will
 * be joined via '/' together. the last argument must be null, as its is
 * used as a sentinal.
 * A char * is allocated and returned, it is up to the caller to free it!
 * ex: char *str = mk_abs_path(var1, var2, var3, null);
 */
char *mk_abs_path(char *base, ...)
{
        unsigned int count = 0;
        unsigned int size = 0;
        size_t s_len;
        char *p, *str, *dest;
        va_list args;
        // compute required memory
        str = (char *)base;
        va_start(args, base);
        while (str) {
                size += strlen(str);
                count++;
                str = va_arg(args, char *);
        }
        va_end(args);
        // allocate memory and copy args into dest separated by delimter.
        p = dest = malloc(sizeof(char *) * (size + count + 1));
        va_start(args, base);
        str = (char *)base;
        for (unsigned int i = 0; i < count; i++) {
			s_len = strlen(str);
			if (s_len) {
				if (i) { *p++ = '/'; }
				memcpy(p, str, s_len);
				p += s_len;
			}
			str = va_arg(args, char *);
        }
        va_end(args);
        *p = '\0';
        return dest;
}

int file_exists(const char *filepath)
{
	struct stat st;
	return !stat(filepath, &st);
}

int mkdir_p(const char *path)
{
	const size_t len = strlen(path);
	char lpath[PATH_MAX];
	char *p;

	errno = 0;

	if (len > sizeof(lpath) - 1) {
		errno = ENAMETOOLONG;
		return -1;
	}

	strcpy(lpath, path);

	for (p = lpath + 1; *p; p++) {
		if (*p == '/') {
			*p = '\0';
			if (mkdir(lpath, dir_perms)) {
				if (errno != EEXIST) {
					return -1;
				}
			}
			*p = '/';
		}
	}
	if (mkdir(lpath, dir_perms)) {
		if (errno != EEXIST) {
			return -1;
		}
	}
	return 0;
}

int write_file(const char *name, const char *data)
{
	// name + ~
	char temp_name[strlen(name) + 2];
	snprintf(temp_name, sizeof(temp_name), "%s~", name);
	if (file_exists(temp_name)) { 
		if (unlink(temp_name)) {
			if (errno != ENOENT) {
				fprintf(stderr, "unable to remove existing temp file: file:%s error:%s\n",
						temp_name, strerror(errno));
				return -1;	
			}
		}
	}
	// open temp file
	int fd = open(temp_name, O_RDWR|O_CREAT|O_TRUNC, perms);
	if (fd == -1) {
		fprintf(stderr, "failed to open file for writing: file: %s error:%s\n",
				temp_name, strerror(errno));
		return -1;
	}
	// write to it
	//dprintf(fd, data); // <- suspect this might be O(n) instead of O(1)
	write(fd, data, strlen(data)); // if len was passed in it'd help
	/*/  flush the buffer	(this accounts for almost 90% of call time) 
	if (fsync(fd)) {
		fprintf(stderr, "failed to fsync file: %s\n", strerror(errno));
		return -1;
	} */
	// change owner & group
	if (fchown(fd, user, group)) {
		fprintf(stderr, "failed to fsync file: file:%s error:%s\n",
				temp_name, strerror(errno));
		return -1;
	}
	// set file perms
	if (fchmod(fd, perms)) {
		fprintf(stderr, "failed to chmod file: file: %s error:%s\n",
				temp_name, strerror(errno));
		return -1;
	}
	if (close(fd)) {
		return -1;
	}
	// *atomic write via rename* [keep from causing an nginx error]
	if (rename(temp_name, name)) {
		fprintf(stderr, "failed to rename file: from-file:%s to-file:%s error:%s\n",
				temp_name, name, strerror(errno));
	   return -1;
	}
	return 0;
}

