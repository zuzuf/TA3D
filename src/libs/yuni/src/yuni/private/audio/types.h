#ifndef __YUNI_PRIVATE_AUDIO_TYPES_H__
# define __YUNI_PRIVATE_AUDIO_TYPES_H__

namespace Yuni
{
namespace Private
{
namespace Audio
{


	struct AudioFile;


	struct AudioStream
	{
		AVCodecContext* CodecContext;
		int StreamIdx;

		unsigned int Format;
		uint64_t Size;

		char* Data;
		size_t DataSize;
		size_t DataSizeMax;
		char* DecodedData;
		size_t DecodedDataSize;

		AudioFile* parent;
	};

	struct AudioFile
	{
		AVFormatContext* FormatContext;
		AudioStream** Streams;
		size_t StreamsSize;
	};




} // namespace Audio
} // namespace Private
} // namespace Yuni

#endif // __YUNI_PRIVATE_AUDIO_TYPES_H__
