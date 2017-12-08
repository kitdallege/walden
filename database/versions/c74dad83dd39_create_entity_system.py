"""create entity system

Revision ID: c74dad83dd39
Revises:
Create Date: 2017-12-07 00:09:49.296418

"""
from alembic import op
import sqlalchemy as sa
from sqlalchemy import INTEGER, VARCHAR, NVARCHAR, Column, TIMESTAMP, func

# revision identifiers, used by Alembic.
revision = 'c74dad83dd39'
down_revision = None
branch_labels = None
depends_on = None

#Create the core EntitySystem
def upgrade():
    # Create the various schemas
    op.execute("CREATE SCHEMA walden-core")
    #op.execute("CREATE SCHEMA walden-data")
    #op.execute("CREATE SCHEMA walden-public")

    # setup VCS system.
    op.create_table('branch',)
    op.create_table('commit',)
    op.create_table('merge',)
    op.create_table('merge_conflict',)
    op.create_table('merge_conflict_resolution',)
    
    # setup entity-system in the walden-core schema.
    op.create_table('type',)
    op.create_table('entity',)
    op.create_table('attribute',)
    op.create_table('entity_attributes',)
    
    # start adding PL functions.
    #

    # create views in the walden-master schema

# Return to an empty db.
def downgrade():
    pass
