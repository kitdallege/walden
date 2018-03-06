#!/usr/bin/env bash

set -e

cd /
echo "pwd: `pwd`"
echo "walden apps: `ls -alh walden`"

echo "Build Extension"
./build-extensions.sh

echo "docker-entrypoint"
./docker-entrypoint.sh "$@"

# used in 01-init-db.sh
POSTGRES_USER='postgres'

echo "PGUSER: ${PGUSER:-postgres}"
echo "PGDATA: $PGDATA"
gosu postgres bash -c 'echo "whoami: $(whoami)"'
echo "pg_ctl start..."
gosu postgres bash -c 'PGUSER="${PGUSER:-postgres}" pg_ctl -D "$PGDATA" -o "-c listen_addresses='localhost'" -w start'

echo "checkin if walden:db-table exists."
set -v
set +e
gosu postgres bash -c 'psql -v ON_ERROR_STOP=1 -U postgres -lqt | cut -d \| -f 1 | grep -qw walden'
DB_EXISTS=$?
set -e
set +v
echo "DB_EXISTS: $DB_EXISTS"

if [[ $DB_EXISTS -eq 0 ]]; then
    echo "walden exists"
else
    echo "walden db not found. creating..."
    gosu postgres bash /walden-entrypoint-initdb.d/01-init-db.sh.bak
    gosu postgres bash -c 'psql -a -v ON_ERROR_STOP=1 -U postgres -d walden -f /walden-entrypoint-initdb.d/02-create-schemas.sql'
    gosu postgres bash -c 'psql -a -v ON_ERROR_STOP=1 -U postgres -d walden -f /walden-entrypoint-initdb.d/03-install-extensions.sql'
fi
echo "pg_ctl stop"
gosu postgres bash -c 'PGUSER="${PGUSER:-postgres}" pg_ctl -D "$PGDATA" -m fast -w stop'

echo "exec: $@"
echo
gosu postgres "$@"
