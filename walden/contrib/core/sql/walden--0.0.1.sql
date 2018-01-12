-- TODO: At some point break the tables out into a function or series of
-- functions. The main problem with having them within the extension is
-- mostly pg_dump not being able to dump the contents. Also if someone
-- where to Drop Extension they'd whipe out all the tables. Whilst that
-- maybe be desired, it shouldn't be default. So instead lets try to make
-- walden_[install, uninstall]_tables functions that a user can use to
-- create/destory the extensions tables/indexes/sequences etc..

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
--\echo Use "CREATE EXTENSION walden" to load this file. \quit

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
CREATE TABLE walden_history.walden_user (LIKE walden_user);
CREATE TRIGGER walden_user_versioning_trigger
BEFORE INSERT OR UPDATE OR DELETE ON walden_user
FOR EACH ROW EXECUTE PROCEDURE versioning('sys_period', 'walden_history.walden_user', true);
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

CREATE FUNCTION walden_unregister_application(name text, schema text DEFAULT current_schema)
RETURNS VOID AS $$
    DELETE FROM application WHERE name = name AND schema = schema;
$$ LANGUAGE SQL;

CREATE FUNCTION walden_register_entity(app_name text, name text, db_object text)
RETURNS INTEGER AS $$
    INSERT INTO entity (application_id, type, name, db_object)
    VALUES (
        (SELECT id FROM application WHERE name = app_name),
        'TABLE', name, db_object
    )
    RETURNING id;
$$ LANGUAGE SQL;

CREATE FUNCTION walden_unregister_entity(app_name text, name text)
RETURNS VOID AS $$
    DELETE FROM entity
    WHERE name = name
      AND application_id = (SELECT id FROM application WHERE name = app_name);
$$ LANGUAGE SQL;

CREATE FUNCTION walden_register_ability(app_name text, name text, func_name_part text, description text)
RETURNS INTEGER AS $$
    INSERT INTO ability (application_id, name, func_name_part, description)
    VALUES (
        (SELECT id FROM application WHERE name = app_name),
        name, func_name_part, description
    )
    RETURNING id;
$$ LANGUAGE SQL;

CREATE FUNCTION walden_unregister_ability(app_name text, name text)
RETURNS VOID AS $$
    DELETE FROM ability
    WHERE name = name
      AND application_id = (SELECT id FROM application WHERE name = app_name);
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


/**************************************************************
 *                      DATA                                  *
 **************************************************************/
 DO $$
 BEGIN
    PERFORM walden_register_application('Walden');
    PERFORM walden_register_entity('Walden', 'User', 'walden_user');
    INSERT INTO walden_user (username, first_name, last_name, email, password)
        VALUES ('kit', 'Kit', 'Dallege', 'kitdallege@gmail.com', '******');
 END$$;

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
    The 'register function' + registery pattern is very useful for augmenting an 'entity' with an 'ability'.
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
