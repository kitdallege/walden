/* Initial Install of walden */
\echo Use "CREATE EXTENSION walden" to load this file. \quit

/**************************************************************
 *                    Tables & Types                          *
 **************************************************************/
create type host_role as enum ('DEVELOPMENT', 'ADMIN', 'PRODUCTION');
--ALTER TYPE host_role OWNER to walden;

create table config
(
    id          serial          not null primary key,
    sys_period  tstzrange       not null default tstzrange(current_timestamp, 'infinity'),
    host_role   host_role       not null default 'DEVELOPMENT',
    name        text    	not null unique,
    configs     jsonb           not null default '{}'::jsonb
);
--ALTER TABLE config OWNER to walden;
SELECT pg_catalog.pg_extension_config_dump('config', '');
--
create table application
(
    id          serial      not null primary key,
    sys_period  tstzrange   not null default tstzrange(current_timestamp, 'infinity'),
    name        text        not null unique,
    schema      text        not null
    -- install_func, update_func, uninstall_func:
    -- going to try to do these by convention. So if you define a
    -- [application.name]_[install, update, uninstall] function(s)
    -- we'll call um, otherwise, we'll do nothing.
);
--ALTER TABLE application OWNER to walden;
comment on table application is 'Applications within the Walden System.';
--SELECT pg_catalog.pg_extension_config_dump('application', '');

create type entity_type as enum ('TABLE', 'VIEW');
comment on type entity_type is 'Types of Entity''s within the walden system';
--ALTER TYPE entity_type OWNER to walden;

create table entity
(
    id              serial      not null primary key,
    sys_period      tstzrange   not null default tstzrange(current_timestamp, 'infinity'),
    application_id  integer     not null references application(id),
    type            entity_type not null default 'TABLE',
    name            text        not null,
    db_object       text        not null,
    unique (application_id, name)
);
--ALTER TABLE entity OWNER to walden;
comment on table entity is 'An Entity within the walden system';

create table ability
(
    id              serial      not null primary key,
    sys_period      tstzrange   not null default tstzrange(current_timestamp, 'infinity'),
    application_id  integer     not null references application(id),
    name            text        not null,
    func_name_part  text        not null,
    description     text        not null default '',
    unique (application_id, name)
);
--ALTER TABLE ability OWNER to walden;
comment on table ability is 'An ability an Entity can choose to gain/use.';

create table entity_ability
(
    id              serial      not null primary key,
    sys_period      tstzrange   not null default tstzrange(current_timestamp, 'infinity'),
    entity_id       integer     not null references entity(id),
    ability_id      integer     not null references ability(id),
    installed       boolean     not null default false,
    unique (entity_id, ability_id)
);
--ALTER TABLE entity_ability OWNER to walden;
comment on table entity_ability is 'An ability possessed by a given Entity.';

/**************************************************************
 *                      Functions                             *
 **************************************************************/
create or replace function
walden_register_application(name text, schema text default current_schema)
returns integer as
$$
    insert into application (name, schema)
    values (name, schema)
    returning id;
$$ language sql volatile;

create or replace function
walden_unregister_application(name text)
returns void as
$$
    delete from application where name = name;
$$ language sql volatile;

create or replace function
walden_get_application(app_name text)
returns application as
$$
    select * from application where name = app_name;
$$ language sql stable;

create or replace function
walden_get_application_id(name text)
returns integer as
$$
    select id from walden_get_application(name);
$$ language sql stable;


create or replace function
walden_register_entity(app_name text, name text, db_object text)
returns integer as
$$
    insert into entity (application_id, type, name, db_object)
    values (
        walden_get_application_id(app_name),
        'TABLE', name, db_object
    )
    returning id;
$$ language sql volatile;

create function
walden_unregister_entity(app_name text, name text)
returns void as
$$
    delete from entity
    where name = name
      and application_id = walden_get_application_id(app_name);
$$ language sql volatile;

create or replace function
walden_register_ability(app_name text, name text, func_name_part text, description text)
returns integer as
$$
    insert into ability (application_id, name, func_name_part, description)
    values (
        walden_get_application_id(app_name),
        name, func_name_part, description
    )
    returning id;
$$ language sql volatile;

/* walden_update_ability() */
create or replace function
walden_unregister_ability(app_name text, name text)
returns void as
$$
    delete from ability
    where name = name
      and application_id = walden_get_application_id(app_name);
$$ language sql volatile;

create or replace function
walden_entity_add_ability(e entity, ability_app text, ability_name text)
returns void as
$$
    insert into entity_ability (entity_id, ability_id)
        values (
            e.id,
            (
                select id
                from ability
                where name = ability_name
                  and application_id = walden_get_application_id(ability_app)
            )
        );
$$ language sql volatile;

create or replace function
walden_add_history(e entity)
returns void as
$$
    declare
    begin
        create table walden_history.walden_user (like entity.db_object);
        -- create trigger walden_user_versioning_trigger
        -- before insert or update or delete on walden_user
        -- for each row execute procedure versioning('sys_period', 'walden_history.walden_user', true);
    end
$$ language plpgsql volatile;
comment on function walden_add_history(e entity) is
    'Adds a mirror table in a [current_schema]_history and sets up triggers for tracking changes and storing them.';

/**************************************************************
 *                 General Functions                          *
 **************************************************************/
/*
CREATE OR REPLACE FUNCTION slugify(text)
RETURNS TEXT AS $$
    SELECT lower(trim(both '-' FROM substring(
        regexp_replace(
            regexp_replace($1, E'[^\\w]', '-', 'g'), E'-+', '-', 'g'
        ) FROM 0 FOR 51)));
$$ LANGUAGE SQL IMMUTABLE;
*/

create or replace function slugify(text) 
returns text 
as 'walden_core.so', 'slugify' 
language c strict immutable;



/**************************************************************
 *                      App Config                            *
 **************************************************************/
do $$
begin
   perform walden_register_application('Walden');
end$$;

