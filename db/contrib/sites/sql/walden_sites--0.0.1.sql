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
create function walden_create_organization(org_name text)
returns integer as 
$$
    insert into organization (name)
    values (org_name) returning id;
$$ language sql volatile;

create function walden_create_site(org_id integer, site_name text, domain text)
returns integer as
$$
    insert into site (organization_id, name, domain)
    values (org_id, site_name, domain)
    returning id;
$$ language sql volatile;

/**************************************************************
 *                      App Config                            *
 **************************************************************/
do $$
begin
   perform walden_register_application('Sites');
   perform walden_register_entity('Sites', 'Organization', 'organization');
   perform walden_register_entity('Sites', 'Site', 'site');
end$$;

