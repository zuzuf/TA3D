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

#include "../stdafx.h"
#include "../ta3dbase.h"			// just to use the global camera object
#include "../ingame/sidedata.h"
#include <fstream>
#include "../misc/math.h"
#include "manager.h"
#include "../logs/logs.h"
#include "../misc/camera.h"
#include "../misc/paths.h"


using namespace TA3D::Interfaces;



TA3D::Audio::Manager* TA3D::VARS::sound_manager;




namespace TA3D
{
    namespace Audio
    {



        Manager::Manager()
            :m_SDLMixerRunning( false ), m_InBattle(false), pBattleTunesCount(0),
            pMusic( NULL ), pBasicSound( NULL ),
            pCurrentItemToPlay(-1)
        {
            pMinTicks = 500;

            doStartUpAudio();
            InitInterface();

            if (!m_SDLMixerRunning)
                return;
        }




        void Manager::setPlayListFileMode(const int idx, bool inBattle, bool disabled)
        {
            if (idx < 0 || idx >= (int)pPlaylist.size())
                return;

            inBattle &= !disabled;
            if (pPlaylist[idx]->battleTune && !inBattle)
                --pBattleTunesCount;
            else
                if (!pPlaylist[idx]->battleTune && inBattle)
                    ++pBattleTunesCount;

            pPlaylist[idx]->battleTune = inBattle;
            pPlaylist[idx]->disabled = disabled;
        }



        bool Manager::getPlayListFiles(String::Vector& out)
        {
            out.resize(pPlaylist.size());
            int indx(0);
            for (String::Vector::iterator i = out.begin(); i != out.end(); ++i, ++indx)
            {
                i->clear();
                if (pPlaylist[indx]->battleTune)
                    *i << "[B] " << pPlaylist[indx]->filename;
                else
                {
                    if (pPlaylist[indx]->disabled)
                        *i << "[ ] " << pPlaylist[indx]->filename;
                    else
                        *i << "[*] " << pPlaylist[indx]->filename;
                }
            }
            return !out.empty();
        }



        void Manager::updatePlayListFiles()
        {
            pMutex.lock();
            doUpdatePlayListFiles();
            pMutex.unlock();
        }

        void Manager::doUpdatePlayListFiles()
        {
            MutexLocker locker(pMutex);

            String search;
            search << TA3D::Paths::Resources << "music/*";

            String::List file_list;
            Paths::GlobFiles( file_list, search, false, true);

            file_list.sort();

            for (Playlist::iterator i = pPlaylist.begin(); i != pPlaylist.end(); ++i)
                (*i)->checked = false;
            bool default_deactivation = !pPlaylist.empty();

            for(String::List::iterator i = file_list.begin() ; i != file_list.end() ; ++i) // Add missing files
            {
                if (String::ToLower(*i) == "playlist.txt" || (*i)[0] == '.')
                    continue;

                String filename;
                filename << *i;

                Playlist::const_iterator i;
                for (i = pPlaylist.begin(); i != pPlaylist.end(); ++i)
                {
                    if ((*i)->filename == filename)
                    {
                        (*i)->checked = true;
                        break;
                    }
                }

                if (i == pPlaylist.end()) // It's missing, add it
                {
                    PlaylistItem *m_Tune = new PlaylistItem();
                    m_Tune->battleTune = false;
                    m_Tune->disabled = default_deactivation;
                    m_Tune->checked = true;
                    m_Tune->filename = filename;
                    LOG_DEBUG(LOG_PREFIX_SOUND << "Added to the playlist: `" << filename << "`");
                    pPlaylist.push_back(m_Tune);
                }
            }

            int e = 0;
            for (unsigned int i = 0 ; i + e < pPlaylist.size() ; ) // Do some cleaning
            {
                if (pPlaylist[i + e]->checked)
                {
                    pPlaylist[i] = pPlaylist[i + e];
                    ++i;
                }
                else
                {
                    delete pPlaylist[i + e];
                    ++e;
                }
            }

            pPlaylist.resize(pPlaylist.size() - e);	// Remove missing files
            doSavePlaylist();
        }


        void Manager::savePlaylist()
        {
            pMutex.lock();
            doSavePlaylist();
            pMutex.unlock();
        }

        void Manager::doSavePlaylist()
        {
            String targetPlaylist;
            targetPlaylist << TA3D::Paths::Resources << "music/playlist.txt";
            std::ofstream play_list_file(targetPlaylist.c_str(), std::ios::out | std::ios::trunc);
            if (!play_list_file.is_open())
                return;

            play_list_file << "#this file has been generated by TA3D_Audio module\n";
            for (Playlist::const_iterator i = pPlaylist.begin(); i != pPlaylist.end(); ++i)
            {
                if ((*i)->battleTune)
                    play_list_file << "*" << (*i)->filename << "\n";
                else
                {
                    if ((*i)->disabled)
                        play_list_file << "!" << (*i)->filename << "\n";
                    else
                        play_list_file << (*i)->filename << "\n";
                }
            }
            play_list_file.flush();
            play_list_file.close();
        }




        void Manager::doLoadPlaylist()
        {
            String filename;
            filename << TA3D::Paths::Resources << "music/playlist.txt";
            std::ifstream file( filename.c_str(), std::ios::in);

            if (!file.is_open()) // try to create the list if it doesn't exist
            {
                doUpdatePlayListFiles();
                file.open(filename.c_str(), std::ios::in);
                if (!file.is_open())
                {
                    LOG_WARNING(LOG_PREFIX_SOUND << "Impossible to load the playlist : '" << filename << "'");
                    return;
                }
            }

            LOG_INFO(LOG_PREFIX_SOUND << "Loading the playlist...");

            String line;
            bool isBattle(false);
            bool isActivated(true);

            pBattleTunesCount = 0;

            while (!file.eof())
            {
                std::getline(file, line, '\n');

                line = String::Trim(line); // strip off spaces, linefeeds, tabs, newlines

                if (!line.length())
                    continue;
                if (line[0] == '#' || line[0] == ';')
                    continue;

                isActivated = true;

                if (line[0] == '*')
                {
                    isBattle = true;
                    line = line.erase(0, 1);
                    ++pBattleTunesCount;
                }
                else
                {
                    if (line[0] == '!')
                        isActivated = false;
                    else
                        isBattle = false;
                }

                PlaylistItem* m_Tune = new PlaylistItem();
                m_Tune->battleTune = isBattle;
                m_Tune->disabled = !isActivated;
                m_Tune->filename = line;

                LOG_DEBUG(LOG_PREFIX_SOUND << "Added to the playlist: `" << line << "`");
                pPlaylist.push_back(m_Tune);
            }

            file.close();
            doUpdatePlayListFiles();
        }





        void Manager::doShutdownAudio(const bool purgeLoadedData)
        {
            if (m_SDLMixerRunning) // only execute stop if we are running.
                doStopMusic();

            if (purgeLoadedData)
            {
                purgeSounds(); // purge sound list.
                doPurgePlaylist(); // purge play list
            }

            if (m_SDLMixerRunning)
            {
                Mix_AllocateChannels(0);

                Mix_CloseAudio();
                DeleteInterface();
                m_SDLMixerRunning = false;
                pMusic = NULL;
            }

            SDL_QuitSubSystem( SDL_INIT_AUDIO );
        }




        bool Manager::doStartUpAudio()
        {
            pMusic = NULL;
            fCounter = 0;

            if (m_SDLMixerRunning)
                return true;

            if (!SDL_WasInit(SDL_INIT_AUDIO))
            {
                if (SDL_InitSubSystem( SDL_INIT_AUDIO ))
                {
                LOG_ERROR(LOG_PREFIX_SOUND << "SDL_InitSubSystem( SDL_INIT_AUDIO ) failed: " << SDL_GetError());
                return false;
                }
            }

            // 44.1KHz, signed 16bit, system byte order,
            // stereo, 4096 bytes for chunks
            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
            {
                LOG_ERROR(LOG_PREFIX_SOUND << "Mix_OpenAudio: " << Mix_GetError());
                return false;
            }

            nbChannels = 16;
            Mix_AllocateChannels(nbChannels);

            SDL_version compiled_version;
            const SDL_version *linked_version;
            MIX_VERSION(&compiled_version);
            LOG_DEBUG(LOG_PREFIX_SOUND << "compiled with SDL_mixer version: " << (int)compiled_version.major << "." << (int)compiled_version.minor << "." << (int)compiled_version.patch);
            linked_version = Mix_Linked_Version();
            LOG_DEBUG(LOG_PREFIX_SOUND << "running with SDL_mixer version: " << (int)linked_version->major << "." << (int)linked_version->minor << "." << (int)linked_version->patch);

            m_SDLMixerRunning = true;
            doLoadPlaylist();
            return true;
        }



        Manager::~Manager()
        {
            doShutdownAudio(true);
        }



        void Manager::stopMusic()
        {
            pMutex.lock();
            doStopMusic();
            pMutex.unlock();
        }

        void Manager::doStopMusic()
        {
            if (m_SDLMixerRunning && pMusic != NULL)
            {
                Mix_HaltMusic();
                Mix_FreeMusic(pMusic);
                pMusic = NULL;
            }
        }




        void Manager::doPurgePlaylist()
        {
            pMutex.lock();
            doStopMusic();
            pCurrentItemToPlay = -1;
            // we don't change this in stop music in case
            // we want to do a play and contine through our list, so
            // we change it here to refelect no index.

            if (!pPlaylist.empty())
            {
                for (Playlist::iterator k_Pos = pPlaylist.begin(); k_Pos != pPlaylist.end(); ++k_Pos)
                    delete *k_Pos ;
                pPlaylist.clear();
            }
            pMutex.unlock();
        }




        void Manager::togglePauseMusic()
        {
            pMutex.lock();
            if (m_SDLMixerRunning && pMusic != NULL)
            {
                if (Mix_PausedMusic())
                    Mix_PauseMusic();
                else
                    Mix_ResumeMusic();
            }
            pMutex.unlock();
        }



        void Manager::pauseMusic()
        {
            pMutex.lock();
            doPauseMusic();
            pMutex.unlock();
        }

        void Manager::doPauseMusic()
        {
            if (m_SDLMixerRunning && pMusic != NULL)
                Mix_PauseMusic();
        }




        String Manager::doSelectNextMusic()
        {
            if (pPlaylist.empty())
                return "";

            sint16 cIndex = -1;
            sint16 mCount = 0;
            String szResult;
            if (m_InBattle && pBattleTunesCount > 0)
            {
                srand((unsigned)time(NULL));
                cIndex =  (sint16)(TA3D_RAND() % pBattleTunesCount) + 1;
                mCount = 1;

                for (Playlist::const_iterator cur = pPlaylist.begin(); cur != pPlaylist.end(); ++cur)
                {
                    if ((*cur)->battleTune && mCount >= cIndex)		// If we get one that match our needs we take it
                    {
                        szResult = TA3D::Paths::Resources + "music/" + (*cur)->filename;
                        break;
                    }
                    else
                    {
                        if ((*cur)->battleTune) // Take the last one that can be taken if we try to go too far
                            szResult = TA3D::Paths::Resources + "music/" + (*cur)->filename;
                    }
                }
                return szResult;
            }

            mCount = 0;
            if (pCurrentItemToPlay > (sint32)pPlaylist.size())
                pCurrentItemToPlay = -1;

            bool found = false;

            for (Playlist::const_iterator cur = pPlaylist.begin(); cur != pPlaylist.end(); ++cur)
            {
                ++mCount;
                if ((*cur)->battleTune || (*cur)->disabled)
                    continue;

                if (pCurrentItemToPlay <= mCount || pCurrentItemToPlay <= 0)
                {
                    szResult = TA3D::Paths::Resources + "music/" + (*cur)->filename;
                    pCurrentItemToPlay = mCount + 1;
                    found = true;
                    break;
                }
            }
            if (!found && pCurrentItemToPlay != -1)
            {
                pCurrentItemToPlay = -1;
                return doSelectNextMusic();
            }
            return szResult;
        }




        void Manager::setMusicMode(const bool battleMode)
        {
            pMutex.lock();
            if (m_InBattle != battleMode)
            {
                m_InBattle = battleMode;
                doPlayMusic();
            }
            pMutex.unlock();
        }




        void Manager::doPlayMusic(const String& filename)
        {
            doStopMusic();

            if (!m_SDLMixerRunning)
                return;

            if (!exists(filename))
            {
                if (!filename.empty())
                    LOG_ERROR(LOG_PREFIX_SOUND << "Failed to find file: `" << filename << "`");
                return;
            }

            pMusic = Mix_LoadMUS( filename.c_str() );

            if (pMusic == NULL)
            {
                LOG_ERROR(LOG_PREFIX_SOUND << "Failed to open music file : `" << filename << "` (" << Mix_GetError() << ")");
                return;
            }

            if (Mix_PlayMusic(pMusic, 0) == -1)
            {
                LOG_ERROR(LOG_PREFIX_SOUND << "Failed to play music file : `" << filename << "` (" << Mix_GetError() << ")");
                return;
            }

            LOG_DEBUG(LOG_PREFIX_SOUND << "Playing music file " << filename);
            Mix_VolumeMusic( MIX_MAX_VOLUME );
        }



        void Manager::playMusic()
        {
            pMutex.lock();
            doPlayMusic();
            pMutex.unlock();
        }

        void Manager::doPlayMusic()
        {
            if (!m_SDLMixerRunning)
                return;

            if (pMusic != NULL)
            {
                if (Mix_PausedMusic())
                {
                    Mix_ResumeMusic();
                    return;
                }
            }
            if (pMusic == NULL || !Mix_PlayingMusic())
                doPlayMusic(doSelectNextMusic());
        }



        // Begin sound managing routines.
        void Manager::setListenerPos(const Vector3D& vec)
        {
            pMutex.lock();
            if (m_SDLMixerRunning)
            {
#warning TODO: implement 3D stereo
                //            FMOD_VECTOR pos     = { vec.x, vec.y, vec.z };
                //            FMOD_VECTOR vel     = { 0, 0, 0 };
                //            FMOD_VECTOR forward = { 0.0f, 0.0f, 1.0f };
                //            FMOD_VECTOR up      = { 0.0f, 1.0f, 0.0f };
                //
                //            pFMODSystem->set3DListenerAttributes(0, &pos, &vel, &forward, &up);
            }
            pMutex.unlock();
        }

        void Manager::update3DSound()
        {
            pMutex.lock();
            doUpdate3DSound();
            pMutex.unlock();
        }

        void Manager::doUpdate3DSound()
        {
            if (!m_SDLMixerRunning)
            {
                pWorkList.clear();
                return;
            }

#warning TODO: implement 3D stereo

            //        pFMODSystem->update();

            for (std::list< WorkListItem >::iterator i = pWorkList.begin() ; i != pWorkList.end() ; ++i)
            {
                if (Mix_PlayChannel(-1, i->sound->sampleHandle, 0) == -1)
                    continue;

                if (i->sound->is3DSound)
                {
                    //                FMOD_VECTOR pos = { i->vec->x, i->vec->y, i->vec->z };
                    //                FMOD_VECTOR vel = { 0, 0, 0 };
                    //                ch->set3DAttributes(&pos, &vel);
                }
            }

            pWorkList.clear();
            if ((fCounter++) < 100)
                return;

            fCounter = 0;

            if (pMusic == NULL)
            {
                doPlayMusic();
                return;
            }

            if (!Mix_PlayingMusic())
                doPlayMusic();
        }

        uint32 Manager::InterfaceMsg(const lpcImsg msg)
        {
            if (msg->MsgID == TA3D_IM_GUI_MSG)	// for GUI messages, test if it's a message for us
            {
                if (msg->lpParm1 == NULL)
                    return INTERFACE_RESULT_HANDLED; // Oups badly written things

                // Get the string associated with the signal
                String message((char*)msg->lpParm1);
                message.toLower();

                if (message == "music play")
                {
                    doPlayMusic();
                    return INTERFACE_RESULT_HANDLED;
                }
                if (message == "music pause")
                {
                    doPauseMusic();
                    return INTERFACE_RESULT_HANDLED;
                }
                if (message == "music stop")
                {
                    doStopMusic();
                    return INTERFACE_RESULT_HANDLED;
                }
            }
            return INTERFACE_RESULT_CONTINUE;
        }




        void Manager::playSoundFileNow(const String& filename)
        {
            stopSoundFileNow();

            uint32 sound_file_size = 0;
            byte *data = HPIManager->PullFromHPI(filename, &sound_file_size);
            if (data)
            {
                pBasicSound = Mix_LoadWAV_RW( SDL_RWFromMem(data, sound_file_size), 1);
                delete[] data;
                if (pBasicSound == NULL)
                {
                    LOG_ERROR(LOG_PREFIX_SOUND << "error loading file `" << filename << "` (" << Mix_GetError() << ")");
                    return;
                }
                Mix_PlayChannel(-1, pBasicSound, 0);
            }
        }


        void Manager::stopSoundFileNow()
        {
            pMutex.lock();
            if (pBasicSound)
            {
                for(int i = 0 ; i < nbChannels ; i++)
                    if (Mix_GetChunk(i) == pBasicSound)
                        Mix_HaltChannel(i);
                Mix_FreeChunk(pBasicSound);
            }

            pBasicSound = NULL;
            pMutex.unlock();
        }


        bool Manager::loadSound(const String& filename, const bool LoadAs3D, const float MinDistance, const float MaxDistance)
        {
            MutexLocker locker(pMutex);
            return doLoadSound(filename, LoadAs3D, MinDistance, MaxDistance);
        }

        bool Manager::doLoadSound(String filename, const bool LoadAs3D, const float MinDistance, const float MaxDistance)
        {
            if (filename.empty())       // We can't load a file with an empty name
                return false;
            filename.toLower();

            //        LOG_DEBUG(LOG_PREFIX_SOUND << "loading sound file " << filename);

            // if it has a .wav extension then remove it.
            String::size_type i = filename.find("wav");
            if (i != String::npos)
                filename.resize(filename.length() - 4);

            // if its already loaded return true.
            if (pSoundList.exists(filename))
            {
                //I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (char*)format("sound file %s is already loaded\n",(char *)filename.c_str()).c_str(), NULL, NULL );
                return true;
            }

            // pull the data from hpi.
            String theSound;
            uint32 Length;
            theSound << "sounds\\" << filename << ".wav";
            byte* data = HPIManager->PullFromHPI(theSound, &Length);
            if (!data) // if no data, log a message and return false.
            {
                LOG_DEBUG( LOG_PREFIX_SOUND << "Manager: LoadSound(" << filename << "), no such sound found in HPI.");
                return false;
            }

            SoundItemList* it = new SoundItemList(LoadAs3D);
            LOG_ASSERT(NULL != it);

            // Now get SDL_mixer to load the sample
            it->sampleHandle = Mix_LoadWAV_RW( SDL_RWFromMem(data, Length), 1 );
            delete[] data; // we no longer need this.

            if (it->sampleHandle == NULL) // ahh crap SDL_mixer couln't load it.
            {
                delete it;  // delete the sound.
                it = NULL;
                // log a message and return false;
                if (m_SDLMixerRunning)
                    LOG_DEBUG( LOG_PREFIX_SOUND << "Manager: LoadSound(" << filename << "), Failed to construct sample.");
                return false;
            }

            // if its a 3d Sound we need to set min/max distance.
#warning TODO: implement 3D stereo
            //        if (it->is3DSound)
            //            it->sampleHandle->set3DMinMaxDistance(MinDistance, MaxDistance);

            // add the sound to our soundlist hash table, and return true.
            pSoundList.insertOrUpdate(filename, it);
            return true;
        }


        void Manager::loadTDFSounds(const bool allSounds)
        {
            pMutex.lock();
            // Which file to load ?
            String filename(ta3dSideData.gamedata_dir);
            filename += (allSounds) ? "allsound.tdf" : "sound.tdf";

            LOG_DEBUG(LOG_PREFIX_SOUND << "Reading `" << filename << "`...");
            // Load the TDF file
            if (pTable.loadFromFile(filename))
            {
                LOG_INFO(LOG_PREFIX_SOUND << "Loading sounds from " << filename);
                // Load each sound file
                pTable.forEach(LoadAllTDFSound(*this));
                LOG_DEBUG(LOG_PREFIX_SOUND << "Reading: Done.");
            }
            else
                LOG_DEBUG(LOG_PREFIX_SOUND << "Reading: Aborted.");
            pMutex.unlock();
        }


        void Manager::purgeSounds()
        {
            pMutex.lock();

            Mix_HaltChannel(-1);

            pSoundList.emptyHashTable();
            pTable.clear();
            pWorkList.clear();
            pMutex.unlock();
        }



        // Play sound directly from our sound pool
        void Manager::playSound(const String& filename, const Vector3D* vec)
        {
            if (filename.empty())
                return;

            MutexLocker locker(pMutex);
            if (vec && Camera::inGame && ((Vector3D)(*vec - Camera::inGame->rpos)).sq() > 360000.0f) // If the source is too far, does not even think about playing it!
                return;
            if (!m_SDLMixerRunning)
                return;

            String szWav(filename); // copy string to szWav so we can work with it.
            // if it has a .wav extension then remove it.
            String::size_type i = szWav.toLower().find(".wav");
            if (i != String::npos)
                szWav.resize(szWav.length() - 4);

            SoundItemList* sound = pSoundList.find(szWav);
            if (!sound)
            {
                LOG_ERROR(LOG_PREFIX_SOUND << "`" << filename << "` not found, aborting");
                return;
            }

            if (msec_timer - sound->lastTimePlayed < pMinTicks)
                return; // Make sure it doesn't play too often, so it doesn't play too loud!

            sound->lastTimePlayed = msec_timer;

            if (!sound->sampleHandle || (sound->is3DSound && !vec))
            {
                if (!sound->sampleHandle)
                    LOG_ERROR(LOG_PREFIX_SOUND << "`" << filename << "` not played the good way");
                else
                    LOG_ERROR(LOG_PREFIX_SOUND << "`" << filename << "` sound->sampleHandle is false");
                return;
            }

            pWorkList.push_back(WorkListItem(sound, (Vector3D*)vec));
        }



        void Manager::playTDFSoundNow(const String& Key, const Vector3D* vec)
        {
            pMutex.lock();
            String szWav = pTable.pullAsString(String::ToLower(Key)); // copy string to szWav so we can work with it.
            String::size_type i = szWav.toLower().find(".wav");
            if (i != String::npos)
                szWav.resize(szWav.length() - 4);

            SoundItemList* it = pSoundList.find(szWav);
            if (it)
            {
                it->lastTimePlayed = msec_timer - 1000 - pMinTicks; // Make sure it'll be played
                doPlayTDFSound(Key, vec);
            }
            doUpdate3DSound();
            pMutex.unlock();
        }


        void Manager::playTDFSound(const String& key, const Vector3D* vec)
        {
            pMutex.lock();
            doPlayTDFSound(key, vec);
            pMutex.unlock();
        }


        void Manager::doPlayTDFSound(String key, const Vector3D* vec)
        {
            if (!key.empty())
            {
                if (!pTable.exists(key.toLower()))
                {
                    // output a report to the console but only once
                    LOG_WARNING(LOG_PREFIX_SOUND << "Can't find key `" << key << "`");
                    pTable.insertOrUpdate(key, "");
                    return;
                }
                String wav = pTable.pullAsString(key);
                if (!wav.empty())
                    playSound(wav, vec);
            }
        }


        void Manager::doPlayTDFSound(const String& keyA, const String& keyB, const Vector3D* vec)
        {
            if (!keyA.empty() && !keyB.empty())
            {
                String key;
                key << keyA << "." << keyB;
                doPlayTDFSound(key, vec);
            }
        }

        void Manager::playTDFSound(const String& keyA, const String& keyB, const Vector3D* vec)
        {
            if (!keyA.empty() && !keyB.empty())
            {
                String key;
                key << keyA << "." << keyB;
                playTDFSound(key, vec);
            }
        }


        Manager::SoundItemList::~SoundItemList()
        {
            if (sampleHandle)
            {
                for(int i = 0 ; i < sound_manager->nbChannels ; i++)
                    if (Mix_GetChunk(i) == sampleHandle)
                        Mix_HaltChannel(i);
                Mix_FreeChunk(sampleHandle);
            }
            sampleHandle = NULL;
        }

    } // namespace Interfaces
} // namespace TA3D
