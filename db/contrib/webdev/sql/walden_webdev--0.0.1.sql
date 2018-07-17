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
    name            text        not null,
    parent_path     ltree       not null,
    checksum        text        not null,
    unique (name, parent_path)
);

-- Store includes so that @ update we can determine 'ALL' effected resources.
create table template_include
(
    id                  serial      not null primary key,
    date_created        timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated        timestamp   with time zone not null default (now() at time zone 'utc'),
    source_template_id  integer references template(id),
    include_template_id integer references template(id),
    unique (source_template_id, include_template_id)
);

create table query
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    name            text        not null,
    parent_path     ltree       not null,
    checksum        text        not null,
    unique (name, parent_path)
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
    date_created        timestamp with time zone
                                    not null default (now() at time zone 'utc'),
    date_updated        timestamp with time zone
                                    not null default (now() at time zone 'utc'),
    name                text        not null,
    mount               ltree       not null,
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
do $$
begin
   perform walden_register_application('Webdev');
   --perform walden_register_entity('Webdev', 'Asset',    'asset');
   perform walden_register_entity('Webdev', 'Page',     'page');
   perform walden_register_entity('Webdev', 'Resource', 'resource');
   perform walden_register_entity('Webdev', 'Widget',   'widget');
   --perform walden_register_entity('Webdev', 'Query',    'wquery');
end$$;

