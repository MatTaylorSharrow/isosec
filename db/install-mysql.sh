#!/bin/bash

cat createschema.mysql.sql schema.mysql.sql privilleges.mysql.sql seed-data.sql | mysql -uroot -p
