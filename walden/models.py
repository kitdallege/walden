from sqlalchemy import (
    Column, DateTime, String, Integer, ForeignKey, Table, UnicodeText,
    text,
    func
)
from sqlalchemy.orm import relationship, backref
from sqlalchemy.ext.declarative import declarative_base, declared_attr

Base = declarative_base(metadata=MetaData(schema='walden_core'))
PublishedBase = declarative_base(metadata=MetaData(schema='walden_published'))

class TemporalMixin(object):
    timestamp_start = Column(
        DateTime, nullable=False, server_default=text('now()')
    )
    timestamp_end = Column(
        DateTime, nullable=False, server_default=text('infinity')
    )

class HistoryMixin(object):
    # requires a way to denote object_id vs instance_id
    # should also store parent_instance_id or 0 if root.
    pass

class VersionedMixin(object):
    parent_id = Column(Integer, nullable=False)
    @declared_attr
    def branch_id(cls):
        return Column('branch_id', ForeignKey('branch.id'))
    
    def branch(cls):
        return relationship('Branch')


# VCS
# need branch for Entities
class Branch(TemporalMixin, HistoryMixin, Base):
    __tablename__ = 'branch'
    id = Column(Integer, primary_key=True, autoincrement=True)
    branch_id = Column(Integer, nullable=False)
    name = Column(String(length=255), unique=True, nullable=False)
    description = Column(UnicodeText(), nullable=False)


# Entities

class Entity(TemporalMixin, HistoryMixin, VersionedMixin):
    __tablename__ = 'entity'
    id = Column(Integer, primary_key=True)
    entity_id = Column(Integer, nullable=False)
    name = Column(String(length=255), unique=True, nullable=False)
    description = Column(UnicodeText(), nullable=False)
    attributes = relationship('Atttribute', secondary='entity_attributes')
    # dealing with the physical model
    # logical_id = OneOrMore(self.attributes)
    # table = Column(String(length=255), unique=True, nullable=False)
    # inherit 
    # ordering
    # Would be nice to have 'sealed attributes' those which are not allowed 
    # to change.
    # Abilities ? - these would be groups of columns w/sql functions etc.

class Attribute(TemporalMixin, HistoryMixin, VersionedMixin):
    __tablename__ = 'attribute'
    id = Column(Integer, primary_key=True)
    attribute_id = Column(Integer)
    name = Column(String(length=255), unique=True, nullable=False)
    description = Column(UnicodeText())
    type = Column(nullable=False, ForeignKey('attribute_type.id'))

class AttributeType(Base):
    __tablename__ = 'attribute_type'
    id = Column(Integer, primary_key=True)
    attribute_id = Column(Integer)
    name = Column(String(length=255), unique=True, nullable=False)
    description = Column(UnicodeText())
    #pg_type = Column(Integer, ForeignKey("pg_catalog.pg_type.oid")) # ? 

# Immutable M2M's
# Need to store a valid time-span so that we know what items are valid for
# what other items (when).. So asking what 'requires/implies' when with all
# things temporal.
entity_attribute = Table(
    'entity_attribute', Base.metadata,
    # TemportalMixin 
    Column(
        'entity_id', Integer,
        ForeignKey('entity.id'), nullable=False
    ),
    Column(
        'attribute_id', Integer,
        ForeignKey('attribute.id'), nullable=False
    ),
    Column(
        'timestamp_start', DateTime,
        nullable=False, server_default=text('now()')
    ),
    Column(
        'timestamp_end', DateTime,
        nullable=False, server_default=text('infinity')
    ),
    # dealing with the physical model
    # type
    # type-args 
    # required (NULL/NOT NULL)
    # unique constraints (etc.)    
    Column('order', Integer) # hidden but used for column order
)


# Entity dependent VCS
class Commit(TemporalMixin, HistoryMixin, Base):
    __tablename__ = 'commit'

class CommitEntity(TemporalMixin, HistoryMixin, Base):
    __tablename__ = 'commit_entity'

class Merge(TemporalMixin, HistoryMixin, Base):
    __tablename__ = 'merge'

class MergeConflict(TemporalMixin, HistoryMixin, Base):
    __tablename__ = 'merge_conflict'

class MergeConflictResolution(TemporalMixin, HistoryMixin, Base):
    __tablename__ = 'merge_conflict_resolution'

# Move a commit `id` to walden_publish from walden_data
class Publish(TemporalMixin, HistoryMixin, Base):
    __tablename__ = 'publish'

class PublishLog(TemporalMixin, HistoryMixin, Base):
    __tablename__ = 'publish_log'


#
# Base Entity Types
# Question: Do you make the core Page/Routing types Entities or concrete ?
# Ya want to make um concrete @ first but maybe its better that they are
# Entities then if i need to glob attributes on, i'm not forced to do so
# via raw sql migrations. also could support concurrent versions as far as 
# dev_master/branch using something other than _published.

# Page Designer
class Template(Base):pass
class Asset(Base):pass
class AssetType(Base):pass
class Widget(Base):pass
widget_on_page = Table()
class Query(Base):pass
class BoundQuery(Base):pass

class Route(Base):pass
class Page(Base):pass


