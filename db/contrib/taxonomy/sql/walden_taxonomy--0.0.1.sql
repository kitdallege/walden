/* Initial Install of walden_taxonoy */
--\echo Use "CREATE EXTENSION walden" to load this file. \quit

/**************************************************************
 *                    Tables & Types                          *
 **************************************************************/
create table taxonomy
(
    id      serial  not null primary key,
    site_id integer not null references site(id) unique,
    name    text    not null unique,
    unique (site_id, name)
);
--ALTER TABLE taxonomy OWNER to walden;
select pg_catalog.pg_extension_config_dump('taxonomy', '');
comment on table taxonomy is 'Classification tree for site content.';

--enum ('index', 'detail', 'list-by-occurrence_date');
create table taxon_type
(
    id          serial  not null primary key,
    name        text    not null unique,
    handler     text    not null,
    arg_spec    json not null default '{}'
);
create table taxon
(
    id                  serial      not null primary key,
    taxonomy_id         integer     not null references taxonomy(id),
    taxon_type_id       integer     not null,
    parent_path         ltree       not null,
    name                text        not null,
    unique (taxonomy_id, parent_path, name)
);
select pg_catalog.pg_extension_config_dump('taxon', '');
-- Needed so that when a template/query changes it can be linked to the
-- resources which need to be flagged as dirty.
create table taxon_page_spec
(
    id                  serial      not null primary key,
    taxon_id            integer     not null references taxon(id),
    page_spec_id        integer     not null references page_spec(id),
    key                 text        not null,
    --factory_func text not null
    unique (taxon_id, page_spec_id, key)
    --role                taxon_type  not null default 'index';
);
-- ties a taxon to the entity 'type'(s)  it displays
-- This allows a trigger to determine what taxon function(s) need to be 
-- called when an object is updated...
create table taxon_entity_type
(
    id              serial  not null primary key,
    taxon_id        integer not null references taxon(id),
    entity_id       integer not null references entity(id),
    handler_args    json not null default '{}',
    where_clause    text not null default '',
    unique (taxon_id, entity_id)
    --selectable      text    not null
    --key_fields    text[]  not null default '{}'::text[];
);

/* The renderer generates a single where clause /  group by on a
'pre defined' 'key'. It expects to find a 'context' column containing
json. 

[The Plan]
-----------------------------------------------------------------------------
V1:
 app provides a 'selectable' with pre-defined 'key' & 'context columns.
 key is used in 'where clause' and for record linkage within the renderer
 context, is well, the page context (minus the site globals which 
 the renderer adds).

V2:
 app provides a selectable returning context with the key fields and a 
 'key fields array', and the backend generates the key.

V3:
 app provides a selectable returning 'items' with an array of key fields, and 
 the query/context is built automatically.

V4:
 backend generates query, key, and context. user selects attributes
 and can provide custom attributes in the form of a mapping of text name
 to a function taking a record of the underlying 'selectables' type. 
 user denotes the key fields.. all sql is generated from there.
-----------------------------------------------------------------------------
*/

/**************************************************************
 *                      Functions                             *
 **************************************************************/
create or replace function
walden_taxonomy_get_or_create(_site_id integer)
returns integer as 
$$
    with ins as (
            insert into taxonomy(site_id, name)
            values (_site_id, (select domain from site where id = _site_id))
            on conflict (site_id, name)
            -- never executed, but locks the row
            -- if no lock needed then 'do nothing' is less overhead. 
            do update set name = null where false
            returning id
        )
        select id from ins
            union all
        select id from taxonomy where site_id = _site_id
        limit 1;
$$ language sql volatile;



create or replace function
walden_taxon_get_or_create(_taxonomy_id integer, _taxon_type_id integer, _fullpath text)
returns taxon as 
$$
    with 
        ins as (
            insert into taxon(taxonomy_id, taxon_type_id, parent_path, name)
            values (
                _taxonomy_id,
                _taxon_type_id,
                walden_dirname(_fullpath),
                walden_basename(_fullpath)
            )
            on conflict (taxonomy_id, parent_path, name)
            do update set name = walden_basename(_fullpath) where false 
            returning *
        )
        select * from ins
            union all
        select * from taxon
        where   taxonomy_id = _taxonomy_id and
                parent_path = walden_dirname(_fullpath) and
                name = walden_basename(_fullpath)
        limit 1;
$$ language sql;

create or replace function
walden_taxon_type_get_or_create(
    _name text,
    _handler text,
    _arg_spec json default '{}'::json
) returns taxon_type as 
$$
    with 
        ins as (
            insert into taxon_type(name, handler, arg_spec)
            values (_name, _handler, _arg_spec)
            on conflict (name)
            do update set handler = _handler, arg_spec = _arg_spec 
            returning *
        )
        select * from ins
            union all
        select * from taxon_type
        where name = _name
        limit 1;
$$ language sql;

create or replace function
walden_taxon_type_get_by_name(_name text)
returns taxon_type as 
$$
        select * from taxon_type
        where name = _name
        limit 1;
$$ language sql;

create or replace function
walden_taxon_add_page_spec(_taxon_id integer, _page_spec_id integer, _key text)
returns integer as
$$
    with ins as (
            insert into taxon_page_spec(taxon_id, page_spec_id, key)
            values (_taxon_id, _page_spec_id, _key)
            on conflict (taxon_id, page_spec_id, key)
            -- never executed, but locks the row
            -- if no lock needed then 'do nothing' is less overhead. 
            do update set taxon_id = null where false
            returning id
        )
        select id from ins
            union all
        select id from taxon_page_spec
        where   taxon_id = _taxon_id and
                page_spec_id = _page_spec_id and
                key = _key
        limit 1;
$$ language sql volatile;

create or replace function
walden_taxon_add_allowed_type(
    _taxon          taxon, -- should pass whole taxon in
    _entity_type_id integer, -- should pass whole entity in.
    _handler_args   json,
    _where_clause   text
)
returns void as
$$
    declare
        handler_fn  text;
        _name_part   text;
        _taxon_id   integer;
        _entity_table text;
    begin
        _taxon_id := _taxon.id;
        -- add taxon_entity_type record for this.
        insert into taxon_entity_type (taxon_id, entity_id, handler_args, where_clause)
        values (_taxon_id, _entity_type_id, _handler_args, _where_clause)
        on conflict (taxon_id, entity_id)
        do update set handler_args = _handler_args, where_clause = _where_clause;
        -- generate trigger function for entity.db_object
        -- call taxon_type.handler(*args)
        -- this function will create trigger/function uniquely based on the
        -- 'taxon-type'
        select into handler_fn handler
        from taxon_type where taxon_type.id = (
            select taxon_type_id from taxon where id = _taxon_id);
        
        select into _entity_table db_object
        from entity where id = _entity_type_id; 
        -- unique part of the generated functions name
        select into _name_part
            lower(application.name || '_' || entity.name || '__' ||
                'taxon_' || _taxon.id::text)
        from entity join application on application.id = entity.application_id
        where entity.id = _entity_type_id;
        -- create the handler function that is called on every row
        execute format('select %s($1, $2, $3, $4);', handler_fn)
        using _taxon, _name_part, _entity_table, _handler_args;

        -- create trigger function which calls above handler function
        execute format('
            create or replace
            function %s__trigger() returns trigger as 
            $fn$
            declare
                _taxon taxon;
            begin
                select into _taxon from taxon where id = %L;
                perform %s__handler(_taxon, object)
                    from v_new_table as object
                where %s ;
                perform pg_notify(''page'', ''dirty'');
                return new;
            end; 
           $fn$ language plpgsql;',
                _name_part, _taxon.id, 
               format('%s_taxon_-handler', _name_part),
               coalesce(_where_clause, 'true')
        );

        -- create statement level trigger(s) 
        -- which call the above trigger function 
        execute format('
            drop trigger if exists %s__trigger_on_update on %s; 
            create trigger %s__trigger_on_update 
            after update on %s 
            referencing new table as v_new_table
            for each statement
            execute procedure %s__trigger();',
                _name_part, _entity_table,
                _name_part, _entity_table,
                _name_part);

        execute format('
            drop trigger if exists %s__trigger_on_insert on %s; 
            create trigger %s__trigger_on_insert 
            after insert on %s 
            referencing new table as v_new_table
            for each statement
            execute procedure %s__trigger();',
                _name_part, _entity_table,
                _name_part, _entity_table,
                _name_part);
        -- TODO: _on_delete [need logical delete ability]
    end;
$$ language plpgsql volatile;

create or replace function
walden_taxon_handler_index(
    _taxon taxon,
    _name_part text,
    _entity_table text,
    _args json
) returns void as 
$$
    begin
        -- create trigger fn
        execute format('
            create or replace
            function %s__handler(t taxon, obj %s)
            returns void as
            $fn$
                insert into page (
                    site_id, name, mount, page_spec_id,
                    query_args, date_updated, dirty
                )
                values (
                    %L,
                    t.name,
                    t.parent_path,
                    %L,
                    default,
                    (now() at time zone ''utc''),
                    true
                ) on conflict (name, mount)
                    do update
                        set date_updated = (now() at time zone ''utc''),
                        dirty = true;
            $fn$ language sql;', _name_part, _entity_table,
            (select site_id from taxonomy where id = _taxon.taxonomy_id),
            (select page_spec_id from taxon_page_spec
                where taxon_id = _taxon.id and key = 'main')
        ); 
    end;
$$ language plpgsql;

create or replace function
walden_taxon_handler_detail(
    _taxon taxon,
    _name_part text,
    _entity_table text,
    _args json
) returns void as 
$$
    declare
        _date_field text;
        _name_field text;
        _key_field  text;
    begin
        _date_field := _args::json#>>'{fields,date-field}';
        _name_field := _args::json#>>'{fields,name-field}';
        _key_field  := _args::json#>>'{fields,key-field}';
        execute format('
            create or replace
            function %s__handler(t taxon, obj %s)
            returns void as
            $fn$
                insert into page (
                    site_id, name, mount, page_spec_id,
                    query_args, date_updated, dirty
                )
                values (
                    %L,
                    obj.%s,
                    subpath(t.parent_path, 0) || to_char(obj.%s::date, ''yyyy.mm.dd''),
                    %L,
                    obj.%s, 
                    (now() at time zone ''utc''),
                    true
                )
                on conflict (name, mount)
                do update
                    set date_updated = (now() at time zone ''utc''),
                    dirty = true;
           $fn$ language sql;', _name_part, _entity_table,
            (select site_id from taxonomy where id = _taxon.taxonomy_id),
            _name_field,
            _date_field,
            (select page_spec_id from taxon_page_spec
                where taxon_id = _taxon.id and key = 'main'),
            _key_field
         );
    end;
$$ language plpgsql;


create or replace function
walden_taxon_handler_date_based(
    _taxon taxon,
    _name_part text,
    _entity_table text,
    _args json
) returns void as 
$$
    declare
        --
        _date_field     text;
        _index_key_fn   text;
        _day_key_fn     text;
    begin
        _date_field     := _args::json#>>'{fields,date-field}';
        _index_key_fn   := _args::json#>>'{key-funcs,index}';
        _date_field     := _args::json#>>'{key-funcs,day}';
        -- index
        execute format('
            create or replace
            function %s__index_handler(t taxon, obj %s)
            returns void as
            $fn$
                insert into page (
                    site_id, name, mount, page_spec_id,
                    query_args, date_updated, dirty
                )
                values (
                    %L,
                    t.name,
                    t.parent_path, 
                    %L,
                    %s(obj),
                    (now() at time zone ''utc''),
                    true
                ) on conflict (name, mount)
                    do update
                        set date_updated = (now() at time zone ''utc''),
                        dirty = true;
            $fn$ language sql;', _name_part, _entity_table,
            (select site_id from taxonomy where id = _taxon.taxonomy_id),
            (select page_spec_id from taxon_page_spec
                where taxon_id = _taxon.id and key = 'main'),
            _index_key_fn
        );
        -- year
        -- month
        -- day
        execute format('
            create or replace
            function %s__day_handler(t taxon, obj %s)
            returns void as
            $fn$
                insert into page (
                    site_id, name, mount, page_spec_id,
                    query_args, date_updated, dirty
                )
                values (
                    %L,
                    to_char(obj.%s::date, ''dd''),
                    subpath(t.parent_path, 0) || t.name || to_char(obj.%s::date, ''yyyy.mm''),
                    %L,
                    %s(obj),
                    (now() at time zone ''utc''),
                    true
                ) on conflict (name, mount)
                    do update
                        set date_updated = (now() at time zone ''utc''),
                        dirty = true;
            $fn$ language sql;', _name_part, _entity_table,
            (select site_id from taxonomy where id = _taxon.taxonomy_id),
            _date_field, _date_field,
            (select page_spec_id from taxon_page_spec
                where taxon_id = _taxon.id and key = 'day'),
            _day_key_fn
        );
        -- today
        -- detail

        -- wrapper
        -- This is kinda lame: but atm the 'trigger' only knows how to
        -- call into one function. As such, we wrap all the above functions
        -- in a single function.
        execute format('
            create or replace
            function %s__handler(t taxon, obj %s)
            returns void as
            $fn$
                begin
                    perform %s__index_handler(t, obj);
                    perform %s__day_handler(t, obj);
                end;
            $fn$ language plpgsql;',
            _name_part, _entity_table, _name_part, _name_part
        );
    end;
$$ language plpgsql;
/**************************************************************
 *                      App Config                            *
 **************************************************************/
do
$$
    declare
        app_id      application.id%TYPE;
    begin
        app_id := (walden_application_get_or_create('Taxonomy')).id;
        perform walden_entity_get_or_create(app_id, 'Taxonomy',     'taxonomy');
        perform walden_entity_get_or_create(app_id, 'Taxon',        'taxon');
        perform walden_entity_get_or_create(app_id, 'Taxon Type',   'taxon_type');
        perform walden_taxon_type_get_or_create('index', 'walden_taxon_handler_index');
        perform walden_taxon_type_get_or_create('detail', 'walden_taxon_handler_detail');
        perform walden_taxon_type_get_or_create('date-based', 'walden_taxon_handler_date_based');
    end;
$$ language plpgsql;

