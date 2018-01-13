/* Initial Install of walden_admin */
\echo Use "CREATE EXTENSION walden_admin" to load this file. \quit

/**************************************************************
 *                      Schemas                               *
 **************************************************************/
CREATE SCHEMA IF NOT EXISTS walden;
CREATE SCHEMA IF NOT EXISTS walden_history;

/**************************************************************
 *                    Tables & Types                          *
 **************************************************************/
CREATE TABLE entity_form
(
    id          SERIAL      NOT NULL PRIMARY KEY,
    sys_period  TSTZRANGE   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    entity_id   INTEGER     NOT NULL REFERENCES entity(id),
    json_schema JSON        NOT NULL,
    ui_schema   JSON        NOT NULL,
    is_default  BOOLEAN     NOT NULL DEFAULT FALSE
);


CREATE FUNCTION admin_create_default_form(e entity)
RETURNS VOID AS $$
    import json
    # select schema info about the entity from the information_schema
    # include refs by default ?

    #INSERT INTO entity_form (entity_id, json_schema, ui_schema, is_default)
    #    VALUES (e.id, "", "", TRUE);
$$ LANGUAGE plpythonu VOLATILE;

DO $$
BEGIN
   PERFORM walden_register_application('Admin');
   PERFORM walden_register_entity('Admin', 'EntityForm', 'entity_form');
END$$;
