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


#ifndef __TA3D_XX_MISC_RECTTEST_H__
# define __TA3D_XX_MISC_RECTTEST_H__

# include "../stdafx.h"
# include "math.h"
# include "rect.h"
# include "camera.h"

namespace TA3D
{

    /* This class powers drag-a-box selection.  Dragging a rectangle
     * in the GUI does not correspond to a rectangle on the map due
     * to 3D projection.  A RectTest will tell you if a point on the
     * 3D map is inside or outside a rectangle in 2D gui coordinates.
     */

    class RectTest
    {
    public:
        RectTest (Camera& cam, const Rect<int>& pos);
        bool contains(const Vector3D &point) const;

    private:
        Camera &cam;
        float VW, VH;
        MATRIX_4x4 T;
        int X1, Y1, X2, Y2;
    }; // class RectTest

} // namespace TA3D

#endif // __TA3D_XX_MISC_RECTTEST_H__
