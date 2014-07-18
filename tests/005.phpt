--TEST--
INI settings (epoch)
--INI--
snowflake.node_id=1
snowflake.epoch = 1402160300000
--FILE--
<?php 
$id = snowflake_next_id();
$def = snowflake_explain($id);
var_dump($def["timestamp"] < time() + 1);
?>
--EXPECT--
bool(true)
