#-------------------------------------
# Makefile for resource-manager in posix C
# using epoll with inotify.
# dynamically (re)loads app.so at runtime.
#-------------------------------------
APP		= resource-mgr 
SRCDIR		= src
INCDIR 		= ./include
OBJDIR		= build
DEPSDIR		= ./deps
LIB 		= $(OBJDIR)/libresource-mgr.so
CC		= gcc

# TODO: Break up c/ld flags so that were only linking the app
# against libpq (etc).
# -flto -march=native -Ofast
CFLAGS  += -g -O0 -Wall -Wstrict-prototypes -Werror -Wmissing-prototypes
CFLAGS  += -Wmissing-declarations -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS  += -Wsign-compare -std=gnu11 -pedantic 

# this is needed for anonymous struct members
CFLAGS  += -fms-extensions 

# gcc memory debugging
CFLAGS  += -fno-omit-frame-pointer #-fsanitize=address -ggdb 
CFLAGS  += -I`pg_config --includedir` 

LDFLAGS = -ldl -lpq 
#LDFLAGS += $(shell pkg-config libpq --libs)
SRCS 	= $(wildcard $(SRCDIR)/*.c)
S_OBJS 	= $(SRCS:src/%.c=$(OBJDIR)/%.o)

.PHONEY: rebuild all clean

all: $(APP) $(LIB)

#########################################
# Application main build.
#########################################
$(APP): $(OBJDIR)/main.o $(OBJDIR)/reload.o 
	$(CC) -rdynamic $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/main.o: $(SRCDIR)/main.c 
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR) -I$(DEPSDIR)

#########################################
# App.so build
#########################################
$(LIB): $(OBJDIR)/app.o $(OBJDIR)/reload.o $(OBJDIR)/ini.o $(OBJDIR)/sha1.o
$(LIB): $(OBJDIR)/walker.o $(OBJDIR)/handlers.o $(OBJDIR)/watcher.o
$(LIB): $(OBJDIR)/config.o $(OBJDIR)/hash_table.o $(OBJDIR)/fnv_hash.o
	$(CC) -shared $(CFLAGS) -Wl,-soname,$(@F) -o $@ $^ -lc

$(OBJDIR)/app.o: $(SRCDIR)/app.c
	$(CC) -fpic $(CFLAGS) -c $< -o $@ -I$(INCDIR) -I$(DEPSDIR)

$(OBJDIR)/handlers.o: $(SRCDIR)/handlers.c
	$(CC) -fpic $(CFLAGS) -c $< -o $@ -I$(INCDIR) -I$(DEPSDIR)

$(OBJDIR)/walker.o: $(SRCDIR)/walker.c
	$(CC) -fpic $(CFLAGS) -c $< -o $@ -I$(INCDIR) -I$(DEPSDIR)

$(OBJDIR)/watcher.o: $(SRCDIR)/watcher.c
	$(CC) -fpic $(CFLAGS) -c $< -o $@ -I$(INCDIR) -I$(DEPSDIR)

$(OBJDIR)/config.o: $(SRCDIR)/config.c
	$(CC) -fpic $(CFLAGS) -c $< -o $@ -I$(INCDIR) -I$(DEPSDIR)

#########################################
# Deps
#########################################
$(OBJDIR)/reload.o: $(DEPSDIR)/reload/reload.c
	$(CC) -fpic $(CFLAGS) -c $< -o $@ -I$(DEPSDIR)/reload/

$(OBJDIR)/sha1.o: $(DEPSDIR)/sha1/sha1.c
	$(CC) -fpic $(CFLAGS) -c $< -o $@ -I$(DEPSDIR)/sha1/

$(OBJDIR)/hash_table.o: $(DEPSDIR)/hash_table/hash_table.c
	$(CC) -fpic $(CFLAGS) -c $< -o $@ -I$(DEPSDIR)/hash_table/
$(OBJDIR)/fnv_hash.o: $(DEPSDIR)/hash_table/fnv_hash.c
	$(CC) -fpic $(CFLAGS) -c $< -o $@ -I$(DEPSDIR)/hash_table/

$(OBJDIR)/ini.o: $(DEPSDIR)/inih/ini.c
	$(CC) -fpic $(CFLAGS) -c $< -o $@ -I$(DEPSDIR)/inih/


# make build dir
$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	@rm -f $(OBJDIR)/*
	@rm -f $(APP)

rebuild:
	$(MAKE) clean
	$(MAKE) all

