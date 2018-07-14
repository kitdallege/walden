#ifndef WALKER_H
#define WALKER_H

typedef struct Dirs 
{
	size_t count;
	char **paths;
} Dirs;

typedef struct Files
{
	size_t count;
	char **paths;
} Files;
	

Dirs* find_dirs(const char *dirpath);
void free_dirs(Dirs *dirs);

Files* find_files(const char *dirpath);
void free_files(Files *dirs);

#endif
