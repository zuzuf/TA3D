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


namespace TA3D
{

void install_TA_files( String HPI_file, String filename )
{
	HPIManager = new TA3D::UTILS::HPI::cHPIHandler(HPI_file);

	uint32 file_size32 = 0;
	byte *data = HPIManager->PullFromHPI( filename, &file_size32);			// Extract the file
	if (data)
	{
		FILE *dst = TA3D_OpenFile(Paths::Resources + Paths::ExtractFileName(filename), "wb");

		if (dst)
		{
			fwrite(data, file_size32, 1, dst);

			fflush(dst);
			fclose(dst);
		}
		delete[] data;
	}

	delete HPIManager;
}

} // namespace TA3D
