#ifndef WALKER_H
#define WALKER_H

typedef struct Dirs 
{
	size_t count;
	char **paths;
} Dirs;

Dirs* find_dirs(const char *dirpath);
void free_dirs(Dirs *dirs);

#endif
