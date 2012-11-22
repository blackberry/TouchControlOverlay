/*
 * Copyright (c) 2011 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pngreader.h"

PNGReader::PNGReader(FILE *file, screen_context_t context)
	: m_context(context)
	, m_read(0)
	, m_info(0)
	, m_data(0)
	, m_rows(0)
	, m_width(0)
	, m_height(0)
	, m_stride(0)
	, m_file(file)
	, m_pixmap(0)
	, m_buffer(0)
{}

PNGReader::~PNGReader()
{
	delete [] m_rows;
	delete [] m_data;

	if (m_read)
		png_destroy_read_struct(&m_read, m_info ? &m_info : (png_infopp) 0, (png_infopp) 0);
	m_read = 0;

	if (m_file)
		fclose(m_file);
	m_file = 0;

	if (m_pixmap)
		screen_destroy_pixmap(m_pixmap);
	else if (m_buffer)
		screen_destroy_buffer(m_buffer);

	m_pixmap = 0;
	m_buffer = 0;
}

bool PNGReader::doRead()
{
	m_read = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!m_read) {
		fprintf(stderr, "Failed to create PNG read struct\n");
		return false;
	}

	m_info = png_create_info_struct(m_read);
	if (!m_info) {
		fprintf(stderr, "Failed to create PNG info struct\n");
		return false;
	}

	// Exception handling
	if (setjmp(png_jmpbuf(m_read))) {
		fprintf(stderr, "PNG jumped to failure\n");
		return false;
	}

	png_init_io(m_read, m_file);
	png_read_info(m_read, m_info);

	m_width = png_get_image_width(m_read, m_info);
	if (m_width <= 0) {
		fprintf(stderr, "Invalid PNG width: %d\n", m_width);
		return false;
	}

	m_height = png_get_image_height(m_read, m_info);
	if (m_height <= 0) {
		fprintf(stderr, "Invalid PNG height: %d\n", m_height);
		return false;
	}

	const int channels = 4;
	png_set_palette_to_rgb(m_read);
	png_set_tRNS_to_alpha(m_read);
	png_set_bgr(m_read);
	png_set_expand(m_read);
	png_set_strip_16(m_read);
	png_set_gray_to_rgb(m_read);
	if (png_get_channels(m_read, m_info) < channels)
		png_set_filler(m_read, 0xff, PNG_FILLER_AFTER);

	m_data = new unsigned char[m_width * m_height * channels];
	m_stride = m_width * channels;
	m_rows = new png_bytep[m_height];

	for (int i=m_height - 1; i >= 0; --i) {
		m_rows[i] = (png_bytep)(m_data + i * m_stride);
	}
	png_read_image(m_read, m_rows);

	int rc;
	{
		int format = SCREEN_FORMAT_RGBA8888;
		int size[2] = {m_width, m_height};

		rc = screen_create_pixmap(&m_pixmap, m_context); // FIXME: Check failure
		rc = screen_set_pixmap_property_iv(m_pixmap, SCREEN_PROPERTY_FORMAT, &format);
		rc = screen_set_pixmap_property_iv(m_pixmap, SCREEN_PROPERTY_BUFFER_SIZE, size);
		rc = screen_create_pixmap_buffer(m_pixmap);

		unsigned char *realPixels;
		int realStride;
		rc = screen_get_pixmap_property_pv(m_pixmap, SCREEN_PROPERTY_RENDER_BUFFERS, (void**)&m_buffer);
		rc = screen_get_buffer_property_pv(m_buffer, SCREEN_PROPERTY_POINTER, (void **)&realPixels);
		rc = screen_get_buffer_property_iv(m_buffer, SCREEN_PROPERTY_STRIDE, &realStride);

		for (int i=0; i<m_height; i++) {
			memcpy(realPixels + i * realStride, m_data + i * m_stride, m_stride);
		}
	}
	return true;
}
