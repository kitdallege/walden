/* Initial Install of walden_webdev */
\echo Use "CREATE EXTENSION walden_webdev" to load this file. \quit

/**************************************************************
 *                    Tables & Types                          *
 **************************************************************/
create type asset_type as enum ('CSS', 'JS', 'IMG', 'FILE');
create table asset
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    name            text        not null ,
    asset_type      asset_type  not null default 'FILE',
    parent_path     ltree       not null,
    checksum        text        not null,
    unique (name, parent_path)
);

create table template
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    site_id         integer     not null references site(id),
    name            text        not null,
    parent_path     ltree       not null,
    checksum        text        not null,
    unique (site_id, name, parent_path)
);

-- Store includes so that @ update we can determine 'ALL' effected resources.
create table template_include
(
    id                  serial      not null primary key,
    date_created        timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated        timestamp   with time zone not null default (now() at time zone 'utc'),
    site_id             integer     not null references site(id),
    source_template_id  integer     references template(id),
    include_template_id integer     references template(id),
    unique (source_template_id, include_template_id)
);

create table query
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    site_id         integer     not null references site(id),
    name            text        not null,
    parent_path     ltree       not null,
    checksum        text        not null,
    unique (site_id, name, parent_path)
);

-- TODO: consider a better name
create table page_spec 
(
    id              serial  not null primary key,
    template_id     integer not null references template(id),
    query_id        integer not null references query(id),
    unique (template_id, query_id)
);

create table resource 
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    site_id         integer     not null references site(id),
    name            text        not null,
    mount           ltree       not null,
    -- Pointer back to the entity that created us. 
    -- be it: taxon, widget
    --for_entity_type
    --for_entity_pk
    -- for_entity  generic_relationship *optimization*
    -- used in admin to allow us to tie together the resource
    -- the the entities that it displays.
    -- optimization for renderer
    dirty           boolean     not null default true,
    unique (mount, name)
);

create table page(
    page_spec_id    integer not null references page_spec(id),
    query_args      text    not null default ''
) inherits (resource);

create type static_page_format as enum ('RAW', 'MARKDOWN');
create table static_page
(
    format      static_page_format  not null default 'RAW',
    template_id integer             not null references template(id),
    content     text                not null
) inherits (resource);

/* Taxonomy, widgets, are resource factories.
   as they attach handlers to entity-types to create/update/remove
   resources in sync with the entities they represent.
*/
create table widget(
    id                  serial      not null primary key,
    date_created        timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated        timestamp   with time zone not null default (now() at time zone 'utc'),
    site_id             integer     not null references site(id),
    name                text        not null,
    mount               ltree       not null,
    --page_specs        List(page_spec)
    unique (mount, name)
);

-- Needed so that when a template/query changes it can be linked to the
-- resources which need to be flagged as dirty.
create table widget_page_spec
(
    id                  serial      not null primary key,
    widget_id           integer     not null references widget(id),
    page_spec_id        integer     not null references page_spec(id),
    unique (widget_id, page_spec_id)
    --role                taxon_type  not null default 'index';
);

/**************************************************************
 *                      Functions                             *
 **************************************************************/
-- TODO: this could be code golf'd into being 'language sql immutable';
create or replace function
walden_dirname(fullpath text)
returns ltree as
$$
    declare 
        fp_arr text[] := string_to_array(trim(leading '/' from fullpath), '/');
        parent_path ltree := concat_ws('.', 'root',
            nullif(array_to_string(fp_arr[0:array_upper(fp_arr, 1)-1], '.'), '')
        )::ltree;
    begin
        return parent_path;
    end;
$$ language plpgsql;

-- TODO: this could be code golf'd into being 'language sql immutable';
create or replace function
walden_basename(fullpath text)
returns text as 
$$
    declare 
        fp_arr text[] := string_to_array(trim(leading '/' from fullpath), '/');
        name text := fp_arr[array_upper(fp_arr, 1)];
    begin
        return coalesce(name, '');
    end;
$$ language plpgsql;

create or replace function
walden_template_get_or_create(_site_id integer, _fullpath text, _checksum text)
returns template as
$$
    with ins as (
        insert into template (site_id, name, parent_path, checksum)
        values (
            _site_id,
            walden_basename(_fullpath),
            walden_dirname(_fullpath),
            _checksum
        )
        on conflict (site_id, name, parent_path)
        do update set checksum = _checksum 
        returning *
    )
    select * from ins
        union all
    select * from template 
    where   site_id = _site_id and
            name = walden_basename(_fullpath) and
            parent_path = walden_dirname(_fullpath)
    limit 1;
$$ language sql;

create or replace function
walden_query_get_or_create(_site_id integer, _fullpath text, _checksum text)
returns template as
$$
    with ins as (
        insert into query (site_id, name, parent_path, checksum)
        values (
            _site_id,
            walden_basename(_fullpath),
            walden_dirname(_fullpath),
            _checksum
        )
        on conflict (site_id, name, parent_path)
        do update set checksum = _checksum 
        returning *
    )
    select * from ins
        union all
    select * from query 
    where   site_id = _site_id and 
            name = walden_basename(_fullpath) and
            parent_path = walden_dirname(_fullpath) 
    limit 1;
$$ language sql;

create or replace function
walden_page_spec_get_or_create(_template_id integer, _query_id integer)
returns page_spec as 
$$
    declare
        temp page_spec;
    begin
        with ins as (
            insert into page_spec(template_id, query_id)
            values (_template_id, _query_id)
            on conflict (template_id, query_id)
            do nothing 
            returning *
        )
        select * into temp from (
            select * from ins
                union all
            select * from page_spec 
            where template_id = _template_id and query_id = _query_id 
            limit 1
        ) as combined;
        return temp;
    end;
$$ language plpgsql;


/*
given: (_fullpath text)
fp_arr text[] := string_to_array(trim(leading '/' from _fullpath), '/');
_name text := fp_arr[array_upper(fp_arr, 1)];
_parent_path ltree := concat_ws('.', 'root', nullif(array_to_string(fp_arr[0:array_upper(fp_arr, 1)-1], '.'), ''))::ltree;
*/

create or replace function
walden_template_get_by_fullpath(_site_id integer, _fullpath text)
returns template as
$$
    select * from template
    where site_id = _site_id and
          name = walden_basename(_fullpath) and
          parent_path = walden_dirname(_fullpath)
    limit 1;
$$ language sql;

create or replace function
walden_query_get_by_fullpath(_site_id integer, _fullpath text)
returns query as
$$
    select * from query 
    where site_id = _site_id and
          name = walden_basename(_fullpath) and
          parent_path = walden_dirname(_fullpath)
    limit 1;
$$ language sql;

/*
create or replace function render(text, text) 
returns text 
as 'pgstach.so', 'render' 
language c strict immutable;

comment on function render(tmpl text, context json) is
    'Returns rendered string from  mustache template and json context.';
*/
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
do
$$
    declare
        app_id      application.id%TYPE;
    begin
        app_id := (walden_application_get_or_create('Webdev', current_schema)).id;
        perform walden_entity_get_or_create(app_id, 'Resource',  'resource');
        perform walden_entity_get_or_create(app_id, 'Page',      'page');
        perform walden_entity_get_or_create(app_id, 'StaticPage','static_page');
        perform walden_entity_get_or_create(app_id, 'Widget',    'widget');
        perform walden_entity_get_or_create(app_id, 'Query',     'query');
        perform walden_entity_get_or_create(app_id, 'Template',  'template');
        perform walden_entity_get_or_create(app_id, 'PageSpec',  'page_spec');
    end
$$ language plpgsql;

