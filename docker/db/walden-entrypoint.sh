#!/usr/bin/env bash

set -e

cd /
echo "pwd: `pwd`"
echo "ls walden: `ls -alh walden`"

echo "Build Extension"
./build-extensions.sh

echo "docker-entrypoint"
./docker-entrypoint.sh "$@"

POSTGRES_USER='postgres'

echo "POSTGRES_USER: $POSTGRES_USER"
echo "PATH: $PATH"
echo "PGDATA: $PGDATA"
echo "whoami: $(whoami)"

gosu postgres bash -c 'echo "whoami: $(whoami)"'
echo "pg_ctl start..."

gosu postgres bash -c 'PGUSER="${PGUSER:-postgres}" pg_ctl -D "$PGDATA" -o "-c listen_addresses='localhost'" -w start'
gosu postgres bash /walden-entrypoint-initdb.d/01-init-db.sh.bak
gosu postgres bash -c 'psql -a -v ON_ERROR_STOP=1 -U postgres -d walden -f /walden-entrypoint-initdb.d/02-create-schemas.sql'
gosu postgres bash -c 'psql -a -v ON_ERROR_STOP=1 -U postgres -d walden -f /walden-entrypoint-initdb.d/03-install-extensions.sql'
gosu postgres bash -c 'PGUSER="${PGUSER:-postgres}" pg_ctl -D "$PGDATA" -m fast -w stop'

echo "exec: $@"
echo
gosu postgres "$@"
