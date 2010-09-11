#ifndef FTP_H
#define FTP_H

#include <misc/string.h>
#include <threads/thread.h>

namespace TA3D
{
	class Ftp : public Thread
	{
	public:
		Ftp();
		virtual ~Ftp();

		virtual void proc(void* param);
		virtual void signalExitThread();

		/*!
		* \brief starts downloading a file in a separate thread
		*/
		void get(const String &filename, const String &url);
		void stop();
		bool isDownloading() const;

		int getTransferedBytes() const;
		float getProgress() const;

	public:
		/*!
		 * \brief download a file (as a String or a real file), does the work in current thread so this is blocking
		 */
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
