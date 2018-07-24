#!/bin/bash
POSTGRES_USER=postgres

db_name=walden
db_owner=walden_admin
db_pass=pass
db_search_path=walden,public

start_path=`pwd`

#dropdb -e --if-exists walden && createdb -e walden -O walden_admin
dropdb --username "$POSTGRES_USER" -e --if-exists "$db_name"

psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" <<-EOSQL
    DROP ROLE IF EXISTS $db_owner;
    CREATE ROLE $db_owner LOGIN PASSWORD '$db_pass';
    CREATE DATABASE $db_name OWNER $db_owner;
    ALTER DATABASE $db_name SET search_path = $db_search_path;
    GRANT ALL PRIVILEGES ON DATABASE $db_name TO $db_owner;
    ALTER ROLE $db_owner IN DATABASE $db_name SET search_path = $db_search_path;
EOSQL

# get around having to specify search_path within the sql files themselves.
PGOPTIONS="--search_path=$db_search_path"
echo "PGOPTIONS: $PGOPTIONS"
export PGOPTIONS

psql -d walden -U $db_owner -f ${BASH_SOURCE%/*}/../utils/create-schemas.sql

WALDEN_CONTRIB_DIR=../db/contrib

cd $WALDEN_CONTRIB_DIR
echo "user: $(whoami)"
for d in $(ls .); do
    cd $d
    echo "building: $d"
    make clean
    make  
    make install
    cd -
done

INSTALL_DIR=/tmp
cd $INSTALL_DIR
git clone https://github.com/arkhipov/temporal_tables.git
cd temporal_tables
make
make install


cd $start_path 
psql -d walden -U $POSTGRES_USER -f ${BASH_SOURCE%/*}/../utils/install-extensions.sql
psql -d walden -U $POSTGRES_USER -f /entrypoint-initdb.d/05-install-on-walden.sql


