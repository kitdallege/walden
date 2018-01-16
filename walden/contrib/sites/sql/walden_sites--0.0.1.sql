/* Initial Install of walden_sites */
--\echo Use "CREATE EXTENSION walden_sites" to load this file. \quit

/**************************************************************
 *                      Schemas                               *
 **************************************************************/
CREATE SCHEMA IF NOT EXISTS walden;
CREATE SCHEMA IF NOT EXISTS walden_history;


/**************************************************************
 *                    Tables & Types                          *
 **************************************************************/
CREATE TABLE organization
(
    id          SERIAL      NOT NULL PRIMARY KEY,
    sys_period  TSTZRANGE   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    name        TEXT        NOT NULL UNIQUE
);
ALTER TABLE organization OWNER to walden;

CREATE TABLE site
(
    id              SERIAL      NOT NULL PRIMARY KEY,
    sys_period      TSTZRANGE   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    organization_id INTEGER NOT NULL REFERENCES organization(id),
    name            TEXT        NOT NULL UNIQUE,
    domain          TEXT        NOT NULL UNIQUE
);
ALTER TABLE site OWNER to walden;

CREATE TABLE site_setting
(
    id              SERIAL      NOT NULL PRIMARY KEY,
    sys_period      TSTZRANGE   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    site_id         INTEGER     NOT NULL REFERENCES site(id),
    name            TEXT        NOT NULL,
    value           TEXT        NOT NULL,
    UNIQUE (site_id, name)
);
ALTER TABLE site_setting OWNER to walden;

/**************************************************************
 *                      Functions                             *
 **************************************************************/
CREATE FUNCTION walden_create_organization(org_name text)
RETURNS INTEGER AS $$
    INSERT INTO organization (name) VALUES (org_name) RETURNING id;
$$ LANGUAGE SQL VOLATILE;

CREATE FUNCTION walden_create_site(org_id integer, site_name text, domain text)
RETURNS INTEGER AS $$
    INSERT INTO site (organization_id, name, domain)
    VALUES (org_id, site_name, domain)
    RETURNING id;
$$ LANGUAGE SQL VOLATILE;


/**************************************************************
 *                      App Config                            *
 **************************************************************/
DO $$
BEGIN
   PERFORM walden_register_application('Sites');
   PERFORM walden_register_entity('Sites', 'Organization', 'organization');
   PERFORM walden_register_entity('Sites', 'Site', 'site');
END$$;
