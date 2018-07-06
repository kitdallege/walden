#-------------------------------------
# Makefile for irc bot in posix C
# using epoll with dynamically 
# loaded .so for plugins
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
CFLAGS  += -g -O0 -Wall -Wstrict-prototypes -Werror -Wmissing-prototypes
CFLAGS  += -Wmissing-declarations -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS  += -Wsign-compare -std=gnu11 -pedantic 
CFLAGS  += -fno-omit-frame-pointer #-fsanitize=address -ggdb 
CFLAGS  += -I`pg_config --includedir` 

LDFLAGS = -ldl -lpq -pthread
LDFLAGS += $(shell pkg-config libpq --libs)
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
$(LIB): $(OBJDIR)/app.o $(OBJDIR)/reload.o $(OBJDIR)/ini.o $(OBJDIR)/walker.o $(OBJDIR)/handlers.o $(OBJDIR)/watcher.o $(OBJDIR)/config.o $(OBJDIR)/hash_table.o $(OBJDIR)/fnv_hash.o
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

