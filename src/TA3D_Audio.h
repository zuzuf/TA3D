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


#ifndef __TA3D_AUDIO_H__
# define __TA3D_AUDIO_H__

# include "misc/vector.h"
# include "cTAFileParser.h"
# include "misc/interface.h"
# include "threads/thread.h"
# include <list>
# include <vector>

# undef stricmp  // TODO Must be removed

# ifdef TA3D_PLATFORM_DARWIN
#   include "tools/darwin/fmod/4.14.07/include/fmod.hpp"
#   include "tools/darwin/fmod/4.14.07/include/fmod_errors.h"
# endif

# ifdef TA3D_PLATFORM_LINUX
#   include "tools/linux/FMOD/inc/fmod.hpp"
#   include "tools/linux/FMOD/inc/fmod_errors.h"
#   include "tools/linux/FMOD/inc/wincompat.h"
# endif

# ifdef TA3D_PLATFORM_WINDOWS
#   include "tools/win32/fmod/fmod.hpp"
#   include "tools/win32/fmod/fmod_errors.h"
#   ifdef TA3D_PLATFORM_MSVC
#      pragma comment(lib, "tools/win32/libs/fmodex_vc.lib")
#   endif
# endif




namespace TA3D
{
namespace Interfaces
{


    class cAudio : public ObjectSync, protected TA3D::IInterface, protected TA3D::UTILS::cTAFileParser
    {
    private:
        struct m_PlayListItem
        {
            String		m_Filename;
            bool		m_BattleTune;
            bool		m_Deactivated;		// Only to tell the file is theres
            bool		m_checked;			// Used by the playlist generator

            m_PlayListItem()
            {
                m_Filename = String( "" );
                m_BattleTune = false;
                m_Deactivated = false;
                m_checked = false;
            }
        };

    private:
        typedef std::vector< m_PlayListItem * >		Playlist;
        typedef Playlist::iterator					plItor;

    private:
        bool		m_FMODRunning;      // Is fmod running
        bool		m_InBattle;         // Are we in battle
        sint32		m_BattleTunes;      // Number of battle tunes;
        Playlist	m_Playlist;         // Vector of PlayList.

#ifdef TA3D_PLATFORM_MINGW
        FMOD_SYSTEM     *m_lpFMODSystem;
        FMOD_SOUND      *m_lpFMODMusicsound;
        FMOD_CHANNEL    *m_lpFMODMusicchannel;
#else
        FMOD::System     *m_lpFMODSystem;
        FMOD::Sound      *m_lpFMODMusicsound;
        FMOD::Channel    *m_lpFMODMusicchannel;
#endif

        sint16			m_curPlayIndex;   // current play index.
        uint32			m_min_ticks;

    public:

        bool GetPlayListFiles(std::vector<String>& out);
        void						SetPlayListFileMode( int idx, bool Battle, bool Deactivated );

        cAudio( const float DistanceFactor,
                const float DopplerFactor,
                const float RolloffFactor );
        ~cAudio();

        // PlayList Members
    public:
        void	UpdatePlayListFiles( void );
        void	SavePlayList( void );
    private:
        void ShutdownAudio( bool PurgeLoadedData );
        bool StartUpAudio( void );

        void	LoadPlayList( void );
        void	PurgePlayList( void );

        const String SelectNextMusic( void );
        void  PlayMusic( const String &FileName );

    public:
        void PlayMusic( void );
        void SetMusicMode( bool battleMode );

        void TogglePauseMusic( void );
        void PauseMusic( void );
        void StopMusic( void );

        // Sound Members
    private:
        struct m_SoundListItem
        {
            bool			m_3DSound;
#ifdef TA3D_PLATFORM_MINGW
            FMOD_SOUND		*m_SampleHandle;
#else
            FMOD::Sound		*m_SampleHandle;
#endif
            uint32			last_time_played;

            m_SoundListItem()
            {
                last_time_played = 0;
                m_3DSound = false;
                m_SampleHandle = NULL;
            }
            ~m_SoundListItem()
            {
#ifdef TA3D_PLATFORM_MINGW
                if( m_SampleHandle )
                    FMOD_Sound_Release( m_SampleHandle );
#else
                if( m_SampleHandle )
                    m_SampleHandle->release();
#endif

                m_SampleHandle = NULL;
            }
        };

        struct m_WorkListItem
        {
            m_SoundListItem		*m_Sound;
            VECTOR				*vec;

            m_WorkListItem()
            {
                m_Sound = NULL;
                vec = NULL;
            }
        };

    private:
        TA3D::UTILS::clpHashTable< m_SoundListItem * >		*m_SoundList;
        sint32												fCounter;
        std::list<m_WorkListItem> WorkList;			// List to store work to do when entering main thread

#ifdef TA3D_PLATFORM_MINGW
        FMOD_SOUND						*basic_sound;
        FMOD_CHANNEL					*basic_channel;
#else
        FMOD::Sound						*basic_sound;
        FMOD::Channel					*basic_channel;
#endif

    private:
        uint32 InterfaceMsg( const lpcImsg msg );

    public:
        void LoadTDFSounds( const String &FileName );
        void PurgeSounds( void );

        void LoadTDFSounds( const bool allSounds );
        bool LoadSound( const String &Filename, const bool LoadAs3D,
                        const float MinDistance = 1.0f, const float MaxDistance = 100.0f  );

        // Play sound directly from our sound pool
        void PlaySound( const String &Filename, const VECTOR3D *vec = NULL );

        // Play sound from TDF by looking up sound filename from internal hash
        void PlayTDFSound( const String &Key, const VECTOR3D *vec = NULL );
        void PlayTDFSoundNow( const String &Key, const VECTOR3D *vec = NULL );		// Wrapper to PlayTDFSound + Update3DSound

        // keys will be added together and then PlayTDF( key, vec ); called
        // if either key is null or "" aborts.
        void PlayTDFSound( const String &Key1,  const String Key2, const VECTOR3D *vec = NULL );

        void PlaySoundFileNow( const String &Filename );				// Loads and play a sound

        void StopSoundFileNow();											// Stop playing

        void SetListenerPos( const VECTOR3D *vec );

        void Update3DSound( void );

        bool IsFMODRunning();
    }; // class cAudio
} // namespace AUDIO
} // namespace TA3D



#endif // __TA3D_AUDIO_H__
