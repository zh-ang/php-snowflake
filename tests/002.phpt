--TEST--
Check for snowflake_next_id() generating number
--SKIPIF--
<?php if (!extension_loaded("snowflake")) print "skip"; ?>
--FILE--
<?php 
var_dump(snowflake_next_id());
?>
--EXPECTF--
int(%d)
