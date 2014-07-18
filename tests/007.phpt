--TEST--
INI settings (node_id range)
--INI--
snowflake.epoch = 0
snowflake.node_id = 99999
display_errors = Off
--FILE--
--EXPECTF--
PHP Warning:  PHP Startup: "snowflake.node_id" must be set smaller than %d in %s on line %d
PHP Fatal error:  Unable to start snowflake module in %s on line %d
