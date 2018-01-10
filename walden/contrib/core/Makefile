EXTENSION = walden            # the extensions name
EXTVERSION = 0.0.1

DATA = sql/walden--0.0.1.sql  # script files to install

# postgres build stuff
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
