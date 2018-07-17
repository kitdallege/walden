/* Initial Install of walden_admin */
\echo Use "CREATE EXTENSION walden_admin" to load this file. \quit

/**************************************************************
 *                    Tables & Types                          *
 **************************************************************/
create table entity_form
(
    id          serial      not null primary key,
    sys_period  tstzrange   not null default tstzrange(current_timestamp, 'infinity'),
    entity_id   integer     not null references entity(id),
    json_schema json        not null,
    ui_schema   json        not null,
    is_default  boolean     not null default false
);

/**************************************************************
 *                      Functions                             *
 **************************************************************/
create function admin_create_default_form(e entity)
returns void as $$
    import json
    # select schema info about the entity from the information_schema
    # include refs by default ?

    #insert into entity_form (entity_id, json_schema, ui_schema, is_default)
    #    values (e.id, "", "", true);
$$ language plpythonu volatile;

/**************************************************************
 *                      App Config                            *
 **************************************************************/
do $$
begin
   perform walden_register_application('Admin');
   perform walden_register_entity('Admin', 'EntityForm', 'entity_form');
end$$;

