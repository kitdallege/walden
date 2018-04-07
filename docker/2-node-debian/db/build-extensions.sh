#!/bin/bash
# Install's walden extensions as well as thirdpary deps.
INSTALL_DIR=/installs
WALDEN_CONTRIB_DIR=/walden/contrib

cd $WALDEN_CONTRIB_DIR
echo "user: $(whoami)"
for d in $(ls .); do 
    cd $d 
    echo "building $i"
    make install 
    cd - 
done

mkdir -p $INSTALL_DIR

# temporal tables
cd $INSTALL_DIR
git clone https://github.com/arkhipov/temporal_tables.git
cd temporal_tables
make
make install

# Pg file io. 
cd $INSTALL_DIR
git clone https://github.com/csimsek/pgsql-fio.git
cd pgsql-fio
make
make install

# TODO: Checkout https://github.com/petere/postgresqlfs
