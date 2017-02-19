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

#ifndef __TA3D_SOUNDS_MANAGER_H__
# define __TA3D_SOUNDS_MANAGER_H__

# include <misc/string.h>
# include <misc/vector.h>
# include <misc/tdf.h>
# include <misc/interface.h>
# include <threads/thread.h>
# include <list>
# include <vector>

# include <SDL/SDL_mixer.h>


namespace TA3D
{
	namespace Audio
	{


		/*! \class Manager
		**
		** \brief The Audio Engine
		*/
        class Manager : public TA3D::IInterface
		{
		public:
            typedef zuzuf::smartptr<Manager>	Ptr;
		public:
			//! \name Constructor & Destructor
			//@{
			/*!
			** \brief Constructor
			*/
			Manager();
			//! Destructor
            virtual ~Manager();
			//@}

            /*!
            ** \brief Set sound volume
            ** \param The volume (0 - 128)
            */
            void setVolume(int volume);

            /*!
            ** \brief Set music volume
            ** \param The volume (0 - 128)
            */
            void setMusicVolume(int volume);

            /*!
			** \brief Get a copy of the playlist
			** \param[out] The playlist
			** \return True if the playlist is not empty, false otherwise
			*/
			bool getPlayListFiles(String::Vector& out);

			/*!
			** \brief Set the properties of a single item in the playlist
			**
			** \param idx The index in the playlist to modify
			** \param inBattle The item should be played during a battle
			** \param disabled The item is disabled
			*/
			void setPlayListFileMode(const int idx, bool inBattle, bool disabled);

			//! \name Load & Save
			//@{

			/*!
			** \brief Reload the playlist
			*/
			void updatePlayListFiles();

			/*!
			** \brief Save the playlist into a single file
			*/
			void savePlaylist();

			void loadTDFSounds(const bool allSounds);

			/*!
			** \brief Load a single sound file
			*/
			bool loadSound(const String& filename, const bool LoadAs3D, const float MinDistance = 1.0f, const float MaxDistance = 100.0f);

			//@}


			/*!
			** \brief Switch the current mode for the music
			** \param battleMode
			*/
			void setMusicMode(const bool battleMode);

			/*!
			** \brief Play/Pause the music (Toggle the state)
			** \see pauseMusic()
			** \see playMusic()
			*/
			void togglePauseMusic();

			/*!
			** \brief Enable the music and play the next track in the list
			*/
			void playMusic();

			/*!
			** \brief Pause the music
			*/
			void pauseMusic();

			/*!
			** \brief Stop the music
			*/
			void stopMusic();

			/*!
			** \brief Clear all playlists
			*/
			void purgeSounds();

			/*!
			** \brief Play sound directly from our sound pool
			*/
			void playSound(const String& filename, const Vector3D* vec = NULL);

			// Play sound from TDF by looking up sound filename from internal hash
			void playTDFSound(const String& key, const Vector3D* vec = NULL);

			/*!
			** \brief Play a sound file from its key
			**
			** It is a convenient method to deal with playTDFSound + update3DSound
			*/
			void playTDFSoundNow(const String& Key, const Vector3D* vec = NULL);

			// keys will be added together and then PlayTDF( key, vec ); called
			// if either key is null or "" aborts.
			void playTDFSound(const String& keyA, const String& keyB, const Vector3D* vec = NULL);

			/*!
			** \brief Play a sound file right now
			*/
			void playSoundFileNow(const String& filename); // Loads and play a sound

			/*!
			** \brief Stop playing
			*/
			void stopSoundFileNow();

			/*!
			** \brief Reset the position of the camera
			*/
			void setListenerPos(const Vector3D& vec);

			/*!
			** \brief ReUpdate the list of 3D sounds
			*/
			void update3DSound();

			/*!
			** \brief Get if the system is running
			*/
            bool isRunning() {MutexLocker locker(pMutex); return m_SDLMixerRunning;}


		private:
			/*! \class PlaylistItem
			**
			** \brief A single item in the playlist
			*/
			struct PlaylistItem
			{
				//! Default constructor
                PlaylistItem() :battleTune(false), disabled(false), checked(false), cdromID(-1), trackID(0), cd(NULL) {}
				//! Filename
				String filename;
				//!
				bool battleTune;
                //! Only to tell the file is there
				bool disabled;
				//! Used by the playlist generator
				bool checked;
                //! Does it come from an audio CD ?
                int cdromID;
                //! What track is it ? (audio cd only)
                int trackID;
                //! The SDL CD handler
                SDL_CD *cd;

			}; // class PlaylistItem

			//! Definition of a playlist
			typedef std::vector<PlaylistItem*>  Playlist;

			/*! \class SoundItemList
			**
			** \brief A single sound file contained in the hash table (pTable)
			*/
			struct SoundItemList
			{
				SoundItemList() :is3DSound(false), sampleHandle(NULL), lastTimePlayed(0) {}
				SoundItemList(const bool a3DSound) :is3DSound(a3DSound), sampleHandle(NULL), lastTimePlayed(0) {}
				~SoundItemList();

				bool is3DSound;
				Mix_Chunk* sampleHandle;
				uint32 lastTimePlayed;

			}; // class SoundItemList


			/*!
			** \brief A single sound file which is currently playing
			*/
			struct WorkListItem
			{
				//! Default constructor
				WorkListItem() :sound(NULL), vec(NULL) {}
				//! Constructor by copy
				WorkListItem(const WorkListItem& c) : sound(c.sound), vec(c.vec) {}
				WorkListItem(SoundItemList* s, const Vector3D* v) : sound(s), vec(v) {}

				//!
				SoundItemList* sound;
				//! Vector
				const Vector3D* vec;

			}; // class WorkListItem

			//! The list of all currently played sounds
			typedef std::list<WorkListItem>  WorkList;

			/*!
			** \brief Predicate to load all single files from a hash table
			*/
			class LoadAllTDFSound
			{
			public:
				LoadAllTDFSound(Manager& a) : pAudio(a) {}
				bool operator () (const String&, const String& value)
				{
					pAudio.doLoadSound(value, false);
					return true; // True to not stop the process
				}
			private:
				//! Self reference
				Manager& pAudio;
			}; // class LoadAllTDFSound


		private:
			//! \name Non thread-safe methods
			//@{

			//! \see playMusic()
			void doPlayMusic();
			//! \see pauseMusic()
			void doPauseMusic();
			//! \see stopMusic()
			void doStopMusic();
			//! \see updatePlayListFiles()
			void doUpdatePlayListFiles();
			//! \see savePlaylist()
			void doSavePlaylist();
			//! \see loadSound()
			bool doLoadSound(String filename, const bool LoadAs3D, const float MinDistance = 1.0f, const float MaxDistance = 100.0f);
			//! \see playMusic()
			void doPlayMusic(const String& filename);
			//! \see playTDFSoundNow(const String&, const Vector3D*)
			void doPlayTDFSound(String key, const Vector3D* vec);
			//! \see playTDFSound(const String&, const String&, const Vector3D*)
			void doPlayTDFSound(const String& keyA, const String& keyB, const Vector3D* vec);
			//! \see update3DSound()
			void doUpdate3DSound();

			//! Initialize FMOD
			bool doStartUpAudio();
			//! Release FMOD
			void doShutdownAudio(const bool purgeLoadedData);

			//! Load the playlist from music/playlist.txt
			void doLoadPlaylist();

			//! \brief Clear the playlist
			void doPurgePlaylist();

			//! \brief Get the next music to play in the playlist
			String doSelectNextMusic();

			//@}

			virtual uint32 InterfaceMsg(const uint32 MsgID, const String &msg);

		private:
			//! Mutex
			Mutex pMutex;
			//!
			TDFParser pTable;

			//! Is SDL_mixer running ?
			bool m_SDLMixerRunning;
			//! Are we in battle ?
			bool m_InBattle;
			//! Number of battle tunes
			sint32  pBattleTunesCount;
			//! The complete playlist
			Playlist  pPlaylist;

			Mix_Music   *pMusic;
            bool bPlayMusic;

			Mix_Chunk   *pBasicSound;

			//! Current index to play (-1 means `none`)
			sint32  pCurrentItemToPlay;
            //! Current index to play (-1 means `none`)
			sint32  pCurrentItemPlaying;
            //!
			uint32  pMinTicks;

			//!
			TA3D::UTILS::HashMap<SoundItemList*>::Dense pSoundList;
			//!
			WorkList pWorkList;	// List to store work to do when entering main thread
			//!
			sint32 fCounter;
		}; // class Manager




	} // namespace AUDIO

	namespace VARS
	{

# ifndef TA3D_NO_SOUND // Only for the hpiview program
		//! The sound manager
		extern TA3D::Audio::Manager::Ptr sound_manager;
# endif

	} // namespace VARS


} // namespace TA3D

#endif // __TA3D_SOUNDS_MANAGER_H__
