APP		= walden
SRCDIR		= src
INCDIR 		= ./include
OBJDIR		= build
DEPSDIR		= ./deps

#PREFIX?=/usr/local
PREFIX?=.
INSTALL_DIR=$(PREFIX)/bin

# TODO: Break up c/ld flags so that were only linking the app
# against libpq (etc).
# -flto -march=native -Ofast
CFLAGS  += -g -O0 -Wall -Wstrict-prototypes -Werror -Wmissing-prototypes
CFLAGS  += -Wmissing-declarations -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS  += -Wsign-compare -std=gnu11 -pedantic 
# this is needed for anonymous struct members
#CFLAGS  += -fms-extensions 
# gcc memory debugging
CFLAGS  += -fno-omit-frame-pointer #-fsanitize=address -ggdb 
CFLAGS  += -I`pg_config --includedir` 

LDFLAGS = -ldl -lpq 
#LDFLAGS += $(shell pkg-config libpq --libs)
SRCS 	= $(wildcard $(SRCDIR)/*.c)
S_OBJS 	= $(SRCS:src/%.c=$(OBJDIR)/%.o)

.PHONEY: rebuild all clean

all: $(APP) $(LIB)

$(APP): $(OBJDIR) $(S_OBJS)
	$(CC) $(S_OBJS) $(LDFLAGS) -o $(OBJDIR)/$@

$(OBJDIR)/%.o: src/%.c 
	$(CC) $(CFLAGS) -c $< -o $@ 

$(OBJDIR):
	@mkdir -p $(OBJDIR)

install: $(APP)
	mkdir -p $(INSTALL_DIR)
	install -m 555 $(OBJDIR)/$(APP) $(INSTALL_DIR)/$(APP)	

uninstall:
	rm -f $(INSTALL_DIR)/$(APP)

clean:
	@rm -f $(OBJDIR)/*.o
	@rm -f $(OBJDIR)/$(APP)

rebuild:
	$(MAKE) clean
	$(MAKE) all

