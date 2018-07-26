APP 	= renderer

SRCDIR 	= src
OBJDIR  = build

CC	= gcc

PREFIX?=/usr/local
INSTALL_DIR=$(PREFIX)/bin

# -flto -march=native -Ofast
CFLAGS  += -g -ggdb -O0 -Wall -Wstrict-prototypes -Wmissing-prototypes
CFLAGS  += -Wmissing-declarations -Wshadow -Wpointer-arith -Wno-unused-function 
CFLAGS  += -I. -I./include -I`pg_config --includedir`
CFLAGS  += `pkg-config json-c --cflags`
#TODO: move mustache to ./deps/ 
CFLAGS  += -I./deps/mustach -I./deps/bqueue
#TODO: it'd be nice to drop the gnu and use std c11 (fchown vs. chown .. etc).
CFLAGS  += -Wsign-compare -std=gnu11 -pedantic

LDFLAGS = $(shell pkg-config libpq json-c --libs)
LDFLAGS += -lrt -lz -pthread
SRCS	= $(wildcard $(SRCDIR)/*.c) 
#SRCS    += ./deps/mustach/mustach-json-c.c ./deps/mustach/mustach.c
SRCS    += $(wildcard ./deps/*/*.c)

S_OBJS=	$(SRCS:src/%.c=$(OBJDIR)/%.o)

all: $(APP)

$(APP): $(OBJDIR) $(S_OBJS)
	$(CC) $(S_OBJS) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: src/%.c 
	$(CC) $(CFLAGS) -c $< -o $@ 

$(OBJDIR):
	@mkdir -p $(OBJDIR)

install: $(APP)
	mkdir -p $(INSTALL_DIR)
	install -m 555 $(APP) $(INSTALL_DIR)/$(APP)	

uninstall:
	rm -f $(INSTALL_DIR)/$(APP)

clean:
	@rm -f $(OBJDIR)/*.o
	@rm -f $(APP)

.PHONY: all clean
