# Isosec Programming Test


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

The server component only requires the files in the  server/ and conf/ directories. The server/index.php files reads its conf file from conf/server-app.ini so the directory structure should be preserved. If you are hosting the server comonent on the server where the code is checked out to, simply setup a web server vhost to point to the server directory.  Otherwise if you are hosting the server on a remote server, upload both the server/ and conf/ directories together to the same target, then continue to set up your remote web server vhost.

Set up the virtual host as api.isosec.com (as this is hardcoded into the desktop client).  You can make entries in your /etc/hosts file (or C:\Windows\System32\drivers\etc\hosts on windows) eg:

    127.0.0.1       api.isosec.com

Once the host file entry is set up you can point a web browser at http://api.isosec.com and you should see a simple page describing the Rest API.

An example virtual host file for apache is below.  Ensure the DocuementRoot and Directory directives are the same path, ie the path to the server component code.

Also important to note is the use of the FallbackResource directive.  This is used to ensure all web requests are directed to index.php.  If your web server does not support this directive or something similar, you may need to use the RewriteEngine to perform the same operation.

```httpd.conf
<VirtualHost *:80>
    ServerName api.isosec.com
    DocumentRoot /path/to/checked/out/code/for/isosec/server

    ServerAdmin webmaster@api.isosec.com
    # if not specified, the global error log is used
    ErrorLog /var/log/apache2/api.isosec.com-error_log
    CustomLog /var/log/apache2/api.isosec.com-access_log combined
    # don't loose time with IP address lookups
    HostnameLookups Off
    # needed for named virtual hosts
    UseCanonicalName Off
    ServerSignature Off
    DirectoryIndex index.php
    
    <Directory "/path/to/checked/out/code/for/isosec/server">
        Options Indexes FollowSymLinks
        AllowOverride None

        <IfModule !mod_access_compat.c>
            Require all granted
        </IfModule>
        <IfModule mod_access_compat.c>
            Order allow,deny
            Allow from all
        </IfModule>

        FallbackResource "index.php"
#       RewriteEngine  on
#       RewriteRule    "^(.*)$"  "/index.php?action=$1" [PT]
    </Directory>
</VirtualHost>
```

The server component was written using PHP 7.2.5 so any later version will work fine.  Mysqli must be available, it's usually built in by default. 

You must ensure that MySQL is running the event scheduler.  This is done via db install however.  If you need to turn it on manually than run the following command from a mysql root terminal: 

    SET GLOBAL event_scheduler = ON;


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
