
#include "../core/math.h"
#include "../core/system/gettimeofday.h"
#include "emitter.h"
#include "../private/audio/av.h"
#include "../private/audio/openal.h"



namespace Yuni
{
namespace Audio
{
	const float Emitter::DefaultPitch = 1.0f;
	const float Emitter::DefaultGain = 1.0f;
	const bool Emitter::DefaultAttenuation = true;
	const bool Emitter::DefaultLooping = false;

	bool Emitter::attachBufferDispatched(Sound::Ptr& buffer)
	{
		// Check buffer validity
		if (!buffer || !buffer->valid())
		{
			std::cerr << "Invalid Buffer !" << std::endl;
			return false;
		}

		pBuffer = buffer;
		if (!pBuffer->prepareDispatched(pID))
		{
			std::cerr << "Failed loading buffers !" << std::endl;
			return false;
		}
		return true;
	}


	bool Emitter::playSoundDispatched()
	{
		if (!pBuffer)
			return false;

		std::cout << "Beginning playback on emitter " << pID << "..." << std::endl;
		pPlaying = Private::Audio::OpenAL::PlaySource(pID);
		if (!pPlaying)
		{
			std::cerr << "Emitter " << pID << " failed playing !" << std::endl;
			Private::Audio::OpenAL::UnqueueBufferFromSource(pID);
			return false;
		}
		// Store start time
		Yuni::timeval now;
		YUNI_SYSTEM_GETTIMEOFDAY(&now, NULL);
		pStartTime = now.tv_sec;

		std::cout << "Playback started successfully !" << std::endl;
		return true;
	}


	bool Emitter::playSoundDispatched(Sound::Ptr& buffer)
	{
		if (!pReady && !prepareDispatched())
			return false;

		if (!attachBufferDispatched(buffer))
			return false;

		return playSoundDispatched();
	}


	bool Emitter::updateDispatched()
	{
		if (!pReady)
			return false;
		pPlaying = Private::Audio::OpenAL::IsSourcePlaying(pID);
		if (!pPlaying)
			return false;
		if (pModified)
		{
			if (!Private::Audio::OpenAL::MoveSource(pID, pPosition, pVelocity, pDirection))
			{
				std::cerr << "Source position update failed !" << std::endl;
				return false;
			}
			if (!Private::Audio::OpenAL::ModifySource(pID, DefaultPitch, pGain, DefaultAttenuation, pLoop))
			{
				std::cerr << "Source characteristics update failed !" << std::endl;
				return false;
			}
		}
		if (pBuffer)
			pBuffer->updateDispatched(pID);
		return true;
	}


	bool Emitter::prepareDispatched()
	{
		if (pReady)
			return true;

		unsigned int source = Private::Audio::OpenAL::CreateSource(pPosition, pVelocity,
			pDirection, DefaultPitch, pGain, DefaultAttenuation, pLoop);

		pID = source;
		pReady = (source > 0);
		return pReady;
	}


	sint64 Emitter::elapsedTime() const
	{
		ThreadingPolicy::MutexLocker locker(*this);

		if (!pPlaying)
			return 0;
		Yuni::timeval now;
		YUNI_SYSTEM_GETTIMEOFDAY(&now, NULL);
		return now.tv_sec - pStartTime;
	}


} // namespace Audio
} // namespace Yuni
