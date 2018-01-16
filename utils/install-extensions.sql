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

CREATE EXTENSION IF NOT EXISTS walden_auth WITH SCHEMA walden CASCADE;
COMMENT ON EXTENSION walden_auth IS
    'This extension provides Auth/Permissions for the Walden system.';


CREATE EXTENSION IF NOT EXISTS walden_sites WITH SCHEMA walden CASCADE;
COMMENT ON EXTENSION walden_sites IS
    'This extension provides Site object for the Walden system.';

CREATE EXTENSION IF NOT EXISTS walden_webdev WITH SCHEMA walden CASCADE;
COMMENT ON EXTENSION walden_webdev IS
    'This extension provides Site/Page Builder tools for the Walden system.';

CREATE EXTENSION IF NOT EXISTS walden_taxonomy WITH SCHEMA walden CASCADE;
COMMENT ON EXTENSION walden_taxonomy IS
    'This extension provides a Taxonomy based resource tree for the Walden system';

CREATE EXTENSION IF NOT EXISTS walden_admin WITH SCHEMA walden CASCADE;
COMMENT ON EXTENSION walden_admin IS
    'This extension provides Admin UI for the Walden system.';
