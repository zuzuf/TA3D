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
#include "stdafx.h"
#include "TA3D_NameSpace.h"			// our namespace, a MUST have.
#include "ta3dbase.h"
#include "misc/math.h"
#include "misc/paths.h"
#include <yuni/core/io/file/stream.h>

using namespace Yuni::Core::IO::File;


namespace TA3D
{

void install_TA_files( QString HPI_file, QString filename )
{
    zuzuf::smartptr<Archive> archive = Archive::load(HPI_file);
	if (!archive)
    {
        LOG_ERROR("archive not found : '" << HPI_file << "'");
        return;
    }
	std::deque<Archive::FileInfo*> lFiles;
    archive->getFileList(lFiles);
	File *file = archive->readFile(filename);			// Extract the file
	if (file)
	{
		Stream dst(QString(Paths::Resources) << Paths::ExtractFileName(filename), Yuni::Core::IO::OpenMode::write);

		if (dst.opened())
		{
			dst.write(file->data(), file->size());

			dst.flush();
			dst.close();
		}
		delete file;
	}
}

} // namespace TA3D
