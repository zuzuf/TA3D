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
#include "ingame/sidedata.h"
#include <fstream>
#include "misc/math.h"

using namespace TA3D::Interfaces;

# define TA3D_LOG_SECTION_AUDIO_PREFIX "[Audio] "

TA3D::Audio::Manager* TA3D::VARS::sound_manager;


namespace TA3D
{
namespace Audio
{



    Manager::Manager( const float DistanceFactor, const float DopplerFactor, const float RolloffFactor )
        :m_FMODRunning( false ), m_InBattle(false), pBattleTunesCount(0),
        pFMODMusicSound( NULL ), pFMODMusicchannel( NULL ),
        pCurrentItemToPlay(-1)
    {
        pMinTicks = 500;

        pBasicSound = NULL;
        pBasicChannel = NULL;
        doStartUpAudio();
        InitInterface();

        if (!m_FMODRunning)
            return;

        # ifdef TA3D_PLATFORM_MINGW
        FMOD_System_Set3DSettings(pFMODSystem, DopplerFactor, DistanceFactor, RolloffFactor);
        # else
        pFMODSystem->set3DSettings(DopplerFactor, DistanceFactor, RolloffFactor);
        # endif
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

        struct al_ffblk info;
        String search;
        search << GetClientPath() << "music/";

        for (Playlist::iterator i = pPlaylist.begin(); i != pPlaylist.end(); ++i)
            (*i)->checked = false;
        bool default_deactivation = !pPlaylist.empty();

        if (al_findfirst((search + "*").c_str(), &info, FA_ALL) == 0) // Add missing files
        {
            do
            {
                if (String::ToLower(info.name) == "playlist.txt" || info.name[0] == '.')
                    continue;

                String filename;
                filename << search << info.name;

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
                    LOG_DEBUG(TA3D_LOG_SECTION_AUDIO_PREFIX << "Added to the playlist: `" << filename << "`");
                    pPlaylist.push_back(m_Tune);
                }

            } while (!al_findnext(&info));
            al_findclose(&info);
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
        targetPlaylist << GetClientPath() << "music/playlist.txt";
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
        filename << GetClientPath() << "music/playlist.txt";
        std::ifstream file( filename.c_str(), std::ios::in);

        if (!file.is_open()) // try to create the list if it doesn't exist
        {
            doUpdatePlayListFiles();
            file.open(filename.c_str(), std::ios::in);
            if (!file.is_open())
            {
                LOG_WARNING(TA3D_LOG_SECTION_AUDIO_PREFIX << "Impossible to load the playlist");
                return;
            }
        }

        Console->AddEntry("%sLoading the playlist...", TA3D_LOG_SECTION_AUDIO_PREFIX);

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

            LOG_DEBUG(TA3D_LOG_SECTION_AUDIO_PREFIX << "Added to the playlist: `" << line << "`");
            pPlaylist.push_back(m_Tune);
        }

        file.close();
        doUpdatePlayListFiles();
        if (!pPlaylist.empty())
            doPlayMusic();
    }





    void Manager::doShutdownAudio(const bool purgeLoadedData)
    {
        if (m_FMODRunning) // only execute stop if we are running.
            doStopMusic();

        if (purgeLoadedData)
        {
            purgeSounds(); // purge sound list.
            doPurgePlaylist(); // purge play list
        }

        if (m_FMODRunning)
        {
#ifdef TA3D_PLATFORM_MINGW
            if (pBasicSound)
                FMOD_Sound_Release(pBasicSound);
#else
            if (pBasicSound)
                pBasicSound->release();
#endif
            pBasicSound = NULL;
            pBasicChannel = NULL;
#ifdef TA3D_PLATFORM_MINGW
            FMOD_System_Close(pFMODSystem);
            FMOD_System_Release(pFMODSystem);
#else
            pFMODSystem->close(); // Commented because crashes with some FMOD versions, and since we're going to end the program ...
            pFMODSystem->release();
#endif
            DeleteInterface();
            m_FMODRunning = false;
        }
    }




    bool Manager::doStartUpAudio()
    {
        uint32 FMODVersion;

        pFMODMusicSound = NULL;
        pFMODMusicchannel = NULL;
        fCounter = 0;

        if (m_FMODRunning)
            return true;

#ifdef TA3D_PLATFORM_MINGW
        if (FMOD_System_Create(&pFMODSystem) != FMOD_OK)
        {
            Console->AddEntryWarning("%s[FMOD] Failed to System_Create, sound disabled", TA3D_LOG_SECTION_AUDIO_PREFIX);
            return false;
        }

        if (FMOD_System_GetVersion(pFMODSystem, &FMODVersion) != FMOD_OK)
        {
            Console->AddEntryWarning("%s[FMOD] Invalid Version, sound disabled", TA3D_LOG_SECTION_AUDIO_PREFIX);
            return false;
        }

        LOG_INFO(TA3D_LOG_SECTION_AUDIO_PREFIX << "FMOD Version: " << ((FMODVersion & 0xFFFF0000) >> 16)
                 << "." << ((FMODVersion & 0xFF00) >> 8) << "." << (FMODVersion & 0xFF));

        if (FMOD_System_SetStreamBufferSize( pFMODSystem, 32768, FMOD_TIMEUNIT_RAWBYTES ) != FMOD_OK)
        {
            Console->AddEntryWarning("%s[FMOD] Failed to set Stream Buffer Size, sound disabled", TA3D_LOG_SECTION_AUDIO_PREFIX);
            return false;
        }

#ifndef TA3D_NO_SOUND
        // 32 channels, normal init, with 3d right handed.
        if (FMOD_System_Init( pFMODSystem, 32, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, 0 ) != FMOD_OK)
        {
            Console->AddEntryWarning("%s[FMOD] Failed to init FMOD, sound disabled", TA3D_LOG_SECTION_AUDIO_PREFIX);
            return false;
        }

        m_FMODRunning = true;
        doLoadPlaylist();
#endif
#else
        if (FMOD::System_Create(&pFMODSystem) != FMOD_OK)
        {
            Console->AddEntryWarning("%s[FMOD] Failed to System_Create, sound disabled", TA3D_LOG_SECTION_AUDIO_PREFIX);
            return false;
        }
        if (pFMODSystem->getVersion(&FMODVersion) != FMOD_OK)
        {
            Console->AddEntryWarning("%s[FMOD] Invalid Version of FMOD, sound disabled", TA3D_LOG_SECTION_AUDIO_PREFIX);
            return false;
        }

        LOG_INFO(TA3D_LOG_SECTION_AUDIO_PREFIX << "FMOD Version: " << ((FMODVersion & 0xFFFF0000) >> 16)
                 << "." << ((FMODVersion & 0xFF00) >> 8) << "." << (FMODVersion & 0xFF));

        if (pFMODSystem->setStreamBufferSize( 32768, FMOD_TIMEUNIT_RAWBYTES) != FMOD_OK)
        {
            Console->AddEntryWarning("%s[FMOD] Failed to set Stream Buffer Size, sound disabled", TA3D_LOG_SECTION_AUDIO_PREFIX);
            return false;
        }

#ifndef TA3D_NO_SOUND
#ifdef TA3D_PLATFORM_LINUX
        if (pFMODSystem->setOutput(FMOD_OUTPUTTYPE_ALSA) != FMOD_OK)
        {
            Console->AddEntryWarning("%s[FMOD] Failed to init FMOD, sound disabled", TA3D_LOG_SECTION_AUDIO_PREFIX);
            return false;
        }
#endif
        // 32 channels, normal init, with 3d right handed.
        if (pFMODSystem->init(32, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, 0) != FMOD_OK)
        {
            Console->AddEntryWarning("%s[FMOD] Failed to init FMOD, sound disabled", TA3D_LOG_SECTION_AUDIO_PREFIX);
            return false;
        }
        m_FMODRunning = true;
        doLoadPlaylist();
#endif
#endif
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
        if (m_FMODRunning && pFMODMusicSound != NULL)
        {
            # ifdef TA3D_PLATFORM_MINGW
            FMOD_Channel_Stop(pFMODMusicchannel);
            FMOD_Sound_Release(pFMODMusicSound);
            # else
            pFMODMusicchannel->stop();
            pFMODMusicSound->release();
            # endif
            pFMODMusicSound = NULL;
            pFMODMusicchannel = NULL;
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
        if (m_FMODRunning && pFMODMusicchannel != NULL)
        {
            # ifdef TA3D_PLATFORM_MINGW
            FMOD_BOOL paused;
            FMOD_Channel_GetPaused(pFMODMusicchannel, &paused);
            FMOD_Channel_SetPaused(pFMODMusicchannel, !paused);
            # else
            bool paused;
            pFMODMusicchannel->getPaused(&paused);
            pFMODMusicchannel->setPaused(!paused);
            # endif
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
        if (m_FMODRunning && pFMODMusicchannel != NULL)
        {
            # ifdef TA3D_PLATFORM_MINGW
            FMOD_Channel_SetPaused(pFMODMusicchannel, true);
            # else
            pFMODMusicchannel->setPaused(true);
            # endif
        }
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
                    szResult = (*cur)->filename;
                    break;
                }
                else
                {
                    if ((*cur)->battleTune) // Take the last one that can be taken if we try to go too far
                        szResult = (*cur)->filename;
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
                szResult = (*cur)->filename;
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

        if (!m_FMODRunning)
            return;

        if (!file_exists( filename.c_str() ,FA_RDONLY | FA_ARCH,NULL ) )
        {
            if (!filename.empty())
                Console->AddEntryWarning("%s [FMOD] Failed to find file: %s for play.", TA3D_LOG_SECTION_AUDIO_PREFIX, filename.c_str());
            return;
        }

#ifdef TA3D_PLATFORM_MINGW

        if (FMOD_System_CreateStream( pFMODSystem, filename.c_str(),
                                      FMOD_HARDWARE | FMOD_LOOP_OFF | FMOD_2D | FMOD_IGNORETAGS, 0,
                                      &pFMODMusicSound ) != FMOD_OK )
        {
            Console->AddEntryWarning("%s[FMOD] Failed to create stream. (%s)", TA3D_LOG_SECTION_AUDIO_PREFIX, filename.c_str() );
            return;
        }

        if (FMOD_System_playSound( pFMODSystem, FMOD_CHANNEL_FREE, pFMODMusicSound,
                                   false, &pFMODMusicchannel) != FMOD_OK )
        {
            Console->AddEntryWarning("%s[FMOD] Failed to playSound/stream.", TA3D_LOG_SECTION_AUDIO_PREFIX);
            return;
        }

        LOG_DEBUG(TA3D_LOG_SECTION_AUDIO_PREFIX << "[FMOD] Playing music file " << filename);
        FMOD_Channel_SetVolume( pFMODMusicchannel, 1.0f);

#else

        if (pFMODSystem->createStream( filename.c_str(),
                                          FMOD_HARDWARE | FMOD_LOOP_OFF | FMOD_2D | FMOD_IGNORETAGS, 0,
                                          &pFMODMusicSound ) != FMOD_OK )
        {
            Console->AddEntryWarning("%s[FMOD] Failed to create stream. (%s)", TA3D_LOG_SECTION_AUDIO_PREFIX, filename.c_str());
            return;
        }

        if (pFMODSystem->playSound(FMOD_CHANNEL_FREE, pFMODMusicSound, false, &pFMODMusicchannel) != FMOD_OK)
        {
            Console->AddEntryWarning("%s[FMOD] Failed to playSound/stream.", TA3D_LOG_SECTION_AUDIO_PREFIX);
            return;
        }

        LOG_DEBUG(TA3D_LOG_SECTION_AUDIO_PREFIX << "[FMOD] Playing music file " << filename);
        pFMODMusicchannel->setVolume(1.0f);
#endif
    }



    void Manager::playMusic()
    {
        pMutex.lock();
        doPlayMusic();
        pMutex.unlock();
    }

    void Manager::doPlayMusic()
    {
        if (!m_FMODRunning)
            return;

        if (pFMODMusicchannel != NULL)
        {
            # ifdef TA3D_PLATFORM_MINGW
            FMOD_BOOL paused;
            FMOD_Channel_GetPaused(pFMODMusicchannel, &paused);

            if (paused)
            {
                FMOD_Channel_SetPaused(pFMODMusicchannel, false);
                return;
            }
            # else
            bool paused;

            pFMODMusicchannel->getPaused(&paused);
            if (paused)
            {
                pFMODMusicchannel->setPaused(false);
                return;
            }
            # endif
        }
        doPlayMusic(doSelectNextMusic());
    }



    // Begin sound managing routines.
    void Manager::setListenerPos(const VECTOR3D& vec)
    {
        pMutex.lock();
        if (m_FMODRunning)
        {
            FMOD_VECTOR pos     = { vec.x, vec.y, vec.z };
            FMOD_VECTOR vel     = { 0, 0, 0 };
            FMOD_VECTOR forward = { 0.0f, 0.0f, 1.0f };
            FMOD_VECTOR up      = { 0.0f, 1.0f, 0.0f };

            # ifdef TA3D_PLATFORM_MINGW
            FMOD_System_Set3DListenerAttributes(pFMODSystem, 0, &pos, &vel, &forward, &up);
            # else
            pFMODSystem->set3DListenerAttributes(0, &pos, &vel, &forward, &up);
            # endif
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
        if (!m_FMODRunning)
        {
            pWorkList.clear();
            return;
        }

        # ifdef TA3D_PLATFORM_MINGW

        FMOD_System_Update(pFMODSystem);
        for (std::list<WorkListItem>::iterator i = pWorkList.begin(); i != pWorkList.end(); ++i)
        {
            FMOD_CHANNEL *ch;
            if (FMOD_System_playSound(pFMODSystem, FMOD_CHANNEL_FREE, i->sound->sampleHandle, true, &ch) != FMOD_OK)
            {
                continue;
            }
            if (i->sound->is3DSound)
            {
                FMOD_VECTOR pos = { i->vec->x, i->vec->y, i->vec->z };
                FMOD_VECTOR vel = { 0,0,0 };
                FMOD_Channel_Set3DAttributes( ch, &pos, &vel );
            }
            FMOD_Channel_SetPaused(ch, false);
        }

        # else // TA3D_PLATFORM_MINGW

        pFMODSystem->update();

        for (std::list< WorkListItem >::iterator i = pWorkList.begin() ; i != pWorkList.end() ; ++i)
        {
            FMOD::Channel *ch;
            if (pFMODSystem->playSound( FMOD_CHANNEL_FREE,
                                           i->sound->sampleHandle, true, &ch ) != FMOD_OK )
                continue;

            if (i->sound->is3DSound)
            {
                FMOD_VECTOR pos = { i->vec->x, i->vec->y, i->vec->z };
                FMOD_VECTOR vel = { 0, 0, 0 };
                ch->set3DAttributes(&pos, &vel);
            }

            ch->setPaused(false);
        }

        # endif // TA3D_PLATFORM_MINGW

        pWorkList.clear();
        if ((fCounter++) < 100)
            return;

        fCounter = 0;

        if (pFMODMusicchannel == NULL)
        {
            doPlayMusic();
            return;
        }

        # ifdef TA3D_PLATFORM_MINGW
        FMOD_BOOL playing;
        FMOD_Channel_IsPlaying(pFMODMusicchannel, &playing);
        # else // TA3D_PLATFORM_MINGW
        bool playing;
        pFMODMusicchannel->isPlaying(&playing);
        # endif // TA3D_PLATFORM_MINGW
        if (!playing)
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
        if (pBasicSound)
        {
            # ifdef TA3D_PLATFORM_MINGW
            FMOD_Sound_Release(pBasicSound);
            # else // TA3D_PLATFORM_MINGW
            pBasicSound->release();
            # endif // TA3D_PLATFORM_MINGW
        }
        
        pBasicSound = NULL;
        pBasicChannel = NULL;
        uint32 sound_file_size = 0;
        byte *data = HPIManager->PullFromHPI(filename, &sound_file_size);
        if (data)
        {
            FMOD_CREATESOUNDEXINFO exinfo;
            memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
            exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
            exinfo.length = sound_file_size;

#ifdef TA3D_PLATFORM_MINGW
            FMOD_System_CreateSound( pFMODSystem, (const char *)data, FMOD_SOFTWARE | FMOD_OPENMEMORY, &exinfo, &pBasicSound );
            FMOD_Sound_SetMode( pBasicSound, FMOD_LOOP_OFF );
            FMOD_System_playSound( pFMODSystem, FMOD_CHANNEL_FREE, pBasicSound, 0, &pBasicChannel);
#else
            pFMODSystem->createSound( (const char *)data, FMOD_SOFTWARE | FMOD_OPENMEMORY, &exinfo, &pBasicSound);
            pBasicSound->setMode(FMOD_LOOP_OFF);
            pFMODSystem->playSound( FMOD_CHANNEL_FREE, pBasicSound, 0, &pBasicChannel);
#endif
            delete[] data;
        }
    }


    void Manager::stopSoundFileNow()
    {
        pMutex.lock();
        # ifdef TA3D_PLATFORM_MINGW
        if (pBasicSound)
            FMOD_Sound_Release(pBasicSound);
        # else
        if (pBasicSound)
            pBasicSound->release();
        # endif

        pBasicSound = NULL;
        pBasicChannel = NULL;
        pMutex.unlock();
    }


    bool Manager::loadSound(const String& filename, const bool LoadAs3D, const float MinDistance, const float MaxDistance)
    {
        MutexLocker locker(pMutex);
        return doLoadSound(filename, LoadAs3D, MinDistance, MaxDistance);
    }

    bool Manager::doLoadSound(String filename, const bool LoadAs3D, const float MinDistance, const float MaxDistance)
    {
        filename.toLower();

        I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (char*)format("loading sound file %s\n",(char *)filename.c_str()).c_str(), NULL, NULL );

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
            filename = format("FMOD: LoadSound(%s), no such sound found in HPI.\n", filename.c_str());
            I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)filename.c_str(), NULL, NULL);
            return false;
        }

        SoundItemList* it = new SoundItemList(LoadAs3D);
        LOG_ASSERT(NULL != it);
        FMOD_CREATESOUNDEXINFO exinfo;
        memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.length = Length;

        # ifdef TA3D_PLATFORM_MINGW

        // Now get fmod to load the sample
        FMOD_RESULT FMODResult = FMOD_System_CreateSound( pFMODSystem, (const char*)data,
                                                          FMOD_HARDWARE | FMOD_OPENMEMORY | ( (LoadAs3D) ? FMOD_3D : FMOD_2D ),
                                                          &exinfo,
                                                          &it->sampleHandle);
        delete[] data;

        if (FMODResult != FMOD_OK) // ahh crap fmod couln't load it.
        {
            delete it;  // delete the sound.
            it = NULL;

            // log a message and return false;
            if (m_FMODRunning)
            {
                filename = format("FMOD: LoadSound(%s), Failed to construct sample.\n", filename.c_str());
                I_Msg( TA3D::TA3D_IM_DEBUG_MSG, (void *)filename.c_str(), NULL, NULL);
            }
            return false;
        }

        // if its a 3d Sound we need to set min/max distance.
        if (it->is3DSound)
            FMOD_Sound_Set3DMinMaxDistance(it->sampleHandle, MinDistance, MaxDistance);

        # else // TA3D_PLATFORM_MINGW

        // Now get fmod to load the sample
        FMOD_RESULT FMODResult = pFMODSystem->createSound( (const char *)data,
                                                              FMOD_HARDWARE | FMOD_OPENMEMORY | ( (LoadAs3D) ? FMOD_3D : FMOD_2D ),
                                                              &exinfo,
                                                              &it->sampleHandle);
        free(data); // we no longer need this.

        if (FMODResult != FMOD_OK) // ahh crap fmod couln't load it.
        {
            delete it;  // delete the sound.
            it = NULL;
            // log a message and return false;
            if (m_FMODRunning)
            {
                filename = format("FMOD: LoadSound(%s), Failed to construct sample.\n", filename.c_str());
                I_Msg(TA3D::TA3D_IM_DEBUG_MSG, (void *)filename.c_str(), NULL, NULL);
            }
            return false;
        }

        // if its a 3d Sound we need to set min/max distance.
        if (it->is3DSound)
            it->sampleHandle->set3DMinMaxDistance(MinDistance, MaxDistance);

        # endif // TA3D_PLATFORM_MINGW

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

        LOG_DEBUG(TA3D_LOG_SECTION_AUDIO_PREFIX << "Reading `" << filename << "`...");
        // Load the TDF file
        pTable.load(filename);
        Console->AddEntry("%sLoading sounds from %s", TA3D_LOG_SECTION_AUDIO_PREFIX, filename.c_str());
        // Load each sound file
        pTable.forEach(LoadAllTDFSound(*this));
        LOG_DEBUG(TA3D_LOG_SECTION_AUDIO_PREFIX << "Reading: Done.");
        pMutex.unlock();
    }


    void Manager::purgeSounds()
    {
        pMutex.lock();
        pSoundList.emptyHashTable();
        pTable.clear();
        pWorkList.clear();
        pMutex.unlock();
    }



    // Play sound directly from our sound pool
    void Manager::playSound(const String& filename, const VECTOR3D* vec)
    {
        MutexLocker locker(pMutex);
        if (vec && Camera::inGame && ((VECTOR)(*vec - Camera::inGame->rpos)).sq() > 360000.0f) // If the source is too far, does not even think about playing it!
            return;
        if (!m_FMODRunning)
            return;

        String szWav(filename); // copy string to szWav so we can work with it.
        // if it has a .wav extension then remove it.
        String::size_type i = szWav.toLower().find(".wav");
        if (i != String::npos)
            szWav.resize(szWav.length() - 4);

        SoundItemList* sound = pSoundList.find(szWav);
        if (!sound)
        {
            Console->AddEntryWarning("%s`%s` not found, aborting", TA3D_LOG_SECTION_AUDIO_PREFIX, filename.c_str());
            return;
        }

        if (msec_timer - sound->lastTimePlayed < pMinTicks)
            return; // Make sure it doesn't play too often, so it doesn't play too loud!

        sound->lastTimePlayed = msec_timer;

        if (!sound->sampleHandle || (sound->is3DSound && !vec))
        {
            if (!sound->sampleHandle)
                Console->AddEntryWarning("%s`%s` not played the good way", TA3D_LOG_SECTION_AUDIO_PREFIX, (char*)filename.c_str());
            else
                Console->AddEntryWarning("%s`%s` : sound->sampleHandle is false", TA3D_LOG_SECTION_AUDIO_PREFIX, (char*)filename.c_str());
            return;
        }

        pWorkList.push_back(WorkListItem(sound, (VECTOR *)vec));
    }



    void Manager::playTDFSoundNow(const String& Key, const VECTOR3D* vec)
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


    void Manager::playTDFSound(const String& key, const VECTOR3D* vec)
    {
        pMutex.lock();
        doPlayTDFSound(key, vec);
        pMutex.unlock();
    }


    void Manager::doPlayTDFSound(String key, const VECTOR3D* vec)
    {
        if (!key.empty())
        {
            if (!pTable.exists(key.toLower()))
            {
                // output a report to the console but only once
                Console->AddEntryWarning("%sCan't find key %s", TA3D_LOG_SECTION_AUDIO_PREFIX, key.c_str());
                pTable.insertOrUpdate(key, "");
                return;
            }
            String wav = pTable.pullAsString(key);
            if (!wav.empty())
                playSound(wav, vec);
        }
    }


    void Manager::doPlayTDFSound(const String& keyA, const String& keyB, const VECTOR3D* vec)
    {
        if (!keyA.empty() && !keyB.empty())
        {
            String key;
            key << keyA << "." << keyB;
            doPlayTDFSound(key, vec);
        }
    }

    void Manager::playTDFSound(const String& keyA, const String& keyB, const VECTOR3D* vec)
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
        # ifdef TA3D_PLATFORM_MINGW
        if (sampleHandle)
            FMOD_Sound_Release(sampleHandle);
        # else
        if (sampleHandle)
            sampleHandle->release();
        # endif
    }

} // namespace Interfaces
} // namespace TA3D
