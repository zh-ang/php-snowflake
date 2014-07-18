--TEST--
Check for snowflake_explain()
--INI--
snowflake.node_id=1
snowflake.epoch=0
--SKIPIF--
<?php if (!extension_loaded("snowflake")) print "skip"; ?>
--FILE--
<?php 
$id = snowflake_next_id();
$def = snowflake_explain($id);
var_dump($def);
var_dump($def["timestamp"] < time() + 1);
?>
--EXPECTF--
array(3) {
  ["timestamp"]=>
  float(%f)
  ["node_id"]=>
  int(%d)
  ["sequence"]=>
  int(%d)
}
bool(true)
