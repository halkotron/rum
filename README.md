# rum

## What is rum
* event-based one process tcp redirector with socket file support and more listen ports/socket files, using libuv
* mysql reverse proxy for more mysql servers (the key to select destination server is username send by client)

## Database Support
* MySQL / MariaDB
* PostgreSQL 

## Requirments
* libcdb
* libuv

## Compilation
```
apt-get install libcdb-dev libuv1-dev
make
```

## Supported
 * Tested & Developed on: Ubuntu 14.04-18.04

## Usage
```
./rum -s tcp:host:port [-s tcp:host:port [-s sock:path]] -d tcp:host:port [-b] [-m tcp:host:port] [-M /path/to/mysql.cdb]
    -s - listen host:port or sock:path (host must be some ip address or 0.0.0.0 for all inerfaces)
    -d - destination host:port

    optional:
    -b - goto background
    -m - statistics port (example: tcp:localhost:510)
    -M - enable handling of mysql connection with more destination servers, argument is path to cdb file (-M /etc/rum/mysql.cdb)
```

## Examples as simple tcp redirector

* redirect port 110 to pop3.example.net:110

  `rum -s tcp:0.0.0.0:110 -d tcp:pop3.example.net:110 -b`

* redirect port 3306 and /var/run/mysqld/mysqld.sock to destination server in mysql.cdb

  `rum -s tcp:localhost:3306 -s sock:/var/run/mysqld/mysqld.sock -d tcp:1.2.3.4:3306 -b`


  As you see when you use cdb file you must also define one "default" destination server. From this server we store and re-use initial packet
  from MySQL server and send it to client.

## Dns lookups
hostname -> ip resolving is done only once at start, it is not refreshed.

## MySQL/PostgreSQL reverse proxy feature
![alt text](https://github.com/websupport-sk/rum/blob/master/images/rum.png?raw=true "this is how it works")
* this feature is enabled when -M /path/to/cdb is used
* cdb database is used for searching destination server from username send by client
* hashed user passwords from every mysql/posgresql server must be stored in cdb database to successfuly create connections - -d must be also used as default destination server (when user is not found in cdb database rum connect to this server, but auth always fails, we need to know user password)
* in cdb file the format is:
    key -> value
    username -> password\0dst_server\0

Why is user password needed? there is explanation:

[http://forge.mysql.com/wiki/MySQL_Internals_ClientServer_Protocol#4.1_and_later]

## Creating cdb database for proxy
* contrib/export_mysql_cdb.pl is perl script which can be run every 1 minute or so. cdb file is automatically regenerated from mysql server list and rum doesn't need a restart.
* contrib/export_postgresql_cdb.pl	the same script, but for postgresql
