--TEST--
INI settings (epoch range)
--INI--
snowflake.epoch = -1
snowflake.node_id = 1 
display_errors = Off
--FILE--
--EXPECTF--
PHP Warning:  PHP Startup: "snowflake.epoch" must be set greater than zero in %s on line %d
PHP Fatal error:  Unable to start snowflake module in %s on line %d
