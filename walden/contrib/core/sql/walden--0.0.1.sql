/* Initial Install of walden */
\echo Use "CREATE EXTENSION walden" to load this file. \quit

/**************************************************************
 *                      Schemas                               *
 **************************************************************/
CREATE SCHEMA IF NOT EXISTS walden;
CREATE SCHEMA IF NOT EXISTS walden_history;


/**************************************************************
 *                    Tables & Types                          *
 **************************************************************/
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
--
CREATE TABLE application
(
    id          SERIAL      NOT NULL PRIMARY KEY,
    sys_period  tstzrange   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    name        TEXT        NOT NULL UNIQUE,
    schema      TEXT        NOT NULL
    -- install_func, update_func, uninstall_func:
    -- going to try to do these by convention. So if you define a
    -- [application.name]_[install, update, uninstall] function(s)
    -- we'll call um, otherwise, we'll do nothing.
);
ALTER TABLE application OWNER to walden;
COMMENT ON TABLE application IS 'Applications within the Walden System.';
--SELECT pg_catalog.pg_extension_config_dump('application', '');

CREATE TYPE entity_type AS ENUM ('TABLE', 'VIEW');
COMMENT ON TYPE entity_type is 'Types of Entity''s within the walden system';
CREATE TABLE entity
(
    id              SERIAL      NOT NULL PRIMARY KEY,
    sys_period      tstzrange   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    application_id  INTEGER     NOT NULL REFERENCES application(id),
    type            entity_type NOT NULL DEFAULT 'TABLE',
    name            TEXT        NOT NULL,
    db_object       TEXT        NOT NULL,
    UNIQUE (application_id, name)
);
ALTER TABLE entity OWNER to walden;
COMMENT ON TABLE entity is 'An Entity within the walden system';

CREATE TABLE ability
(
    id              SERIAL      NOT NULL PRIMARY KEY,
    sys_period      tstzrange   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    application_id  INTEGER     NOT NULL REFERENCES application(id),
    name            TEXT        NOT NULL,
    func_name_part  TEXT        NOT NULL,
    description     TEXT        NOT NULL DEFAULT '',
    UNIQUE (application_id, name)
);
ALTER TABLE ability OWNER to walden;
COMMENT ON TABLE ability is 'An ability an Entity can choose to gain/use.';

CREATE TABLE entity_ability
(
    id              SERIAL      NOT NULL PRIMARY KEY,
    sys_period      tstzrange   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    entity_id       INTEGER     NOT NULL REFERENCES entity(id),
    ability_id      INTEGER     NOT NULL REFERENCES ability(id),
    installed       BOOLEAN     NOT NULL DEFAULT FALSE,
    UNIQUE (entity_id, ability_id)
);
ALTER TABLE entity_ability OWNER to walden;
COMMENT ON TABLE entity_ability is 'An ability possessed by a given Entity.';

/**************************************************************
 *                      Functions                             *
 **************************************************************/
CREATE FUNCTION walden_register_application(name text, schema text DEFAULT current_schema)
RETURNS INTEGER AS $$
    INSERT INTO application (name, schema)
    VALUES (name, schema)
    RETURNING id;
$$ LANGUAGE SQL;

CREATE FUNCTION walden_unregister_application(name text)
RETURNS VOID AS $$
    DELETE FROM application WHERE name = name;
$$ LANGUAGE SQL;

CREATE FUNCTION walden_get_application(name text)
RETURNS application AS $$
    SELECT * FROM application WHERE name = name;
$$ LANGUAGE SQL;

CREATE FUNCTION walden_get_application_id(name text)
RETURNS INTEGER AS $$
    SELECT id FROM walden_get_application(name);
$$ LANGUAGE SQL;


CREATE FUNCTION walden_register_entity(app_name text, name text, db_object text)
RETURNS INTEGER AS $$
    INSERT INTO entity (application_id, type, name, db_object)
    VALUES (
        walden_get_application_id(app_name),
        'TABLE', name, db_object
    )
    RETURNING id;
$$ LANGUAGE SQL;

CREATE FUNCTION walden_unregister_entity(app_name text, name text)
RETURNS VOID AS $$
    DELETE FROM entity
    WHERE name = name
      AND application_id = walden_get_application_id(app_name);
$$ LANGUAGE SQL;

CREATE FUNCTION walden_register_ability(app_name text, name text, func_name_part text, description text)
RETURNS INTEGER AS $$
    INSERT INTO ability (application_id, name, func_name_part, description)
    VALUES (
        walden_get_application_id(app_name),
        name, func_name_part, description
    )
    RETURNING id;
$$ LANGUAGE SQL;

/* walden_update_ability() */

CREATE FUNCTION walden_unregister_ability(app_name text, name text)
RETURNS VOID AS $$
    DELETE FROM ability
    WHERE name = name
      AND application_id = walden_get_application_id(app_name);
$$ LANGUAGE SQL;


CREATE FUNCTION walden_entity_add_ability(e entity, ability_app text, ability_name text)
RETURNS VOID AS $$
    INSERT INTO entity_ability (entity_id, ability_id)
        VALUES (
            e.id,
            (
                SELECT id
                FROM ability
                WHERE name = ability_name
                  AND application_id = walden_get_application_id(ability_app)
            )
        );
$$ LANGUAGE SQL;


CREATE FUNCTION walden_add_history(e entity)
RETURNS VOID AS $$
DECLARE
BEGIN
    CREATE TABLE walden_history.walden_user (LIKE entity.db_object);
    -- CREATE TRIGGER walden_user_versioning_trigger
    -- BEFORE INSERT OR UPDATE OR DELETE ON walden_user
    -- FOR EACH ROW EXECUTE PROCEDURE versioning('sys_period', 'walden_history.walden_user', true);
END
$$ LANGUAGE PLPGSQL;
COMMENT ON FUNCTION walden_add_history(e entity) IS
    'Adds a mirror table in a [current_schema]_history and sets up triggers for tracking changes and storing them.';

/**************************************************************
 *                 General Functions                          *
 **************************************************************/
 -- TODO: Create C function for this as its used fairly heavily.
 -- Extra points for a SLUG datatype that slugify's automatically.
 CREATE OR REPLACE FUNCTION slugify(text)
 RETURNS TEXT AS $$
    SELECT lower(trim(both '-' FROM substring(
        regexp_replace(
            regexp_replace($1, E'[^\\w]', '-', 'g'), E'-+', '-', 'g'
        ) FROM 0 FOR 51)));
$$ LANGUAGE SQL IMMUTABLE;
