# isosec programming test


## Contents ##

1. Install
   1. Database
   1. Server Component
   1. Desktop Client
1. Setup & Configuration
1. User Guide
   1. REST API format
1. Design descisions and known problems
1. Todo

## Install ##

First, check the code from github:

    git clone https://github.com/MatTaylorSharrow/isosec.git

### Database ###

The database can be installed in one go using the help script db/install-mysql.sql .  The script simply concatenates the required .sql files and pipes the output to the local mysql server.  If you want to send to a remote server, simply edit this file with the database connection details.

```bash
cd isosec/db
install-mysql.sh
```

### Server Component ###

php
apache
mysql

### Desktop Client ###

Installing libraries
building
Install


## Setup & Configuration ##
config files
db server events turned on

### REST API format ###

```json
{
	"customer": "BMW",
	"product": "KPI", 
	"event_timestamp": "2020-10-23 10:09:23", 
	"device_id": "61c00a83-3058-4f43-a8b7-3fe97d2a649f"
}
```

## User Guide ##


## Design descisions and known problems ##


## Todo ##

Server:
- [ ] Move html to template file
- [ ] Move to individual files 
- [ ] Record device_ids in seperate table and link to audit log to reduce size
- [ ] Update View to select from backed up audit log tables

Client:
- [ ] Read config values from file
- [ ] Exception catching
- [ ] Move json object creation to seperate method
- [ ] Change to use Qt UUID
- [ ] Change to use Qt NetworkManager
- [ ] Remove debug
- [ ] Exit action
