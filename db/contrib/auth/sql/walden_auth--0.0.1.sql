/* Initial Install of walden_auth */
\echo Use "CREATE EXTENSION walden_auth" to load this file. \quit

/**************************************************************
 *                    Tables & Types                          *
 **************************************************************/
create table walden_user
(
    id              serial      not null primary key,
    date_created    timestamp   with time zone not null default (now() at time zone 'utc'),
    date_updated    timestamp   with time zone not null default (now() at time zone 'utc'),
    username        text        not null unique,
    first_name      text        not null,
    last_name       text        not null,
    email           text        not null,
    password        text        not null,
    is_super        boolean     not null default false
);
comment on column walden_user.password IS 'Password uses [algo]$[salt]$[hexdigest].';
comment on table walden_user is 'User within the walden system.';
select pg_catalog.pg_extension_config_dump('walden_user', '');


/**************************************************************
 *                      Functions                             *
 **************************************************************/


/**************************************************************
 *                      App Config                            *
 **************************************************************/
do $$
begin
   perform walden_register_application('Auth');
   perform walden_register_entity('Auth', 'User', 'walden_user');
   insert into walden_user (username, first_name, last_name, email, password)
       values ('kit', 'Kit', 'Dallege', 'kitdallege@gmail.com', '******');
end$$;

