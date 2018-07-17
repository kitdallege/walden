-- pl/pgsql
-- CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;
-- COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';
-- python
create extension if not exists plpythonu with schema pg_catalog;
comment on extension plpythonu is 'PL/PythonU untrusted procedural language';
-- Walden
create extension if not exists walden with schema walden cascade;
comment on extension walden is
    'This extension provides a web development platform within PostgreSQL';

create extension if not exists walden_auth with schema walden cascade;
comment on extension walden_auth is
    'This extension provides Auth/Permissions for the Walden system.';


create extension if not exists walden_sites with schema walden cascade;
comment on extension walden_sites is
    'This extension provides Site object for the Walden system.';

create extension if not exists walden_webdev with schema walden cascade;
comment on extension walden_webdev is
    'This extension provides Site/Page Builder tools for the Walden system.';

create extension if not exists walden_taxonomy with schema walden cascade;
comment on extension walden_taxonomy is
    'This extension provides a Taxonomy based resource tree for the Walden system';

create extension if not exists walden_admin with schema walden cascade;
comment on extension walden_admin is
    'This extension provides Admin UI for the Walden system.';
