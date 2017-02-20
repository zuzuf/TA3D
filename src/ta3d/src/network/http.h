#ifndef HTTP_H
#define HTTP_H

#include <yuni/yuni.h>
#include <misc/string.h>
#include <threads/thread.h>

namespace TA3D
{
    class Http : public Thread
    {
    public:
        Http();
        virtual ~Http();

        virtual void proc(void* param);
        virtual void signalExitThread();

        /*!
         * \brief starts downloading a file in a separate thread
         */
        void get(const QString &filename, const QString &url);
        void stop();
        bool isDownloading() const;

        int getTransferedBytes() const;
        float getProgress() const;

    public:
        /*!
         * \brief download a file (as a QString or a real file), does the work in current thread so this is blocking
         */
        static QString request( const QString &servername, const QString &_request );
        static bool getFile( const QString &filename, const QString &servername, const QString &_request );

    private:
        bool bStop;
        int pos;
        int size;
        QString filename;
        QString _request;
        QString servername;
    };




} // namespace TA3D

#endif
