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
create or replace function
walden_user_get_or_create(_username text, _first text, _last text, _email text, _pass text, _is_super boolean)
returns walden_user as 
$$
    insert into walden_user (username, first_name, last_name, email, password, is_super)
    values (_username, _first, _last, _email, _pass, _is_super)
    on conflict (username)
    do update set
        first_name = _first,
        last_name = _last,
        email = _email,
        password = _pass,
        is_super = _is_super
    returning *;
$$ language sql volatile;

/**************************************************************
 *                      App Config                            *
 **************************************************************/
do
$$
    declare
        app_id      application.id%TYPE;
    begin
        app_id := (walden_application_get_or_create('Auth')).id;
        perform walden_entity_get_or_create(app_id, 'User', 'walden_user');
        perform walden_user_get_or_create('kit', 'Kit', 'Dallege', 'kitdallege@gmail.com', 'pass', true);
    end;
$$ language plpgsql;

