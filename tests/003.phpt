--TEST--
Check for snowflake_next_id() generating different number
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
