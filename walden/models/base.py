from sqlalchemy import (
    Column, DateTime, String, Integer, ForeignKey, Table, UnicodeText,
    text, MetaData,
    func
)
from sqlalchemy.orm import relationship, backref
from sqlalchemy.ext.declarative import declarative_base, declared_attr
from sqlalchemy.dialects.postgresql import JSONB

#_data : stores all data temporally (eg: denormalized fk's etc.)
#_master : view into 'master' branch of _data (normalized fk's etc.)
#_{branch.name} : development
Base          = declarative_base(metadata=MetaData(schema='walden_data'))
PublishedBase = declarative_base(metadata=MetaData(schema='walden_published'))
MasterBase    = declarative_base(metadata=MetaData(schema='walden_master'))


# Std Columns
# TODO: Make Partials so more args can be provided. ex: server_default
RequiredIntegerColumn = Column(Integer, nullable=False)
PKColumn = Column(Integer, primary_key=True, autoincrement=True)
FKColumn = RequiredIntegerColumn
NameColumn = Column(String(length=255), unique=True, nullable=False)
DescriptionColumn = Column(UnicodeText(), nullable=False)

class TemporalMixin(object):
    timestamp_start = Column(
        DateTime, nullable=False, server_default=text('now()')
    )
    timestamp_end = Column(
        DateTime, nullable=False, server_default=text("'infinity'")
    )

class HistoryMixin(object):
    # requires a way to denote object_id vs instance_id
    # should also store parent_instance_id or 0 if root.
    parent_id = Column(Integer, nullable=False) #, server_default=text('0'))
    # default makes it to easy to forget and then everything has parent = 0

class VersionedMixin(object):
    @declared_attr
    def branch_id(cls):
        return Column('branch_id', Integer, nullable=False)

    # def branch(cls):
    #     return relationship('Branch')
