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


To build the desktop client, Qt and Boost Libraries will be required.  I've currently only built against my system installed libraries.  The Desktop Client builds against Boost >= 1.66 and Qt >= 5.15.0.

- Installing libraries

First download and install the libraries for your operating system:
 Qt - https://doc.qt.io/qt-5/gettingstarted.html
 Boost - https://www.boost.org/doc/libs/1_66_0/more/getting_started/
 
If you're on linux you could use your package manager to install the -devel packages for the two libraries.  


- Building

The client app uses Cmake to generate a build for your build system.  As I'm on linux and using gcc/autotool I follow the following procedure:

From the root of the checked out sources:

```bash
cd isosec/client/AuditClientCPP/build
cmake ../
make
```

This will create a binary file in the build current build directory so all you should need to execute it is run it

```bash
./auditclientcpp
```

- Install

To install using the build system simply type:

```bash
make install
```

On linux this will copy the binary to  /usr/local/bin/


## Setup & Configuration ##

The server component contains a config file which contains the database connection details.  The file resides in the isosec/conf folder and is called conf/server-app.ini . The user details were created when the database was created / installed.  All you should need to do is change the host if you are not connecting to the local database server.

The desktop client has hardcoded values for connecting to the web service.  If you need to change these then edit the below file and rebuild the client app

    client/AuditClientCPP/src/app.h



The MySQL must have the events scheduler turned on.  We attempt to do this during install but you may also need to edit my.cnf (my.ini on windows) and set the config value to:

    event_scheduler=ENABLED

See https://dev.mysql.com/doc/refman/8.0/en/events-configuration.html for further details



### REST API format ###

The rest API only supports two requests, 1, The Help / Usage request which is a

Method | Request Path | Success Result Code | Success response body type | Error result code | Error response body type
------------ | ------------- | ------------- | ------------- | ------------- | -------------
GET | / | 200 | html |  ?? | ??
POST | /AuditLog | 204 | No Body |  400 | json - @see example below

All json requests and responses use "application/json" as the content-type

/AuditLog Request Body example
```json
{
    "customer": "BMW",
    "product": "KPI", 
    "event_timestamp": "2020-10-23 10:09:23", 
    "device_id": "61c00a83-3058-4f43-a8b7-3fe97d2a649f"
}
```

/AuditLog Failure response bodies
```json
{
    "database_errors":{
       "query":"Execution of the query failed: DB generated message"
    }
}

{
    "validation_errors":{
        "product":[
	    "Does not match the criteria for valid input."
	]
    }
}

{
    "validation_errors":{
        "customer":[
	    "Does not match the criteria for valid input."
	]
    }
}
```


## User Guide ##

Once the server component is set up and running the client Application can be launched.  

You will be presented with a TextArea with two buttons "Simulate (Stampede)" and "Simulate (Queue)".  

Pressing "Simulate (Stampede)" will start sending 1000 audit log messages to the web server.  Error messages will be displayed in the text area. After the messages have been sent, a message will tell the user how many were sent successfully and how many failed.  You'll also be informed of how long it took to send the messages.  Pressing the button again will send more audit log messges to the server.  Again the user will be advised of failures.

Pressing "Simulate (Queue)" will do nothing other than display a message in the text area. It was intended to do some fancy things will the outgoing messages but this has not been implemented. 

To close the application use the OS window's close (x) icon in the top corner of the window.


## Design descisions and known problems ##

Design decisions
- Rest API
- Database design, foreign keys, UUID
- Random creation of messages to send 
- UUID generation
- HTTP Client
- Input validation
- DB Queries
	- prepare statement to avoid sql injection
	- privileges for api user
- Audit Log Truncate / Backup
	

Know Problems
- boost libraries used for sockets and uuid when existing Qt libraries would have done, this causes extra complications for distributing and building the software.  I simply neglected to search for Qt versions of classes required to do the job.
- char encoding - html, php, apache vhost, mysql database/tables/connections and client app should all communicate in utf8.  Nothing has been specified in any location so if multi byte characters were to be used, there will probably be char encoding problems observed.
- exception handling is basically missing.  Exception handing is required around the boost library calls and possibly the whole app.
- Line ending / carriage returns
- Config values hardcoded.  Just ran out of time.
- Unit tests

Comments
- Ugh Qt Json response object processing !!!!
- Connection re-use (web sockets perhaps)




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
