/*
CREATE OR REPLACE RULE "_RETURN" AS
    ON SELECT TO walden_master.application
    DO INSTEAD
    SELECT DISTINCT on (application.id)
        application.timestamp_start,
        application.timestamp_end,
        application.parent_id,
        application.inst_id,
        application.id,
        application.name,
        application.description
    FROM walden_data.application
    ORDER BY id ASC, application.timestamp_end DESC;
*/

--- create 'proxy' views between walden_master -> walden_data schemas.
create or replace function install_views()
returns void as $f1$
    views = [
        'walden_application', 'walden_attribute', 'walden_attribute_type',
        'walden_entity', 'walden_entity_attribute', 'walden_postgres_type',
        'walden_branch'
    ]
    with plpy.subtransaction():
        for view in views:
            plpy.execute('DROP VIEW IF EXISTS walden_master.{0} CASCADE;'.format(view)) # remove view
        for view in views:
            stmt = 'CREATE OR REPLACE VIEW walden_master.{0} AS SELECT * FROM walden_data.{0};'.format(view)
            plpy.execute(stmt) # create view
$f1$ language plpythonu;
COMMENT ON FUNCTION install_views() is 'Installs views in the walden_master schema for the core tables in walden_data.';

create or replace function install_branch_fks()
returns void as $f1$
    views = [
        'walden_application', 'walden_attribute', 'walden_attribute_type',
        'walden_entity', 'walden_entity_attribute', 'walden_postgres_type'
    ]
    with plpy.subtransaction():
        for view in views:
            plpy.execute('DROP FUNCTION IF EXISTS walden_master.{0}_walden_branch(walden_master.{0})'.format(view))
        for view in views:
            stmt = (
                'CREATE OR REPLACE FUNCTION walden_master.{0}_walden_branch(walden_master.{0})'
                'RETURNS walden_master.walden_branch AS $$ '
                'SELECT walden_branch.* '
                'FROM walden_master.walden_branch '
                'WHERE $1.branch_id = walden_branch.id '
                'ORDER BY walden_branch.inst_id '
                'LIMIT 1;'
                '$$ LANGUAGE SQL STABLE;'
            ).format(view)
            plpy.execute(stmt) # branch func
$f1$ language plpythonu;
COMMENT ON FUNCTION install_branch_fks() is 'Generates and installs functions which get walden_master.walden_branch for a given row.';


CREATE OR REPLACE FUNCTION walden_master.walden_entity_application (walden_master.walden_entity)
RETURNS walden_master.walden_application AS $$
    SELECT walden_application.*
    FROM walden_master.walden_application
    WHERE $1.application_id = walden_application.id
    ORDER BY walden_application.inst_id
    LIMIT 1;
$$ LANGUAGE SQL STABLE;

CREATE OR REPLACE FUNCTION walden_master.walden_entity_entity_attribute (walden_master.walden_entity)
RETURNS SETOF walden_master.walden_entity_attribute AS $$
    SELECT DISTINCT ON (walden_entity_attribute.id)
        walden_entity_attribute.*
    FROM walden_master.walden_entity_attribute
    WHERE $1.id = walden_entity_attribute.entity_id
    ORDER BY walden_entity_attribute.id ASC,
             walden_entity_attribute.inst_id DESC;
$$ LANGUAGE SQL STABLE;

CREATE OR REPLACE FUNCTION walden_master.walden_entity_attribute_attribute (walden_master.walden_entity_attribute)
RETURNS walden_master.walden_attribute AS $$
    SELECT DISTINCT ON (walden_attribute.id)
        walden_attribute.*
    FROM walden_master.walden_attribute
    WHERE $1.attribute_id = walden_attribute.id
    ORDER BY walden_attribute.id ASC,
             walden_attribute.inst_id DESC
     LIMIT 1;
$$ LANGUAGE SQL STABLE;

CREATE OR REPLACE FUNCTION walden_master.walden_attribute_attribute_type (walden_master.walden_attribute)
RETURNS walden_master.walden_attribute_type AS $$
    SELECT DISTINCT ON (walden_attribute_type.id)
        walden_attribute_type.*
    FROM walden_master.walden_attribute_type
    WHERE $1.attribute_type_id = walden_attribute_type.id;
$$ LANGUAGE SQL STABLE;

CREATE OR REPLACE FUNCTION walden_master.walden_entity_attribute_postgres_type (walden_master.walden_entity_attribute)
RETURNS walden_master.walden_postgres_type AS $$
    SELECT DISTINCT ON (walden_postgres_type.id)
        walden_postgres_type.*
    FROM walden_master.walden_postgres_type
    WHERE $1.pgtype_id = walden_postgres_type.id
    ORDER BY walden_postgres_type.id ASC
    LIMIT 1;
$$ LANGUAGE SQL STABLE;
