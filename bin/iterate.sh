#!/bin/bash
dropdb -e --if-exists walden && createdb -e walden -O walden
psql -d walden -f ${BASH_SOURCE%/*}/../utils/create-schemas.sql
psql -d walden -f ${BASH_SOURCE%/*}/../utils/install-extensions.sql
