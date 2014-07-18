Welcome to use php-snowflake
----------------------------

**php-snowflake** is an ID Generator which implemented as PHP Extension, based on [Snowflake Algorithm _(Twitter announced)_](https://blog.twitter.com/2010/announcing-snowflake).

Installation
------------

```
$ phpize
$ ./configure
$ make
$ make test
$ make install
```

add to php.ini
```ini
extension="snowflake.so"
snowflake.node_id = 1
snowflake.epoch = 1402160300000
```

Functions list
--------------
1. snowflake_next_id()
2. snowflake_explain()

TODO list
---------
1. support 32bit environment
