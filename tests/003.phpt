--TEST--
Check for snowflake_next_id() generating different number
--INI--
snowflake.node_id=1
snowflake.epoch=0
--SKIPIF--
<?php if (!extension_loaded("snowflake")) print "skip"; ?>
--FILE--
<?php 
$a = snowflake_next_id();
$b = snowflake_next_id();
var_dump($a < $b);
?>
--EXPECT--
bool(true)
