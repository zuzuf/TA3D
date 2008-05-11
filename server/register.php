<?php
if( $_SERVER["HTTP_USER_AGENT"] != "TA3D" )	exit();			// Only accepts requests from TA3D clients

include "functions.php";

clear_list();

$address = $_SERVER["REMOTE_ADDR"];
$name = $_GET["name"];
$mod = $_GET["mod"];
$version = $_GET["version"];
$slots = $_GET["slots"];

$time = time();

$file = fopen("serverlist.txt","rt");

if( !empty( $file ) ) {
	$backup = fopen("serverlist.bak","w");

	$nb_entry = fgets( $file ) + 0;
	fwrite( $backup, $nb_entry . "\n" );
	
	$removed_entries = 0;

	for( $i = 0 ; $i < $nb_entry ; $i++ ) {
		$_name = fgets( $file );
		$_IP = fgets( $file );
		$_mod = fgets( $file );
		$_version = fgets( $file );
		$_slots = fgets( $file );
		$_time = fgets( $file );
		
		if( $_IP != $address . "\n" ) {
			fwrite( $backup, $_name );
			fwrite( $backup, $_IP );
			fwrite( $backup, $_mod );
			fwrite( $backup, $_version );
			fwrite( $backup, $_slots );
			fwrite( $backup, $_time );
			}
		else
			$removed_entries++;
		}

	fclose( $file );
	fclose( $backup );
	
	$file = fopen("serverlist.txt","wt");
	$backup = fopen("serverlist.bak","rt");
	
	$nb_entry = fgets( $backup ) + 0;
	$nb_entry -= $removed_entries;

	if( !empty( $name ) && !empty( $version ) && !empty( $slots ) )			// When slots is 0, there is no reason to store it in the database
		fwrite( $file, ($nb_entry + 1) . "\n" );
	else
		fwrite( $file, $nb_entry . "\n" );

	for( $i = 0 ; $i < $nb_entry ; $i++ ) {
		$_name = fgets( $backup );
		$_IP = fgets( $backup);
		$_mod = fgets( $backup );
		$_version = fgets( $backup );
		$_slots = fgets( $backup );
		$_time = fgets( $backup );
		
		fwrite( $file, $_name );
		fwrite( $file, $_IP );
		fwrite( $file, $_mod );
		fwrite( $file, $_version );
		fwrite( $file, $_slots );
		fwrite( $file, $_time );
		}

	fclose( $file );
	fclose( $backup );
	$first = false;
	}
else
	$first = true;

if( !empty( $name ) && !empty( $version ) && !empty( $slots ) )	{			// When slots is 0, there is no reason to store it in the database
	$file = fopen("serverlist.txt","at");

	if( $first )
		fwrite( $file, "1\n" );

	fwrite( $file, $name . "\n" );
	fwrite( $file, $address . "\n" );
	fwrite( $file, $mod . "\n" );
	fwrite( $file, $version . "\n" );
	fwrite( $file, $slots . "\n" );
	fwrite( $file, $time . "\n" );

	fclose( $file );
	}
?>
