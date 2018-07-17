EXTENSION = walden
EXTVERSION = 0.0.1

MODULE_big = walden_core
OBJS = $(patsubst %.c,%.o,$(wildcard src/*.c)) 
DATA = sql/walden--0.0.1.sql  # script files to install

# postgres build stuff
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
