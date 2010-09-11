#include <TA3D_NameSpace.h>
#include <misc/files.h>
#include <misc/paths.h>
#include "ftp.h"
#include <logs/logs.h>
#include "socket.tcp.h"
#include <yuni/core/io/file/stream.h>

using namespace Yuni::Core::IO::File;

namespace TA3D
{
    Ftp::Ftp() : bStop(false), filename(), _request(), servername(), pos(0), size(0)
    {
    }

    Ftp::~Ftp()
    {
        Thread::destroyThread();
    }

	int Ftp::getTransferedBytes() const
    {
        return pos;
    }

	float Ftp::getProgress() const
    {
        if (size == 0)
            return 0;
        return (float)(pos * 100.0 / size);
    }

    void Ftp::proc(void* param)
    {
        bStop = false;

        if (!Paths::Exists(Paths::ExtractFilePath(filename)))
            Paths::MakeDir(Paths::ExtractFilePath(filename));

        SocketTCP   sockCtrl;
        SocketTCP   sockPasv;
        char        buffer[4096];
        String      realFilename = filename;
        String      tmpFile = filename + ".part";
		Stream		f(tmpFile, Yuni::Core::IO::OpenMode::write);
        int         count;
        int         crfound = 0;
        int         lffound = 0;

		if (!f.opened())
        {
            LOG_ERROR(LOG_PREFIX_NET << "Ftp: Could not open file " << tmpFile << " for writing !");
            return;        // Error can't open file
        }

        /* open the socket and connect to the server */
        sockCtrl.open(servername, 21);
        if(!sockCtrl.isOpen())
        {
            LOG_ERROR(LOG_PREFIX_NET << "Ftp: Could not open socket !");
            f.close();
            remove(tmpFile.c_str());
            return;
        }

        sockCtrl.setNonBlockingMode(true);      // We want it to be able to detect end of file ;)

        uint32 timer(msec_timer);
        if (!sockCtrl.isOpen())
        {
            LOG_ERROR(LOG_PREFIX_NET << "Ftp: Could not send request to server !");
            f.close();
            remove(tmpFile.c_str());
            return;
        }

		sockCtrl.send("USER anonymous\r\n");
		sockCtrl.send("PASSWORD anonymous\r\n");
		String line = sockCtrl.getLine();
		sockCtrl.getLine();

		sockCtrl.send("PASV ");
		sockCtrl.send("RETR " + _request);

        pos = 0;
        size = 0;
        String header;

        while (!bStop)
        {
            timer = msec_timer;
            do
            {
                count = sockPasv.recv(buffer, sizeof(buffer) - 1);
                if (count == 0)
                    rest(1);
            }
            while(count == 0 && msec_timer - timer < 1000);
            if (msec_timer - timer >= 1000)
                sockPasv.close();
            if(count < 0)
            {
                sockPasv.close();
                sockCtrl.close();
                f.close();
                Paths::Files::Copy(tmpFile, realFilename, true);
                remove(tmpFile.c_str());
                LOG_DEBUG(LOG_PREFIX_NET << "File successfully downloaded : " << realFilename);
                return;
            }
            if(count > 0)
            {
                f.write( (char*)buffer, count );
                pos += count;
                size = Math::Max(size, pos);
            }
        }
        sockCtrl.close();
        sockPasv.close();
        f.close();
        remove(tmpFile.c_str());
        LOG_ERROR(LOG_PREFIX_NET << "Ftp: Download interrupted!");
    }

    void Ftp::signalExitThread()
    {
        while(pDead == 0)
        {
            bStop = true;
            rest(1);
        }
    }

	bool Ftp::isDownloading() const
    {
        return isRunning();
    }

    void Ftp::get(const String &filename, const String &url)
    {
        destroyThread();
        this->filename = filename;
        String tmp = url;
        if (tmp.startsWith("ftp://"))
            tmp.erase(0, 6);
        this->servername = tmp.substr(0, tmp.find_first_of("/"));
        this->_request = tmp.erase(0, this->servername.size());

        bStop = false;
        pos = 0;
        size = 0;
        start();
    }

    void Ftp::stop()
    {
        destroyThread();
    }

	bool Ftp::getFile( const String &filename, const String &servername, const String &_request)
	{
		Ftp downloader;
		if (servername.startsWith("ftp://"))
			downloader.get(filename, servername + '/' + _request);
		else
			downloader.get(filename, "ftp://" + servername + '/' + _request);
		while(downloader.isDownloading())
			rest(10);

		return Paths::Exists(filename);
	}
}
