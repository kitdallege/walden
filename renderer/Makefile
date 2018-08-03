APP 	= renderer

SRCDIR 	= src
OBJDIR  = build

CC	= gcc

PREFIX?=/usr/local
INSTALL_DIR=$(PREFIX)/bin

# -flto -march=native -Ofast
CFLAGS  += -g -O0 -Wall -Wstrict-prototypes -Wmissing-prototypes
CFLAGS  += -Wmissing-declarations -Wshadow -Wpointer-arith -Wno-unused-function 
CFLAGS  += -I. -I./include -I`pg_config --includedir`
CFLAGS  += `pkg-config json-c --cflags`
#TODO: move mustache to ./deps/ 
CFLAGS  += -I./deps/mustach -I./deps/bqueue  -I./build/
#TODO: it'd be nice to drop the gnu and use std c11 (fchown vs. chown .. etc).
CFLAGS  += -Wsign-compare -std=gnu11 -pedantic

LDFLAGS = $(shell pkg-config libpq json-c --libs)
LDFLAGS += -lrt -lz -pthread
SRCS	= $(wildcard $(SRCDIR)/*.c) 
#SRCS    += ./deps/mustach/mustach-json-c.c ./deps/mustach/mustach.c
SRCS    += $(wildcard ./deps/*/*.c)

S_OBJS=	$(SRCS:src/%.c=$(OBJDIR)/%.o)

all: $(APP)

$(APP): $(OBJDIR) $(S_OBJS) $(OBJDIR)/queries.h
	$(CC) $(S_OBJS) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: src/%.c 
	$(CC) $(CFLAGS) -c $< -o $@ 

$(OBJDIR)/queries.h: $(OBJDIR)
	@echo "Generating $@"
	@mkdir -p $(OBJDIR)/sql && \
	cd queries && \
	find . -type f -name '*.sql' | cut -d/ -f2 |  \
	xargs -I % bash -c "cat %  > ../$(OBJDIR)/sql/%"  && \
	cd ../$(OBJDIR)/sql && \
	truncate -s 0 ../queries.h && \
	find . -type f | cut -d/ -f2  | \
       	xargs -I % bash -c "xxd -i % >> ../queries.h"
	@cd $(OBJDIR) && sed -i 's/, 0x\([0-9a-f]\{2\}\)$$/\, 0x00/' queries.h
	@rm -f sql
	@echo "Generated $@"
#@cd $(OBJDIR) && sed -i 's/, 0x\([0-9a-f]\{2\}\)$$/\, 0x00/' queries.h

$(OBJDIR):
	@mkdir -p $(OBJDIR)

install: $(APP)
	mkdir -p $(INSTALL_DIR)
	install -m 555 $(APP) $(INSTALL_DIR)/$(APP)	

uninstall:
	rm -f $(INSTALL_DIR)/$(APP)

clean:
	@rm -rf $(OBJDIR)/*
	@rm -f $(APP)

.PHONY: all clean
