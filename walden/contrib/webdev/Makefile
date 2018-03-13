EXTENSION = walden_webdev
EXTVERSION = 0.0.1

MODULE_big = pgstach
OBJS = $(patsubst %.c,%.o,$(wildcard src/*.c)) 

DATA = sql/walden_webdev--0.0.1.sql  # script files to install

PG_CPPFLAGS = -I./src
SHLIB_LINK += $(filter -ljson-c, $(LIBS)) -ljson-c

# postgres build stuff
PG_CONFIG = pg_config

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
