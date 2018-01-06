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

CREATE TABLE walden_user
(
    id          SERIAL          NOT NULL PRIMARY KEY,
    sys_period  TSTZRANGE       NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    username    VARCHAR(30)     NOT NULL UNIQUE,
    first_name  VARCHAR(30)     NOT NULL,
    last_name   VARCHAR(30)     NOT NULL,
    email       VARCHAR(75)     NOT NULL,
    password    VARCHAR(128)    NOT NULL
);
ALTER TABLE walden_user OWNER to walden;
COMMENT ON COLUMN walden_user.password IS 'Password uses [algo]$[salt]$[hexdigest].';
COMMENT ON TABLE walden_user is 'User within the walden system.';

-- Create a history table in walden_history
CREATE TABLE walden_history.walden_user (LIKE walden_user);

-- Add the trigger for versioning.
CREATE TRIGGER walden_user_versioning_trigger
BEFORE INSERT OR UPDATE OR DELETE ON walden_user
FOR EACH ROW EXECUTE PROCEDURE versioning('sys_period',
                                          'walden_history.walden_user',
                                          true);
/*
 Application : Typicall a schema, used as a unique namespace. ? extensions name

 Entity : table/view (technically its an entity-type but that sounds clunky
          so we use Entity to refer to the type and EntityInstance to refer
          to an instance of that type.

 Taxonomy : Tree of urls *classification of data from /least/to/greatest specifity.
 Taxon : A given ('node') in the taxonomy tree.

 ResourceType/Format/ContentType : the type of file were writing
 Resource : Exposes an entity for a given ResourceType

 So we create a Taxonomy for a given site:
 At each 'taxon' we define a resource.
 A resource can be anything a flatpage to a redirect (ln)
 Non-leaf resources are able to generate more resources.

*/
CREATE TYPE entity_type AS ENUM
(
    'TABLE',
    'VIEW'
);
COMMENT ON TYPE entity_type is 'Types of Entity''s within the walden system';

CREATE TABLE entity
(
    id          SERIAL                 NOT NULL PRIMARY KEY,
    sys_period  tstzrange              NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    type        entity_type            NOT NULL DEFAULT 'TABLE',
    schema      VARCHAR(256) NOT NULL, -- application
    name        VARCHAR(256) NOT NULL,
    UNIQUE (schema, name)
);
ALTER TABLE entity OWNER to walden;
COMMENT ON TABLE entity is 'An Entity within the walden system';
/*
 Need to design 'for' the task @ hand.

 On change a record is wrote in a 'pending_publish' table and a notification
 is sent out for the Publisher

 The publisher job is to watch for notifications, upon recieving one, look
 in the pending_publish table. If it missed the notification it can still
 perform the publish by just polling the pending_publish table.

 The publisher needs to:
    * Find all widgets that use an entity
    * Re-Render those.

 Entity <- Query <- WidgetQuery -> Widget <- WidgetOnPage -> Page(view) -> Route -> Entity

 Taxonomy
  A tree of resources

  they can be either/both of:
    - static : (leaf)
    - dynamic : (branch)

  Both is actually quite common, typically they are landing pages for
  applications.



*/

CREATE TABLE asset
(
    id      SERIAL          NOT NULL PRIMARY KEY,
    name    VARCHAR(256)    NOT NULL UNIQUE
);
ALTER TABLE asset OWNER to walden;

CREATE TABLE widget
(
    id      SERIAL          NOT NULL PRIMARY KEY,
    name    VARCHAR(256)    NOT NULL UNIQUE,
    title   VARCHAR(256)    NOT NULL
    -- js_assets
    -- css_assets
    -- template
    -- queries
);
ALTER TABLE widget OWNER to walden;

CREATE TABLE page
(
    id      SERIAL          NOT NULL PRIMARY KEY,
    name    VARCHAR(256)    NOT NULL UNIQUE,
    title   VARCHAR(256)    NOT NULL
);
ALTER TABLE page OWNER to walden;

CREATE TABLE widget_on_page
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    page_id     INTEGER NOT NULL REFERENCES page(id),
    widget_id   INTEGER NOT NULL REFERENCES widget(id)

);
ALTER TABLE widget_on_page OWNER to walden;

CREATE TYPE resource_type AS ENUM
(
    'STATIC',
    'LIST',
    'DETAIL'
);
CREATE TABLE resource
(
    id          SERIAL          NOT NULL PRIMARY KEY,
    name        VARCHAR(256)    NOT NULL UNIQUE,
    type        resource_type   NOT NULL,
    entity_id   INTEGER         NOT NULL REFERENCES entity(id),
    children    VARCHAR(256)    NOT NULL DEFAULT ''
);

CREATE TABLE taxonomy
(
    id      SERIAL          NOT NULL PRIMARY KEY,
    name    VARCHAR(256)    NOT NULL UNIQUE
);
CREATE TABLE taxon
(
    id          SERIAL          NOT NULL PRIMARY KEY,
    name        VARCHAR(256)    NOT NULL,
    parent_path LTREE           NOT NULL UNIQUE,
    resource_id INTEGER         NOT NULL REFERENCES resource(id),
    page_id     INTEGER         NOT NULL REFERENCES page(id)
);
CREATE TABLE entity_taxon
(
    id          SERIAL          NOT NULL PRIMARY KEY,
    entity_id   INTEGER REFERENCES entity(id),
    taxon_id    INTEGER REFERENCES taxon(id),
    UNIQUE (entity_id, taxon_id)
);
-- Routes
-- Queries
-- View
-- Templates
-- Assets
--


/**************************************************************
 *                      DATA                                  *
 **************************************************************/
INSERT INTO entity (type, schema, name)
    VALUES ('TABLE', 'walden', 'walden_user');
INSERT INTO resource (name, type, entity_id)
    VALUES ('user-list', 'LIST', 1);
INSERT INTO taxonomy (name)
    VALUES ('ComeToVegas');
INSERT INTO page (name, title)
    VALUES ('home', 'my awesome homepage');
INSERT INTO taxon (name, parent_path, resource_id, page_id)
    VALUES ('all users', 'root.users', 1, 1);


/*
    Views make explicit the required columns from a given table. If the
    underlying table is altered to break a requierment and error is raised.

    So if I shove query complexity though views, I gain the ability to provide
    static type checking.

    Functions are the dynamic component. The only checking they recieve is on
    input/output types. As long as your not removing a type which a function
    returns, they could care less how you change the schema. This means if
    a function selects from a table, and that table alters their schema then
    there is a chance a run-time error has been introduced.

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
