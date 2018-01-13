/* Initial Install of walden_auth */
\echo Use "CREATE EXTENSION walden_auth" to load this file. \quit

/**************************************************************
 *                      Schemas                               *
 **************************************************************/
CREATE SCHEMA IF NOT EXISTS walden;
CREATE SCHEMA IF NOT EXISTS walden_history;


/**************************************************************
 *                    Tables & Types                          *
 **************************************************************/

CREATE TABLE walden_user
(
    id          SERIAL      NOT NULL PRIMARY KEY,
    sys_period  TSTZRANGE   NOT NULL DEFAULT tstzrange(current_timestamp, 'infinity'),
    username    TEXT        NOT NULL UNIQUE,
    first_name  TEXT        NOT NULL,
    last_name   TEXT        NOT NULL,
    email       TEXT        NOT NULL,
    password    TEXT        NOT NULL
);
ALTER TABLE walden_user OWNER to walden;
COMMENT ON COLUMN walden_user.password IS 'Password uses [algo]$[salt]$[hexdigest].';
COMMENT ON TABLE walden_user is 'User within the walden system.';
SELECT pg_catalog.pg_extension_config_dump('walden_user', '');
CREATE TABLE walden_history.walden_user (LIKE walden_user);
CREATE TRIGGER walden_user_versioning_trigger
BEFORE INSERT OR UPDATE OR DELETE ON walden_user
FOR EACH ROW EXECUTE PROCEDURE versioning('sys_period', 'walden_history.walden_user', true);




DO $$
BEGIN
   PERFORM walden_register_application('Auth');
   PERFORM walden_register_entity('Auth', 'User', 'walden_user');
   INSERT INTO walden_user (username, first_name, last_name, email, password)
       VALUES ('kit', 'Kit', 'Dallege', 'kitdallege@gmail.com', '******');
END$$;
