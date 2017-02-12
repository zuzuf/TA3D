#ifndef __YUNI_PRIVATE_AUDIO_AV_H__
# define __YUNI_PRIVATE_AUDIO_AV_H__

# include "../../yuni.h"

# if (YUNI_OS_GCC_VERSION >= 40102)
#	pragma GCC diagnostic ignored "-Wconversion"
# endif

extern "C"
{
# include "libavcodec/avcodec.h"
# include "libavformat/avformat.h"
}

# include "types.h"
# include "../../core/string.h"

namespace Yuni
{
namespace Private
{
namespace Audio
{

	/*!
	** \brief This is a wrapper around the AV* libs from ffmpeg.
	*/
	class AV
	{
	public:
		//! Initialize ffmpeg
		static bool Init();

		//! Open a file with ffmpeg and sets up the streams' information
		template<typename StringT>
		static AudioFile* OpenFile(const StringT& fname);

		//! Close an opened file and any of its streams
		static void CloseFile(AudioFile* file);

		/*!
		** \brief Retrieve a handle for the given audio stream number
		**
		** The stream number will generally be 0, but some files can have
		** multiple audio streams in one file.
		*/
		static AudioStream* GetAudioStream(AudioFile* file, int streamIndex);

		/*!
		** \brief Get information about the given audio stream
		**
		** Currently, ffmpeg always decodes audio (even 8-bit PCM) to 16-bit PCM
		** \returns 0 on success
		*/
		static int GetAudioInfo(AudioStream* stream, int& rate, int& channels, int& bits);

		/*!
		** \brief Get the duration of an audio stream
		**
		** \param stream Stream to get duration for
		** \returns The duration of the stream, 0 if null
		*/
		static unsigned int GetStreamDuration(const AudioStream* stream);

		/*!
		** \brief Decode audio and write at most length bytes into the provided data buffer
		**
		** Will only return less for end-of-stream or error conditions
		** \returns The number of bytes written
		*/
		static size_t GetAudioData(AudioStream* stream, void* data, size_t length);

	}; // class AV



} // namespace Audio
} // namespace Private
} // namespace Yuni

#include "av.hxx"

#endif // __YUNI_PRIVATE_AUDIO_AV_H__
