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

//
//	the gfx module
//
//	it contains basic gfx commands like font management, background management, etc...
//

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#include "glfunc.h"
#include "gui.h"
#include "gaf.h"
//#include "gfx.h"			// Included in TA3D_NameSpace.h

#include <allegro/internal/aintern.h>

using namespace TA3D::INTERFACES;

#define BYTE_TO_FLOAT		0.00390625f

void GFX_TEXTURE::draw( const float x1, const float y1 )
{
	gfx->drawtexture( tex, x1, y1, x1+width-1, y1+height-1 );
}

void GFX_TEXTURE::draw( const float x1, const float y1, const uint32 col )
{
	gfx->drawtexture( tex, x1, y1, x1+width-1, y1+height-1, col );
}

GFX_TEXTURE::GFX_TEXTURE( const GLuint gltex )
{
	destroy_tex = true;
	set( gltex );
}

void GFX_TEXTURE::set( const GLuint gltex )
{
	tex = gltex;
	width = gfx->texture_width( tex );
	height = gfx->texture_height( tex );
}

void GFX_TEXTURE::destroy()
{
	width = 0;
	height = 0;
	if( destroy_tex ) {
		gfx->destroy_texture( tex );
		destroy_tex = false;
		}
	else
		tex = 0;
}

GFX::GFX()
{
	CreateCS();

	install_allegro_gl();

		allegro_gl_clear_settings();         // Initialise AllegroGL
		allegro_gl_set (AGL_STENCIL_DEPTH, 8 );
		allegro_gl_set (AGL_SAMPLE_BUFFERS, 0 );
		allegro_gl_set (AGL_SAMPLES, TA3D::VARS::lp_CONFIG->fsaa );
		allegro_gl_set (AGL_VIDEO_MEMORY_POLICY, AGL_RELEASE );
		allegro_gl_set (AGL_COLOR_DEPTH, TA3D::VARS::lp_CONFIG->color_depth );
		allegro_gl_set (AGL_Z_DEPTH, 32 );
		allegro_gl_set (AGL_FULLSCREEN, TA3D::VARS::lp_CONFIG->fullscreen );
		allegro_gl_set (AGL_DOUBLEBUFFER, 1 );
		allegro_gl_set (AGL_RENDERMETHOD, 1 );
		allegro_gl_set (AGL_SUGGEST, AGL_RENDERMETHOD | AGL_COLOR_DEPTH | AGL_Z_DEPTH | AGL_DOUBLEBUFFER | AGL_FULLSCREEN | AGL_SAMPLES | AGL_SAMPLE_BUFFERS | AGL_STENCIL_DEPTH | AGL_VIDEO_MEMORY_POLICY );

		allegro_gl_use_mipmapping(TRUE);

		allegro_gl_flip_texture(false);

		request_refresh_rate(85);

		if( TA3D::VARS::lp_CONFIG->fullscreen )
        {
#ifdef GFX_OPENGL_FULLSCREEN
			set_gfx_mode(GFX_OPENGL_FULLSCREEN, TA3D::VARS::lp_CONFIG->screen_width, TA3D::VARS::lp_CONFIG->screen_height, 0, 0);   // Entre en mode graphique OpenGL (plein écran)
#else
			allegro_message(TRANSLATE("WARNING: AllegroGL has been built without fullscreen support.\nrunning in fullscreen mode impossible\nplease install libxxf86vm and rebuild both Allegro and AllegroGL\nif you want fullscreen modes.").c_str());
			set_gfx_mode(GFX_OPENGL_WINDOWED, TA3D::VARS::lp_CONFIG->screen_width, TA3D::VARS::lp_CONFIG->screen_height, 0, 0);      // Entre en mode graphique OpenGL (fenêtré)
#endif
		}
		else
			set_gfx_mode(GFX_OPENGL_WINDOWED, TA3D::VARS::lp_CONFIG->screen_width, TA3D::VARS::lp_CONFIG->screen_height, 0, 0);      // Entre en mode graphique OpenGL (fenêtré)

		precalculations();

		install_ext();      // Pour les fonctions OpenGl supplémentaires

		if( g_useTextureCompression )         // Active la compression de texture
			allegro_gl_set_texture_format(GL_COMPRESSED_RGB_ARB);
		else
			allegro_gl_set_texture_format(GL_RGB8); 

	ati_workaround = strncasecmp( (const char*)glGetString( GL_VENDOR ), "ATI", 3 ) == 0;		// Enable ATI workarounds if we have an ATI card
	ati_workaround |= strstr( Uppercase( (const char*)glGetString( GL_VENDOR ) ).c_str(), "SIS" ) != NULL;		// Enable ATI workarounds if we have an SIS card

	TA3D::VARS::pal = NULL;

	width = SCREEN_W;
	height = SCREEN_H;
	x = 0;
	y = 0;
	low_def_limit = 600.0f;

	glfond = 0;
	normal_font.init();
	small_font.init();
	TA_font.init();
	ta3d_gui_font.init();

	InitInterface();

	if( Console ) {
		Console->stdout_on();

		Console->AddEntry("OpenGL informations:");
		Console->AddEntry("vendor: %s", glGetString( GL_VENDOR ) );
		Console->AddEntry("renderer: %s", glGetString( GL_RENDERER ) );
		Console->AddEntry("version: %s", glGetString( GL_VERSION ) );
		if( ati_workaround )
			Console->AddEntry("WARNING: ATI card detected! using workarounds for ATI cards");

		Console->stdout_off();
		}
}

GFX::~GFX()
{
	DeleteInterface();

	if( TA3D::VARS::pal )
		delete[]( TA3D::VARS::pal ); 

	normal_font.destroy();
	small_font.destroy();
	TA_font.destroy();
	ta3d_gui_font.destroy();

	DeleteCS();
}

void GFX::Init()
{
	if( Console )	Console->AddEntry( "allocating palette memory" );
	TA3D::VARS::pal=new RGB[256];      // Allocate a new palette

	if( Console )	Console->AddEntry( "loading TA's palette" );
	byte *palette=HPIManager->PullFromHPI( "palettes\\palette.pal" );
	if(palette)
	{
		if( Console )	Console->AddEntry( "allocating palette memory - success" );
		for(int i=0;i<256;i++)
		{
			pal[i].r=palette[i<<2]>>2;
			pal[i].g=palette[(i<<2)+1]>>2;
			pal[i].b=palette[(i<<2)+2]>>2;
		}
		free(palette);
		set_palette(pal);      // Activate the palette
	}
	else if( Console )	Console->AddEntry( "allocating palette memory - failed" );

	if( Console )	Console->AddEntry( "loading hattfint12.gaf" );
	TA_font.load_gaf_font( "anims\\hattfont12.gaf", 1.0f );
	if( Console )	Console->AddEntry( "creating normal font" );
	normal_font.copy( font , 1.0f );
	normal_font.set_clear( true );
	if( Console )	Console->AddEntry( "creating small font" );
	small_font.copy( font , 0.75f );
	small_font.set_clear( true );
	if( Console )	Console->AddEntry( "loading GUI font" );
	ta3d_gui_font.load_gaf_font( "anims\\hattfont12.gaf" , 1.0f );

	if( Console )	Console->AddEntry( "activating palette" );
	if(palette)
		set_palette(pal);      // Activate the palette

	if( Console )	Console->AddEntry( "loading background" );
	load_background();

	gui_font = ta3d_gui_font;
	gui_font_h = gui_font.height();
	use_normal_alpha_function = true;
	alpha_blending_set = false;
	if( Console )	Console->AddEntry( "GFX::Init() : work done!" );
}


				// Color related functions

const void GFX::set_color(const float &r, const float &g, const float &b)					{	glColor3f(r,g,b);	}
const void GFX::set_color(const float &r, const float &g, const float &b, const float &a)	{	glColor4f(r,g,b,a);	}
const void GFX::set_alpha(const float &a)
{
	float gl_color[4];
	glGetFloatv(GL_CURRENT_COLOR, gl_color);
	gl_color[3] = a;
	glColor4fv(gl_color);
}
const float	GFX::get_r(const uint32 &col)	{	return (col&0xFF)*BYTE_TO_FLOAT;	}
const float	GFX::get_g(const uint32 &col)	{	return ((col&0xFF00)>>8)*BYTE_TO_FLOAT;	}
const float	GFX::get_b(const uint32 &col)	{	return ((col&0xFF0000)>>16)*BYTE_TO_FLOAT;	}
const float	GFX::get_a(const uint32 &col)	{	return ((col&0xFF000000)>>24)*BYTE_TO_FLOAT;	}
const void GFX::set_color(const uint32 &col)	{	glColor4ub( col&0xFF, (col&0xFF00)>>8, (col&0xFF0000)>>16, (col&0xFF000000)>>24 );	}

uint32	GFX::makeintcol(float r, float g, float b)			{	return (int)(255.0f*r) | ((int)(255.0f*g)<<8) | ((int)(255.0f*b)<<16) | 0xFF000000;	}
uint32	GFX::makeintcol(float r, float g, float b, float a)	{	return (int)(255.0f*r) | ((int)(255.0f*g)<<8) | ((int)(255.0f*b)<<16) | ((int)(255.0f*a)<<24);	}

const void GFX::set_2D_mode()	{	allegro_gl_set_allegro_mode();		}
const void GFX::unset_2D_mode()	{	allegro_gl_unset_allegro_mode();	}

const void GFX::line(const float &x1, const float &y1, const float &x2, const float &y2)			// Basic drawing routines
{
	glBegin(GL_LINES);
		glVertex2f(x1,y1);
		glVertex2f(x2,y2);
	glEnd();
}
const void GFX::line(const float &x1, const float &y1, const float &x2, const float &y2, const uint32 &col)
{
	set_color(col);
	line(x1,y1,x2,y2);
}


const void GFX::rect(const float &x1, const float &y1, const float &x2, const float &y2)
{
	glBegin(GL_LINE_LOOP);
		glVertex2f(x1,y1);
		glVertex2f(x2,y1);
		glVertex2f(x2,y2);
		glVertex2f(x1,y2);
	glEnd();
}
const void GFX::rect(const float &x1, const float &y1, const float &x2, const float &y2, const uint32 &col)
{
	set_color(col);
	rect(x1,y1,x2,y2);
}


const void GFX::rectfill(const float &x1, const float &y1, const float &x2, const float &y2)
{
	glBegin(GL_QUADS);
		glVertex2f(x1,y1);
		glVertex2f(x2,y1);
		glVertex2f(x2,y2);
		glVertex2f(x1,y2);
	glEnd();
}
const void GFX::rectfill(const float &x1, const float &y1, const float &x2, const float &y2, const uint32 &col)
{
	set_color(col);
	rectfill(x1,y1,x2,y2);
}


const void GFX::circle(const float &x, const float &y, const float &r)
{
	float d_alpha = DB_PI/(r+1.0f);
	glBegin( GL_LINE_LOOP );
		for( float alpha = 0.0f; alpha <= DB_PI; alpha+=d_alpha )
			glVertex2f( x+r*cos(alpha), y+r*sin(alpha) );
	glEnd();
}
const void GFX::circle(const float &x, const float &y, const float &r, const uint32 &col)
{
	set_color(col);
	circle(x,y,r);
}

const void GFX::circle_zoned(const float &x, const float &y, const float &r, const float &mx, const float &my, const float &Mx, const float &My)
{
	float d_alpha = DB_PI/(r+1.0f);
	glBegin( GL_LINE_LOOP );
		for( float alpha = 0.0f; alpha <= DB_PI; alpha+=d_alpha ) {
			float rx = x+r*cos(alpha);
			float ry = y+r*sin(alpha);
			if( rx < mx )		rx = mx;
			else if( rx > Mx )	rx = Mx;
			if( ry < my )		ry = my;
			else if( ry > My )	ry = My;
			glVertex2f( rx, ry );
			}
	glEnd();
}
const void GFX::circle_zoned(const float &x, const float &y, const float &r, const float &mx, const float &my, const float &Mx, const float &My, const uint32 &col)
{
	set_color(col);
	circle_zoned(x,y,r,mx,my,Mx,My);
}

const void GFX::circlefill(const float &x, const float &y, const float &r)
{
	float d_alpha = DB_PI/(r+1.0f);
	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(x,y);
		for( float alpha = 0.0f; alpha <= DB_PI; alpha+=d_alpha )
			glVertex2f( x+r*cos(alpha), y+r*sin(alpha) );
	glEnd();
}
const void GFX::circlefill(const float &x, const float &y, const float &r, const uint32 &col)
{
	set_color(col);
	circlefill(x,y,r);
}


const void GFX::rectdot(const float &x1, const float &y1, const float &x2, const float &y2)
{
	glLineStipple(1, 0x5555);
	glEnable(GL_LINE_STIPPLE);
	rect(x1,y1,x2,y2);
	glDisable(GL_LINE_STIPPLE);
}
const void GFX::rectdot(const float &x1, const float &y1, const float &x2, const float &y2, const uint32 &col)
{
	set_color(col);
	rectdot(x1,y1,x2,y2);
}


const void GFX::putpixel(const float &x, const float &y, const uint32 &col)
{
	set_color(col);
	glBegin(GL_POINTS);
		glVertex2f(x,y);
	glEnd();
}
uint32 GFX::getpixel(const sint32 &x, const sint32 &y)
{
	uint32	col;
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &col);
	return col;
}


const void GFX::drawtexture(const GLuint &tex, const float &x1, const float &y1, const float &x2, const float &y2, const float &u1, const float &v1, const float &u2, const float &v2)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,tex);
	glBegin(GL_QUADS);
		glTexCoord2f(u1,v1);		glVertex2f(x1,y1);
		glTexCoord2f(u2,v1);		glVertex2f(x2,y1);
		glTexCoord2f(u2,v2);		glVertex2f(x2,y2);
		glTexCoord2f(u1,v2);		glVertex2f(x1,y2);
	glEnd();
}
const void GFX::drawtexture(const GLuint &tex, const float &x1, const float &y1, const float &x2, const float &y2)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,tex);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f);		glVertex2f(x1,y1);
		glTexCoord2f(1.0f,0.0f);		glVertex2f(x2,y1);
		glTexCoord2f(1.0f,1.0f);		glVertex2f(x2,y2);
		glTexCoord2f(0.0f,1.0f);		glVertex2f(x1,y2);
	glEnd();
}
const void GFX::drawtexture_flip(const GLuint &tex, const float &x1, const float &y1, const float &x2, const float &y2)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,tex);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f);		glVertex2f(x1,y1);
		glTexCoord2f(0.0f,1.0f);		glVertex2f(x2,y1);
		glTexCoord2f(1.0f,1.0f);		glVertex2f(x2,y2);
		glTexCoord2f(1.0f,0.0f);		glVertex2f(x1,y2);
	glEnd();
}
const void GFX::drawtexture(const GLuint &tex, const float &x1, const float &y1, const float &x2, const float &y2, const uint32 &col)
{
	set_color(col);
	drawtexture( tex, x1, y1, x2, y2 );
}


const void GFX::print(const GFX_FONT &font, const float &x,const float &y,const float &z,const String text)		// Font related routines
{
	print( font, x, y, z, text.c_str() );
}
const void GFX::print(const GFX_FONT &font, const float &x,const float &y,const float &z, const uint32 &col, const String text)		// Font related routines
{
	print( font, x, y, z, col, text.c_str() );
}

const void GFX::print(const GFX_FONT &font, const float &x,const float &y,const float &z,const char *text)		// Font related routines
{
	ReInitTexSys( false );
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);
	if(font.clear)
		glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glScalef(font.size,font.size,1.0f);
	allegro_gl_printf_ex(font._gl, x/font.size, y/font.size, z, text );
	glPopMatrix();
	glPopAttrib();
}
const void GFX::print(const GFX_FONT &font, const float &x,const float &y,const float &z, const uint32 &col, const char *text)		// Font related routines
{
	set_color(col);
	print( font, x, y, z, text );
}

const void GFX::print(const GFX_FONT &font, const float &x,const float &y,const float &z,const String text, float s)		// Font related routines
{
	print( font, x, y, z, text.c_str() , s );
}
const void GFX::print(const GFX_FONT &font, const float &x,const float &y,const float &z, const uint32 &col, const String text, float s)		// Font related routines
{
	print( font, x, y, z, col, text.c_str() , s );
}

const void GFX::print(const GFX_FONT &font, const float &x,const float &y,const float &z,const char *text,float s)		// Font related routines
{
	ReInitTexSys( false );
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);
	if(font.clear)
		glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	if( s > 0.0f )	{
		glScalef(s,s,1.0f);
		allegro_gl_printf_ex(font._gl, x/s, y/s, z, text );
		}
	glPopMatrix();
	glPopAttrib();
}
const void GFX::print(const GFX_FONT &font, const float &x,const float &y,const float &z, const uint32 &col, const char *text,float s)		// Font related routines
{
	set_color(col);
	print( font, x, y, z, text , s );
}


const void GFX::print_center(const GFX_FONT &font, const float &x,const float &y,const float &z,const String text)		// Font related routines
{
	print_center( font, x, y, z, text.c_str() );
}
const void GFX::print_center(const GFX_FONT &font, const float &x,const float &y,const float &z, const uint32 &col, const String text)		// Font related routines
{
	print_center( font, x, y, z, col, text.c_str() );
}

const void GFX::print_center(const GFX_FONT &font, const float &x,const float &y,const float &z,const char *text)		// Font related routines
{
	ReInitTexSys( false );
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);
	if(font.clear)
		glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	float X = x - 0.5f * font.length( text );

	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glScalef(font.size,font.size,1.0f);
	allegro_gl_printf_ex(font._gl, X/font.size, y/font.size, z, text );
	glPopMatrix();
	glPopAttrib();
}
const void GFX::print_center(const GFX_FONT &font, const float &x,const float &y,const float &z, const uint32 &col, const char *text)		// Font related routines
{
	set_color(col);
	print_center( font, x, y, z, text );
}

const void GFX::print_center(const GFX_FONT &font, const float &x,const float &y,const float &z,const String text, float s)		// Font related routines
{
	print_center( font, x, y, z, text.c_str() , s );
}
const void GFX::print_center(const GFX_FONT &font, const float &x,const float &y,const float &z, const uint32 &col, const String text, float s)		// Font related routines
{
	print_center( font, x, y, z, col, text.c_str() , s );
}

const void GFX::print_center(const GFX_FONT &font, const float &x,const float &y,const float &z,const char *text,float s)		// Font related routines
{
	ReInitTexSys( false );
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);
	if(font.clear)
		glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	float X = x - 0.5f * font.length( text );

	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	if( s > 0.0f )	{
		glScalef(s,s,1.0f);
		allegro_gl_printf_ex(font._gl, X/s, y/s, z, text );
		}
	glPopMatrix();
	glPopAttrib();
}
const void GFX::print_center(const GFX_FONT &font, const float &x,const float &y,const float &z, const uint32 &col, const char *text,float s)		// Font related routines
{
	set_color(col);
	print_center( font, x, y, z, text , s );
}


const void GFX::print_right(const GFX_FONT &font, const float &x,const float &y,const float &z,const String text)		// Font related routines
{
	print_right( font, x, y, z, text.c_str() );
}
const void GFX::print_right(const GFX_FONT &font, const float &x,const float &y,const float &z, const uint32 &col, const String text)		// Font related routines
{
	print_right( font, x, y, z, col, text.c_str() );
}

const void GFX::print_right(const GFX_FONT &font, const float &x,const float &y,const float &z,const char *text)		// Font related routines
{
	ReInitTexSys( false );
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);
	if(font.clear)
		glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	float X = x - font.length( text );

	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glScalef(font.size,font.size,1.0f);
	allegro_gl_printf_ex(font._gl, X/font.size, y/font.size, z, text );
	glPopMatrix();
	glPopAttrib();
}
const void GFX::print_right(const GFX_FONT &font, const float &x,const float &y,const float &z, const uint32 &col, const char *text)		// Font related routines
{
	set_color(col);
	print_right( font, x, y, z, text );
}

const void GFX::print_right(const GFX_FONT &font, const float &x,const float &y,const float &z,const String text, float s)		// Font related routines
{
	print_right( font, x, y, z, text.c_str() , s );
}
const void GFX::print_right(const GFX_FONT &font, const float &x,const float &y,const float &z, const uint32 &col, const String text, float s)		// Font related routines
{
	print_right( font, x, y, z, col, text.c_str() , s );
}

const void GFX::print_right(const GFX_FONT &font, const float &x,const float &y,const float &z,const char *text,float s)		// Font related routines
{
	ReInitTexSys( false );
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);
	if(font.clear)
		glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	float X = x - font.length( text );

	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	if( s > 0.0f )	{
		glScalef(s,s,1.0f);
		allegro_gl_printf_ex(font._gl, X/s, y/s, z, text );
		}
	glPopMatrix();
	glPopAttrib();
}
const void GFX::print_right(const GFX_FONT &font, const float &x,const float &y,const float &z, const uint32 &col, const char *text,float s)		// Font related routines
{
	set_color(col);
	print_right( font, x, y, z, text , s );
}

GLuint	GFX::make_texture(BITMAP *bmp, byte filter_type, bool clamp )
{
	EnterCS();

	int max_tex_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_tex_size);

	if( bmp->w > max_tex_size || bmp->h > max_tex_size ) {
		BITMAP *tmp = create_bitmap_ex( bitmap_color_depth( bmp ), min( bmp->w, max_tex_size ), min( bmp->h, max_tex_size ) );
		
		stretch_blit( bmp, tmp, 0, 0, bmp->w, bmp->h, 0, 0, tmp->w, tmp->h );
		
		GLuint tex = make_texture( tmp, filter_type, clamp );
		
		destroy_bitmap( tmp );
		
		return tex;
		}

	glPushAttrib( GL_ALL_ATTRIB_BITS );

	if( ati_workaround && filter_type != FILTER_NONE
		&& ( !IsPowerOfTwo( bmp->w ) || !IsPowerOfTwo( bmp->h ) ) )
		filter_type = FILTER_LINEAR;

	if( filter_type == FILTER_NONE || filter_type == FILTER_LINEAR )
		allegro_gl_use_mipmapping(false);
	else
		allegro_gl_use_mipmapping(true);
	GLuint gl_tex = allegro_gl_make_texture(bmp);
	if( filter_type == FILTER_NONE || filter_type == FILTER_LINEAR )
		allegro_gl_use_mipmapping(true);
	glBindTexture(GL_TEXTURE_2D, gl_tex);

	glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	if( clamp ) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
		}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		}

	switch(filter_type)
	{
	case FILTER_NONE:
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		break;
	case FILTER_LINEAR:
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		break;
	case FILTER_BILINEAR:
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		break;
	case FILTER_TRILINEAR:
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		break;
	};

	glPopAttrib();

	LeaveCS();
	return gl_tex;
}

GLuint	GFX::create_texture(int w, int h, byte filter_type, bool clamp )
{
	BITMAP *tmp = create_bitmap( w, h );
	clear( tmp );

	GLuint tex = make_texture( tmp, filter_type, clamp );

	destroy_bitmap( tmp );

	return tex;
}

void	GFX::blit_texture( BITMAP *src, GLuint &dst )
{
	if( dst == 0 )	return;

	glBindTexture( GL_TEXTURE_2D, dst );

	glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, src->w, src->h, GL_RGBA, GL_UNSIGNED_BYTE, src->line[0] );
}

GLuint GFX::load_texture(String file, byte filter_type, uint32 *width, uint32 *height, bool clamp )
{
	if( !exists( file.c_str() ) )	return 0;		// The file doesn't exist

	set_color_depth(32);
	BITMAP *bmp = load_bitmap( file.c_str(), NULL);
	if( bmp == NULL )	return 0;					// Operation failed
	if( width )		*width = bmp->w;
	if( height )	*height = bmp->h;
	if( bitmap_color_depth(bmp) != 32 ) {
		BITMAP *tmp = create_bitmap_ex( 32, bmp->w, bmp->h );
		blit( bmp, tmp, 0, 0, 0, 0, bmp->w, bmp->h);
		destroy_bitmap(bmp);
		bmp=tmp;
		}
	bool with_alpha = Lowercase( get_extension( file.c_str() ) ) == "tga";
	if( with_alpha ) {
		with_alpha = false;
		for( int y = 0 ; y < bmp->h && !with_alpha ; y++ )
			for( int x = 0 ; x < bmp->w && !with_alpha ; x++ )
				with_alpha |= bmp->line[y][(x<<2)+3] != 255;
		}
	if(g_useTextureCompression)
		allegro_gl_set_texture_format( with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB );
	else
		allegro_gl_set_texture_format( with_alpha ? GL_RGBA8 : GL_RGB8 );
	allegro_gl_use_alpha_channel( with_alpha );
	GLuint gl_tex = make_texture( bmp, filter_type, clamp );
	allegro_gl_use_alpha_channel(false);
	destroy_bitmap(bmp);
	return gl_tex;
}

GLuint	GFX::load_texture_mask( String file, int level, byte filter_type, uint32 *width, uint32 *height, bool clamp )
{
	if( !exists( file.c_str() ) )	return 0;		// The file doesn't exist

	set_color_depth(32);
	BITMAP *bmp = load_bitmap( file.c_str(), NULL);
	if( bmp == NULL )	return 0;					// Operation failed
	if( width )		*width = bmp->w;
	if( height )	*height = bmp->h;
	if( bitmap_color_depth(bmp) != 32 ) {
		BITMAP *tmp = create_bitmap_ex( 32, bmp->w, bmp->h );
		blit( bmp, tmp, 0, 0, 0, 0, bmp->w, bmp->h);
		destroy_bitmap(bmp);
		bmp=tmp;
		}
	bool with_alpha = Lowercase( get_extension( file.c_str() ) ) == "tga";
	if( with_alpha ) {
		with_alpha = false;
		for( int y = 0 ; y < bmp->h && !with_alpha ; y++ )
			for( int x = 0 ; x < bmp->w && !with_alpha ; x++ )
				with_alpha |= bmp->line[y][(x<<2)+3] != 255;
		}
	else
		for( int y = 0 ; y < bmp->h ; y++ )
			for( int x = 0 ; x < bmp->w ; x++ )
				bmp->line[y][(x<<2)+3] = 255;

	for( int y = 0 ; y < bmp->h ; y++ )
		for( int x = 0 ; x < bmp->w ; x++ )
			if( bmp->line[y][(x<<2)] < level && bmp->line[y][(x<<2)+1] < level && bmp->line[y][(x<<2)+2] < level ) {
				bmp->line[y][(x<<2)+3] = 0;
				with_alpha = true;
				}
	if(g_useTextureCompression)
		allegro_gl_set_texture_format( with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB );
	else
		allegro_gl_set_texture_format( with_alpha ? GL_RGBA8 : GL_RGB8 );
	allegro_gl_use_alpha_channel( with_alpha );
	GLuint gl_tex = make_texture( bmp, filter_type, clamp );
	allegro_gl_use_alpha_channel(false);
	destroy_bitmap(bmp);
	return gl_tex;
}

GLuint	GFX::load_texture_from_cache( String file, byte filter_type, uint32 *width, uint32 *height, bool clamp )
{
	if( ati_workaround || !lp_CONFIG->use_texture_cache )	return 0;

	file = TA3D_OUTPUT_DIR + "cache/" + file;

	if(TA3D::FileExists( file ))
    {
		FILE *cache_file = TA3D_OpenFile( file, "rb" );

		uint32 mod_hash;

		fread( &mod_hash, sizeof( mod_hash ), 1, cache_file );

		if( mod_hash != hash_string( TA3D_CURRENT_MOD ) ) {		// Doesn't correspond to current mod
			fclose( cache_file );
			return 0;
			}

		glPushAttrib( GL_ALL_ATTRIB_BITS );

		uint32 rw, rh;

		fread( &rw, 4, 1, cache_file );
		fread( &rh, 4, 1, cache_file );

		if( width )		*width = rw;
		if( height )	*height = rh;

		byte *img = new byte[ rw * rh * 5 ];

		int lod_max = 0;
		GLint size, internal_format;

		fread( &lod_max, sizeof( lod_max ), 1, cache_file );
		fread( &internal_format, sizeof( GLint ), 1, cache_file );

		GLuint	tex = gfx->create_texture( 1, 1 );
		
		glBindTexture( GL_TEXTURE_2D, tex );

		for( int lod = 0 ; lod  < lod_max ; lod++ ) {
			fread( &size, sizeof( GLint ), 1, cache_file );
			fread( &internal_format, sizeof( GLint ), 1, cache_file );
			fread( img, size, 1, cache_file );

			glCompressedTexImage2DARB( GL_TEXTURE_2D, lod, internal_format, rw, rh, 0, size, img);
			}

		delete img;

		fclose( cache_file );

		glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

		if( clamp ) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
			}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			}

		switch(filter_type)
		{
		case FILTER_NONE:
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			break;
		case FILTER_LINEAR:
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			break;
		case FILTER_BILINEAR:
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			break;
		case FILTER_TRILINEAR:
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
			break;
		};

		glPopAttrib();

		return tex;
		}
	return 0;				// File isn't in cache
}

void	GFX::save_texture_to_cache( String file, GLuint tex, uint32 width, uint32 height )
{
	if( ati_workaround || !lp_CONFIG->use_texture_cache )	return;

	file = TA3D_OUTPUT_DIR + "cache/" + file;

	int rw = texture_width( tex ), rh = texture_height( tex );		// Also binds tex

	GLint	compressed;
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed );

	if( !compressed )	return;		// Do not save it if it's not compressed -> save disk space, and it would slow things down

	FILE *cache_file = TA3D_OpenFile( file, "wb" );

	if( cache_file == NULL )
		return;

	uint32 mod_hash = hash_string( TA3D_CURRENT_MOD );			// Save a hash of current mod

	fwrite( &mod_hash, sizeof( mod_hash ), 1, cache_file );

	fwrite( &width, 4, 1, cache_file );
	fwrite( &height, 4, 1, cache_file );

	byte *img = new byte[ rw * rh * 5 ];

	float lod_max_f = max( log( rw ), log( rh ) ) / log( 2.0f );
	int lod_max = ((int) lod_max_f);
	if( lod_max > lod_max_f )
		lod_max++;
		
	fwrite( &lod_max, sizeof( lod_max ), 1, cache_file );

	GLint size, internal_format;
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format );
	fwrite( &internal_format, sizeof( GLint ), 1, cache_file );

	for( int lod = 0 ; lod  < lod_max ; lod++ ) {
		glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &size );
		glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_INTERNAL_FORMAT, &internal_format );
		glGetCompressedTexImageARB( GL_TEXTURE_2D, lod, img );
		fwrite( &size, sizeof( GLint ), 1, cache_file );
		fwrite( &internal_format, sizeof( GLint ), 1, cache_file );
		fwrite( img, size, 1, cache_file );
		}

	delete img;

	fclose( cache_file );
}

GLuint	GFX::load_masked_texture(String file, String mask, byte filter_type )
{
	if( !exists( file.c_str() ) )	return 0;		// The file doesn't exist
	if( !exists( mask.c_str() ) )	return 0;		// The file doesn't exist

	set_color_depth(32);
	BITMAP *bmp=load_bitmap( file.c_str(), NULL );
	if( bmp == NULL )	return 0;					// Operation failed
	BITMAP *alpha;
	set_color_depth(8);
	alpha=load_bitmap( mask.c_str(), NULL );
	if( alpha == NULL )	{
		destroy_bitmap( alpha );
		return 0;					// Operation failed
		}
	set_color_depth(32);
	for(int y=0;y<bmp->h;y++)
		for(int x=0;x<bmp->w;x++)
			bmp->line[y][(x<<2)+3]=alpha->line[y][x];
	allegro_gl_use_alpha_channel(true);
	if(g_useTextureCompression)
		allegro_gl_set_texture_format(GL_COMPRESSED_RGBA_ARB);
	else
		allegro_gl_set_texture_format(GL_RGBA8);
	GLuint gl_tex = make_texture( bmp, filter_type );
	allegro_gl_use_alpha_channel(false);
	destroy_bitmap(bmp);
	destroy_bitmap(alpha);
	return gl_tex;
}

uint32	GFX::texture_width( const GLuint &gltex )
{
	GLint width;
	glBindTexture( GL_TEXTURE_2D, gltex);
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
	return width;
}

uint32	GFX::texture_height( const GLuint &gltex )
{
	GLint height;
	glBindTexture( GL_TEXTURE_2D, gltex);
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );
	return height;
}

void GFX::destroy_texture(GLuint &gltex)
{
	if( gltex )						// Test if the texture exists
		glDeleteTextures(1,&gltex);
	gltex = 0;						// The texture is destroyed
}

GLuint	GFX::make_texture_from_screen( byte filter_type)				// Copy pixel data from screen to a texture
{
	BITMAP *tmp = create_bitmap_ex(32, SCREEN_W, SCREEN_H);
	GLuint gltex = make_texture(tmp, filter_type);
	destroy_bitmap(tmp);

	glBindTexture(GL_TEXTURE_2D,gltex);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, SCREEN_W, SCREEN_H, 0);

	return gltex;
}

void GFX::set_alpha_blending()
{
	glPushAttrib(GL_ENABLE_BIT);					// Push OpenGL attribs to pop them later when we unset alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	alpha_blending_set = true;
}

void GFX::unset_alpha_blending()
{
	if( alpha_blending_set )
		glPopAttrib();								// Pop OpenGL attribs to pop them later when we unset alpha blending
	else
		glDisable(GL_BLEND);
	alpha_blending_set = false;
}

void GFX::ReInitTexSys( bool matrix_reset )
{
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	if( matrix_reset ) {
		glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		}
}

void GFX::ReInitAllTex( bool disable )
{
	for(uint32 i = 0; i < 8; i++) {
		glActiveTextureARB(GL_TEXTURE0_ARB + i);
		ReInitTexSys();
		if( disable )
			glDisable(GL_TEXTURE_2D);
		}
	glActiveTextureARB(GL_TEXTURE0_ARB);
}

void GFX::SetDefState()
{
	glClearColor (0, 0, 0, 0);
	glShadeModel (GL_SMOOTH);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glPolygonMode (GL_BACK, GL_POINTS);
	glDepthFunc( GL_LESS );
	glEnable (GL_DEPTH_TEST);
	glCullFace (GL_BACK);
	glEnable (GL_CULL_FACE);
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glHint(GL_FOG_HINT, GL_FASTEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	ReInitTexSys();
	alpha_blending_set = false;
}

void GFX::ReInitArrays()
{
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

void GFX_FONT::load( const char *filename, const float s)
{
	size = s;
	_al = load_font( filename, NULL, NULL );
	if( _al == NULL ) {
		throw cError( "GFX_FONT::load()", "font could not be loaded, _al = NULL.", true );
		return;
		}
	_gl = allegro_gl_convert_allegro_font_ex(_al,AGL_FONT_TYPE_TEXTURED,-1.0f,GL_RGBA8);
	if( _gl == NULL ) {
		throw cError( "GFX_FONT::load()", "font could not be converted to GL font, _gl = NULL.", true );
		return;
		}
}

void GFX_FONT::load_gaf_font( const char *filename, const float s )
{
	size = s;
	byte *data = HPIManager->PullFromHPI( filename );
	if( data ) {
		ANIM gaf_font;
		gaf_font.load_gaf( data );			// Load the gaf
		int h = 0, mx = 0, my = 0;
		for(int i = 0 ; i < gaf_font.nb_bmp ; i++ ) {
			if( abs( gaf_font.ofs_x[ i ] ) > 50 || abs( gaf_font.ofs_y[ i ] ) > 50 ) continue;
			if( -gaf_font.ofs_x[ i ] < mx )	mx = -gaf_font.ofs_x[ i ];
			if( -gaf_font.ofs_y[ i ] < my )	my = -gaf_font.ofs_y[ i ];
			if( gaf_font.bmp[ i ]->h > h )	h = gaf_font.bmp[ i ]->h;
			}
		my+=2;
		h-=2;

		FONT_COLOR_DATA	*fc = ( FONT_COLOR_DATA* ) malloc( sizeof(FONT_COLOR_DATA) );
		fc->begin = 0;
		fc->end = gaf_font.nb_bmp;
		fc->bitmaps = (BITMAP**) malloc( sizeof( BITMAP* ) * gaf_font.nb_bmp );
		fc->next = NULL;
		for(int i = 0 ; i < gaf_font.nb_bmp ; i++ ) {
			fc->bitmaps[ i ] = create_bitmap_ex( 32, gaf_font.bmp[ i ]->w, h );
			clear_to_color( fc->bitmaps[ i ], 0xFF00FF );
			if( i != 32 )		// Spaces must remain blank
				blit( gaf_font.bmp[ i ], fc->bitmaps[ i ], 0,0, -gaf_font.ofs_x[ i ] - mx, -gaf_font.ofs_y[ i ] - my, gaf_font.bmp[ i ]->w, gaf_font.bmp[ i ]->h );
			for( int y = 0 ; y < fc->bitmaps[ i ]->h ; y++ )
				for( int x = 0 ; x < fc->bitmaps[ i ]->w ; x++ )
					if( getpixel( fc->bitmaps[ i ], x, y ) == 0 )
						putpixel( fc->bitmaps[ i ], x, y, 0xFF00FF );
			}
		gaf_font.destroy();					// Destroy the gaf data, we don't need this any more
		free( data );
		_al = ( FONT* ) malloc( sizeof( FONT ) );
		_al->data = fc;
		_al->height = h;
		_al->vtable = font_vtable_color;
		}
	else {
		_al = NULL;
		throw cError( "GFX_FONT::load_gaf_font()", "file could not be read, data = NULL.", true );
		return;
		}

	_gl = allegro_gl_convert_allegro_font_ex(_al,AGL_FONT_TYPE_TEXTURED,-1.0f,GL_RGBA8);
	if( _gl == NULL ) {
		throw cError( "GFX_FONT::load_gaf_font()", "font could not be converted to GL font, _gl = NULL.", true );
		return;
		}
}

float GFX_FONT::height() const	{	return text_height( _al ) * size;	}

float GFX_FONT::length( const String txt) const	{	return text_length( _al, txt.c_str() ) * size;	}

void GFX_FONT::destroy()
{
	if( _al )	destroy_font( _al );
	if( _gl )	allegro_gl_destroy_font( _gl );
	init();
}

void GFX_FONT::copy( FONT *fnt, const float s)
{
	size = s;
	_al = extract_font_range(font, -1, -1);
	if( _al == NULL ) {
		throw cError( "GFX_FONT::copy()", "font could not be copied, _al = NULL.", true );
		return;
		}
	_gl = allegro_gl_convert_allegro_font_ex(_al,AGL_FONT_TYPE_TEXTURED,-1.0f,GL_RGBA8);
	if( _gl == NULL ) {
		throw cError( "GFX_FONT::copy()", "font could not be converted to GL font, _gl = NULL.", true );
		return;
		}
}

uint32 GFX::InterfaceMsg( const lpcImsg msg )
{
	if( msg->MsgID != TA3D_IM_GFX_MSG )
		return INTERFACE_RESULT_CONTINUE;

	return INTERFACE_RESULT_CONTINUE;
}

const void GFX::flip()	{	allegro_gl_flip();	}

void GFX::disable_texturing()
{
	glDisable( GL_TEXTURE_2D );
}

void GFX::enable_texturing()
{
	glEnable( GL_TEXTURE_2D );
}
