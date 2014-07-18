--TEST--
INI settings (epoch)
--INI--
snowflake.epoch = 2147483647000
snowflake.node_id = 1
display_errors = Off
--FILE--
--EXPECTF--
PHP Warning:  PHP Startup: "snowflake.epoch" must be set earlier before now in %s on line %d
PHP Fatal error:  Unable to start snowflake module in %s on line %d
