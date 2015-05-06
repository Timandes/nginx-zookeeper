Nginx Zookeeper Module
----

[![Build Status](https://api.travis-ci.org/Timandes/nginx-zookeeper.svg?branch=master)](https://travis-ci.org/Timandes/nginx-zookeeper)

Add zookeeper support for Nginx Server.

Requirements
====

* Zookeeper C Client

Install
====

    export LIBZOOKEEPER_PREFIX=/path/to/libzookeeper
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

