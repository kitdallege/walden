/*
    A Taxonomy is a 'tree of increasing classification specifity'. This
    tree also happens to be how the various pages on the site are mapped to
    their url homes.
*/
\echo Use "CREATE EXTENSION walden" to load this file. \quit

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
    UNIQUE (entity_id, taxon_id)
);
ALTER TABLE taxon_resource OWNER to walden;
SELECT pg_catalog.pg_extension_config_dump('taxon_resource', '');


INSERT INTO taxonomy (name)
    VALUES ('ComeToVegas');
INSERT INTO taxon (name, parent_path, resource_id, page_id)
    VALUES ('Home', 'root', 1, 1);
