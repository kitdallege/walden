from sqlalchemy import (
    Column, DateTime, String, Integer, ForeignKey, Table, UnicodeText,
    text,
    func
)
from sqlalchemy.orm import relationship, backref
from sqlalchemy.ext.declarative import declarative_base, declared_attr
from sqlalchemy.dialects.postgresql import JSONB
from walden.models.base import (
    Base,
    PublishedBase,
    HistoryBase,
    # column's
    RequiredIntegerColumn,
    PKColumn,
    FKColumn,
    NameColumn,
    DescriptionColumn,
    # mixin's
    TemporalMixin,
    HistoryMixin,
    VersionedMixin
)

# VCS
# need branch for Entities
class Branch(TemporalMixin, HistoryMixin, Base):
    __tablename__ = 'branch'
    id = Column(Integer, primary_key=True, autoincrement='auto')
    branch_id = Column(Integer, nullable=False)
    name = Column(String(length=255), unique=True, nullable=False)
    description = Column(UnicodeText(), nullable=False)


# Entities
class Application(TemporalMixin, VersionedMixin, Base):
    __tablename__ = 'application'
    id = Column(Integer, primary_key=True, autoincrement='auto')
    application_id = Column(Integer, nullable=False)
    name = Column(String(length=255), unique=True, nullable=False)
    description = Column(UnicodeText(), nullable=False)

class Entity(TemporalMixin, VersionedMixin, Base):
    __tablename__ = 'entity'
    id = Column(Integer, primary_key=True, autoincrement='auto')
    entity_id = Column(Integer, nullable=False)
    application_id = Column(Integer, ForeignKey(Application.id), nullable=False)
    name = Column(String(length=255), unique=True, nullable=False)
    description = Column(UnicodeText(), nullable=False)
    #attributes = relationship('Atttribute', secondary='entity_attribute')
    
    # dealing with the physical model
    # logical_id = OneOrMore(self.attributes)
    # table = Column(String(length=255), unique=True, nullable=False)
    # inherit 
    # ordering
    # Would be nice to have 'sealed attributes' those which are not allowed 
    # to change.
    # Abilities ? - these would be groups of columns w/sql functions etc.

class AttributeType(Base):
    __tablename__ = 'attribute_type'
    id = Column(Integer, primary_key=True, autoincrement='auto')
    name = Column(String(length=255), unique=True, nullable=False)
    description = Column(UnicodeText())
    #pg_type = Column(Integer, ForeignKey("pg_catalog.pg_type.oid")) # ? 

# This abstraction whilst a PITA to work with allows for the concept of 
# interfaces fairly easy also it promotes normalization to some degree.
class Attribute(TemporalMixin, VersionedMixin, Base):
    __tablename__ = 'attribute'
    id = Column(Integer, primary_key=True, autoincrement='auto')
    attribute_id = Column(Integer)
    entity_id = Column(Integer, ForeignKey(Entity.id), nullable=False)
    name = Column(String(length=255), nullable=False)
    description = Column(UnicodeText())
    type_id = Column(Integer, ForeignKey(AttributeType.id), nullable=False)
    # TODO: deal with the physical model
    # type-args 
    # required (NULL/NOT NULL)
    # unique constraints (etc.)

# Immutable M2M's
# Need to store a valid time-span so that we know what items are valid for
# what other items (when).. So asking what 'requires/implies' when with all
# things temporal.
#class EntityAttribute(TemporalMixin, VersionedMixin, Base):
    #__tablename__ = 'entity_attribute'
    #id = Column(Integer, primary_key=True, autoincrement='auto')
    #entity_id = Column(Integer, ForeignKey(Entity.id), nullable=False)
    #attribute_id = Column(Integer, ForeignKey(Attribute.id), nullable=False)
    #Column('order', Integer) # hidden but used for column order

#class EntityConstraint(TemporalMixin, HistoryMixin, VersionedMixin):
    #pass

# Entity dependent VCS
#class Commit(TemporalMixin, HistoryMixin, Base):
    #__tablename__ = 'commit'

#class CommitEntity(TemporalMixin, HistoryMixin, Base):
    #__tablename__ = 'commit_entity'

#class Merge(TemporalMixin, HistoryMixin, Base):
    #__tablename__ = 'merge'

#class MergeConflict(TemporalMixin, HistoryMixin, Base):
    #__tablename__ = 'merge_conflict'

#class MergeConflictResolution(TemporalMixin, HistoryMixin, Base):
    #__tablename__ = 'merge_conflict_resolution'

## Move a commit `id` to walden_publish from walden_data
#class Publish(TemporalMixin, HistoryMixin, Base):
    #__tablename__ = 'publish'

#class PublishLog(TemporalMixin, HistoryMixin, Base):
    #__tablename__ = 'publish_log'


#
# Base Entity Types
# Question: Do you make the core Page/Routing types Entities or concrete ?
# Ya want to make um concrete @ first but maybe its better that they are
# Entities then if i need to glob attributes on, i'm not forced to do so
# via raw sql migrations. also could support concurrent versions as far as 
# dev_master/branch using something other than _published.

# Page Designer
#class Template(Base):pass
#class Asset(Base):pass
#class AssetType(Base):pass
#class Widget(Base):pass
#widget_on_page = Table()
#class Query(Base):pass
#class BoundQuery(Base):pass

#class Route(Base):pass
#class Page(Base):pass




#op.execute("CREATE SCHEMA walden")
#op.execute("CREATE SCHEMA walden_history")
#op.execute("CREATE SCHEMA walden_published")
#bind = op.get_bind()
#session = Session(bind=bind)
#>>> from sqlalchemy.dialects import postgresql
#>>> print str(q.statement.compile(dialect=postgresql.dialect()))