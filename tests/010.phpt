--TEST--
INI settings (epoch)
--INI--
snowflake.epoch = 1402160300000
snowflake.node_id = 1
--FILE--
<?php
$id = snowflake_next_id();
$old_id = file_get_contents("snowflake_id.swp");
unlink("snowflake_id.swp");
var_dump($id < $old_id);
--EXPECT--
bool(true)
