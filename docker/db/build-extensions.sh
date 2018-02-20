#!/bin/bash
cd /walden/contrib

echo "user: $(whoami)"

for d in $(ls .); do 
    cd $d 
    echo "building $i"
    make install 
    cd - 
done

cd /tmp
git clone https://github.com/arkhipov/temporal_tables.git
cd temporal_tables
make install

