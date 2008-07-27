#include "stdafx.h"
#include "TA3D_NameSpace.h"			// our namespace, a MUST have.
#include "ta3dbase.h"
#include "misc/math.h"

#define buf_size		1024*1024

void install_TA_files( String def_path )
{
	allegro_init();

	set_uformat( U_ASCII );

#ifdef TA3D_PLATFORM_WINDOWS					// Possible cd-rom path for windows
	int		nb_possible_path = 23;
	const char	*possible_path[] = {	"D:\\", "E:\\", "F:\\", "G:\\", "H:\\", "I:\\", "J:\\", "K:\\", "L:\\", "M:\\", "N:\\", "O:\\", "P:\\", "Q:\\", "R:\\", "S:\\", "T:\\",
										"U:\\", "V:\\", "W:\\", "X:\\", "Y:\\", "Z:\\" };
#else											// Possible cd-rom path for other platforms
	int		nb_possible_path = 42;
	const char	*possible_path[] = {	"/media/cdrom/", "/media/cdrom0/", "/media/cdrom1/", "/media/cdrom2/", "/media/cdrom3/", "/media/cdrom4/", "/media/cdrom5/",
										"/mnt/cdrom/", "/mnt/cdrom0/", "/mnt/cdrom1/", "/mnt/cdrom2/", "/mnt/cdrom3/", "/mnt/cdrom4/", "/mnt/cdrom5/",
										"/mnt/dvd/", "/mnt/dvd0/", "/mnt/dvd1/", "/mnt/dvd2/", "/mnt/dvd3/", "/mnt/dvd4/", "/mnt/dvd5/",
										"/mnt/dvdrecorder/", "/mnt/dvdrecorder0/", "/mnt/dvdrecorder1/", "/mnt/dvdrecorder2/", "/mnt/dvdrecorder3/", "/mnt/dvdrecorder4/", "/mnt/dvdrecorder5/",
										"/media/dvd/", "/media/dvd0/", "/media/dvd1/", "/media/dvd2/", "/media/dvd3/", "/media/dvd4/", "/media/dvd5/",
										"/media/dvdrecorder/", "/media/dvdrecorder0/", "/media/dvdrecorder1/", "/media/dvdrecorder2/", "/media/dvdrecorder3/", "/media/dvdrecorder4/", "/media/dvdrecorder5/" };
#endif

	String path_to_TA_cd = "";

	if (def_path != "")
	{						// Explicit path given
		path_to_TA_cd = def_path;
		for(unsigned int i = 0 ; i < path_to_TA_cd.size() ; ++i)			// Use UNIX like path
			if( path_to_TA_cd[ i ] == '\\' )
				path_to_TA_cd[ i ] = '/';
		if( path_to_TA_cd[ path_to_TA_cd.size() ] != '/' )			// Check it ends with a '\'
			path_to_TA_cd += "/";
	}
	else									// Look for a path where we can find totala3.hpi
		for (int i = 0 ; i < nb_possible_path ; i++ )
		{
			path_to_TA_cd = possible_path[ i ];
			if( ( exists( ( path_to_TA_cd + "totala3.hpi" ).c_str() ) || exists( ( path_to_TA_cd + "totala4.hpi" ).c_str() ) )
			 && exists( ( path_to_TA_cd + "totala2.hpi" ).c_str() ) )		break;		// We found a path to TA's cd ( in fact to needed files )
		}

	if (!( exists( ( path_to_TA_cd + "totala3.hpi" ).c_str() ) || exists( ( path_to_TA_cd + "totala4.hpi" ).c_str() ) )
	 || !exists( ( path_to_TA_cd + "totala2.hpi" ).c_str() ))
	{															// We need TA's cd ( in fact only the files )
		allegro_message( "please mount/insert your TA cdrom now (TA CD 1)" );

		path_to_TA_cd = "";

		if (def_path != "")
		{						// Explicit path given
			path_to_TA_cd = def_path;
			for(unsigned int i = 0 ; i < path_to_TA_cd.size() ; ++i)			// Use UNIX like path
				if( path_to_TA_cd[ i ] == '\\' )
					path_to_TA_cd[ i ] = '/';
			if( path_to_TA_cd[ path_to_TA_cd.size() ] != '/' )			// Check it ends with a '\'
				path_to_TA_cd += "/";
		}
		else									// Look for a path where we can find totala3.hpi
		{
		    uint32 timer = msec_timer;
		    while (path_to_TA_cd == "" && msec_timer -  timer < 5000)
		    {
		        rest(100);            // To prevent too much load on the CPU
			    for(int i = 0 ; i < nb_possible_path ; i++ )
			    {
				    path_to_TA_cd = possible_path[ i ];
				    if( ( exists( ( path_to_TA_cd + "totala3.hpi" ).c_str() ) || exists( ( path_to_TA_cd + "totala4.hpi" ).c_str() ) )
				     && exists( ( path_to_TA_cd + "totala2.hpi" ).c_str() ) )		break;		// We found a path to TA's cd ( in fact to needed files )
            		path_to_TA_cd = "";
			    }
			}
		}
	}

	HPIManager=new cHPIHandler( path_to_TA_cd );

	uint32 file_size32 = 0;
	byte *data = HPIManager->PullFromHPI_zone( "install\\totala1.hpi", 0, buf_size, &file_size32);			// Extract the totala1.hpi file from the TA CD
	bool success = true;

	if (data)
	{
		set_color_depth(32);
		set_gfx_mode( GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0 );
		clear( screen );

		rectfill( screen, 0, 0, 640, 480, 0x7F7F7F );

		textprintf_centre_ex( screen, font, 320, 40, 0x0, -1, "Extracting totala1.hpi ( step 1/2 )" );

		FILE *dst = TA3D_OpenFile("totala1.hpi","wb");
		
		if (dst)
        {
			fwrite(data,buf_size,1,dst);

			for (uint32 pos = buf_size; pos < file_size32; pos += buf_size)
            {
				int read_size = Math::Min(buf_size, (int)(file_size32 - pos));
				delete[] data;
				data = HPIManager->PullFromHPI_zone( "install\\totala1.hpi", pos, read_size, &file_size32);			// Extract the totala1.hpi file from the TA CD
				fwrite(data+pos,read_size,1,dst);

				rectfill( screen, 100, 60, 540, 80, makecol( 255, 0, 0 ) );
				rectfill( screen, 100, 60, 100 + 440 * (pos+read_size>>10) / (file_size32>>10), 80, makecol( 255, 255, 0 ) );
				textprintf_centre_ex( screen, font, 320, 66, 0x0, -1, "%d%%", 100 * (pos+read_size>>10) / (file_size32>>10) );
			}

			fflush(dst);
			fclose(dst);
		}
		else
			success = false;
		delete[] data;
	}
	else
		success = false;

	delete HPIManager;

	if (exists( ( path_to_TA_cd + "totala2.hpi" ).c_str() ) && success)
	{
		textprintf_centre_ex( screen, font, 320, 140, 0x0, -1, "Copying totala2.hpi ( step 2/2 )" );
		rectfill( screen, 100, 160, 540, 180, makecol( 255, 0, 0 ) );
		textprintf_centre_ex( screen, font, 320, 166, 0x0, -1, "0%%" );

		FILE *src = TA3D_OpenFile( path_to_TA_cd + "totala2.hpi", "rb" );
		if (src)
        {
			FILE *dst = TA3D_OpenFile( "totala2.hpi", "wb" );
			if (dst)
            {
				int limit = FILE_SIZE( ( path_to_TA_cd + "totala2.hpi" ).c_str() );
				byte *buf = new byte[ buf_size ];			// a 1Mo buffer

				for (int pos = 0; pos < limit; pos += buf_size)
                {
					int read_size = Math::Min(buf_size, limit-pos);
					fread( buf, read_size, 1, src );
					fwrite( buf, read_size, 1, dst );

					rectfill( screen, 100, 160, 540, 180, makecol( 255, 0, 0 ) );
					rectfill( screen, 100, 160, 100 + 440 * (pos+read_size>>10) / (limit>>10), 180, makecol( 255, 255, 0 ) );
					textprintf_centre_ex( screen, font, 320, 166, 0x0, -1, "%d%%", 100 * (pos+read_size>>10) / (limit>>10) );
				}
				delete buf;
				fflush(dst);
				fclose(dst);
			}
			else
				success = false;
			fclose(src);
		}
		else
			success = false;
	}
	else
		success = false;
		
	if (success)
	{
	
		if (!exists( ( path_to_TA_cd + "totala4.hpi" ).c_str() ))
		{
			allegro_message( "please mount/insert your TA cdrom now (TA CD 2) if you want to install campaign files" );

											// Look for a path where we can find totala4.hpi
		    uint32 timer = msec_timer;
            path_to_TA_cd = "";
		    while (path_to_TA_cd == "" && msec_timer -  timer < 5000)
		    {
		        rest(100);            // To prevent too much load on the CPU
			    for(int i = 0 ; i < nb_possible_path ; i++ )
			    {
				    path_to_TA_cd = possible_path[ i ];
				    if( exists( ( path_to_TA_cd + "totala4.hpi" ).c_str() ) )		break;		// We found a path to TA's cd ( in fact to needed files )
				    path_to_TA_cd = "";
				}
			}
		}

		if (exists( ( path_to_TA_cd + "totala4.hpi" ).c_str() ))
		{
			rectfill( screen, 0, 0, 640, 480, 0x7F7F7F );

			textprintf_centre_ex( screen, font, 320, 40, 0x0, -1, "Copying totala4.hpi" );
			rectfill( screen, 100, 60, 540, 80, makecol( 255, 0, 0 ) );
			textprintf_centre_ex( screen, font, 320, 66, 0x0, -1, "0%%" );

			FILE *src = TA3D_OpenFile( path_to_TA_cd + "totala4.hpi", "rb" );
			if( src )
			{
				FILE *dst = TA3D_OpenFile( "totala4.hpi", "wb" );
				if( dst )
				{
					int limit = FILE_SIZE( ( path_to_TA_cd + "totala4.hpi" ).c_str() );
					byte *buf = new byte[ buf_size ];			// a 1Mo buffer

					for (int pos = 0; pos < limit; pos += buf_size)
                    {
						int read_size = Math::Min(buf_size, limit-pos);
						fread( buf, read_size, 1, src );
						fwrite( buf, read_size, 1, dst );

						rectfill( screen, 100, 60, 540, 80, makecol( 255, 0, 0 ) );
						rectfill( screen, 100, 60, 100 + 440 * (pos+read_size>>10) / (limit>>10), 80, makecol( 255, 255, 0 ) );
						textprintf_centre_ex( screen, font, 320, 66, 0x0, -1, "%d%%", 100 * (pos+read_size>>10) / (limit>>10) );
					}

					delete buf;

					fflush( dst );

					fclose( dst );
				}
				fclose( src );
			}
		}

		{
			if( !exists( ( path_to_TA_cd + "cc/ccdata.ccx" ).c_str() ) && !exists( ( path_to_TA_cd + "install/ccdata.ccx" ).c_str() ) )
				allegro_message( "please mount/insert your TA:Core Contingency cdrom now (TA:CC CD) if you have it" );
			bool cc_success = true;
			int nb_files = 3;
			int Y = 0;
			String file_to_copy[] = { "cc/ccdata.ccx", "cc/ccmaps.ccx", "cc/ccmiss.ccx" };
			String file_to_copy_alternative[] = { "install/ccdata.ccx", "install/ccmaps.ccx", "install/ccmiss.ccx" };
			String file_destination[] = { "ccdata.ccx", "ccmaps.ccx", "ccmiss.ccx" };
			rectfill( screen, 0, 0, 640, 480, 0x7F7F7F );
											// Look for a path where we can find the files we need
		    uint32 timer = msec_timer;
            path_to_TA_cd = "";
		    while (path_to_TA_cd == "" && msec_timer -  timer < 5000)
		    {
		        rest(100);            // To prevent too much load on the CPU
			    for(int i = 0 ; i < nb_possible_path ; i++ )
			    {
				    path_to_TA_cd = possible_path[ i ];
				    if( exists( ( path_to_TA_cd + file_to_copy[ 0 ] ).c_str() ) )		break;		// We found a path to TA's cd ( in fact to needed files )
				    if( exists( ( path_to_TA_cd + file_to_copy_alternative[ 0 ] ).c_str() ) )		break;		// We found a path to TA's cd ( in fact to needed files )
                    path_to_TA_cd = "";
			    }
			}
			
			if( exists( ( path_to_TA_cd + file_to_copy_alternative[ 0 ] ).c_str() ) )			// We found a path to an alternative TA CD
				for( int i = 0 ; i < nb_files ; i++ )
					file_to_copy[i] = file_to_copy_alternative[i];
			for( int i = 0 ; i < nb_files ; i++ )
				if (exists( ( path_to_TA_cd + file_to_copy[ i ] ).c_str() ) && cc_success)
				{
					textprintf_centre_ex( screen, font, 320, 40 + Y, 0x0, -1, format( "Copying %s (step %d/%d)", file_to_copy[ i ].c_str(), 1+i, nb_files ).c_str() );
					rectfill( screen, 100, 60 + Y, 540, 80 + Y, makecol( 255, 0, 0 ) );
					textprintf_centre_ex( screen, font, 320, 66 + Y, 0x0, -1, "0%%" );

					FILE *src = TA3D_OpenFile( path_to_TA_cd + file_to_copy[ i ], "rb" );
					if (src)
					{
						FILE *dst = TA3D_OpenFile( file_destination[ i ], "wb" );
						if (dst)
						{
							int limit = FILE_SIZE( ( path_to_TA_cd + file_to_copy[ i ] ).c_str() );
							byte *buf = new byte[ buf_size ];			// a 1Mo buffer

							for( int pos = 0 ; pos < limit ; pos+= buf_size )
                            {
								int read_size = Math::Min( buf_size, limit-pos );
								fread( buf, read_size, 1, src );
								fwrite( buf, read_size, 1, dst );

								rectfill( screen, 100, 60 + Y, 540, 80 + Y, makecol( 255, 0, 0 ) );
								rectfill( screen, 100, 60 + Y, 100 + 440 * (pos+read_size>>10) / (limit>>10), 80 + Y, makecol( 255, 255, 0 ) );
								textprintf_centre_ex( screen, font, 320, 66 + Y, 0x0, -1, "%d%%", 100 * (pos+read_size>>10) / (limit>>10) );
							}

							delete buf;

							fflush( dst );

							fclose( dst );
						}
						else
							cc_success = false;
						fclose( src );
					}
					else
						cc_success = false;
					Y += 100;
					if (Y + 80 >= 440 && i + 1 < nb_files)
					{
						Y -= 100;
						blit( screen, screen, 0, 100, 0, 0, SCREEN_W, SCREEN_H - 100 );
					}
				}
				else
					cc_success = false;
		}
		{
			allegro_message( "please mount/insert your TA:Battle Tactics cdrom now (TA:BT CD) if you have it" );
			bool bt_success = true;
			int nb_files = 10;
			String file_to_copy[] = { "bt/btdata.ccx", "bt/btmaps.ccx", "bt/tactics1.hpi", "bt/tactics2.hpi", "bt/tactics3.hpi", "bt/tactics4.hpi", "bt/tactics5.hpi", "bt/tactics6.hpi", "bt/tactics7.hpi", "bt/tactics8.hpi" };
			String file_destination[] = { "btdata.ccx", "btmaps.ccx", "tactics1.hpi", "tactics2.hpi", "tactics3.hpi", "tactics4.hpi", "tactics5.hpi", "tactics6.hpi", "tactics7.hpi", "tactics8.hpi" };
			int Y = 0;
			rectfill( screen, 0, 0, 640, 480, 0x7F7F7F );
											// Look for a path where we can find the files we need
		    uint32 timer = msec_timer;
            path_to_TA_cd = "";
		    while (path_to_TA_cd == "" && msec_timer -  timer < 5000)
		    {
		        rest(100);            // To prevent too much load on the CPU
			    for(int i = 0 ; i < nb_possible_path ; i++ )
			    {
				    path_to_TA_cd = possible_path[ i ];
				    if( exists( ( path_to_TA_cd + file_to_copy[ 0 ] ).c_str() ) )		break;		// We found a path to TA's cd ( in fact to needed files )
                    path_to_TA_cd = "";
			    }
			}
			for( int i = 0 ; i < nb_files ; i++ )
				if (exists( ( path_to_TA_cd + file_to_copy[ i ] ).c_str() ) && bt_success)
				{
					textprintf_centre_ex( screen, font, 320, 40 + Y, 0x0, -1, format( "Copying %s (step %d/%d)", file_to_copy[ i ].c_str(), 1+i, nb_files ).c_str() );
					rectfill( screen, 100, 60 + Y, 540, 80 + Y, makecol( 255, 0, 0 ) );
					textprintf_centre_ex( screen, font, 320, 66 + Y, 0x0, -1, "0%%" );

					FILE *src = TA3D_OpenFile( path_to_TA_cd + file_to_copy[ i ], "rb" );
					if (src)
					{
						FILE *dst = TA3D_OpenFile( file_destination[ i ], "wb" );
						if (dst)
						{
							int limit = FILE_SIZE( ( path_to_TA_cd + file_to_copy[ i ] ).c_str() );
							byte *buf = new byte[ buf_size ];			// a 1Mo buffer

							for (int pos = 0; pos < limit; pos += buf_size)
                            {
								int read_size = Math::Min( buf_size, limit-pos );
								fread( buf, read_size, 1, src );
								fwrite( buf, read_size, 1, dst );

								rectfill( screen, 100, 60 + Y, 540, 80 + Y, makecol( 255, 0, 0 ) );
								rectfill( screen, 100, 60 + Y, 100 + 440 * (pos+read_size>>10) / (limit>>10), 80 + Y, makecol( 255, 255, 0 ) );
								textprintf_centre_ex( screen, font, 320, 66 + Y, 0x0, -1, "%d%%", 100 * (pos+read_size>>10) / (limit>>10) );
							}

							delete buf;
							fflush( dst );
							fclose( dst );
						}
						else
							bt_success = false;
						fclose( src );
					}
					else
						bt_success = false;
					Y += 100;
					if (Y + 80 >= 440 && i + 1 < nb_files)
					{
						Y -= 100;
						blit( screen, screen, 0, 100, 0, 0, SCREEN_W, SCREEN_H - 100 );
					}
				}
				else
					bt_success = false;
		}
	}

	if( ! success )							// Print an error message
		allegro_message( "Installation failed:\n    Unable to find TA's cd path!!\n\nplease use this syntax:\n    ta3d install path_to_TA_cd\n                                         \n" );
	else
		allegro_message( "Installation Successful!!\nNow just run ta3d from its base directory and have fun ;-)!\n\nPS: You can also copy the rev31.gp3 file from TA 3.1 patch !!\nThis installer cannot extract it :(\n" );

	allegro_exit();
}
