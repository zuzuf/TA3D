#ifndef __TA3D_SOUNDS_WAVDECODER_H__
#define __TA3D_SOUNDS_WAVDECODER_H__

class QByteArray;
class QIODevice;

namespace TA3D
{
    namespace Audio
    {
        /*!
         * \brief WAV file decoder which works on a QIODevice
         *
         * This works in the current thread in order to reduce latency
         */
        class WAVDecoder
        {
        public:
            WAVDecoder(QIODevice *dev);

            void decode(QByteArray &data);
        private:
            QIODevice *dev;
        };
    }
}

#endif
