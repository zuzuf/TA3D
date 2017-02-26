#include "mixerdevice.h"
#include <QAudioDecoder>
#include "manager.h"
#include <QTimer>

namespace TA3D
{
    namespace Audio
    {
        MixerDevice::MixerDevice()
        {
            open(QIODevice::ReadOnly);

            QTimer *timer = new QTimer(this);
            timer->setInterval(10);
            timer->setSingleShot(false);
            connect(timer, SIGNAL(timeout()), this, SLOT(genBuffer()));
            timer->start();
        }

        void MixerDevice::genBuffer()
        {
            emit readyRead();

            int new_samples = 1024;
            for(int i = 0 ; i < sources.size() ;)
            {
                QAudioDecoder *decoder = sources[i].first;
                if (decoder->state() == QAudioDecoder::StoppedState)
                {
                    decoder->deleteLater();
                    sources[i] = sources.back();
                    sources.pop_back();
                    continue;
                }
                ++i;

                if(!decoder->bufferAvailable())
                    continue;
                const QAudioBuffer &src_buffer = decoder->read();
                sources[i].second.append(QByteArray::fromRawData((const char*)src_buffer.constData(), src_buffer.byteCount()));
                new_samples = std::min<int>(new_samples, sources[i].second.size() >> 3);
            }

            if (new_samples == 0 || sources.isEmpty())
                return;

            const int offset = buffer.size();
            buffer.resize(buffer.size() + new_samples * (sizeof(float) * 2));
            float *ptr = (float*)(buffer.data() + offset);
            new_samples <<= 1;
            for(int i = 0 ; i < new_samples ; ++i)
                ptr[i] = 0.f;
            for(auto &src : sources)
            {
                const float *src_ptr = (const float*)src.second.constData();
                for(int i = 0 ; i < new_samples ; ++i)
                    ptr[i] += src_ptr[i];
            }
        }

        qint64 MixerDevice::readData(char *data, qint64 maxlen)
        {
            if (maxlen > buffer.size())
                genBuffer();

            // Produce data no matter what
            if (maxlen > buffer.size())
            {
                memcpy(data, buffer.data(), buffer.size());
                memset(data + maxlen, 0, maxlen - buffer.size());
                buffer.clear();
                return maxlen;
            }
            memcpy(data, buffer.data(), maxlen);
            const qint64 data_left = buffer.size() - maxlen;
            memmove(buffer.data(), buffer.data() + maxlen, data_left);
            buffer.resize(data_left);
            return maxlen;
        }

        void MixerDevice::addSource(QIODevice *src)
        {
            QAudioDecoder *decoder = new QAudioDecoder(this);
            decoder->setSourceDevice(src);
            decoder->setAudioFormat(VARS::sound_manager->getAudioFormat());
            connect(decoder, SIGNAL(bufferReady()), this, SLOT(genBuffer()));
            decoder->start();
            sources.push_back(qMakePair(decoder, QByteArray()));
        }

        qint64 MixerDevice::writeData(const char *data, qint64 len)
        {
            return 0;
        }

        qint64 MixerDevice::bytesAvailable() const
        {
            return 16384;
        }
    }
}
