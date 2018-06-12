APP 	= renderer

SRCDIR 	= src
OBJDIR  = build

CC	= gcc

PREFIX?=/usr/local
INSTALL_DIR=$(PREFIX)/bin

CFLAGS  += -g -O0 -Wall -Wstrict-prototypes -Wmissing-prototypes
CFLAGS  += -Wmissing-declarations -Wshadow -Wpointer-arith -Wno-unused-function 
CFLAGS  += -I. -I./include -I`pg_config --includedir`
CFLAGS  += `pkg-config json-c --cflags` -I../mustach -I $(wildcard ./deps/*)
CFLAGS  += -Wsign-compare -std=gnu11 -pedantic

LDFLAGS = $(shell pkg-config libpq json-c --libs)
LDFLAGS += -lrt -lz -pthread
SRCS	= $(wildcard $(SRCDIR)/*.c) 
SRCS    += ../mustach/mustach-json-c.c ../mustach/mustach.c
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
