EXTENSION = walden            # the extensions name
EXTVERSION = 0.1.0

DATA = sql/walden--0.1.0.sql  # script files to install

# postgres build stuff
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
