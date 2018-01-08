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

CREATE TABLE taxon
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    name        TEXT    NOT NULL,
    parent_path LTREE   NOT NULL UNIQUE,
    resource_id INTEGER NOT NULL REFERENCES resource(id),
    page_id     INTEGER NOT NULL REFERENCES page(id)
);
ALTER TABLE taxon OWNER to walden;
SELECT pg_catalog.pg_extension_config_dump('taxonomy', '');

CREATE TABLE entity_taxon
(
    id          SERIAL  NOT NULL PRIMARY KEY,
    entity_id   INTEGER NOT NULL REFERENCES entity(id),
    taxon_id    INTEGER NOT NULL REFERENCES taxon(id),
    UNIQUE (entity_id, taxon_id)
);
ALTER TABLE entity_taxon OWNER to walden;
SELECT pg_catalog.pg_extension_config_dump('taxonomy', '');


INSERT INTO taxonomy (name)
    VALUES ('ComeToVegas');
INSERT INTO taxon (name, parent_path, resource_id, page_id)
    VALUES ('all users', 'root.users', 1, 1);
