<?php
if( $_SERVER["HTTP_USER_AGENT"] != "TA3D" )	exit();			// Only accepts requests from TA3D clients

$db = mysql_connect('host', 'login', 'password');		// Connection to MySQL
mysql_select_db('database',$db);

$address = $_SERVER["REMOTE_ADDR"];
$name = $_GET["name"];
$mod = $_GET["mod"];
$version = $_GET["version"];
$slots = $_GET["slots"];

$time = time();

mysql_query("DELETE FROM `serverlist` WHERE serverlist.`time` <= " . ($time - 160) );		// Stay 160 sec

mysql_query("DELETE FROM serverlist WHERE IP='$address'");

if( !empty( $name ) && !empty( $version ) && !empty( $slots ) )				// When slots is 0, there is no reason to store it in the database
	mysql_query("INSERT INTO serverlist (`name`, `IP`, `time`, `mod`, `version`, `slots`) VALUES( '$name', '$address', '$time', '$mod', '$version', '$slots' )");
?>
