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

--create type taxon_type as enum ('index', 'detail', 'list-by-occurrence_date');
--node_type           taxon_type  not null default 'list-by-occurrence_date',
create table taxon
(
    id                  serial      not null primary key,
    taxonomy_id         integer     not null references taxonomy(id),
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
    unique (taxon_id, page_spec_id)
    --role                taxon_type  not null default 'index';
);
-- ties a taxon to the entity 'type'(s)  it displays
-- This allows a trigger to determine what taxon function(s) need to be 
-- called when an object is updated...
create table taxon_entity_type
(
    id              serial  not null primary key,
    taxon_id        integer not null references taxon(id),
    entity_id       integer not null references entity(id)
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
random thoughts on how to go from v1 to v4

can v4 be done with sql api's.

-- this would build select. 
sql text := generate_query(
    entity,
    {fields array},
    {custom-fields-json},
    'context item(s) name',
    {key-fields}
);

it might be more useful to be able to specify the selectable, this would 
allow me to add custom fields/etc manually and still have the 'work' with
the machinery. (at that point, there are no fk's so determing fields
which are via 'join' become impossible). 

kinda leaning towards that as a default. eg: no related fields for custom
selectables.

on entities (where i'm mapping to a table) we can derive joins if they
are just strait obj.attr_id = rel.pk if there are other conditions
then were back to needing 'user defined stuffs'.

-- given the fields we might have to build joins to rels for those attributes.
sql text := generate_query(
    'generic.object', -- entity
    {'*', 'data_source.name as cite' }, -- all of its fields
    json_build_object(
        'fmt_time': 'object_fmt_time',
        'href': 'object_href'
    ),
    'items', 
    {'object_type.name', 'occurrence_date'}
);
-- TODO: section name shit ?
 or some type of function from array[]::jsonb to jsonb object.
 so it can 'name the items' and add wtf ever it wants.

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
walden_taxon_get_or_create(_taxonomy_id integer, _fullpath text)
returns taxon as 
$$
    declare 
        fp_arr text[] := string_to_array(_fullpath, '/');
        _name text := fp_arr[array_upper(fp_arr, 1)];
        _parent_path ltree := concat_ws('.', 'root',
            nullif(array_to_string(fp_arr[0:array_upper(fp_arr, 1)-1], '.'), '')
        )::ltree;
        temp taxon;
    begin
        with ins as (
                insert into taxon(taxonomy_id, parent_path, name)
                values (_taxonomy_id, _parent_path, _name)
                on conflict (taxonomy_id, parent_path, name)
                do nothing
                returning *
            )
            select into temp from (
                select * from ins
                union all
                select * from taxon
                where   taxonomy_id = _taxonomy_id and 
                        name = _name and
                        parent_path = _parent_path
                limit 1
            ) as combined;
        return temp;
   end;
$$ language plpgsql;

create or replace function
walden_taxon_add_page_spec(_taxon_id integer, _page_spec_id integer)
returns integer as
$$
    with ins as (
            insert into taxon_page_spec(taxon_id, page_spec_id)
            values (_taxon_id, _page_spec_id)
            on conflict (taxon_id, page_spec_id)
            -- never executed, but locks the row
            -- if no lock needed then 'do nothing' is less overhead. 
            do update set taxon_id = null where false
            returning id
        )
        select id from ins
            union all
        select id from taxon_page_spec
        where taxon_id = _taxon_id and page_spec_id = _page_spec_id
        limit 1;
$$ language sql volatile;
/*
create or replace function
walden_taxon_add_observed_type(_taxon_id integer, _entity_id integer, _keyfunc text)
returns integer as
$$

$$ language sql volatile;
*/
/**************************************************************
 *                      App Config                            *
 **************************************************************/
do
$$
    declare
        app_id      application.id%TYPE;
    begin
        app_id := (walden_application_get_or_create('Taxonomy')).id;
        perform walden_entity_get_or_create(app_id, 'Taxonomy', 'taxonomy');
        perform walden_entity_get_or_create(app_id, 'Taxon',    'taxon');
        -- taxon_page_spec
        -- taxon_entity_type
    end;
$$ language plpgsql;

