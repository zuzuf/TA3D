/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include "logs.h"
#include <misc/broadcastingiodevice.h>


namespace TA3D
{


	// The unique instance of the ta3d logger
	Logger logs;

    Logger::Logger()
        : verbosityLevel(3)
    {
        log_file = nullptr;

        main_log_device = new BroadCastingIODevice;
        log_file_stream.reset(new QTextStream(main_log_device));
        QFile *stdoutput = new QFile;
        stdoutput->open(0, QIODevice::WriteOnly);
        main_log_device->addSink(stdoutput);
    }

    Logger::~Logger()
    {
        log_file_stream->flush();
        delete main_log_device;
    }

    bool Logger::logFileIsOpened() const
    {
        return log_file;
    }

    QString Logger::logfile() const
    {
        return log_file ? log_file->fileName() : QString();
    }

    void Logger::openLogFile(const QString &filename)
    {
        if (log_file)
        {
            main_log_device->deleteSink(log_file);
            log_file = nullptr;
        }
        log_file = new QFile(filename);
        log_file->open(QIODevice::WriteOnly);
        if (log_file->isOpen())
            main_log_device->addSink(log_file);
        else
        {
            delete log_file;
            log_file = nullptr;
        }
    }
} // namespace TA3D

