#!/bin/bash
rm -f database/versions/*
dropdb -e --if-exists walden && createdb -e walden -O walden
psql -d walden -f sql/migrations/create-schemas.sql
psql -d walden -f sql/migrations/install-extensions.sql
