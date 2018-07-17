/* Initial Install of walden_taxonoy */
\echo Use "CREATE EXTENSION walden" to load this file. \quit

/**************************************************************
 *                    Tables & Types                          *
 **************************************************************/
create table taxonomy
(
    id      serial  not null primary key,
    site_id integer not null references site(id) unique,
    name    text    not null unique
);
--ALTER TABLE taxonomy OWNER to walden;
select pg_catalog.pg_extension_config_dump('taxonomy', '');
comment on table taxonomy is 'Classification tree for site content.';

create type taxon_type as enum ('index', 'detail', 'list-by-occurrence_date');
create table taxon
(
    id                  serial      not null primary key,
    node_type           taxon_type  not null default 'list-by-occurrence_date',
    name                text        not null,
    parent_path         ltree       not null
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
create table taxon_object_type
(
    id              serial  not null primary key,
    taxon_id        integer not null references taxon(id),
    entity_id       integer not null references entity(id),
    selectable      text    not null
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


/**************************************************************
 *                      App Config                            *
 **************************************************************/
do $$
begin
   perform walden_register_application('Taxonomy');
   perform walden_register_entity('Taxonomy', 'Taxonomy',   'taxonomy');
   perform walden_register_entity('Taxonomy', 'Taxon',      'taxon');
end$$;

