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

# ifdef TA3D_PLATFORM_DARWIN
#   include "tools/darwin/fmod/4.14.07/include/fmod.hpp"
#   include "tools/darwin/fmod/4.14.07/include/fmod_errors.h"
#   define TA3D_FMOD_INCLUDED
# endif

# ifdef TA3D_PLATFORM_LINUX
#   include "tools/linux/FMOD/inc/fmod.hpp"
#   include "tools/linux/FMOD/inc/fmod_errors.h"
#   include "tools/linux/FMOD/inc/wincompat.h"
#   define TA3D_FMOD_INCLUDED
# endif

# ifdef TA3D_PLATFORM_WINDOWS
#   include "tools/win32/fmod/fmod.hpp"
#   include "tools/win32/fmod/fmod_errors.h"
#   ifdef TA3D_PLATFORM_MSVC
#      pragma comment(lib, "tools/win32/libs/fmodex_vc.lib")
#   endif
#   define TA3D_FMOD_INCLUDED
# endif

# ifndef TA3D_FMOD_INCLUDED
#   error "The FMOD headers has not been included. This system is not recognized."
# endif




namespace TA3D
{
namespace Interfaces
{


    class cAudio : public ObjectSync, protected TA3D::IInterface, protected UTILS::cTAFileParser
    {
    public:
        cAudio(const float DistanceFactor, const float DopplerFactor, const float RolloffFactor);
        ~cAudio();

        bool getPlayListFiles(String::Vector& out);
        void setPlayListFileMode(const int idx, bool Battle, bool Deactivated);


        void updatePlayListFiles();
        void savePlayList();

        void playMusic();
        void setMusicMode(const bool battleMode);

        void togglePauseMusic();
        void pauseMusic();
        void stopMusic();


        void loadTDFSounds(const String& FileName);
        void purgeSounds();

        void loadTDFSounds(const bool allSounds);
        bool loadSound(const String& Filename, const bool LoadAs3D,
                       const float MinDistance = 1.0f, const float MaxDistance = 100.0f);

        // Play sound directly from our sound pool
        void playSound(const String& Filename, const VECTOR3D* vec = NULL);

        // Play sound from TDF by looking up sound filename from internal hash
        void playTDFSound(const String& Key, const VECTOR3D* vec = NULL);
        void playTDFSoundNow(const String& Key, const VECTOR3D* vec = NULL); // Wrapper to playTDFSound + update3DSound

        // keys will be added together and then PlayTDF( key, vec ); called
        // if either key is null or "" aborts.
        void playTDFSound(const String& keyA, const String& keyB, const VECTOR3D* vec = NULL);

        void playSoundFileNow(const String& Filename); // Loads and play a sound

        void stopSoundFileNow(); // Stop playing

        void setListenerPos(const VECTOR3D* vec);

        void update3DSound();

        bool isRunning() const {return m_FMODRunning;};


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

        typedef std::vector< m_PlayListItem * >	 Playlist;
        typedef Playlist::iterator	 plItor;

    private:
        struct m_SoundListItem
        {
            m_SoundListItem() :m_3DSound(false), m_SampleHandle(NULL), last_time_played(0) {}
            ~m_SoundListItem();

            bool			m_3DSound;
            # ifdef TA3D_PLATFORM_MINGW
            FMOD_SOUND		*m_SampleHandle;
            # else
            FMOD::Sound		*m_SampleHandle;
            # endif
            uint32			last_time_played;

        }; // class m_SoundListItem

        struct m_WorkListItem
        {
            m_WorkListItem() :m_Sound(NULL), vec(NULL) {}

            m_SoundListItem		*m_Sound;
            VECTOR				*vec;

        }; // class m_WorkListItem


    private:
        void shutdownAudio(bool PurgeLoadedData);
        bool startUpAudio();

        void loadPlayList();
        void purgePlayList();

        const String selectNextMusic();
        void  playMusic(const String& FileName);
        virtual uint32 InterfaceMsg(const lpcImsg msg);


    private:
        bool m_FMODRunning;      // Is fmod running
        bool m_InBattle;         // Are we in battle
        sint32  m_BattleTunes;      // Number of battle tunes;
        Playlist  m_Playlist;         // Vector of PlayList.

        # ifdef TA3D_PLATFORM_MINGW
        FMOD_SYSTEM     *m_lpFMODSystem;
        FMOD_SOUND      *m_lpFMODMusicsound;
        FMOD_CHANNEL    *m_lpFMODMusicchannel;
        # else
        FMOD::System     *m_lpFMODSystem;
        FMOD::Sound      *m_lpFMODMusicsound;
        FMOD::Channel    *m_lpFMODMusicchannel;
        # endif

        sint16  m_curPlayIndex;   // current play index.
        uint32  m_min_ticks;

        TA3D::UTILS::clpHashTable< m_SoundListItem * >* m_SoundList;
        sint32 fCounter;
        std::list<m_WorkListItem> WorkList;			// List to store work to do when entering main thread

        # ifdef TA3D_PLATFORM_MINGW
        FMOD_SOUND* basic_sound;
        FMOD_CHANNEL* basic_channel;
        # else
        FMOD::Sound* basic_sound;
        FMOD::Channel* basic_channel;
        # endif

    }; // class cAudio



} // namespace AUDIO
} // namespace TA3D

#endif // __TA3D_AUDIO_H__
