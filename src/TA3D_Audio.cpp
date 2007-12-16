/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include "stdafx.h"
#include "TA3D_NameSpace.h"		//"TA3D_Audio.h" is in our namespace.
#include "ta3dbase.h"			// just to use the global camera object

using namespace TA3D::INTERFACES;

TA3D::INTERFACES::cAudio *TA3D::VARS::sound_manager;

/* private */void cAudio::LoadPlayList()
{
	String FileName = GetClientPath() + String( "music/playlist.txt" );
	std::ifstream file( FileName.c_str(), std::ios::in );

	Console->AddEntry("opening playlist");

	if( !file.is_open() )
		return;

	Console->AddEntry("loading playlist");

	String line;
	bool isBattle;

	m_BattleTunes = 0;

	while( !file.eof() )
	{
		std::getline( file, line, '\n' );

		line = TrimString( line ); // strip off spaces, linefeeds, tabs, newlines

		if( !line.length() ) continue;
		if (line[0] == '#' || line[0] == ';' ) continue;

		if( line[0] == '*' )
		{
			isBattle=true;

			line = line.erase( 0,1 );
			m_BattleTunes++;
		}
		else
			isBattle = false;

		m_PlayListItem *m_Tune = new m_PlayListItem();

		m_Tune->m_BattleTune = isBattle;
		m_Tune->m_Filename = line;

		Console->AddEntry("playlist adding : %s", (char*)line.c_str());

		m_Playlist.push_back( m_Tune );
	}

	file.close();

	if( m_Playlist.size() > 0 )
		PlayMusic();
}

/* private */void cAudio::ShutdownAudio( bool PurgeLoadedData )
{
	if( m_FMODRunning ) // only execute stop if we are running.
		StopMusic();

	if( PurgeLoadedData )
	{
		PurgeSounds(); // purge sound list.
		PurgePlayList(); // purge play list
	}

	if( m_FMODRunning )
	{
#ifdef TA3D_PLATFORM_MINGW
		FMOD_System_Close(m_lpFMODSystem);
		FMOD_System_Release(m_lpFMODSystem);
#else
		m_lpFMODSystem->close();				// Commented because crashes with some FMOD versions, and since we're going to end the program ...
		m_lpFMODSystem->release();
#endif
		DeleteInterface();
		DeleteCS();

		m_FMODRunning = false;
	}
}

/* private */bool cAudio::StartUpAudio( void )
{
	uint32 FMODVersion;

	m_lpFMODMusicsound = NULL;
	m_lpFMODMusicchannel = NULL;
	fCounter = 0;

	if( m_FMODRunning )
		return true;

#ifdef TA3D_PLATFORM_MINGW
	if( FMOD_System_Create( &m_lpFMODSystem ) != FMOD_OK ) {
		Console->AddEntry( "FMOD: failed to System_Create, sound disabled" );
		return false;
		}

	if( FMOD_System_GetVersion( m_lpFMODSystem, &FMODVersion ) != FMOD_OK ) {
		Console->AddEntry( "FMOD: Invalid Version of FMOD, sound disabled" );
		return false;
		}

	Console->stdout_on();
	Console->AddEntry( "FMOD version: %x.%x.%x", ((FMODVersion & 0xFFFF0000) >> 16), ((FMODVersion & 0xFF00) >> 8), FMODVersion & 0xFF );
	Console->stdout_off();

	if( FMOD_System_SetStreamBufferSize( m_lpFMODSystem, 32768, FMOD_TIMEUNIT_RAWBYTES ) != FMOD_OK ) {
		Console->AddEntry( "FMOD: Failed to set Stream Buffer Size, sound disabled" );
		return false;
		}

#ifndef TA3D_NO_SOUND
	// 32 channels, normal init, with 3d right handed.
if( FMOD_System_Init( m_lpFMODSystem, 32, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, 0 ) != FMOD_OK ) {
	Console->AddEntry( "FMOD: Failed to init FMOD, sound disabled" );
	return false;
	}

	m_FMODRunning = true;

	LoadPlayList();
#endif

#else

	if( FMOD::System_Create( &m_lpFMODSystem ) != FMOD_OK ) {
		Console->AddEntry( "FMOD: failed to System_Create, sound disabled" );
		return false;
		}

	if( m_lpFMODSystem->getVersion( &FMODVersion ) != FMOD_OK ) {
		Console->AddEntry( "FMOD: Invalid Version of FMOD, sound disabled" );
		return false;
		}

	Console->stdout_on();
	Console->AddEntry( "FMOD version: %x.%x.%x", ((FMODVersion & 0xFFFF0000) >> 16), ((FMODVersion & 0xFF00) >> 8), FMODVersion & 0xFF );
	Console->stdout_off();

	if( m_lpFMODSystem->setStreamBufferSize( 32768, FMOD_TIMEUNIT_RAWBYTES ) != FMOD_OK ) {
		Console->AddEntry( "FMOD: Failed to set Stream Buffer Size, sound disabled" );
		return false;
		}

#ifndef TA3D_NO_SOUND
	// 32 channels, normal init, with 3d right handed.
if( m_lpFMODSystem->init( 32, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, 0 ) != FMOD_OK ) {
	Console->AddEntry( "FMOD: Failed to init FMOD, sound disabled" );
	return false;
	}

	m_FMODRunning = true;

	LoadPlayList();
#endif

#endif

	return true;
}

/* public */cAudio::~cAudio()
{
	ShutdownAudio( true );

	delete m_SoundList;
}

/* public */void cAudio::StopMusic( void )
{
	if( !m_FMODRunning )
		return;

	if( m_lpFMODMusicsound != NULL )
	{
		EnterCS();
#ifdef TA3D_PLATFORM_MINGW
			FMOD_Channel_Stop( m_lpFMODMusicchannel );
			FMOD_Sound_Release( m_lpFMODMusicsound );
#else
			m_lpFMODMusicchannel->stop();
			m_lpFMODMusicsound->release();
#endif
			m_lpFMODMusicsound = NULL;
			m_lpFMODMusicchannel = NULL;
		LeaveCS();
	}
}

/* private */void cAudio::PurgePlayList( void )
{
	StopMusic();

	m_curPlayIndex = -1;	// we don't change this in stop music in case
							// we want to do a play and contine through our list, so
							// we change it here to refelect no index.

	if( m_Playlist.size() < 1 ) // nothing in our list.
		return;

	EnterCS();
		// walk through vector and delete all the items.
		for( plItor k_Pos = m_Playlist.begin(); k_Pos != m_Playlist.end(); k_Pos++ )
			delete( *k_Pos );

		m_Playlist.clear();   // now purge the vector.
	LeaveCS();
}

/* public */void cAudio::TogglePauseMusic( void )
{
	if( m_FMODRunning )
		return;

	if( m_lpFMODMusicchannel == NULL )
		return;

#ifdef TA3D_PLATFORM_MINGW

	FMOD_BOOL paused;

	FMOD_Channel_GetPaused( m_lpFMODMusicchannel, &paused );
	FMOD_Channel_SetPaused( m_lpFMODMusicchannel, !paused );

#else

	bool paused;

	m_lpFMODMusicchannel->getPaused( &paused );
	m_lpFMODMusicchannel->setPaused( !paused );

#endif
}

/* public */void cAudio::PauseMusic( void )
{
	if( m_FMODRunning )
		return;

	if( m_lpFMODMusicchannel == NULL )
		return;

#ifdef TA3D_PLATFORM_MINGW
	FMOD_Channel_SetPaused( m_lpFMODMusicchannel, true );
#else
	m_lpFMODMusicchannel->setPaused( true );
#endif
}

/* private */const String cAudio::SelectNextMusic( void )
{
	plItor cur;
	sint16 cIndex = -1;
	sint16 mCount = 0;
	String szResult = "";

	if( m_Playlist.size() == 0 )
		return szResult;

	if( m_InBattle && m_BattleTunes > 0 )
	{
		srand( (unsigned)time( NULL ) );
		cIndex =  (sint16)(TA3D_RAND() % m_BattleTunes ) +1;
		mCount = 1;

		for( cur = m_Playlist.begin(); cur != m_Playlist.end(); cur++ )
		{
			if( (*cur)->m_BattleTune && mCount >= cIndex )		// If we get one that match our needs we take it
			{
				szResult = String( (*cur)->m_Filename );
				break;
			}
			else if( (*cur)->m_BattleTune )						// Take the last one that can be taken if we try to go too far
				szResult = String( (*cur)->m_Filename );
		}

		return szResult;
	}

	mCount = 0;
	if( m_curPlayIndex > (sint32)m_Playlist.size() )
		m_curPlayIndex = -1;

	bool found = false;

	for( cur = m_Playlist.begin(); cur != m_Playlist.end(); cur++ )
	{
		mCount++;

		if( (*cur)->m_BattleTune )
			continue;

		if( m_curPlayIndex <= mCount || m_curPlayIndex <= 0 )
		{
			szResult = (*cur)->m_Filename;
			m_curPlayIndex = mCount+1;
			found = true;
			break;
		}
	}
	if( !found && m_curPlayIndex != -1 ) {
		m_curPlayIndex = -1;
		return SelectNextMusic();
		}

	return szResult;
}
         
/* public */void cAudio::SetMusicMode( bool battleMode )
{
	if( m_InBattle == battleMode )
		return;

	m_InBattle = battleMode;

	PlayMusic();
}

/* private */void cAudio::PlayMusic( const String &FileName )
{
	StopMusic();

	if( !m_FMODRunning )
		return;

	if( !file_exists( FileName.c_str() ,FA_RDONLY | FA_ARCH,NULL ) )
	{
		if( !FileName.empty() )
			Console->AddEntry( "FMOD:Failed to find file: %s for play.", FileName.c_str() );
		return;
	}

#ifdef TA3D_PLATFORM_MINGW

	if( FMOD_System_CreateStream( m_lpFMODSystem, FileName.c_str(),
	    FMOD_HARDWARE | FMOD_LOOP_OFF | FMOD_2D | FMOD_IGNORETAGS, 0,
	    &m_lpFMODMusicsound ) != FMOD_OK )
	{
		Console->AddEntry( "FMOD: Failed to create stream. (%s)", FileName.c_str() );
		return;
	}

	if( FMOD_System_PlaySound( m_lpFMODSystem, FMOD_CHANNEL_FREE, m_lpFMODMusicsound,
	    false, &m_lpFMODMusicchannel) != FMOD_OK )
	{
		Console->AddEntry( "FMOD: Failed to playSound/stream." );

		return;
	}

	Console->AddEntry( "FMOD: playing music file %s", FileName.c_str() );

	FMOD_Channel_SetVolume( m_lpFMODMusicchannel, 1.0f );

#else

	if( m_lpFMODSystem->createStream( FileName.c_str(),
	    FMOD_HARDWARE | FMOD_LOOP_OFF | FMOD_2D | FMOD_IGNORETAGS, 0,
	    &m_lpFMODMusicsound ) != FMOD_OK )
	{
		Console->AddEntry( "FMOD: Failed to create stream. (%s)", FileName.c_str() );
		return;
	}

	if( m_lpFMODSystem->playSound( FMOD_CHANNEL_FREE, m_lpFMODMusicsound,
	    false, &m_lpFMODMusicchannel) != FMOD_OK )
	{
		Console->AddEntry( "FMOD: Failed to playSound/stream." );

		return;
	}

	Console->AddEntry( "FMOD: playing music file %s", FileName.c_str() );

	m_lpFMODMusicchannel->setVolume( 1.0f );

#endif
}

/* private */void cAudio::PlayMusic()
{
	if( !m_FMODRunning )
		return;

	if( m_lpFMODMusicchannel != NULL )
	{
#ifdef TA3D_PLATFORM_MINGW

		FMOD_BOOL paused;

		FMOD_Channel_GetPaused( m_lpFMODMusicchannel, &paused );

		if( paused == true )
		{
			FMOD_Channel_SetPaused( m_lpFMODMusicchannel, false );
			return;
		}

#else

		bool paused;

		m_lpFMODMusicchannel->getPaused( &paused );

		if( paused == true )
		{
			m_lpFMODMusicchannel->setPaused( false );
			return;
		}

#endif
	}

	PlayMusic( SelectNextMusic() );
}





/* public */cAudio::cAudio( const float DistanceFactor,
			const float DopplerFactor,
			const float RolloffFactor ) :
		m_FMODRunning( false ),
		m_InBattle( false ),
		m_lpFMODMusicsound( NULL ),
		m_lpFMODMusicchannel( NULL ),
		m_curPlayIndex( -1 ),
		cTAFileParser(),
		m_Playlist()
{
	m_min_ticks = 500;

	StartUpAudio();

	m_SoundList = new TA3D::UTILS::clpHashTable< m_SoundListItem * >;

	CreateCS();
	InitInterface();

	if( !m_FMODRunning )
		return;

#ifdef TA3D_PLATFORM_MINGW
	FMOD_System_Set3DSettings( m_lpFMODSystem, DopplerFactor, DistanceFactor, RolloffFactor );
#else
	m_lpFMODSystem->set3DSettings( DopplerFactor, DistanceFactor, RolloffFactor );
#endif
}

// Begin sound managing routines.
void cAudio::SetListenerPos( const VECTOR3D *vec )
{
	if( !m_FMODRunning )
		return;

	FMOD_VECTOR pos = { vec->x, vec->y, vec->z };
	FMOD_VECTOR vel = { 0,0,0 };
	FMOD_VECTOR forward        = { 0.0f, 0.0f, 1.0f };
	FMOD_VECTOR up             = { 0.0f, 1.0f, 0.0f };

#ifdef TA3D_PLATFORM_MINGW
	FMOD_System_Set3DListenerAttributes( m_lpFMODSystem, 0, &pos, &vel, &forward, &up );
#else
	m_lpFMODSystem->set3DListenerAttributes( 0, &pos, &vel, &forward, &up );
#endif
}

void cAudio::Update3DSound( void )
{
	if( !m_FMODRunning ) {
		EnterCS();

		WorkList.clear();

		LeaveCS();
		return;
		}

	EnterCS();

#ifdef TA3D_PLATFORM_MINGW

	FMOD_System_Update( m_lpFMODSystem );

	for( List< m_WorkListItem >::iterator i = WorkList.begin() ; i != WorkList.end() ; i++ ) {
		FMOD_CHANNEL *ch;
		if( FMOD_System_PlaySound( m_lpFMODSystem, FMOD_CHANNEL_FREE,
		    i->m_Sound->m_SampleHandle, true, &ch ) != FMOD_OK )
			continue;

		if( i->m_Sound->m_3DSound )
		{
			FMOD_VECTOR pos = { i->vec->x, i->vec->y, i->vec->z };
			FMOD_VECTOR vel = { 0,0,0 };

			FMOD_Channel_Set3DAttributes( ch, &pos, &vel );
		}

		FMOD_Channel_SetPaused( ch, false );
		}

#else

	m_lpFMODSystem->update();

	for( List< m_WorkListItem >::iterator i = WorkList.begin() ; i != WorkList.end() ; i++ ) {
		FMOD::Channel *ch;
		if( m_lpFMODSystem->playSound( FMOD_CHANNEL_FREE,
		    i->m_Sound->m_SampleHandle, true, &ch ) != FMOD_OK )
			continue;

		if( i->m_Sound->m_3DSound )
		{
			FMOD_VECTOR pos = { i->vec->x, i->vec->y, i->vec->z };
			FMOD_VECTOR vel = { 0,0,0 };

			ch->set3DAttributes( &pos, &vel );
		}

		ch->setPaused( false );
		}

#endif

	WorkList.clear();

LeaveCS();

	if( (fCounter++) < 100 ) return;

	fCounter = 0;

	if( m_lpFMODMusicchannel == NULL ) {
		PlayMusic();
		return;
		}

	EnterCS();

#ifdef TA3D_PLATFORM_MINGW
	FMOD_BOOL playing;
	FMOD_Channel_IsPlaying( m_lpFMODMusicchannel, &playing );
#else
	bool playing;
	m_lpFMODMusicchannel->isPlaying( &playing );
#endif
	if( !playing )
		PlayMusic();

LeaveCS();
}

uint32 cAudio::InterfaceMsg( const lpcImsg msg )
{
	if( msg->MsgID == TA3D_IM_GUI_MSG )	{			// for GUI messages, test if it's a message for us
		if( msg->lpParm1 == NULL )
			return INTERFACE_RESULT_HANDLED;		// Oups badly written things
		String message = Lowercase( (char*) msg->lpParm1 );				// Get the string associated with the signal

		if( message == "music play" ) {
			PlayMusic();
			return INTERFACE_RESULT_HANDLED;
			}
		else if( message == "music pause" ) {
			PauseMusic();
			return INTERFACE_RESULT_HANDLED;
			}
		else if( message == "music stop" ) {
			StopMusic();
			return INTERFACE_RESULT_HANDLED;
			}
		}

	return INTERFACE_RESULT_CONTINUE;
}

bool cAudio::LoadSound( const String &Filename, const bool LoadAs3D,
						const float MinDistance, const float MaxDistance )
{
	String szWav = Lowercase( Filename ); // copy string to szWav so we can work with it.

	I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (char*)format("loading sound file %s\n",(char *)szWav.c_str()).c_str(), NULL, NULL );

	// if it has a .wav extension then remove it.
	int i = (int)szWav.find( "wav" );   
	if( i != -1 )
		szWav.resize( szWav.length() - 4 );

	// if its already loaded return true.
	if( m_SoundList->Exists( szWav ) ) {
//		I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (char*)format("sound file %s is already loaded\n",(char *)szWav.c_str()).c_str(), NULL, NULL );
		return true;
		}

	uint32 Length;
	byte *data;

	// pull the data from hpi.
	data=HPIManager->PullFromHPI( String( "sounds\\" ) + szWav + String( ".wav" ), &Length );

	if( !data ) // if no data, log a message and return false.
	{
		szWav = format( "FMOD: LoadSound(%s), no such sound found in HPI.\n", szWav.c_str() );

		I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)szWav.c_str(), NULL, NULL );
		return false;
	}

	m_SoundListItem *m_Sound = new m_SoundListItem;
	m_Sound->m_3DSound = LoadAs3D;

	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.length = Length;

#ifdef TA3D_PLATFORM_MINGW

	// Now get fmod to load the sample
	FMOD_RESULT FMODResult = FMOD_System_CreateSound( m_lpFMODSystem, (const char *)data,
		FMOD_HARDWARE | FMOD_OPENMEMORY | ( (LoadAs3D) ? FMOD_3D : FMOD_2D ),
		&exinfo,
		&m_Sound->m_SampleHandle);

	free( data ); // we no longer need this.

	if( FMODResult != FMOD_OK ) // ahh crap fmod couln't load it.
	{
		delete m_Sound;  // delete the sound.
		m_Sound = NULL;

		// log a message and return false;
		if( m_FMODRunning ) {
			szWav = format( "FMOD: LoadSound(%s), Failed to construct sample.\n", szWav.c_str() );
			I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)szWav.c_str(), NULL, NULL );
			}

		return false;
	}

	// if its a 3d Sound we need to set min/max distance.
	if( m_Sound->m_3DSound )
		FMOD_Sound_Set3DMinMaxDistance( m_Sound->m_SampleHandle, MinDistance, MaxDistance );

#else

	// Now get fmod to load the sample
	FMOD_RESULT FMODResult = m_lpFMODSystem->createSound( (const char *)data,
		FMOD_HARDWARE | FMOD_OPENMEMORY | ( (LoadAs3D) ? FMOD_3D : FMOD_2D ),
		&exinfo,
		&m_Sound->m_SampleHandle);

	free( data ); // we no longer need this.

	if( FMODResult != FMOD_OK ) // ahh crap fmod couln't load it.
	{
		delete m_Sound;  // delete the sound.
		m_Sound = NULL;

		// log a message and return false;
		if( m_FMODRunning ) {
			szWav = format( "FMOD: LoadSound(%s), Failed to construct sample.\n", szWav.c_str() );
			I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)szWav.c_str(), NULL, NULL );
			}

		return false;
	}

	// if its a 3d Sound we need to set min/max distance.
	if( m_Sound->m_3DSound )
		m_Sound->m_SampleHandle->set3DMinMaxDistance( MinDistance, MaxDistance );

#endif

	// add the sound to our soundlist hash table, and return true.
	m_SoundList->InsertOrUpdate( szWav, m_Sound );

	return true;
}

void cAudio::LoadTDFSounds( const bool allSounds )
{
	EnterCS();
		String FileName;

		if( allSounds )
			FileName = ta3d_sidedata.gamedata_dir + "allsound.tdf";
		else
			FileName = ta3d_sidedata.gamedata_dir + "sound.tdf";

		Console->AddEntry( "Reading %s", FileName.c_str() );

		Load( FileName );

		Console->AddEntry( "Loading sounds from %s", FileName.c_str() );

		for( iterator iter = begin() ; iter != end() ; iter++ )   
		{
			for( std::list< cBucket<String> >::iterator cur = iter->begin() ; cur != iter->end() ; cur++ )
			{
				String szWav = String( (*cur).m_T_data );

				LoadSound( szWav, false );
			}
		}
	LeaveCS();
}

void cAudio::PurgeSounds( void )
{
	EnterCS();
		m_SoundList->EmptyHashTable();
		EmptyHashTable();
		WorkList.clear();
	LeaveCS();
}

// Play sound directly from our sound pool
void cAudio::PlaySound( const String &Filename, const VECTOR3D *vec )
{
	if( vec != NULL && game_cam != NULL && ((VECTOR)(*vec - game_cam->RPos)).Sq() > 360000.0f )	return;		// If the source is too far, does not even think about playing it!

//	Console->AddEntry("playing %s", (char*)Filename.c_str());

	if( !m_FMODRunning )
		return;

EnterCS();

	String szWav = Lowercase( Filename ); // copy string to szWav so we can work with it.

	// if it has a .wav extension then remove it.
	int i = (int)szWav.find( ".wav" );
	if( i != -1 )
		szWav.resize( szWav.length() - 4 );

	m_SoundListItem *m_Sound = m_SoundList->Find( szWav );

	if( !m_Sound ) {
		Console->AddEntry("%s not found, aborting", (char*)Filename.c_str());
LeaveCS();
		return;
		}

	if( msec_timer - m_Sound->last_time_played < m_min_ticks ) {
LeaveCS();
		return;			// Make sure it doesn't play too often, so it doesn't play too loud!
		}

	m_Sound->last_time_played = msec_timer;

	if( !m_Sound->m_SampleHandle || (m_Sound->m_3DSound && !vec) ) {
		if(!m_Sound->m_SampleHandle)
			Console->AddEntry("%s not played the good way", (char*)Filename.c_str());
		else
			Console->AddEntry("%s : m_Sound->m_SampleHandle is false", (char*)Filename.c_str());
LeaveCS();
		return;
		}

//	Console->AddEntry("plays %s", (char*)Filename.c_str());

	m_WorkListItem	m_Work;

	m_Work.m_Sound = m_Sound;
	m_Work.vec = (VECTOR *)vec;

	WorkList.push_back( m_Work );

LeaveCS();
}

void cAudio::PlayTDFSoundNow( const String &Key, const VECTOR3D *vec )		// Wrapper to PlayTDFSound + Update3DSound
{
	PlayTDFSound( Key, vec );
	Update3DSound();
}

// Play sound from TDF by looking up sound filename from internal hash
void cAudio::PlayTDFSound( const String &Key, const VECTOR3D *vec  )
{
	if( Key == "" )	return;
//	Console->AddEntry("trying to play '%s'", (char*)Key.c_str());
	if( !Exists( Lowercase( Key ) ) ) {
		Console->AddEntry("can't find key %s", Key.c_str() );		// output a report to the console but only once
		InsertOrUpdate( Lowercase( Key ), "" );
		return;
		}
//	Console->AddEntry("%s found", (char*)Key.c_str());

	String szWav = Find( Lowercase( Key ) );
	if( szWav != "" )
		PlaySound( szWav, vec );
}

// keys will be added together and then PlayTDF( key, vec ); if either key is
// "" it aborts.
void cAudio::PlayTDFSound( const String &Key1,  const String Key2, const VECTOR3D *vec )
{
	if( Key1 == "" || Key2 == "" )
		return;

	String Key = Key1 + String( "." ) + Key2;

	PlayTDFSound( Key, vec );
}

bool cAudio::IsFMODRunning()
{
	return m_FMODRunning;
}
