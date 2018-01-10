-- pl/pgsql
-- CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;
-- COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';
-- python
CREATE EXTENSION IF NOT EXISTS plpythonu WITH SCHEMA pg_catalog;
COMMENT ON EXTENSION plpythonu IS 'PL/PythonU untrusted procedural language';
-- Walden
CREATE EXTENSION IF NOT EXISTS walden WITH SCHEMA walden CASCADE;
COMMENT ON EXTENSION walden IS
    'This extension provides a web development platform within PostgreSQL';

CREATE EXTENSION IF NOT EXISTS walden_taxonomy WITH SCHEMA walden_taxonomy CASCADE;
COMMENT ON EXTENSION walden_taxonomy IS
    'This extension provides a Taxonomy based resource tree for the Walden system';
