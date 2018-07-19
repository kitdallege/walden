-- pl/pgsql
-- CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;
-- COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';
-- python
create extension if not exists plpythonu; -- with schema pg_catalog;
comment on extension plpythonu is 'PL/PythonU untrusted procedural language';
create extension if not exists "uuid-ossp";
create extension if not exists "ltree";
create extension if not exists "temporal_tables";
-- Walden
drop extension if exists walden;
create extension if not exists walden; -- with schema walden cascade;
comment on extension walden is 'This extension provides a web development platform within PostgreSQL';

drop extension if exists walden_auth;
create extension if not exists walden_auth; -- with schema walden cascade;
comment on extension walden_auth is 'This extension provides Auth/Permissions for the Walden system.';

drop extension if exists walden_sites;
create extension if not exists walden_sites; -- with schema walden cascade;
comment on extension walden_sites is 'This extension provides Site object for the Walden system.';

drop extension if exists walden_webdev;
create extension if not exists walden_webdev; -- with schema walden cascade;
comment on extension walden_webdev is 'This extension provides Site/Page Builder tools for the Walden system.';

drop extension if exists walden_taxonomy;
create extension if not exists walden_taxonomy; -- with schema walden cascade;
comment on extension walden_taxonomy is 'This extension provides a Taxonomy based resource tree for the Walden system';

drop extension if exists walden_admin;
create extension if not exists walden_admin; -- with schema walden cascade;
comment on extension walden_admin is 'This extension provides Admin UI for the Walden system.';

