#include "wavdecoder.h"
#include <QIODevice>
#include <QByteArray>
#include <logs/logs.h>

namespace TA3D
{
    namespace Audio
    {
        namespace
        {
            struct wav_header_t
            {
                quint32 FileTypeBlocID; // (4 octets) : Constante «RIFF»  (0x52,0x49,0x46,0x46)
                quint32 FileSize;       // (4 octets) : Taille du fichier moins 8 octets
                quint32 FileFormatID;   // (4 octets) : Format = «WAVE»  (0x57,0x41,0x56,0x45)

                quint32 FormatBlocID;   // (4 octets) : Identifiant «fmt »  (0x66,0x6D, 0x74,0x20)
                quint32 BlocSize;       // (4 octets) : Nombre d'octets du bloc - 8  (0x10)

                quint16 AudioFormat;    // (2 octets) : Format du stockage dans le fichier (1: PCM, ...)
                quint16 NbrCanaux;      // (2 octets) : Nombre de canaux (de 1 à 6, cf. ci-dessous)
                quint32 Frequence;      // (4 octets) : Fréquence d'échantillonnage (en hertz) [Valeurs standardisées : 11025, 22050, 44100 et éventuellement 48000 et 96000]
                quint32 BytePerSec;     // (4 octets) : Nombre d'octets à lire par seconde (i.e., Frequence * BytePerBloc).
                quint16 BytePerBloc;    // (2 octets) : Nombre d'octets par bloc d'échantillonnage (i.e., tous canaux confondus : NbrCanaux * BitsPerSample/8).
                quint16 BitsPerSample;  // (2 octets) : Nombre de bits utilisés pour le codage de chaque échantillon (8, 16, 24)

                quint32 DataBlocID;     // (4 octets) : Constante «data»  (0x64,0x61,0x74,0x61)
                quint32 DataSize;       // (4 octets) : Nombre d'octets des données (i.e. "Data[]", i.e. taille_du_fichier - taille_de_l'entête  (qui fait 44 octets normalement).
            } __attribute__ ((__packed__));
        }

        WAVDecoder::WAVDecoder(QIODevice *dev)
            : dev(dev)
        {
        }

        void WAVDecoder::decode(QByteArray &data)
        {
            data.clear();
            if (!dev)           // Why would you want to decode a NULL device ?
                return;

            wav_header_t header;
            dev->read((char*)&header, sizeof(header));

#define ABORT() do { dev->seek(0);  data.clear();   return; } while(false)

            if (header.FileTypeBlocID != 0x46464952)
                ABORT();
            if (header.FileFormatID != 0x45564157)
                ABORT();
            if (header.FormatBlocID != 0x20746D66)
                ABORT();
            if (header.AudioFormat != 1)     // Check for PCM format
                ABORT();

            dev->seek(20);
            header.DataSize = header.BlocSize;
            do
            {
                dev->seek(dev->pos() + header.DataSize);
                dev->read((char*)&(header.DataBlocID), sizeof(header.DataBlocID));
                dev->read((char*)&(header.DataSize), sizeof(header.DataSize));
            } while(header.DataBlocID != 0x61746164);

            const quint32 nb_samples = header.DataSize / (header.BitsPerSample / 8 * header.NbrCanaux);

            LOG_DEBUG(LOG_PREFIX_SOUND << "Decoding WAV : " << nb_samples << " samples @ " << header.Frequence << "Hz in " << header.BitsPerSample << "bits x " << header.NbrCanaux << " channels");

            const int scale_factor = 44100 / header.Frequence;
            data.resize(nb_samples * sizeof(float) * scale_factor);
            const QByteArray &dev_data = dev->readAll();
            if (dev_data.size() < nb_samples * header.NbrCanaux * header.BitsPerSample / 8)
                ABORT();

            float *ptr = (float*)data.data();
            switch(header.BitsPerSample)
            {
            case 8:
                if (true)
                {
                    const float K = float(1. / (255. * header.NbrCanaux));
                    const quint8 *src = (const quint8*)dev_data.constData();
                    for(quint32 pos = 0 ; pos < nb_samples ; ++pos)
                    {
                        int s = 0;
                        for(int i = 0 ; i < header.NbrCanaux ; ++i, ++src)
                            s += *src;
                        const float x = float(s) * K;
                        for(int i = 0 ; i < scale_factor ; ++i)
                            *(ptr++) = x;
                    }
                }
                break;
            case 16:
                if (true)
                {
                    const float K = float(1. / (32768. * header.NbrCanaux));
                    const qint16 *src = (const qint16*)dev_data.constData();
                    for(quint32 pos = 0 ; pos < nb_samples ; ++pos)
                    {
                        int s = 0;
                        for(int i = 0 ; i < header.NbrCanaux ; ++i, ++src)
                            s += *src;
                        const float x = float(s) * K;
                        for(int i = 0 ; i < scale_factor ; ++i)
                            *(ptr++) = x;
                    }
                }
                break;
            case 24:
                if (true)
                {
                    const float K = float(1. / (16777216. * header.NbrCanaux));
                    const quint8 *src = (const quint8*)dev_data.constData();
                    for(quint32 pos = 0 ; pos < nb_samples ; ++pos)
                    {
                        quint32 s = 0;
                        for(int i = 0 ; i < header.NbrCanaux ; ++i, src += 3)
                            s += ((quint32)src[0] << 16) + ((quint32)src[1] << 8) + src[2];
                        const float x = float(s) * K;
                        for(int i = 0 ; i < scale_factor ; ++i)
                            *(ptr++) = x;
                    }
                }
                break;
            case 32:
                if (true)
                {
                    const float K = float(1. / (2147483648. * header.NbrCanaux));
                    const qint32 *src = (const qint32*)dev_data.constData();
                    for(quint32 pos = 0 ; pos < nb_samples ; ++pos)
                    {
                        qint64 s = 0;
                        for(int i = 0 ; i < header.NbrCanaux ; ++i, ++src)
                            s += *src;
                        const float x = float(s) * K;
                        for(int i = 0 ; i < scale_factor ; ++i)
                            *(ptr++) = x;
                    }
                }
                break;
            }
        }
    }
}
