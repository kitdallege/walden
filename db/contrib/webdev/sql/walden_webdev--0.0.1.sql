/* Initial Install of walden_webdev */
\echo Use "CREATE EXTENSION walden_webdev" to load this file. \quit

/**************************************************************
 *                      Schemas                               *
 **************************************************************/
CREATE SCHEMA IF NOT EXISTS walden;
CREATE SCHEMA IF NOT EXISTS walden_history;


/**************************************************************
 *                    Tables & Types                          *
 **************************************************************/
CREATE TYPE asset_type AS ENUM ('CSS', 'JS', 'IMG', 'FILE');
ALTER TYPE asset_type OWNER to walden;

CREATE TABLE asset
(
    id      SERIAL      NOT NULL PRIMARY KEY,
    name    TEXT        NOT NULL UNIQUE,
    type    asset_type  NOT NULL DEFAULT 'FILE'
);
ALTER TABLE asset OWNER to walden;
/* maybe a table per asset type using inheritance ?*/



CREATE TABLE widget
(
    id      SERIAL  NOT NULL PRIMARY KEY,
    name    TEXT    NOT NULL UNIQUE,
    title   TEXT    NOT NULL
    -- js_assets
    -- css_assets
    -- template
    -- queries
);
ALTER TABLE widget OWNER to walden;

CREATE TABLE page
(
    id          SERIAL      NOT NULL PRIMARY KEY,
    sys_period  tstzrange   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    name        TEXT        NOT NULL UNIQUE,
    title       TEXT        NOT NULL
);
ALTER TABLE page OWNER to walden;
-- Create a history table in walden_history
CREATE TABLE walden_history.page (LIKE page);
-- Add the trigger for versioning.
CREATE TRIGGER walden_user_versioning_trigger
BEFORE INSERT OR UPDATE OR DELETE ON page
FOR EACH ROW EXECUTE PROCEDURE versioning('sys_period', 'walden_history.page', true);

CREATE TABLE widget_on_page
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    page_id     INTEGER NOT NULL REFERENCES page(id),
    widget_id   INTEGER NOT NULL REFERENCES widget(id)

);
ALTER TABLE widget_on_page OWNER to walden;

CREATE TABLE wquery
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    name        TEXT    NOT NULL UNIQUE,
    statement   TEXT    NOT NULL,
    params      JSONB   NOT NULL DEFAULT '{}'::JSONB
);
ALTER TABLE wquery OWNER to walden;

CREATE TABLE query_entity_ref
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    entity_id   INTEGER NOT NULL REFERENCES entity(id),
    query_id    INTEGER NOT NULL REFERENCES wquery(id)
);
ALTER TABLE query_entity_ref OWNER to walden;

CREATE TABLE widget_query
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    widget_id   INTEGER NOT NULL REFERENCES widget(id),
    query_id    INTEGER NOT NULL REFERENCES wquery(id)
);
ALTER TABLE widget_query OWNER to walden;

CREATE TYPE resource_type AS ENUM
(
    'STATIC',
    'LIST',
    'DETAIL'
);
ALTER TYPE resource_type OWNER to walden;

CREATE TABLE resource
(
    id          SERIAL          NOT NULL PRIMARY KEY,
    name        TEXT            NOT NULL UNIQUE,
    type        resource_type   NOT NULL,
    entity_id   INTEGER         NOT NULL REFERENCES entity(id),
    children    TEXT            NOT NULL DEFAULT ''
);
ALTER TABLE resource OWNER to walden;


CREATE TABLE static_page
(
    id          SERIAL      NOT NULL PRIMARY KEY,
    sys_period  tstzrange   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    name        TEXT        NOT NULL UNIQUE,
    title       TEXT        NOT NULL,
    content     TEXT        NOT NULL
);
ALTER TABLE static_page OWNER to walden;
-- Routes
-- Queries
-- View
-- Templates
CREATE TABLE template
(
    id          SERIAL      NOT NULL PRIMARY KEY,
    sys_period  tstzrange   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    name        TEXT        NOT NULL UNIQUE

)
-- Assets
/**************************************************************
 *                      Functions                             *
 **************************************************************/
CREATE OR REPLACE FUNCTION render(text, text) 
RETURNS text 
AS 'pgstach.so', 'render' 
LANGUAGE C STRICT IMMUTABLE;

COMMENT ON FUNCTION render(tmpl TEXT, context JSON) IS
    'Returns rendered string from  mustache template and json context.';

--CREATE OR REPLACE FUNCTION render_template(tmpl TEXT, context JSON)
--RETURNS TEXT AS $$
--    # TODO: cache modules in GD to avoid import cost.
--	import pystache
--	import json
--	return pystache.render(tmpl, json.loads(context))
--$$ LANGUAGE plpythonu STABLE;

/**************************************************************
 *                      App Config                            *
 **************************************************************/
DO $$
BEGIN
   PERFORM walden_register_application('Webdev');
   PERFORM walden_register_entity('Webdev', 'Asset',    'asset');
   PERFORM walden_register_entity('Webdev', 'Page',     'page');
   PERFORM walden_register_entity('Webdev', 'Resource', 'resource');
   PERFORM walden_register_entity('Webdev', 'Widget',   'widget');
   PERFORM walden_register_entity('Webdev', 'Query',    'wquery');
END$$;
