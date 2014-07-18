--TEST--
Check for snowflake_explain()
--SKIPIF--
<?php if (!extension_loaded("snowflake")) print "skip"; ?>
--FILE--
<?php 
$id = snowflake_next_id();
var_dump(snowflake_explain($id));
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
