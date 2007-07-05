<?php
	$db_host = 'localhost';
	$db_user = 'root';
	$db_password = 'root';
	$database = 'lofar-craft';
	mysql_connect($db_host, $db_user, $db_password);
	mysql_select_db($database);
?>