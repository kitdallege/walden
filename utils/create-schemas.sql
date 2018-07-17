create schema walden;
--CREATE SCHEMA walden_history;
-- CREATE SCHEMA walden_taxonomy;

-- Walden
grant all on schema walden to walden_admin;
alter default privileges in schema walden
    grant all on tables to walden_admin;
alter default privileges in schema walden
    grant usage on sequences to walden_admin;
alter default privileges in schema walden
    grant execute on functions to walden_admin;
alter default privileges in schema walden
    grant usage on types to walden_admin;

-- History
-- grant all on schema walden_history to walden;
-- alter default privileges in schema walden_history
--     grant all on tables to walden;
-- alter default privileges in schema walden_history
--     grant usage on sequences to walden;
-- alter default privileges in schema walden_history
--     grant execute on functions to walden;
-- alter default privileges in schema walden_history
--     grant usage on types to walden;

-- Taxonomy
-- GRANT ALL ON SCHEMA walden_taxonomy TO walden;
-- ALTER DEFAULT PRIVILEGES IN SCHEMA walden_taxonomy
--     GRANT ALL ON TABLES TO walden;
-- ALTER DEFAULT PRIVILEGES IN SCHEMA walden_taxonomy
--     GRANT USAGE ON SEQUENCES TO walden;
-- ALTER DEFAULT PRIVILEGES IN SCHEMA walden_taxonomy
--     GRANT EXECUTE ON FUNCTIONS TO walden;
-- ALTER DEFAULT PRIVILEGES IN SCHEMA walden_taxonomy
--     GRANT USAGE ON TYPES TO walden;
