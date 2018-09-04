/* Initial Install of walden_webdev */
\echo Use "CREATE EXTENSION walden_webdev" to load this file. \quit

/**************************************************************
 *                    Tables & Types                          *
 **************************************************************/
create table resource_tree
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    site_id         integer     not null references site(id),
    name            text        not null,
	unique(site_id, name)
);

create type asset_type as enum ('CSS', 'JS', 'IMG', 'FILE');
create table asset
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_deleted    timestamp   with time zone not null default 'infinity',
    asset_type      asset_type  not null default 'FILE',
    parent_path     ltree       not null,
    name            text        not null,
    checksum        text        not null,
    unique (name, parent_path)
);

create table template
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_deleted    timestamp   with time zone not null default 'infinity',
    tree_id         integer     not null references resource_tree(id),
    parent_path     ltree       not null,
    name            text        not null,
    checksum        text        not null,
    unique (tree_id, name, parent_path)
);

-- Store includes so that @ update we can determine 'ALL' effected resources.
create table template_include
(
    id                  serial      not null primary key,
    date_created        timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated        timestamp   with time zone not null default (now() at time zone 'utc'),
    date_deleted        timestamp   with time zone not null default 'infinity',
    source_template_id  integer     references template(id),
    include_template_id integer     references template(id),
    unique (source_template_id, include_template_id)
);

create table query
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_deleted    timestamp   with time zone not null default 'infinity',
    tree_id         integer     not null references resource_tree(id),
    parent_path     ltree       not null,
    name            text        not null,
    checksum        text        not null,
    unique (tree_id, name, parent_path)
);


-- resource_view_type enum ('index', 'list', 'detail')
create table resource_view
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    tree_id         integer     not null references resource_tree(id),
    template_id     integer     not null references template(id),
    query_id        integer     not null references query(id),
    unique (tree_id, template_id, query_id)
);

create table resource_route
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    --
    tree_id         integer     not null references resource_tree(id),
    -- How to denote data driven mount/name.
    -- date.year date.month date.day .. pk, attribute/joined (name)..
    mount           ltree       not null,
    name            text        not null,
    --
    view_id         integer     not null references resource_view(id),
    unique (tree_id, mount, name)
);

-- this is the handler
create table resource_entity_handler
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    handler         text        not null,
    arg_spec        json        not null default '{}'
);

-- this is the handler in use. eg: multiple routes can use the same handler
create table resource_route_entity_handler
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    route_id        integer     not null references resource_route(id),
    entity_id       integer     not null references entity(id),
    handler_id      integer     not null references resource_entity_handler(id),
    handler_args    json        not null default '{}',
    where_clause    text        not null default '',
    unique (route_id, entity_id)
);
/*
create table resource_type
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    name            text        not null unique,
    desc            text        not null default '',
 -- render_priority integer     not null default 0
);

Do we want a resource.type so we can filter by widget/page/etc.
Or do we derive that from the resources.view..

I'm thinking the former, as the less joins i can do the better.
But were already joining in query/template via the render_spec. so maybe
db design trumps denormalization-for-perf.

Do types need to be data driven (eg: how often we creating these things) ?
Or is it an enum who's value is controlled in the code only.
the benefit is no joins but normalized value to query on.

Are taxons/widgets/etc just their own 'virtual' resource_tree or really
just a group of 'view' functions.

so 'what' taxons and widgets bring to the table then is a 1-to-many with
resource_view(s). how those are mounted (the args to the factories) is
done at 'mount time'.



*/
create table resource 
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    -- ://tree/mount/name
    tree_id         integer     not null references resource_tree(id),
    mount           ltree       not null,
    name            text        not null,
    -- ref to view(mounted_factory) and get the spec from there ?
    -- render spec & args for it.
    view_id         integer     not null references resource_view(id),
    view_args       text        not null default '',
    --view_args       jsonb       not null default '{}',
    -- TODO: need at least another boolean for 'enqued' or a more
    -- robust task system ? (will need to solve before renderer can be multi-threaded
    dirty           boolean     not null default true,
    unique (tree_id, mount, name)
);

create table widget(
    id                  serial      not null primary key,
    date_created        timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated        timestamp   with time zone not null default (now() at time zone 'utc'),
    tree_id             integer     not null references resource_tree(id),
    mount               ltree       not null,
    name                text        not null,
    --handler     text    not null,
    --arg_spec    json not null default '{}'
    unique (tree_id, mount, name)
);

create table widget_view
(
    id              serial      not null primary key,
    widget_id       integer     not null references widget(id),
    view_id         integer     not null references resource_view(id),
    key             text        not null,
    unique (widget_id, view_id, key)
);
-- function mount_widget(tree_id, widget_id, path, name)

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
$$ language plpgsql
    returns null on null input;

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
$$ language plpgsql
    returns null on null input;

create or replace function
walden_resource_tree_get_or_create(_site_id integer, _name text)
returns resource_tree as 
$$
    with ins as (
        insert into resource_tree (site_id, name)
        values (_site_id, _name)
        on conflict (site_id, name)
        do nothing 
        returning *
    )
    select * from ins
        union all
    select * from resource_tree 
    where   site_id = _site_id and
            name = _name
    limit 1;
$$ language sql;

create or replace function
walden_template_get_or_create(_tree_id integer, _fullpath text, _checksum text)
returns template as
$$
    with ins as (
        insert into template (tree_id, name, parent_path, checksum)
        values (
            _tree_id,
            walden_basename(_fullpath),
            walden_dirname(_fullpath),
            _checksum
        )
        on conflict (tree_id, name, parent_path)
        do update set checksum = _checksum 
        returning *
    )
    select * from ins
        union all
    select * from template 
    where   tree_id = _tree_id and
            name = walden_basename(_fullpath) and
            parent_path = walden_dirname(_fullpath)
    limit 1;
$$ language sql;

create or replace function
walden_query_get_or_create(_tree_id integer, _fullpath text, _checksum text)
returns template as
$$
    with ins as (
        insert into query (tree_id, name, parent_path, checksum)
        values (
            _tree_id,
            walden_basename(_fullpath),
            walden_dirname(_fullpath),
            _checksum
        )
        on conflict (tree_id, name, parent_path)
        do update set checksum = _checksum 
        returning *
    )
    select * from ins
        union all
    select * from query 
    where   tree_id = _tree_id and 
            name = walden_basename(_fullpath) and
            parent_path = walden_dirname(_fullpath) 
    limit 1;
$$ language sql volatile;

create or replace function
walden_view_get_or_create(_tree_id integer, _template_id integer, _query_id integer)
returns resource_view as 
$$
    with ins as (
        insert into resource_view(tree_id, template_id, query_id)
        values (_tree_id, _template_id, _query_id)
        on conflict (tree_id, template_id, query_id)
        do nothing 
        returning *
    )
    select * from ins
        union all
    select * from resource_view 
    where   tree_id = _tree_id and 
            template_id = _template_id and
            query_id = _query_id 
    limit 1;
$$ language sql volatile;

create or replace function
walden_widget_get_or_create(_tree_id integer, _fullpath text)
returns widget as
$$
    with ins as (
        insert into widget (tree_id, name, mount)
        values (
            _tree_id,
            walden_basename(_fullpath),
            walden_dirname(_fullpath)
        )
        on conflict (tree_id, name, mount)
        do nothing 
        returning *
    )
    select * from ins
        union all
    select * from widget 
    where   tree_id = _tree_id and
            name = walden_basename(_fullpath) and
            mount = walden_dirname(_fullpath)
    limit 1; 
$$ language sql volatile;

create or replace function
walden_widget_add_view(_widget_id integer, _view_id integer, _key text)
returns integer as
$$
    with ins as (
            insert into widget_view(widget_id, view_id, key)
            values (_widget_id, _view_id, _key)
            on conflict (widget_id, view_id, key)
            -- never executed, but locks the row
            -- if no lock needed then 'do nothing' is less overhead. 
            do update set widget_id = null where false
            returning id
        )
        select id from ins
            union all
        select id from widget_view
        where   widget_id = _widget_id and
                view_id = _view_id and
                key = _key
        limit 1;
$$ language sql volatile;

create or replace function
walden_template_get_by_fullpath(_tree_id integer, _fullpath text)
returns template as
$$
    select * from template
    where tree_id = _tree_id and
          name = walden_basename(_fullpath) and
          parent_path = walden_dirname(_fullpath)
    limit 1;
$$ language sql;

create or replace function
walden_query_get_by_fullpath(_tree_id integer, _fullpath text)
returns query as
$$
    select * from query 
    where tree_id = _tree_id and
          name = walden_basename(_fullpath) and
          parent_path = walden_dirname(_fullpath)
    limit 1;
$$ language sql;

create or replace function
walden_template_delete_by_fullpath(_tree_id integer, _fullpath text)
returns void as
$$
    update template set date_deleted = now()
    where tree_id = _tree_id and
          name = walden_basename(_fullpath) and
          parent_path = walden_dirname(_fullpath);
$$ language sql;

create or replace function
walden_query_delete_by_fullpath(_tree_id integer, _fullpath text)
returns void as
$$
    update query set date_deleted = now()
    where tree_id = _tree_id and
          name = walden_basename(_fullpath) and
          parent_path = walden_dirname(_fullpath);
$$ language sql;

create or replace function
walden_template_set_includes(_tree_id integer, _fullpath text, _includes text[])
returns void as
$$
    with
        -- source_template
        templ as (
            select * from template
            where tree_id = _tree_id and
                name = walden_basename(_fullpath) and
                parent_path = walden_dirname(_fullpath)
        ),
        -- template _includes
        includes as (
            select * from template
            where (tree_id, name, parent_path) in (
                select _tree_id,  walden_basename(inc::text), walden_dirname(inc::text)
                from unnest(_includes) as inc
            )
        ),
        -- current includes that are not in _includes
        -- NOTE: might want to use  date_deleted on template_include
        -- so we can know what changed after the fact? *eg: during re-render*
        deletes as (
            delete from template_include t
            where not exists (
                select 1 from includes
                where source_template_id = (select id from templ) and include_template_id in (
                    select t.id from includes as t
                )
        )
    )
    insert into template_include (source_template_id, include_template_id)
    select (select id from templ), id
    from includes
    where not exists (
        select 1
        from template_include
        where (source_template_id, include_template_id) not in (
            select (select id from templ), t.id from includes as t
        )
    );
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
        --perform walden_entity_get_or_create(app_id, 'RenderSpec','render_spec');
    end
$$ language plpgsql;

