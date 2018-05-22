# TODO: need to split this out as there will probably be 
# a html2json app which shares a majority of the codebase.
# @ that point the makefile needs to handle both targets.
APP 	= renderer

SRCDIR 	= src
OBJDIR  = build

CC	= gcc

PREFIX?=/usr/local
INSTALL_DIR=$(PREFIX)/bin

CFLAGS  += -g -O0 -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes
CFLAGS  += -Wmissing-declarations -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS  += -I. -I./include `pg_config --includedir`
CFLAGS  += -Wsign-compare -std=c11 -pedantic

LDFLAGS = $(shell pkg-config libpq --libs)
LDFLAGS += -lm
SRCS	= $(wildcard $(SRCDIR)/*.c) 

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
