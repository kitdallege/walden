/* Initial Install of walden_sites */
\echo Use "CREATE EXTENSION walden_sites" to load this file. \quit

/**************************************************************
 *                      Schemas                               *
 **************************************************************/
CREATE SCHEMA IF NOT EXISTS walden;
CREATE SCHEMA IF NOT EXISTS walden_history;


CREATE TABLE site
(
    id          SERIAL      NOT NULL PRIMARY KEY,
    sys_period  TSTZRANGE   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    name        TEXT        NOT NULL UNIQUE,
    domain      TEXT        NOT NULL UNIQUE
);
ALTER TABLE site OWNER to walden;

DO $$
BEGIN
   PERFORM walden_register_application('Sites');
   PERFORM walden_register_entity('Sites', 'Site', 'site');
END$$;
