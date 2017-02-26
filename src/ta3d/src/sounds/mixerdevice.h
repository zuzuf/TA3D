#ifndef __TA3D_SOUNDS_MIXERDEVICE_H__
#define __TA3D_SOUNDS_MIXERDEVICE_H__

#include <QIODevice>
#include <QByteArray>
#include <QList>

class QAudioDecoder;

namespace TA3D
{
    namespace Audio
    {
        class MixerDevice : public QIODevice
        {
            Q_OBJECT
        public:
            MixerDevice();

            virtual qint64 bytesAvailable() const;

            void addSource(QIODevice *src);
        private slots:
            void genBuffer();

        protected:
            virtual qint64 readData(char *data, qint64 maxlen);
            virtual qint64 writeData(const char *data, qint64 len);

        private:
            QList<QPair<QAudioDecoder*, QByteArray> > sources;
            QByteArray buffer;
        };
    }
}

#endif
