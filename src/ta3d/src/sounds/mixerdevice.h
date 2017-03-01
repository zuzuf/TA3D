#ifndef __TA3D_SOUNDS_MIXERDEVICE_H__
#define __TA3D_SOUNDS_MIXERDEVICE_H__

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QElapsedTimer>

class QAudioDecoder;
class QIODevice;

namespace TA3D
{
    namespace Audio
    {
        class MixerDevice : public QObject
        {
            Q_OBJECT
        public:
            MixerDevice();

            void setSink(QIODevice *sink);
            void addSource(QIODevice *src);

        private slots:
            void genBuffer();

        private:
            QIODevice *sink;
            QList<QPair<QAudioDecoder*, QByteArray> > sources;
            QByteArray buffer;
            QElapsedTimer clock;
            qint64 last_sample;
        };
    }
}

#endif
