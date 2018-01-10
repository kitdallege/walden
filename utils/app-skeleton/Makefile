EXTENSION = walden_$APP_NAME           # the extensions name
DATA = sql/walden_$APP_NAME--0.1.0.sql  # script files to install

# postgres build stuff
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
