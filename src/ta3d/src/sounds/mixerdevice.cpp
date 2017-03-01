#include "mixerdevice.h"
#include <QAudioDecoder>
#include <QIODevice>
#include "manager.h"
#include "wavdecoder.h"
#include <logs/logs.h>
#include <QTimer>

namespace TA3D
{
    namespace Audio
    {
        MixerDevice::MixerDevice()
        {
            sink = NULL;

            clock.start();
            last_sample = 0;

            QTimer *timer = new QTimer;
            timer->setInterval(25);
            timer->setSingleShot(false);
            connect(timer, SIGNAL(timeout()), this, SLOT(genBuffer()));
            timer->start();
        }

        void MixerDevice::genBuffer()
        {
            if (!sink)
                return;

            const qint64 tic = clock.elapsed() + 93;
            const qint64 delta_s = tic * 44100 / 1000 - last_sample;
            last_sample += delta_s;
            const int new_samples = delta_s;
            for(int i = 0 ; i < sources.size() ;)
            {
                QAudioDecoder *decoder = sources[i].first;
                if (!decoder)
                {
                    if (sources[i].second.isEmpty())
                    {
                        sources[i] = sources.back();
                        sources.pop_back();
                    }
                    else
                        ++i;
                    continue;
                }

                if (decoder->state() == QAudioDecoder::StoppedState && decoder->position() >= 0)
                {
                    sources[i] = sources.back();
                    sources.pop_back();
//                    decoder->deleteLater();
                    continue;
                }

                if(!decoder->bufferAvailable())
                {
                    ++i;
                    continue;
                }
                const QAudioBuffer &src_buffer = decoder->read();
                const QAudioFormat &fmt = src_buffer.format();
                LOG_DEBUG(LOG_PREFIX_SOUND << fmt.channelCount() << " " << fmt.codec() << " " << fmt.sampleRate() << " " << fmt.sampleSize() << " " << src_buffer.sampleCount());
                sources[i].second.append(QByteArray::fromRawData((const char*)src_buffer.constData(), src_buffer.byteCount()));
                ++i;
            }

            buffer.clear();
            buffer.resize(new_samples * sizeof(float));
            float *ptr = (float*)buffer.data();
            memset(ptr, 0, new_samples * sizeof(float));
            for(auto &src : sources)
            {
                if (src.second.isEmpty())
                    continue;
                const float *src_ptr = (const float*)src.second.constData();
                const int src_samples = std::min<int>(new_samples, src.second.size() >> 2);
                for(int i = 0 ; i < src_samples ; ++i)
                    ptr[i] += src_ptr[i];
                const size_t total_sample_data = src_samples * sizeof(float);
                const size_t total_buffer_data = src.second.size();
                const size_t data_left = total_buffer_data - total_sample_data;
                if (data_left > 0)
                    memmove(src.second.data(), src.second.data() + total_sample_data, data_left);
                src.second.resize(data_left);
            }

            sink->write(buffer);
            sink->waitForBytesWritten(0);
        }

        void MixerDevice::addSource(QIODevice *src)
        {
            QByteArray data;
            WAVDecoder wav_decoder(src);
            wav_decoder.decode(data);
            if (!data.isEmpty())
            {
                delete src;
                sources.push_back(qMakePair(nullptr, data));
                LOG_DEBUG(LOG_PREFIX_SOUND << "WAV decoded!");
                return;
            }

            QAudioDecoder *decoder = new QAudioDecoder(this);
            decoder->setSourceDevice(src);
            decoder->setAudioFormat(VARS::sound_manager->getAudioFormat());
//            connect(decoder, SIGNAL(bufferReady()), this, SLOT(genBuffer()));
            connect(src, SIGNAL(aboutToClose()), decoder, SLOT(stop()));
            decoder->start();
            sources.push_back(qMakePair(decoder, QByteArray()));
        }

        void MixerDevice::setSink(QIODevice *sink)
        {
            this->sink = sink;
        }
    }
}
