/*
 *         __   _____    ______   ______   ___    ___
 *        /\ \ /\  _ `\ /\  ___\ /\  _  \ /\_ \  /\_ \
 *        \ \ \\ \ \L\ \\ \ \__/ \ \ \L\ \\//\ \ \//\ \      __     __
 *      __ \ \ \\ \  __| \ \ \  __\ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\
 *     /\ \_\/ / \ \ \/   \ \ \L\ \\ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \
 *     \ \____//  \ \_\    \ \____/ \ \_\ \_\/\____\/\____\ \____\ \____ \
 *      \/____/    \/_/     \/___/   \/_/\/_/\/____/\/____/\/____/\/___L\ \
 *                                                                  /\____/
 *                                                                  \_/__/
 *
 *      From JPGAlleg Version 2.5, by Angelo Mottola, 2000-2004
 *
 *      Public library functions.
 *
 *		This code has been adapted to support 64 bits platforms without crashing
 *
 *		This file is a module of TA3D
 *
 */


#include "internal.h"


HUFFMAN_TABLE _jpeg_huffman_ac_table[4];
HUFFMAN_TABLE _jpeg_huffman_dc_table[4];
IO_BUFFER _jpeg_io;

const unsigned char _jpeg_zigzag_scan[64] = {
	 0, 1, 5, 6,14,15,27,28,
	 2, 4, 7,13,16,26,29,42,
	 3, 8,12,17,25,30,41,43,
	 9,11,18,24,31,40,44,53,
	10,19,23,32,39,45,52,54,
	20,22,33,38,46,51,55,60,
	21,34,37,47,50,56,59,61,
	35,36,48,49,57,58,62,63
};

const char *_jpeg_component_name[] = { "Y", "Cb", "Cr" };

int jpgalleg_error = JPG_ERROR_NONE;


/* _jpeg_trace:
 *  Internal debugging routine: prints error to stderr if in debug mode.
 */
void
_jpeg_trace(const char *msg, ...)
{
#ifdef DEBUG
	va_list ap;

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
	fprintf(stderr, "\n");
#else
	(void)msg;
#endif
}


/* load_datafile_jpg:
 *  Hook function for loading a JPEG object from a datafile. Returns the
 *  decoded JPG into a SDL_Surface or NULL on error.
 */
static void *
load_datafile_jpg(FILE *f, long size)
{
	SDL_Surface *bmp;
	char *buffer;

	buffer = (char *)malloc(size);
	if (!buffer)
		return NULL;
	fread(buffer, size, 1, f);
	bmp = load_memory_jpg(buffer, size, NULL);
	free(buffer);

	return (void *)bmp;
}


/* destroy_datafile_jpg:
 *  Hook function for freeing memory of JPEG objects in a loaded datafile.
 */
static void
destroy_datafile_jpg(void *data)
{
	if (data)
		SDL_FreeSurface((SDL_Surface *)data);
}


/* jpgalleg_init:
 *  Initializes JPGalleg by registering the file format with the Allegro image
 *  handling and datafile subsystems.
 */
int
jpgalleg_init(void)
{
	jpgalleg_error = JPG_ERROR_NONE;

	return 0;
}


/* load_jpg:
 *  Loads a JPG image from a file into a BITMAP, with no progress callback.
 */
SDL_Surface *
load_jpg(const char *filename, SDL_Color *palette)
{
	return load_jpg_ex(filename, palette, NULL);
}


/* load_jpg_ex:
 *  Loads a JPG image from a file into a BITMAP.
 */
SDL_Surface *
load_jpg_ex(const char *filename, SDL_Color *palette, void (*callback)(int progress))
{
	FILE *f;
	SDL_Surface *bmp;
	SDL_Color pal[256];
	int size;

	if (!palette)
		palette = pal;

	size = TA3D::file_size(filename);

	_jpeg_io.buffer = _jpeg_io.buffer_start = (unsigned char *)malloc(size);
	_jpeg_io.buffer_end = _jpeg_io.buffer_start + size;
	if (!_jpeg_io.buffer)
	{
		TRACE("Out of memory");
		jpgalleg_error = JPG_ERROR_OUT_OF_MEMORY;
		return NULL;
	}
	f = fopen(filename, "r");
	if (!f)
	{
		TRACE("Cannot open %s for reading", filename);
		jpgalleg_error = JPG_ERROR_READING_FILE;
		free(_jpeg_io.buffer);
		return NULL;
	}
	fread(_jpeg_io.buffer, size, 1, f);
	fclose(f);

	TRACE("Loading JPG from file %s", filename);

	bmp = _jpeg_decode(palette, callback);

	free(_jpeg_io.buffer_start);
	return bmp;
}


/* load_memory_jpg:
 *  Loads a JPG image from a memory buffer into a BITMAP, with no progress
 *  callback.
 */
SDL_Surface *
load_memory_jpg(const void *buffer, int size, SDL_Color *palette)
{
	return load_memory_jpg_ex(buffer, size, palette, NULL);
}


/* load_memory_jpg:
 *  Loads a JPG image from a memory buffer into a BITMAP.
 */
SDL_Surface *
load_memory_jpg_ex(const void *buffer, int size, SDL_Color *palette, void (*callback)(int progress))
{
	SDL_Surface *bmp;
	SDL_Color pal[256];

	if (!palette)
		palette = pal;

	_jpeg_io.buffer = _jpeg_io.buffer_start = (unsigned char*)buffer;
	_jpeg_io.buffer_end = _jpeg_io.buffer_start + size;

	TRACE("Loading JPG from memory buffer at %p (size = %d)", buffer, size);

	bmp = _jpeg_decode(palette, callback);

	return bmp;
}


/* save_jpg:
 *  Saves specified BITMAP into a JPG file with quality 75, no subsampling
 *  and no progress callback.
 */
int
save_jpg(const char *filename, SDL_Surface *bmp, const SDL_Color *palette)
{
	return save_jpg_ex(filename, bmp, palette, DEFAULT_QUALITY, DEFAULT_FLAGS, NULL);
}


/* save_jpg_ex:
 *  Saves a BITMAP into a JPG file using given quality, subsampling mode and
 *  progress callback.
 */
int
save_jpg_ex(const char *filename, SDL_Surface *bmp, const SDL_Color *palette, int quality, int flags, void (*callback)(int progress))
{
	FILE *f;
	SDL_Color pal[256];
	int result, size;

	if (!palette)
		palette = pal;

	size = (bmp->w * bmp->h * 3) + 1000;    /* This extimation should be more than enough in all cases */
	_jpeg_io.buffer = _jpeg_io.buffer_start = (unsigned char *)malloc(size);
	_jpeg_io.buffer_end = _jpeg_io.buffer_start + size;
	if (!_jpeg_io.buffer)
    {
		TRACE("Out of memory");
		jpgalleg_error = JPG_ERROR_OUT_OF_MEMORY;
		return -1;
	}
	f = fopen(filename, "r");
	if (!f) {
		TRACE("Cannot open %s for writing", filename);
		jpgalleg_error = JPG_ERROR_WRITING_FILE;
		free(_jpeg_io.buffer);
		return -1;
	}

	TRACE("Saving JPG to file %s", filename);

	result = _jpeg_encode(bmp, palette, quality, flags, callback);
	if (!result)
		fwrite(_jpeg_io.buffer_start, _jpeg_io.buffer - _jpeg_io.buffer_start, 1, f);

	free(_jpeg_io.buffer_start);
	fclose(f);
	return result;
}


/* save_memory_jpg:
 *  Saves a BITMAP in JPG format and stores it into a memory buffer. The JPG
 *  is saved with quality 75, no subsampling and no progress callback.
 */
int
save_memory_jpg(void *buffer, int *size, SDL_Surface *bmp, const SDL_Color *palette)
{
	return save_memory_jpg_ex(buffer, size, bmp, palette, DEFAULT_QUALITY, DEFAULT_FLAGS, NULL);
}


/* save_memory_jpg_ex:
 *  Saves a BITMAP in JPG format using given quality and subsampling settings
 *  and stores it into a memory buffer.
 */
int
save_memory_jpg_ex(void *buffer, int *size, SDL_Surface *bmp, const SDL_Color *palette, int quality, int flags, void (*callback)(int progress))
{
	int result;

	if (!buffer)
	{
		TRACE("Invalid buffer pointer");
		return -1;
	}

	TRACE("Saving JPG to memory buffer at %p (size = %d)", buffer, *size);

	_jpeg_io.buffer = _jpeg_io.buffer_start = (unsigned char*)buffer;
	_jpeg_io.buffer_end = _jpeg_io.buffer_start + *size;
	*size = 0;

	result = _jpeg_encode(bmp, palette, quality, flags, callback);

	if (result == 0)
		*size = _jpeg_io.buffer - _jpeg_io.buffer_start;
	return result;
}
