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



namespace TA3D
{


	// The unique instance of the ta3d logger
	Logger logs;

    Logger::Logger()
        : verbosityLevel(3)
    {
    }

    Logger::~Logger()
    {
        log_file_stream->flush();
        log_file.flush();
    }

    bool Logger::logFileIsOpened() const
    {
        return log_file.isOpen();
    }

    QString Logger::logfile() const
    {
        return log_file.fileName();
    }

    void Logger::openLogFile(const QString &filename)
    {
        if (log_file.isOpen())
            log_file.close();
        log_file.setFileName(filename);
        log_file.open(QIODevice::WriteOnly);

        log_file_stream.reset(new QTextStream(&log_file));
    }
} // namespace TA3D

