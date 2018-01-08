-- TODO: At some point break the tables out into a function or series of
-- functions. The main problem with having them within the extension is
-- mostly pg_dump not being able to dump the contents. Also if someone
-- where to Drop Extension they'd whipe out all the tables. Whilst that
-- maybe be desired, it shouldn't be default. So instead lets try to make
-- walden_[install, uninstall]_tables functions that a user can use to
-- create/destory the extensions tables/indexes/sequences etc..
-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION walden" to load this file. \quit


CREATE TYPE host_role AS ENUM ('DEVELOPMENT', 'ADMIN', 'PRODUCTION');
CREATE TABLE config
(
    id          SERIAL          NOT NULL PRIMARY KEY,
    sys_period  TSTZRANGE       NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    host_role   host_role       NOT NULL DEFAULT 'DEVELOPMENT',
    configs     JSONB           NOT NULL DEFAULT '{}'::JSONB
);
ALTER TABLE config OWNER to walden;
SELECT pg_catalog.pg_extension_config_dump('config', '');


CREATE TABLE walden_user
(
    id          SERIAL      NOT NULL PRIMARY KEY,
    sys_period  TSTZRANGE   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    username    TEXT        NOT NULL UNIQUE,
    first_name  TEXT        NOT NULL,
    last_name   TEXT        NOT NULL,
    email       TEXT        NOT NULL,
    password    TEXT        NOT NULL
);
ALTER TABLE walden_user OWNER to walden;
COMMENT ON COLUMN walden_user.password IS 'Password uses [algo]$[salt]$[hexdigest].';
COMMENT ON TABLE walden_user is 'User within the walden system.';
SELECT pg_catalog.pg_extension_config_dump('walden_user', '');

-- Create a history table in walden_history
CREATE TABLE walden_history.walden_user (LIKE walden_user);

-- Add the trigger for versioning.
CREATE TRIGGER walden_user_versioning_trigger
BEFORE INSERT OR UPDATE OR DELETE ON walden_user
FOR EACH ROW EXECUTE PROCEDURE versioning('sys_period',
                                          'walden_history.walden_user',
                                          true);

CREATE TABLE application
(
    id          SERIAL      NOT NULL PRIMARY KEY,
    sys_period  tstzrange   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    name        TEXT        NOT NULL UNIQUE
    -- install_func, update_func, uninstall_func:
    -- going to try to do these by convention. So if you define a
    -- [application.name]_[install, update, uninstall] function(s)
    -- we'll call um, otherwise, we'll do nothing.
);
CREATE TYPE entity_type AS ENUM
(
    'TABLE',
    'VIEW'
);
COMMENT ON TYPE entity_type is 'Types of Entity''s within the walden system';

CREATE TABLE entity
(
    id          SERIAL      NOT NULL PRIMARY KEY,
    sys_period  tstzrange   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    application_id INTEGER REFERENCES application(id),
    type        entity_type NOT NULL DEFAULT 'TABLE',
    schema      TEXT        NOT NULL, -- application
    name        TEXT        NOT NULL,
    UNIQUE (schema, name)
);
ALTER TABLE entity OWNER to walden;
COMMENT ON TABLE entity is 'An Entity within the walden system';

CREATE TYPE asset_type AS ENUM
(
    'CSS',
    'JS',
    'IMG',
    'FILE'
);
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
FOR EACH ROW EXECUTE PROCEDURE versioning('sys_period',
                                          'walden_history.page',
                                          true);

CREATE TABLE widget_on_page
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    page_id     INTEGER NOT NULL REFERENCES page(id),
    widget_id   INTEGER NOT NULL REFERENCES widget(id)

);
ALTER TABLE widget_on_page OWNER to walden;

CREATE TABLE db_query
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    name        TEXT    NOT NULL UNIQUE,
    statement   TEXT    NOT NULL,
    params      JSONB   NOT NULL DEFAULT '{}'::JSONB
);
ALTER TABLE db_query OWNER to walden;

CREATE TABLE query_entity_ref
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    entity_id   INTEGER NOT NULL REFERENCES entity(id),
    query_id    INTEGER NOT NULL REFERENCES db_query(id)
);
ALTER TABLE query_entity_ref OWNER to walden;

CREATE TABLE widget_query
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    widget_id   INTEGER NOT NULL REFERENCES widget(id),
    query_id    INTEGER NOT NULL REFERENCES db_query(id)
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

/*
CREATE TABLE taxonomy
(
    id      SERIAL  NOT NULL PRIMARY KEY,
    name    TEXT    NOT NULL UNIQUE
);
ALTER TABLE taxonomy OWNER to walden;

CREATE TABLE taxon
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    name        TEXT    NOT NULL,
    parent_path LTREE   NOT NULL UNIQUE,
    resource_id INTEGER NOT NULL REFERENCES resource(id),
    page_id     INTEGER NOT NULL REFERENCES page(id)
);
ALTER TABLE taxon OWNER to walden;

CREATE TABLE entity_taxon
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    entity_id   INTEGER NOT NULL REFERENCES entity(id),
    taxon_id    INTEGER NOT NULL REFERENCES taxon(id),
    UNIQUE (entity_id, taxon_id)
);
ALTER TABLE entity_taxon OWNER to walden;
*/
-- Routes
-- Queries
-- View
-- Templates
-- Assets
--


/**************************************************************
 *                      DATA                                  *
 **************************************************************/
INSERT INTO walden_user (username, first_name, last_name, email, password)
    VALUES ('kit', 'Kit', 'Dallege', 'kitdallege@gmail.com', '******');
INSERT INTO entity (type, schema, name)
    VALUES ('TABLE', 'walden', 'walden_user');
INSERT INTO resource (name, type, entity_id)
    VALUES ('user-list', 'LIST', 1);
INSERT INTO page (name, title)
    VALUES ('home', 'my awesome homepage');
INSERT INTO db_query (name, statement)
    VALUES ('get_users_list', 'allWaldenUsers{ users:nodes{ id firstName lastName username } }');
INSERT INTO query_entity_ref (entity_id, query_id)
    VALUES (1, 1);

/*
    Views make explicit the required columns from a given table. If the
    underlying table is altered to break a requierment and error is raised.

    So if I shove query complexity though views, I gain the ability to provide
    static type checking.

    Functions are the dynamic component. The only checking they recieve is on
    creation and their input/output types. Otherwise , as long as your not
    removing a type which a function takes/returns, they could care less how
    you change the schema. This means if a function selects from a
    table, and that table alters their schema then there is a chance a
    run-time error has been introduced.

    * Test functions which take and/or return Entities.

    So tabels are models.
    Functions which take a table are model methods.

    Intefaces & Abilities.
    the 'register function' + registery pattern is very useful for providing
    augmenting an 'entity' with an 'ability'.
    For more complex 'abilities' types can be used to simulate interfaces.
    create a type which describes your interface.
    create model methods to implement the type.
    then just create a registry with a register function
    that allows ya to hook (table, inteface_methods...).

    At some point split models.sql into /models/[specific-files].sql
    Sort of the typical django(ish) web app layout, just with a lot of sql.
        * maybe a tool @ some point that runs off an app.yaml
          to do schema migrations and like auto register stuff.




*/
