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

/*----------------------------------------------------------------------\
|                                draw.list.h                            |
|      This modules contains the basics elements from the mechanism     |
| that allow Lua scripts to draw on the screen.                         |
\----------------------------------------------------------------------*/

#ifndef __DrawList_H__
# define __DrawList_H__

# include <misc/string.h>
# include <zuzuf/smartptr.h>

# define DRAW_TYPE_NONE     0x0
# define DRAW_TYPE_POINT    0x1
# define DRAW_TYPE_LINE     0x2
# define DRAW_TYPE_CIRCLE   0x3
# define DRAW_TYPE_TRIANGLE 0x4
# define DRAW_TYPE_BOX      0x5
# define DRAW_TYPE_FILLBOX  0x6
# define DRAW_TYPE_TEXT     0x7
# define DRAW_TYPE_BITMAP   0x8



namespace TA3D
{


    struct DrawObject                  // Pour mémoriser le traçage des primitives
    {
        byte    type;
        float   x[4];
        float   y[4];
        float   r[2],g[2],b[2];
        String  text;
        GLuint  tex;
    };

    class DrawList : public zuzuf::ref_count
    {
	public:
        typedef zuzuf::smartptr<DrawList>	Ptr;
    public:
		DrawObject			prim;
		DrawList::Ptr		next;

        void init();

        void destroy();

		DrawList()  {   init(); }
		~DrawList()  {   destroy(); }

        void add(DrawObject &obj);

        void draw(Font *fnt);
    };

} // namespace TA3D

#endif
