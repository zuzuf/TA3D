<?php
function clear_list()
{
	$file = fopen("serverlist.txt","rt");

	if( !empty( $file ) ) {
		$time = time() - 160;

		$backup = fopen("serverlist.bak","wt");

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
			
			if( ($_time + 0) >= $_time ) {
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
		}
}
?>