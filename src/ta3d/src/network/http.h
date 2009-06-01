#ifndef HTTP_H
#define HTTP_H

#include "../misc/string.h"
#include "../threads/thread.h"

namespace TA3D
{
    class Http : public Thread
    {
    public:
        Http();
        virtual ~Http();

        virtual void proc(void* param);
        virtual void signalExitThread();
        virtual const char *className();

        /*!
         * \brief starts downloading a file in a separate thread
         */
        void get(const String &filename, const String &url);
        void stop();
        bool isDownloading();

        int getTransferedBytes();
        float getProgress();

    public:
        /*!
         * \brief download a file (as a String or a real file), does the work in current thread so this is blocking
         */
        static String request( const String &servername, const String &_request );
        static bool getFile( const String &filename, const String &servername, const String &_request );

    private:
        bool bStop;
        int pos;
        int size;
        String filename;
        String _request;
        String servername;
    };
}

#endif
