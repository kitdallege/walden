--- create 'proxy' views between walden_master -> walden_data schemas
--- for the core `walden` non-entity types.
CREATE OR REPLACE FUNCTION install_views()
RETURNS VOID AS $f1$
    views = [
        'walden_application', 'walden_attribute', 'walden_attribute_type',
        'walden_entity', 'walden_entity_attribute', 'walden_postgres_type',
        'walden_branch'
    ]
    with plpy.subtransaction():
        for view in views:
            plpy.execute('DROP VIEW IF EXISTS walden_master.{0} CASCADE;'.format(view)) # remove view
        for view in views:
            if view != 'walden_branch':
                stmt = 'CREATE OR REPLACE VIEW walden_master.{0} AS SELECT * FROM walden_data.{0} WHERE {0}.branch_id = 1;'.format(view)
            else:
                stmt = 'CREATE OR REPLACE VIEW walden_master.{0} AS SELECT * FROM walden_data.{0};'.format(view)
            plpy.execute(stmt)
$f1$ language plpythonu;
COMMENT ON FUNCTION install_views() is 'Installs views in the walden_master schema for the core tables in walden_data.';

-- Computed Columns
CREATE OR REPLACE FUNCTION install_branch_fks()
RETURNS VOID AS $f1$
    views = [
        'walden_application', 'walden_attribute', 'walden_attribute_type',
        'walden_entity', 'walden_entity_attribute', 'walden_postgres_type'
    ]
    with plpy.subtransaction():
        for view in views:
            plpy.execute('DROP FUNCTION IF EXISTS walden_master.{0}_branch(walden_master.{0})'.format(view))
        for view in views:
            stmt = (
                'CREATE OR REPLACE FUNCTION walden_master.{0}_branch(walden_master.{0}) '
                'RETURNS walden_master.walden_branch AS $$ '
                'SELECT walden_branch.* '
                'FROM walden_master.walden_branch '
                'WHERE $1.branch_id = walden_branch.id '
                'ORDER BY walden_branch.inst_id '
                'LIMIT 1;'
                '$$ LANGUAGE SQL STABLE;'
            ).format(view)
            plpy.execute(stmt)
$f1$ language plpythonu;
COMMENT ON FUNCTION install_branch_fks() is 'Generates and installs functions which get walden_master.walden_branch for a given row.';

CREATE OR REPLACE FUNCTION walden_master.walden_entity_application (walden_master.walden_entity)
RETURNS walden_master.walden_application AS $$
    SELECT walden_application.*
    FROM walden_master.walden_application
    WHERE $1.application_id = walden_application.id
        AND walden_application.branch_id = 1
    ORDER BY walden_application.inst_id
    LIMIT 1;
$$ LANGUAGE SQL STABLE;
COMMENT ON FUNCTION walden_master.walden_entity_application (walden_master.walden_entity) is 'walden_entity -> walden_application.';

CREATE OR REPLACE FUNCTION walden_master.walden_entity_entity_attribute (walden_master.walden_entity)
RETURNS SETOF walden_master.walden_entity_attribute AS $$
    SELECT DISTINCT ON (walden_entity_attribute.id)
        walden_entity_attribute.*
    FROM walden_master.walden_entity_attribute
    WHERE $1.id = walden_entity_attribute.entity_id
        AND walden_entity_attribute.branch_id = 1
    ORDER BY walden_entity_attribute.id ASC,
             walden_entity_attribute.inst_id DESC;
$$ LANGUAGE SQL STABLE;

CREATE OR REPLACE FUNCTION walden_master.walden_entity_attribute_attribute (walden_master.walden_entity_attribute)
RETURNS walden_master.walden_attribute AS $$
    SELECT DISTINCT ON (walden_attribute.id)
        walden_attribute.*
    FROM walden_master.walden_attribute
    WHERE $1.attribute_id = walden_attribute.id
        AND walden_attribute.branch_id = 1
    ORDER BY walden_attribute.id ASC,
             walden_attribute.inst_id DESC
     LIMIT 1;
$$ LANGUAGE SQL STABLE;

CREATE OR REPLACE FUNCTION walden_master.walden_attribute_attribute_type (walden_master.walden_attribute)
RETURNS walden_master.walden_attribute_type AS $$
    SELECT DISTINCT ON (walden_attribute_type.id)
        walden_attribute_type.*
    FROM walden_master.walden_attribute_type
    WHERE $1.attribute_type_id = walden_attribute_type.id
        AND walden_attribute_type.branch_id = 1;
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

-- Mutations
-- get all{table-name} and create{table-name} by default.
-- would normally also get:
--   {table-name}
--   {table-name}By[id || inst_id]
--   [create, update, delete]{table-name.title()}
--   [upcate, delete]{table-name.title}By[id || inst_id]

-- get_object
CREATE OR REPLACE FUNCTION walden_master.get_walden_entity (id INTEGER)
RETURNS walden_master.walden_entity AS $$
    SELECT DISTINCT ON (walden_entity.id)
        walden_entity.*
    FROM walden_master.walden_entity
    WHERE walden_entity.id = id
    ORDER BY walden_entity.id, walden_entity.inst_id DESC
    LIMIT 1;
$$ LANGUAGE SQL STABLE;

CREATE OR REPLACE FUNCTION walden_master.get_walden_application (id INTEGER)
RETURNS walden_master.walden_application AS $$
    SELECT DISTINCT ON (walden_application.id)
        walden_application.*
    FROM walden_master.walden_application
    WHERE walden_application.id = id
    ORDER BY walden_application.id, walden_application.inst_id DESC
    LIMIT 1;
$$ LANGUAGE SQL STABLE;

-- Future but ASAP mmmkay.
CREATE OR REPLACE FUNCTION upgrade_entity(entity walden_master.walden_entity)
RETURNS VOID AS $$

    # all the logic here could be avoided w/a `walden_entity_version` table
    # which holds what version of an entity is installed where
    # [[schema name], [entity_id], [inst_id]]
    # SELECT * FROM walden_installed_entity_version
    # WHERE schema_name = current_schema()
    # AND entity_id = entity.id
    # HAVING max(inst_id)
    # LIMIT 1;
    with plpy.subtransaction():
        # pull the entity/app
        stmt = (
            'SELECT walden_entity.* FROM walden_data.walden_entity '
            'WHERE walden_entity.inst_id = {0}'
        ).format(plpy.quote_literal(entity['inst_id']))
        r = plpy.execute(stmt)
        # pull the attrs w/[type, pg-type]

        # hit info schema and see if the table exists
        # if no then its a clean creation
        # else: pull the column info
$$ LANGUAGE plpythonu;
COMMENT ON FUNCTION upgrade_entity(entity walden_master.walden_entity) is 'Upgrades the schema of an entity to the version specified';

/*
-- just saving as a howto/example thing-er.
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
