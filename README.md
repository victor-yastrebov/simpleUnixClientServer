# SimpleUnixClientServer

Simple client - server application with communications via Unix Domain Sockets.
Server side application is running as daemon, holds database and can process client queries.

The following queries are supported:
```
  put <key> <value> : insert key/value pair into db
  get <key>         : select value associated with key
  erase [key]       : delete key/value pair
  list [prefix]     : select all keys starting with prefix if defined
```

## How to compile
Need C++ compiler that supports C++11/14/17 standarts and `CMake` for generation `make` files.
Compilaton is tested on CentOS 7.4 with `g++ 9.2.0` and `CMake 3.6.3`.
Run `./compileGCC.sh` for project compilation. Result artifacts will in `build` folder.

## Usage example
```
 $ #start of server
 $ ./kvd
 $ ./kvctl get foo
 kvctl: no key "foo"
 $ echo $?
 1
 $ ./kvctl put foo 123
 $ ./kvctl get foo
 123
 $ ./kvctl put bar 321
 $ ./kvctl put baz 312
 $ ./kvctl list
 foo
 bar
 baz
 $ ./kvctl list ba
 bar
 baz
 $ ./kvctl erase foo
 $ ./kvctl list
 bar
 baz
 $ ./kvctl erase
 $ ./kvctl list | wc -l
 0
```
