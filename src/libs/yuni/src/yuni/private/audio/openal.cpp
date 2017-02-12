# include <cstdlib>
# include <iostream>
# include "openal.h"

namespace Yuni
{
namespace Private
{
namespace Audio
{

	bool OpenAL::Init()
	{
		ALCdevice* device = alcOpenDevice(NULL);
		if (!device)
			return false;
		ALCcontext* context = alcCreateContext(device, NULL);
		if (!context)
			return false;
		alcMakeContextCurrent(context);

		// Set the listener at (0,0,0)
		ALfloat listenerPos[3] = {0.0,0.0,0.0};
		// Set the listener's velocity to 0
		ALfloat listenerVel[3] = {0.0,0.0,0.0};
		// Listener is oriented towards the screen (-Z), with the Up along Y
		ALfloat listenerOri[6] = {0.0,0.0,-1.0, 0.0,1.0,0.0};
		SetListener(listenerPos, listenerVel, listenerOri);

		// Clear errors
		alGetError();
		return true;
	}

	bool OpenAL::Close()
	{
		ALCcontext* context = alcGetCurrentContext();
		ALCdevice* device = alcGetContextsDevice(context);
		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);
		return true;
	}

	ALenum OpenAL::GetFormat(unsigned int bits, unsigned int channels)
	{
		switch (bits)
		{
		case 8:
			if (channels == 1)
				return AL_FORMAT_MONO8;
			if (channels == 2)
				return AL_FORMAT_STEREO8;
			if (alIsExtensionPresent("AL_EXT_MCFORMATS"))
			{
				if (channels == 4)
					return alGetEnumValue("AL_FORMAT_QUAD8");
				if (channels == 6)
					return alGetEnumValue("AL_FORMAT_51CHN8");
			}
			break;
		case 16:
			if (channels == 1)
				return AL_FORMAT_MONO16;
			if (channels == 2)
				return AL_FORMAT_STEREO16;
			if (alIsExtensionPresent("AL_EXT_MCFORMATS"))
			{
				if (channels == 4)
					return alGetEnumValue("AL_FORMAT_QUAD16");
				if (channels == 6)
					return alGetEnumValue("AL_FORMAT_51CHN16");
			}
			break;
		}
		return 0;
	}

	void OpenAL::SetDistanceModel(DistanceModel model)
	{
		ALenum modelName;
		switch (model)
		{
		case None:
			modelName = AL_NONE;
			break;
		case InverseDistance:
			modelName = AL_INVERSE_DISTANCE;
			break;
		case InverseDistanceClamped:
			modelName = AL_INVERSE_DISTANCE_CLAMPED;
			break;
		case LinearDistance:
			modelName = AL_LINEAR_DISTANCE;
			break;
		case LinearDistanceClamped:
			modelName = AL_LINEAR_DISTANCE_CLAMPED;
			break;
		case ExponentDistance:
			modelName = AL_EXPONENT_DISTANCE;
			break;
		case ExponentDistanceClamped:
			modelName = AL_EXPONENT_DISTANCE_CLAMPED;
			break;
		default:
			modelName = AL_INVERSE_DISTANCE_CLAMPED;
			break;
		}
		alDistanceModel(modelName);
	}

	bool OpenAL::CreateBuffers(int nbBuffers, unsigned int* buffers)
	{
		alGetError();
		alGenBuffers(nbBuffers, buffers);
		return alGetError() == AL_NO_ERROR;
	}

	void OpenAL::DestroyBuffers(int nbBuffers, unsigned int* buffers)
	{
		alDeleteBuffers(nbBuffers, buffers);
	}

	void OpenAL::SetListener(float position[3], float velocity[3], float orientation[6])
	{
		alListenerfv(AL_POSITION, position);
		alListenerfv(AL_VELOCITY, velocity);
		alListenerfv(AL_ORIENTATION, orientation);
	}

	unsigned int OpenAL::CreateSource(Point3D<> position, Vector3D<> velocity,
		Vector3D<> direction, float pitch, float gain, bool attenuate, bool loop)
	{
		alGetError();
		unsigned int source;
		alGenSources(1, &source);
		if (alGetError() != AL_NO_ERROR)
			return 0;

		UnbindBufferFromSource(source);

		alSourcef(source, AL_MIN_GAIN, 0.0f); // Allow the sound to fade to nothing
		alSourcef(source, AL_MAX_GAIN, 1.5f); // Max amplification
		alSourcef(source, AL_MAX_DISTANCE, 10000.0f);

		if (!MoveSource(source, position, velocity, direction))
		{
			DestroySource(source);
			return 0;
		}
		if (!ModifySource(source, pitch, gain, attenuate, loop))
		{
			DestroySource(source);
			return 0;
		}
		return source;
	}

	void OpenAL::DestroySource(unsigned int source)
	{
		alDeleteSources(1, &source);
	}

	bool OpenAL::PlaySource(ALuint source)
	{
		alGetError();
		alSourceRewind(source);
		alSourcePlay(source);
		return alGetError() == AL_NO_ERROR;
	}

	bool OpenAL::IsSourcePlaying(unsigned int source)
	{
		ALint state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		return state == AL_PLAYING;
	}

	bool OpenAL::ModifySource(unsigned int source, float pitch, float gain,
		bool attenuate, bool loop)
	{
		alGetError();
		alSourcef(source, AL_PITCH, pitch);
		alSourcef(source, AL_GAIN, gain);
		alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
		alSourcef(source, AL_ROLLOFF_FACTOR, attenuate ? 1.0f : 0.0f);
		return alGetError() == AL_NO_ERROR;
	}

	bool OpenAL::MoveSource(unsigned int source, const Point3D<>& position,
		const Vector3D<>& velocity, const Vector3D<>& direction)
	{
		// Uncomment this if you want the position / velocity / cone /
		// direction properties to be relative to listener position:
		//alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);

		float pos[3] = { position.x, position.y, position.z};
		float vel[3] = { velocity.x, velocity.y, velocity.z};
		float dir[3] = { direction.x, direction.y, direction.z};
		alGetError();
		alSourcefv(source, AL_POSITION, pos);
		alSourcefv(source, AL_VELOCITY, vel);
		alSourcefv(source, AL_DIRECTION, dir);
		return alGetError() == AL_NO_ERROR;
	}

	bool OpenAL::BindBufferToSource(unsigned int buffer, unsigned int source)
	{
		alGetError();
		alSourcei(source, AL_BUFFER, (int)buffer);
		return alGetError() == AL_NO_ERROR;
	}

	void OpenAL::UnbindBufferFromSource(unsigned int source)
	{
		alSourcei(source, AL_BUFFER, 0);
	}

	bool OpenAL::QueueBufferToSource(unsigned int buffer, unsigned int source)
	{
		alGetError();
		alSourceQueueBuffers(source, 1, &buffer);
		return alGetError() == AL_NO_ERROR;
	}

	unsigned int OpenAL::UnqueueBufferFromSource(unsigned int source)
	{
		unsigned int buf;
		alSourceUnqueueBuffers(source, 1, &buf);
		return buf;
	}

	float OpenAL::SourcePlaybackPosition(unsigned int source)
	{
		float pos = 0;
		alGetSourcef(source, AL_SEC_OFFSET, &pos);
		std::cout << "Getting position at " << pos << std::endl;
		return pos / 60.0f; // Normalize the time
	}

	void OpenAL::SetSourcePlaybackPosition(unsigned int source, float position)
	{
		alSourcef(source, AL_SEC_OFFSET, position * 60.0f);
	}


	bool OpenAL::SetBufferData(unsigned int buffer, int format, void* data, size_t count, int rate)
	{
		alGetError();
 		alBufferData(buffer, format, data, static_cast<ALsizei>(count), rate);
		return alGetError() == AL_NO_ERROR;
	}


} // namespace Audio
} // namespace Private
} // namespace Yuni
