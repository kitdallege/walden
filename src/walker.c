#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>

#include "walker.h"

#define MAX_LEN 256
#define MAX_NUM_PATHS 256
#define NOT_DOT(d_name) (strcmp(d_name, "..") != 0 && strcmp(d_name, ".") != 0)

Dirs* find_dirs(const char *dirpath)
{
	DIR *dir;
	char *dir_q[256];
	memset(dir_q, 0, MAX_LEN * sizeof (*dir_q));
	size_t dir_q_len = 0;
	char *path = malloc(strlen(dirpath) + 1);
    strcpy(path, dirpath);
	path[strlen(dirpath)] = '\0';
	Dirs *result = malloc(sizeof *result);
	result->paths = malloc(sizeof *result->paths * MAX_NUM_PATHS);
	result->count = 0;
	dir_q[0] = path;
	dir_q_len++;
	do {
		path = dir_q[--dir_q_len];
		dir = opendir(path);
		if (!dir) {
			fprintf(stderr, "missing dir:: %s \n", path);
			continue;
		}
		//fprintf(stderr, "path: %s size: %lu\n", path, strlen(path));
		struct dirent *entry;
		while ((entry = readdir(dir))) {
			if (entry->d_type & DT_DIR) {
				if (NOT_DOT(entry->d_name)) {
					char name[strlen(path) + strlen(entry->d_name) + 2];
					int ret = snprintf(name, 256, "%s/%s", path, entry->d_name);
					name[ret] = '\0';
					dir_q[dir_q_len++] = strdup(name);
				}
			}
		}
		closedir(dir);
		result->paths[result->count++] = path;
	} while (dir_q_len);
	return result;
}

void free_dirs(Dirs *dirs)
{
	while (dirs->count--) {
		free(dirs->paths[dirs->count]);
	}
	free(dirs->paths);
	free(dirs);
}

Files* find_files(const char *dirpath)
{
	DIR *dir;
	char *dir_q[256];
	memset(dir_q, 0, MAX_LEN * sizeof (*dir_q));
	size_t dir_q_len = 0;
	char *path = malloc(strlen(dirpath) + 1);
    strcpy(path, dirpath);
	path[strlen(dirpath)] = '\0';
	Files *result = malloc(sizeof *result);
	result->paths = malloc(sizeof *result->paths * MAX_NUM_PATHS);
	result->count = 0;
	dir_q[0] = path;
	dir_q_len++;
	do {
		path = dir_q[--dir_q_len];
		dir = opendir(path);
		if (!dir) {
			fprintf(stderr, "missing dir:: %s \n", path);
			continue;
		}
		//fprintf(stderr, "path: %s size: %lu\n", path, strlen(path));
		struct dirent *entry;
		while ((entry = readdir(dir))) {
			if (entry->d_type & DT_DIR) {
				if (NOT_DOT(entry->d_name)) {
					char name[strlen(path) + strlen(entry->d_name) + 2];
					int ret = snprintf(name, 256, "%s/%s", path, entry->d_name);
					name[ret] = '\0';
					dir_q[dir_q_len++] = strdup(name);
				}
			} else if (entry->d_type != DT_DIR && entry->d_type == DT_REG) {
				char name[strlen(path) + strlen(entry->d_name) + 2];
				int ret = snprintf(name, 256, "%s/%s", path, entry->d_name);
				name[ret] = '\0';
				result->paths[result->count++] = strdup(name);
			}
		}
		closedir(dir);
		free(path);
	} while (dir_q_len);
	return result;
}

void free_files(Files *files)
{
	while (files->count--) {
		free(files->paths[files->count]);
	}
	free(files->paths);
	free(files);
}

