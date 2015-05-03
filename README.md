Nginx Zookeeper Module
----

Add zookeeper support for Nginx Server.

Requirements
====

* Zookeeper C Client

Install
====

    $ ./configure --add-module=/path/to/nginx-zookeeper
    $ make
    # make install

Configuration
====

* zookeeper_host

    CSV list of host:port values.

* zookeeper_path

    Which path will module create when Nginx server starts.

Examples
====

    zookeeper_host "192.168.0.2:2181,192.168.0.3:2181"
    zookeeper_path "/nginx/foo"

