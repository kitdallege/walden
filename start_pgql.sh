#!/bin/bash 
cd /home/kit/.nvm/versions/node/v8.9.3/bin/
postgraphile -c postgresql://kit:@localhost/walden -s walden_master -w -o -n 0.0.0.0 

