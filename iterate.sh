#!/bin/bash
rm -f database/versions/*
dropdb -e --if-exists walden && createdb -e walden -O walden
psql -d walden -f sql/migrations/install-extensions.sql
psql -d walden -U walden -c "CREATE SCHEMA walden_data;"
psql -d walden -U walden -c "CREATE SCHEMA walden_master;"
psql -d walden -U walden -c "CREATE SCHEMA walden_published;"
alembic revision --autogenerat
alembic upgrade head
python walden/models/fixtures.py
