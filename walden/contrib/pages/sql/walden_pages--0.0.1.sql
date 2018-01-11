/* Initial Install of walden_%(APP_NAME) */
\echo Use "CREATE EXTENSION walden_pages" to load this file. \quit

--- For now we'll stick contrib apps in with walden proper.
CREATE SCHEMA IF NOT EXISTS walden;
CREATE SCHEMA IF NOT EXISTS walden_history;

CREATE TYPE asset_type AS ENUM ('CSS', 'JS', 'IMG', 'FILE');

CREATE TABLE asset
(
    id      SERIAL  NOT NULL PRIMARY KEY,
    name    TEXT    NOT NULL UNIQUE,
    type    asset_type NOT NULL DEFAULT 'FILE'
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
    id          SERIAL  NOT NULL PRIMARY KEY,
    sys_period  tstzrange   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    name        TEXT    NOT NULL UNIQUE,
    title       TEXT    NOT NULL
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

-- Routes
-- Queries
-- View
-- Templates
-- Assets
--
INSERT INTO resource (name, type, entity_id)
    VALUES ('user-list', 'LIST', 1);
INSERT INTO page (name, title)
    VALUES ('home', 'my awesome homepage');
INSERT INTO wquery (name, statement)
    VALUES ('get_users_list', 'allWaldenUsers{ users:nodes{ id firstName lastName username } }');
INSERT INTO query_entity_ref (entity_id, query_id)
    VALUES (1, 1);
