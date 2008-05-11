<?php
if( $_SERVER["HTTP_USER_AGENT"] != "TA3D" )	exit();			// Only accepts requests from TA3D clients

include "functions.php";

clear_list();

$file = fopen("serverlist.txt", "rt");

if( !empty( $file ) ) {
	$nb_entry = fgets( $file ) + 0;
	echo $nb_entry . " servers\n";
	
	for( $i = 0 ; $i < $nb_entry ; $i++ ) {
		$_name = fgets( $file );
		$_IP = fgets( $file );
		$_mod = fgets( $file );
		$_version = fgets( $file );
		$_slots = fgets( $file );
		$_time = fgets( $file );
		
		echo $i . " name: " . $_name;
		echo $i . " IP: " . $_IP;
		echo $i . " mod: " . $_mod;
		echo $i . " version: " . $_version;
		echo $i . " slots: " . $_slots;
		}
	
	fclose( $file );
	}
else
	echo "0 servers\n";
?>
