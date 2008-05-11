<?php
if( $_SERVER["HTTP_USER_AGENT"] != "TA3D" )	exit();			// Only accepts requests from TA3D clients

$db = mysql_connect('host', 'login', 'password');		// Connection to MySQL
mysql_select_db('database',$db);

$time = time();
mysql_query("DELETE FROM `serverlist` WHERE serverlist.`time` <= " . ($time - 160) );		// Stay 160 sec

$req = "SELECT * FROM serverlist";
$result = mysql_query( $req );
echo mysql_num_rows( $result ) . " servers\n";
$i = 0;
while( $data = mysql_fetch_assoc( $result ) ) {
	echo $i . " name: " . $data["name"] . "\n";
	echo $i . " IP: " . $data["IP"] . "\n";
	echo $i . " mod: " . $data["mod"] . "\n";
	echo $i . " version: " . $data["version"] . "\n";
	echo $i . " slots: " . $data["slots"] . "\n";
	$i++;
	}
?>
