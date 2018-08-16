SHELL = bash

APP 	= renderer

SRCDIR 	= src
OBJDIR  = build
DEPSDIR = ../deps
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
CFLAGS  += -I$(DEPSDIR)

#TODO: it'd be nice to drop the gnu and use std c11 (fchown vs. chown .. etc).
CFLAGS  += -Wsign-compare -std=gnu11 -pedantic

LDFLAGS = $(shell pkg-config libpq json-c --libs)
LDFLAGS += -lrt -lz -pthread
SRCS	= $(wildcard $(SRCDIR)/*.c) 
#SRCS    += ./deps/mustach/mustach-json-c.c ./deps/mustach/mustach.c
SRCS    += $(wildcard ./deps/*/*.c)
SRCS    += $(wildcard ../deps/*/*.c)

S_OBJS=	$(SRCS:src/%.c=$(OBJDIR)/%.o)

all: $(APP)

$(APP): $(OBJDIR) queries.o $(S_OBJS)
	$(CC) $(S_OBJS) $(OBJDIR)/queries.o $(LDFLAGS) -o $@

$(OBJDIR)/%.o: src/%.c 
	$(CC) $(CFLAGS) -c $< -o $@ 

#$(OBJDIR)/ini.o: $(DEPSDIR)/inih/ini.c
#	$(CC) $(CFLAGS) -c $< -o $@ 

queries.o: $(OBJDIR)
	@echo "Generating $@"
	@mkdir -p $(OBJDIR)/sql && \
	cd queries && \
	find *  -type f -name '*.sql' | \
	xargs -I % bash -c "cat %  > ../$(OBJDIR)/sql/%" && \
	cd ../$(OBJDIR)/sql && \
	truncate -s 0 ../queries.c && truncate -s 0 ../queries.h && \
	echo -e '#include "queries.h"\n\n' >> ../queries.c && \
	find * -type f | \
       	xargs -I % bash -c "xxd -i % >> ../queries.c" && \
	cd ../ && sed -i 's/, 0x\([0-9a-f]\{2\}\)$$/\, 0x00/' queries.c && \
	echo -e "#ifndef QUERIES_H\n#define QUERIES_H\n\n" >> queries.h && \
	cat queries.c | sed 's/unsigned/extern unsigned/g' | \
		grep extern | cut -d'=' -f1 | sed 's/$$/;/g' >> queries.h && \
	echo -e "\n#endif\n" >> queries.h
	@rm -f sql
	$(CC) $(CFLAGS) -c $(OBJDIR)/queries.c -o $(OBJDIR)/queries.o 
	@echo "Generated $@"

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
