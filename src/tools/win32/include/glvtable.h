#ifndef ALLEGRO_GL_WINDOWS_VTABLE_H
#define ALLEGRO_GL_WINDOWS_VTABLE_H

#include <allegro.h>


/* Special structure for holding video bitmaps. This piggy-backs
 * over the regular BITMAP structure to provide AllegroGL with the
 * necessary information for maintaining video bitmap over OpenGL
 */
typedef struct AGL_VIDEO_BITMAP {
	GLenum target;         /* GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_NV, etc */
	BITMAP *memory_copy;   /* Memory copy for reading -> subbitmap */
	GLuint tex;            /* Associated texture number */
	int x_ofs, y_ofs;      /* Offsets of this block */
	int width, height;     /* Size of the bitmap block */
	struct AGL_VIDEO_BITMAP *next; /* Next block, if bitmap is too large to fit in one texture */
} AGL_VIDEO_BITMAP;


void __allegro_gl__glvtable_update_vtable (GFX_VTABLE **vtable);
BITMAP *allegro_gl_create_video_bitmap(int w, int h);
void allegro_gl_destroy_video_bitmap(BITMAP *bmp);
void allegro_gl_created_sub_bitmap(BITMAP *bmp, BITMAP *parent);
void split_color(int color, GLubyte *r, GLubyte *g, GLubyte *b, GLubyte *a,
		int color_depth);
void allegro_gl_screen_blit_to_self (struct BITMAP *source, struct BITMAP *dest,
		int source_x, int source_y, int dest_x, int dest_y, int width,
		int height);
void allegro_gl_video_blit_from_memory(struct BITMAP *source,
		struct BITMAP *dest, int source_x, int source_y, int dest_x,
		int dest_y, int width, int height);
void allegro_gl_video_blit_to_memory(struct BITMAP *source, struct BITMAP *dest,
		int source_x, int source_y, int dest_x, int dest_y, int width,
		int height);
#endif

