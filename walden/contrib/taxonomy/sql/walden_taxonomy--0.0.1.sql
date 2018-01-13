/*
    A Taxonomy is a 'tree of increasing classification specifity'. This
    tree also happens to be how the various pages on the site are mapped to
    their url homes.
*/
\echo Use "CREATE EXTENSION walden" to load this file. \quit

--- For now we'll stick contrib apps in with walden proper.
CREATE SCHEMA IF NOT EXISTS walden;
CREATE SCHEMA IF NOT EXISTS walden_history;


CREATE TABLE taxonomy
(
    id      SERIAL  NOT NULL PRIMARY KEY,
    name    TEXT    NOT NULL UNIQUE
);
ALTER TABLE taxonomy OWNER to walden;
SELECT pg_catalog.pg_extension_config_dump('taxonomy', '');
COMMENT ON TABLE taxonomy is '';
/*
CREATE TABLE walden_history.walden_user (LIKE walden_user);

-- Add the trigger for versioning.
CREATE TRIGGER walden_user_versioning_trigger
BEFORE INSERT OR UPDATE OR DELETE ON walden_user
FOR EACH ROW EXECUTE PROCEDURE versioning('sys_period',
                                          'walden_history.walden_user',
                                          true);
*/


CREATE TABLE taxon
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    name        TEXT    NOT NULL,
    parent_path LTREE   NOT NULL UNIQUE,
    resource_id INTEGER NOT NULL REFERENCES resource(id),
    page_id     INTEGER NOT NULL REFERENCES page(id)
);
ALTER TABLE taxon OWNER to walden;
SELECT pg_catalog.pg_extension_config_dump('taxon', '');
/*
    Need trigger contraint on UPDATE to make sure the name isn't changed
    once a taxon is published.
*/

CREATE TABLE taxon_resource
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    taxon_id    INTEGER NOT NULL REFERENCES taxon(id),
    resource_id INTEGER NOT NULL REFERENCES resource(id),
    UNIQUE (taxon_id, resource_id)
);
ALTER TABLE taxon_resource OWNER to walden;
SELECT pg_catalog.pg_extension_config_dump('taxon_resource', '');


DO $$
BEGIN
   PERFORM walden_register_application('Taxonomy');
   PERFORM walden_register_entity('Taxonomy', 'Taxonomy',   'taxonomy');
   PERFORM walden_register_entity('Taxonomy', 'Taxon',      'taxon');
END$$;
