#ifndef FILES_H
#define FILES_H

char *read_file(const char *filename);
char *mk_abs_path(char *base, ...);
int mkdir_p(const char *path);
int file_exists(const char *filepath);
int write_file(const char *name, const char *data);

#endif

