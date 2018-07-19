/* Initial Install of walden_sites */
\echo Use "CREATE EXTENSION walden_sites" to load this file. \quit

/**************************************************************
 *                    Tables & Types                          *
 **************************************************************/
create table organization
(
    id          serial      not null primary key,
    sys_period  tstzrange   not null default tstzrange(current_timestamp, 'infinity'),
    name        text        not null unique
);
--ALTER TABLE organization OWNER to walden;

create table site
(
    id              serial      not null primary key,
    sys_period      tstzrange   not null default tstzrange(current_timestamp, 'infinity'),
    organization_id integer     not null references organization(id),
    name            text        not null unique,
    domain          text        not null unique
);
--ALTER TABLE site OWNER to walden;

create table site_setting
(
    id              serial      not null primary key,
    sys_period      tstzrange   not null default tstzrange(current_timestamp, 'infinity'),
    site_id         integer     not null references site(id),
    name            text        not null,
    value           text        not null,
    unique (site_id, name)
);
--ALTER TABLE site_setting OWNER to walden;

/**************************************************************
 *                      Functions                             *
 **************************************************************/
create or replace function
walden_organization_get_or_create(org_name text)
returns organization as 
$$
    insert into organization (name)
    values (org_name) returning *;
$$ language sql volatile;

create or replace function
walden_site_get_or_create(org_id integer, site_name text, domain text)
returns site as
$$
    insert into site (organization_id, name, domain)
    values (org_id, site_name, domain)
    returning *;
$$ language sql volatile;

create or replace function
walden_site_settings_set(_site_id integer, _name text, _value text)
returns void as 
$$
    insert into site_setting (site_id, name, value)
    values (_site_id, _name, _value)
    on conflict (site_id, name) do update set value = _value;
$$ language sql volatile;


/**************************************************************
 *                      App Config                            *
 **************************************************************/
do
$$
    declare
        app_id      application.id%TYPE;
    begin
        app_id := (walden_application_get_or_create('Sites')).id;
        perform walden_entity_get_or_create(app_id, 'Organization', 'organization');
        perform walden_entity_get_or_create(app_id, 'Site', 'site');
    end;
$$ language plpgsql;

