from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

from walden.models import core




class Fixtures(object):
    def __init__(self, session):
        self.session = session
    
    def get_or_create(self, model, **kwargs):
        '''
        Creates an object or returns the object if exists
        credit to Kevin @ StackOverflow
        from: http://stackoverflow.com/questions/2546207/does-sqlalchemy-have-an-equivalent-of-djangos-get-or-create
        '''
        instance = self.session.query(model).filter_by(**kwargs).first()
    
        if not instance:
            instance = model(**kwargs)
            self.session.add(instance)
            self.session.commit()
        return instance
    
    def run(self):
        # Create master branch
        master = self.get_or_create(core.Branch, **dict(
            branch_id=1, name="master",
            description=u"The mainline branch. Everything depends on this guy."
        ))
        # Create 'walden' application / entity namespace.
        walden_app = self.get_or_create(core.Application, **dict(
            branch_id=master.id,
            application_id=1,
            name="walden",
            description=u"Contains all the 'builtins' from walden."
        ))
        # Add AttributeType(s)
        try:
            self.session.add_all(self.get_attributes_types())
            self.session.commit()
        except:
            self.session.rollback()
        # Create some entities.
        self.add_entities()
        # Commit changes to db.
        self.session.commit()
    
    def attribute_type_id(self, name):
        return self.session.query(core.AttributeType)\
               .filter(core.AttributeType.name == name)\
               .one()
    
    def application_type_id(self, name):
        return self.session.query(core.Application)\
               .filter(core.Application.name == name)\
               .one()
    
    @staticmethod
    def get_attributes_types():
        return [
            core.AttributeType(name="abstime", description=u"absolute, limited-range date and time (Unix system time)"),
            core.AttributeType(name="aclitem", description=u"access control list"),
            core.AttributeType(name="any", description=u""),
            core.AttributeType(name="anyarray", description=u""),
            core.AttributeType(name="anyelement", description=u""),
            core.AttributeType(name="anyenum", description=u""),
            core.AttributeType(name="anynonarray", description=u""),
            core.AttributeType(name="anyrange", description=u""),
            core.AttributeType(name="bigint", description=u"~18 digit integer, 8-byte storage"),
            core.AttributeType(name="bit", description=u"fixed-length bit string"),
            core.AttributeType(name="bit varying", description=u"variable-length bit string"),
            core.AttributeType(name="boolean", description=u"boolean, 'true'/'false'"),
            core.AttributeType(name="box", description=u"geometric box '(lower left,upper right)'"),
            core.AttributeType(name="bytea", description=u"variable-length string, binary values escaped"),
            core.AttributeType(name="char", description=u"single character"),
            core.AttributeType(name="character", description=u"char(length), blank-padded string, fixed storage length"),
            core.AttributeType(name="character varying", description=u"varchar(length), non-blank-padded string, variable storage length"),
            core.AttributeType(name="cid", description=u"command identifier type, sequence in transaction id"),
            core.AttributeType(name="cidr", description=u"network IP address/netmask, network address"),
            core.AttributeType(name="circle", description=u"geometric circle '(center,radius)'"),
            core.AttributeType(name="cstring", description=u""),
            core.AttributeType(name="date", description=u"date"),
            core.AttributeType(name="daterange", description=u"range of dates"),
            core.AttributeType(name="double precision", description=u"double-precision floating point number, 8-byte storage"),
            core.AttributeType(name="event_trigger", description=u""),
            core.AttributeType(name="fdw_handler", description=u""),
            core.AttributeType(name="gtsvector", description=u"GiST index internal text representation for text search"),
            core.AttributeType(name="index_am_handler", description=u""),
            core.AttributeType(name="inet", description=u"IP address/netmask, host address, netmask optional"),
            core.AttributeType(name="int2vector", description=u"array of int2, used in system tables"),
            core.AttributeType(name="int4range", description=u"range of integers"),
            core.AttributeType(name="int8range", description=u"range of bigints"),
            core.AttributeType(name="integer", description=u"-2 billion to 2 billion integer, 4-byte storage"),
            core.AttributeType(name="internal", description=u""),
            core.AttributeType(name="interval", description=u"@ <number> <units>, time interval"),
            core.AttributeType(name="json", description=u""),
            core.AttributeType(name="jsonb", description=u"Binary JSON"),
            core.AttributeType(name="language_handler", description=u""),
            core.AttributeType(name="line", description=u"geometric line"),
            core.AttributeType(name="lseg", description=u"geometric line segment '(pt1,pt2)'"),
            core.AttributeType(name="macaddr", description=u"XX:XX:XX:XX:XX:XX, MAC address"),
            core.AttributeType(name="money", description=u"monetary amounts, $d,ddd.cc"),
            core.AttributeType(name="name", description=u"63-byte type for storing system identifiers"),
            core.AttributeType(name="numeric", description=u"numeric(precision, decimal), arbitrary precision number"),
            core.AttributeType(name="numrange", description=u"range of numerics"),
            core.AttributeType(name="oid", description=u"object identifier(oid), maximum 4 billion"),
            core.AttributeType(name="oidvector", description=u"array of oids, used in system tables"),
            core.AttributeType(name="opaque", description=u""),
            core.AttributeType(name="path", description=u"geometric path '(pt1,...)'"),
            core.AttributeType(name="pg_ddl_command", description=u"internal type for passing CollectedCommand"),
            core.AttributeType(name="pg_lsn", description=u"PostgreSQL LSN datatype"),
            core.AttributeType(name="pg_node_tree", description=u"string representing an internal node tree"),
            core.AttributeType(name="point", description=u"geometric point '(x, y)'"),
            core.AttributeType(name="polygon", description=u"geometric polygon '(pt1,...)'"),
            core.AttributeType(name="real", description=u"single-precision floating point number, 4-byte storage"),
            core.AttributeType(name="record", description=u""),
            core.AttributeType(name="refcursor", description=u"reference to cursor (portal name)"),
            core.AttributeType(name="regclass", description=u"registered class"),
            core.AttributeType(name="regconfig", description=u"registered text search configuration"),
            core.AttributeType(name="regdictionary", description=u"registered text search dictionary"),
            core.AttributeType(name="regnamespace", description=u"registered namespace"),
            core.AttributeType(name="regoper", description=u"registered operator"),
            core.AttributeType(name="regoperator", description=u"registered operator (with args)"),
            core.AttributeType(name="regproc", description=u"registered procedure"),
            core.AttributeType(name="regprocedure", description=u"registered procedure (with args)"),
            core.AttributeType(name="regrole", description=u"registered role"),
            core.AttributeType(name="regtype", description=u"registered type"),
            core.AttributeType(name="reltime", description=u"relative, limited-range time interval (Unix delta time)"),
            core.AttributeType(name="smallint", description=u"-32 thousand to 32 thousand, 2-byte storage"),
            core.AttributeType(name="smgr", description=u"storage manager"),
            core.AttributeType(name="text", description=u"variable-length string, no limit specified"),
            core.AttributeType(name="tid", description=u"(block, offset), physical location of tuple"),
            core.AttributeType(name="timestamp without time zone", description=u"date and time"),
            core.AttributeType(name="timestamp with time zone", description=u"date and time with time zone"),
            core.AttributeType(name="time without time zone", description=u"time of day"),
            core.AttributeType(name="time with time zone", description=u"time of day with time zone"),
            core.AttributeType(name="tinterval", description=u"(abstime,abstime), time interval"),
            core.AttributeType(name="trigger", description=u""),
            core.AttributeType(name="tsm_handler", description=u""),
            core.AttributeType(name="tsquery", description=u"query representation for text search"),
            core.AttributeType(name="tsrange", description=u"range of timestamps without time zone"),
            core.AttributeType(name="tstzrange", description=u"range of timestamps with time zone"),
            core.AttributeType(name="tsvector", description=u"text representation for text search"),
            core.AttributeType(name="txid_snapshot", description=u"txid snapshot"),
            core.AttributeType(name="unknown", description=u""),
            core.AttributeType(name="uuid", description=u"UUID datatype"),
            core.AttributeType(name="void", description=u""),
            core.AttributeType(name="xid", description=u"transaction id"),
            core.AttributeType(name="xml", description=u"XML content")
        ]
    
    def add_entities(self):
        # Add Entities & associate attributes with them.
        User = self.get_or_create(core.Entity, **dict(
            entity_id=1,
            application_id=self.application_type_id('walden').id,
            name="User",
            description=u"A user within the walden system."
        ))
        self.session.add(User)
        self.session.commit()
        self.session.add_all([
            self.get_or_create(core.Attribute, **dict(
                entity_id=User.id,
                attribute_id=1, # TODO: attribute_id should be query based max(attribute_id) + 1
                name="username",
                description=u"Username. alphanumeric string with no spaces.",
                type_id=self.attribute_type_id('character varying').id,
            )),
            self.get_or_create(core.Attribute, **dict(
                entity_id=User.id,
                attribute_id=2, # TODO: attribute_id should be query based max(attribute_id) + 1
                name="first_name",
                description=u"Persons first name.",
                type_id=self.attribute_type_id('character varying').id,
            )),            
            self.get_or_create(core.Attribute, **dict(
                entity_id=User.id,
                attribute_id=3, # TODO: attribute_id should be query based max(attribute_id) + 1
                name="last_name",
                description=u"Persons last name.",
                type_id=self.attribute_type_id('character varying').id,
            )),
            self.get_or_create(core.Attribute, **dict(
                entity_id=User.id,
                attribute_id=4, # TODO: attribute_id should be query based max(attribute_id) + 1
                name="email",
                description=u"Email address.",
                type_id=self.attribute_type_id('character varying').id,
            )),
            self.get_or_create(core.Attribute, **dict(
                entity_id=User.id,
                attribute_id=5, # TODO: attribute_id should be query based max(attribute_id) + 1
                name="password",
                description=u"Super spy password.",
                type_id=self.attribute_type_id('character varying').id,
            )),
        ])


if __name__ == '__main__':
    engine = create_engine('postgresql://walden:walden@localhost/walden')
    Session = sessionmaker(bind=engine)
    session = Session()
    fixtures = Fixtures(session)
    fixtures.run()
    session.close()
