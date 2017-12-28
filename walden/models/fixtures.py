from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

from walden.models import core




class Fixtures(object):
    def __init__(self, session):
        self.session = session
        self.master = None
    
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
        self.master = self.get_or_create(core.Branch, **dict(
            name="master",
            id=1,
            parent_id=0,
            description=u"The mainline branch. Everything depends on this guy."
        ))
        # Create 'walden' application / entity namespace.
        self.walden_app = self.get_or_create(core.Application, **dict(
            branch_id=self.master.id,
            id=1,
            parent_id=0,
            name="walden",
            description=u"Contains all the 'builtins' from walden."
        ))
        # Add AttributeType(s)
        try:
            self.session.add_all(self.get_attributes_types())
            self.session.add_all(self.get_pg_attributes_types())
            self.session.commit()
        except Exception as err:
            self.session.rollback()
            raise err
        # 
        # Create some entities.
        self.add_entities()
        # Commit changes to db.
        self.session.commit()
    
    def attribute_type_id(self, name):
        return self.session.query(core.AttributeType)\
               .filter(core.AttributeType.name == name)\
               .one().id
    
    def pg_attribute_type_id(self, name):
        return self.session.query(core.PgType)\
               .filter(core.PgType.name == name)\
               .one().id
    
    def application_type_id(self, name):
        return self.session.query(core.Application)\
               .filter(core.Application.name == name)\
               .one().id
    
    @staticmethod
    def get_attributes_types():
        return [
            core.AttributeType(
                name="String", description=u"",
                branch_id=1, parent_id=0
            ),
            core.AttributeType(
                name="Number", description=u"",
                branch_id=1, parent_id=0
            ),
            core.AttributeType(
                name="Boolean", description=u"",
                branch_id=1, parent_id=0
            ),
            core.AttributeType(
                name="List", description=u"",
                branch_id=1, parent_id=0
            ),
            core.AttributeType(
                name="Object", description=u"",
                branch_id=1, parent_id=0
            ),
            core.AttributeType(
                name="Enum", description=u"",
                branch_id=1, parent_id=0
            ),
            core.AttributeType(
                name="Computed", description=u"",
                branch_id=1, parent_id=0
            )
        ]
    
    @staticmethod
    def get_pg_attributes_types():
        pg_types = [
            core.PgType(name="abstime", description=u"absolute, limited-range date and time (Unix system time)"),
            core.PgType(name="aclitem", description=u"access control list"),
            core.PgType(name="any", description=u""),
            core.PgType(name="anyarray", description=u""),
            core.PgType(name="anyelement", description=u""),
            core.PgType(name="anyenum", description=u""),
            core.PgType(name="anynonarray", description=u""),
            core.PgType(name="anyrange", description=u""),
            core.PgType(name="bigint", description=u"~18 digit integer, 8-byte storage"),
            core.PgType(name="bit", description=u"fixed-length bit string"),
            core.PgType(name="bit varying", description=u"variable-length bit string"),
            core.PgType(name="boolean", description=u"boolean, 'true'/'false'"),
            core.PgType(name="box", description=u"geometric box '(lower left,upper right)'"),
            core.PgType(name="bytea", description=u"variable-length string, binary values escaped"),
            core.PgType(name="char", description=u"single character"),
            core.PgType(name="character", description=u"char(length), blank-padded string, fixed storage length"),
            core.PgType(name="character varying", description=u"varchar(length), non-blank-padded string, variable storage length"),
            core.PgType(name="cid", description=u"command identifier type, sequence in transaction id"),
            core.PgType(name="cidr", description=u"network IP address/netmask, network address"),
            core.PgType(name="circle", description=u"geometric circle '(center,radius)'"),
            core.PgType(name="cstring", description=u""),
            core.PgType(name="date", description=u"date"),
            core.PgType(name="daterange", description=u"range of dates"),
            core.PgType(name="double precision", description=u"double-precision floating point number, 8-byte storage"),
            core.PgType(name="event_trigger", description=u""),
            core.PgType(name="fdw_handler", description=u""),
            core.PgType(name="gtsvector", description=u"GiST index internal text representation for text search"),
            core.PgType(name="index_am_handler", description=u""),
            core.PgType(name="inet", description=u"IP address/netmask, host address, netmask optional"),
            core.PgType(name="int2vector", description=u"array of int2, used in system tables"),
            core.PgType(name="int4range", description=u"range of integers"),
            core.PgType(name="int8range", description=u"range of bigints"),
            core.PgType(name="integer", description=u"-2 billion to 2 billion integer, 4-byte storage"),
            core.PgType(name="internal", description=u""),
            core.PgType(name="interval", description=u"@ <number> <units>, time interval"),
            core.PgType(name="json", description=u""),
            core.PgType(name="jsonb", description=u"Binary JSON"),
            core.PgType(name="language_handler", description=u""),
            core.PgType(name="line", description=u"geometric line"),
            core.PgType(name="lseg", description=u"geometric line segment '(pt1,pt2)'"),
            core.PgType(name="macaddr", description=u"XX:XX:XX:XX:XX:XX, MAC address"),
            core.PgType(name="money", description=u"monetary amounts, $d,ddd.cc"),
            core.PgType(name="name", description=u"63-byte type for storing system identifiers"),
            core.PgType(name="numeric", description=u"numeric(precision, decimal), arbitrary precision number"),
            core.PgType(name="numrange", description=u"range of numerics"),
            core.PgType(name="oid", description=u"object identifier(oid), maximum 4 billion"),
            core.PgType(name="oidvector", description=u"array of oids, used in system tables"),
            core.PgType(name="opaque", description=u""),
            core.PgType(name="path", description=u"geometric path '(pt1,...)'"),
            core.PgType(name="pg_ddl_command", description=u"internal type for passing CollectedCommand"),
            core.PgType(name="pg_lsn", description=u"PostgreSQL LSN datatype"),
            core.PgType(name="pg_node_tree", description=u"string representing an internal node tree"),
            core.PgType(name="point", description=u"geometric point '(x, y)'"),
            core.PgType(name="polygon", description=u"geometric polygon '(pt1,...)'"),
            core.PgType(name="real", description=u"single-precision floating point number, 4-byte storage"),
            core.PgType(name="record", description=u""),
            core.PgType(name="refcursor", description=u"reference to cursor (portal name)"),
            core.PgType(name="regclass", description=u"registered class"),
            core.PgType(name="regconfig", description=u"registered text search configuration"),
            core.PgType(name="regdictionary", description=u"registered text search dictionary"),
            core.PgType(name="regnamespace", description=u"registered namespace"),
            core.PgType(name="regoper", description=u"registered operator"),
            core.PgType(name="regoperator", description=u"registered operator (with args)"),
            core.PgType(name="regproc", description=u"registered procedure"),
            core.PgType(name="regprocedure", description=u"registered procedure (with args)"),
            core.PgType(name="regrole", description=u"registered role"),
            core.PgType(name="regtype", description=u"registered type"),
            core.PgType(name="reltime", description=u"relative, limited-range time interval (Unix delta time)"),
            core.PgType(name="smallint", description=u"-32 thousand to 32 thousand, 2-byte storage"),
            core.PgType(name="smgr", description=u"storage manager"),
            core.PgType(name="text", description=u"variable-length string, no limit specified"),
            core.PgType(name="tid", description=u"(block, offset), physical location of tuple"),
            core.PgType(name="timestamp without time zone", description=u"date and time"),
            core.PgType(name="timestamp with time zone", description=u"date and time with time zone"),
            core.PgType(name="time without time zone", description=u"time of day"),
            core.PgType(name="time with time zone", description=u"time of day with time zone"),
            core.PgType(name="tinterval", description=u"(abstime,abstime), time interval"),
            core.PgType(name="trigger", description=u""),
            core.PgType(name="tsm_handler", description=u""),
            core.PgType(name="tsquery", description=u"query representation for text search"),
            core.PgType(name="tsrange", description=u"range of timestamps without time zone"),
            core.PgType(name="tstzrange", description=u"range of timestamps with time zone"),
            core.PgType(name="tsvector", description=u"text representation for text search"),
            core.PgType(name="txid_snapshot", description=u"txid snapshot"),
            core.PgType(name="unknown", description=u""),
            core.PgType(name="uuid", description=u"UUID datatype"),
            core.PgType(name="void", description=u""),
            core.PgType(name="xid", description=u"transaction id"),
            core.PgType(name="xml", description=u"XML content")
        ]
        for ptype in pg_types:
            ptype.parent_id = 0
            ptype.branch_id = 1
        return pg_types
    
    def add_entities(self):
        # Add Entities & associate attributes with them.
        User = self.get_or_create(core.Entity, **dict(
            id=1,
            parent_id=0,
            branch_id=self.master.id,
            application_id=self.application_type_id('walden'),
            name="User",
            description=u"A user within the walden system."
        ))
        self.session.add(User)
        self.session.commit()
        attrs_for_user = [
            self.get_or_create(core.Attribute, **dict(
                id=1, # TODO: attribute_id should be query based max(attribute_id) + 1
                parent_id=0,
                branch_id=self.master.id,
                name="username",
                description=u"Username. alphanumeric string with no spaces.",
                type_id=self.attribute_type_id('String'),
            )),
            self.get_or_create(core.Attribute, **dict(
                #entity_id=User.id,
                id=2, # TODO: attribute_id should be query based max(attribute_id) + 1
                parent_id=0,
                branch_id=self.master.id,
                name="first_name",
                description=u"Persons first name.",
                type_id=self.attribute_type_id('String'),
            )),
            self.get_or_create(core.Attribute, **dict(
                #entity_id=User.id,
                id=3, # TODO: attribute_id should be query based max(attribute_id) + 1
                parent_id=0,
                branch_id=self.master.id,
                name="last_name",
                description=u"Persons last name.",
                type_id=self.attribute_type_id('String'),
            )),
            self.get_or_create(core.Attribute, **dict(
                #entity_id=User.id,
                id=4, # TODO: attribute_id should be query based max(attribute_id) + 1
                parent_id=0,
                branch_id=self.master.id,
                name="email",
                description=u"Email address.",
                type_id=self.attribute_type_id('String'),
            )),
            self.get_or_create(core.Attribute, **dict(
                #entity_id=User.id,
                id=5, # TODO: attribute_id should be query based max(attribute_id) + 1
                parent_id=0,
                branch_id=self.master.id,
                name="password",
                description=u"Super spy password.",
                type_id=self.attribute_type_id('String'),
            )),
        ]
        # create attrs.
        self.session.add_all(attrs_for_user)
        # tie atttrs to entity instance.
        for idx, attr in enumerate(attrs_for_user):
            self.get_or_create(core.EntityAttribute, **dict(
                id=idx,
                entity_id=User.id,
                attribute_id=attr.id,
                branch_id=self.master.id,
                parent_id=0,
                pgtype_id=self.pg_attribute_type_id('character varying')
            ))
        


if __name__ == '__main__':
    engine = create_engine('postgresql://walden:walden@localhost/walden')
    Session = sessionmaker(bind=engine)
    session = Session()
    fixtures = Fixtures(session)
    fixtures.run()
    session.close()
