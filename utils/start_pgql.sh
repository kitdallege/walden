#!/bin/bash
cd /home/kit/.nvm/versions/node/v8.9.3/bin/
postgraphile -c postgresql://walden:walden@localhost/walden -w -o -n 0.0.0.0 -s walden,walden_history
