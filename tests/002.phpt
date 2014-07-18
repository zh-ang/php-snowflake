--TEST--
Check for snowflake_next_id() generating number
--INI--
snowflake.node_id=1
snowflake.epoch=0
--SKIPIF--
<?php if (!extension_loaded("snowflake")) print "skip"; ?>
--FILE--
<?php 
$id = snowflake_next_id();
file_put_contents("snowflake_id.swp", $id);
var_dump($id);
?>
--EXPECTF--
int(%d)
