#-------------------------------------
# Makefile for irc bot in posix C
# using epoll with dynamically 
# loaded .so for plugins
#-------------------------------------
APP		= resource-mgr 
SRCDIR		= src
INCDIR 		= ./include
OBJDIR		= build
PLUGIN_DIR	= plugins

CC		= gcc

CFLAGS  += -g -O0 -Wall -Wstrict-prototypes -Werror -Wmissing-prototypes
CFLAGS  += -Wmissing-declarations -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS  += -Wsign-compare -std=gnu99 -pedantic 
CFLAGS  += -fno-omit-frame-pointer #-fsanitize=address 
CFLAGS  += -I`pg_config --includedir`

LDFLAGS = -ldl -lpq -pthread
LDFLAGS += $(shell pkg-config libpq --libs)
SRCS 	= $(wildcard $(SRCDIR)/*.c)
S_OBJS 	= $(SRCS:src/%.c=$(OBJDIR)/%.o)

# TODO: same thing for plugin_srcs/plugin_s_objs & maybe plugin_libs
# should allow me to use 2 rules to build all the plugins instead of 2per

.PHONEY: rebuild all clean
all: $(APP) $(OBJDIR)/libresource-mgr.so

# Application
$(APP): $(OBJDIR)/main.o $(OBJDIR)/mem.o $(OBJDIR)/reload.o $(OBJDIR)/ini.o $(OBJDIR)/app.o
	$(CC) -rdynamic $(CFLAGS) -o $@ $^ $(LDFLAGS)

#$(OBJDIR)/%.o: $(SRCDIR)/%.c
$(OBJDIR)/ini.o: $(SRCDIR)/ini.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)
$(OBJDIR)/reload.o: $(SRCDIR)/reload.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)
$(OBJDIR)/mem.o: $(SRCDIR)/mem.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)
$(OBJDIR)/main.o: $(SRCDIR)/main.c 
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCDIR)

# Plugins build & .so
$(OBJDIR)/libresource-mgr.so: $(OBJDIR)/app.o
	$(CC) -shared $(CFLAGS) -Wl,-soname,$(@F) -o $@ $< -lc

$(OBJDIR)/app.o: $(SRCDIR)/app.c
	$(CC) -fpic $(CFLAGS) -c $< -o $@ -I$(INCDIR)

# make build dir
$(OBJDIR):
	@mkdir -p $(OBJDIR)
	# TODO: once there are plugins move to a build/plugins/ dir.
	# so theres not a rule per plugin. 
#	@mkdir -p $(OBJDIR)/$(PLUGIN_DIR)

clean:
	@rm -f $(OBJDIR)/*
	@rm -f $(APP)


rebuild:
	$(MAKE) clean
	$(MAKE) all
