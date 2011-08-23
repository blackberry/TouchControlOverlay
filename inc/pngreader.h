/*
 * pngreader.h
 *
 *  Created on: Aug 12, 2011
 *      Author: jnicholl
 */

#ifndef PNGREADER_H_
#define PNGREADER_H_

#include <png.h>
#include <screen/screen.h>

class PNGReader
{
public:
	PNGReader(FILE *file, screen_context_t context);
	~PNGReader();

	bool doRead();

	screen_context_t m_context;
	png_structp m_read;
	png_infop m_info;
	unsigned char* m_data;
	png_bytep* m_rows;
	int m_width;
	int m_height;
	int m_stride;
	FILE *m_file;
	screen_pixmap_t m_pixmap;
	screen_buffer_t m_buffer;
};

#endif /* PNGREADER_H_ */
