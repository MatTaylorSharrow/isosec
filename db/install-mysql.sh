#!/bin/bash

cat createschema.mysql.sql schema.mysql.sql privilleges.mysql.sql | mysql -uroot -p
