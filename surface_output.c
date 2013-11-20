/*
 * Copyright (c) 2013 Jens Kuske <jenskuske@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <string.h>
#include "vdpau_private.h"

VdpStatus vdp_output_surface_create(VdpDevice device, VdpRGBAFormat rgba_format, uint32_t width, uint32_t height, VdpOutputSurface  *surface)
{
	if (!surface)
		return VDP_STATUS_INVALID_POINTER;

	if (!width || !height || width >= 16384 || height >= 16384)
		return VDP_STATUS_INVALID_SIZE;

	device_ctx_t *dev = handle_get(device);
	if (!dev)
		return VDP_STATUS_INVALID_HANDLE;

	output_surface_ctx_t *out = calloc(1, sizeof(output_surface_ctx_t));
	if (!out)
		return VDP_STATUS_RESOURCES;

	out->width = width;
	out->height = height;
	out->rgba_format = rgba_format;
	out->contrast = 1.0;
	out->saturation = 1.0;
	out->device = dev;
	// Do not allocate data yet, we might not need to
	out->data = NULL;

	int handle = handle_create(out);
	if (handle == -1)
	{
		free(out);
		return VDP_STATUS_RESOURCES;
	}

	*surface = handle;

	return VDP_STATUS_OK;
}

VdpStatus vdp_output_surface_destroy(VdpOutputSurface surface)
{
	output_surface_ctx_t *out = handle_get(surface);
	if (!out)
		return VDP_STATUS_INVALID_HANDLE;

	handle_destroy(surface);
	free(out);

	return VDP_STATUS_OK;
}

VdpStatus vdp_output_surface_get_parameters(VdpOutputSurface surface, VdpRGBAFormat *rgba_format, uint32_t *width, uint32_t *height)
{
	output_surface_ctx_t *out = handle_get(surface);
	if (!out)
		return VDP_STATUS_INVALID_HANDLE;

	if (rgba_format)
		*rgba_format = out->rgba_format;

	if (width)
		*width = out->width;

	if (height)
		*height = out->height;

	return VDP_STATUS_OK;
}

VdpStatus vdp_output_surface_get_bits_native(VdpOutputSurface surface, VdpRect const *source_rect, void *const *destination_data, uint32_t const *destination_pitches)
{
	output_surface_ctx_t *out = handle_get(surface);
	if (!out)
		return VDP_STATUS_INVALID_HANDLE;



	return VDP_STATUS_ERROR;
}

VdpStatus vdp_output_surface_put_bits_native(VdpOutputSurface surface, void const *const *source_data, uint32_t const *source_pitches, VdpRect const *destination_rect)
{
	output_surface_ctx_t *out = handle_get(surface);
	if (!out)
		return VDP_STATUS_INVALID_HANDLE;

	VDPAU_DBG_ONCE("%s called but unimplemented!", __func__);



	return VDP_STATUS_OK;
}

static int valid_rect(const VdpRect *rect, const output_surface_ctx_t *out)
{
	return rect->x0 <= rect->x1 &&
	       rect->y0 <= rect->y1 &&
	       rect->x1 < out->width &&
	       rect->y1 < out->height;
}

VdpStatus vdp_output_surface_put_bits_indexed(VdpOutputSurface surface, VdpIndexedFormat source_indexed_format, void const *const *source_data, uint32_t const *source_pitch, VdpRect const *destination_rect, VdpColorTableFormat color_table_format, void const *color_table)
{
	output_surface_ctx_t *out = handle_get(surface);
	if (!out)
		return VDP_STATUS_INVALID_HANDLE;

	if (!valid_rect(destination_rect, out))
		return VDP_STATUS_INVALID_SIZE;

	if (out->rgba_format != VDP_RGBA_FORMAT_B8G8R8A8 ||
	    source_indexed_format != VDP_INDEXED_FORMAT_I8A8 ||
	    color_table_format != VDP_COLOR_TABLE_FORMAT_B8G8R8X8)
	{
		VDPAU_DBG_ONCE("%s called but unimplemented!", __func__);
		return VDP_STATUS_OK;
	}

	if (!out->data)
		out->data = calloc(out->width * 4, out->height);
	if (!out->data)
		return VDP_STATUS_RESOURCES;

	const uint32_t *palette = color_table;
	uint32_t w = destination_rect->x1 - destination_rect->x0;
	uint32_t h = destination_rect->y1 - destination_rect->y0;
	uint32_t *dst = out->data;
	dst += destination_rect->y0 * out->width + destination_rect->x0;
	uint32_t dst_step = out->width - w;
	const uint8_t *src = source_data[0];
	if (source_pitch[0] < 2 * w)
		return VDP_STATUS_INVALID_SIZE;
	uint32_t src_step = source_pitch[0] - 2*w;
	uint32_t x, y;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			*dst = palette[*src++] & 0x00ffffffu;
			*dst++ |= *src++ << 24;
		}
		src += src_step;
		dst += dst_step;
	}
	out->data_clear = 0;
	return VDP_STATUS_OK;
}

VdpStatus vdp_output_surface_put_bits_y_cb_cr(VdpOutputSurface surface, VdpYCbCrFormat source_ycbcr_format, void const *const *source_data, uint32_t const *source_pitches, VdpRect const *destination_rect, VdpCSCMatrix const *csc_matrix)
{
	output_surface_ctx_t *out = handle_get(surface);
	if (!out)
		return VDP_STATUS_INVALID_HANDLE;

	VDPAU_DBG_ONCE("%s called but unimplemented!", __func__);



	return VDP_STATUS_OK;
}

VdpStatus vdp_output_surface_render_output_surface(VdpOutputSurface destination_surface, VdpRect const *destination_rect, VdpOutputSurface source_surface, VdpRect const *source_rect, VdpColor const *colors, VdpOutputSurfaceRenderBlendState const *blend_state, uint32_t flags)
{
	output_surface_ctx_t *out = handle_get(destination_surface);
	if (!out)
		return VDP_STATUS_INVALID_HANDLE;

	if (source_surface == VDP_INVALID_HANDLE)
	{
		// for now assume destination_rect == NULL and
		// always clear the whole surface
		if (out->data)
		{
			free(out->data);
			out->data = NULL;
		}
		return VDP_STATUS_OK;
	}

	output_surface_ctx_t *in = handle_get(source_surface);
	if (!in)
		return VDP_STATUS_INVALID_HANDLE;

	if (!valid_rect(source_rect, in) ||
	    !valid_rect(destination_rect, out))
		return VDP_STATUS_INVALID_SIZE;

	if (out->rgba_format != VDP_RGBA_FORMAT_B8G8R8A8 ||
	    !in->data)
	{
		VDPAU_DBG_ONCE("%s called but unimplemented!", __func__);
		return VDP_STATUS_OK;
	}

	if (out->data && out->data_clear)
		memset(out->data, 0, out->width * 4 * out->height);
	if (!out->data)
		out->data = calloc(out->width * 4, out->height);
	if (!out->data)
		return VDP_STATUS_RESOURCES;

	uint32_t w = destination_rect->x1 - destination_rect->x0;
	uint32_t ws = source_rect->x1 - source_rect->x0;
	if (ws < w) w = ws;
	uint32_t h = destination_rect->y1 - destination_rect->y0;
	uint32_t hs = source_rect->y1 - source_rect->y0;
	if (hs < h) h = hs;
	uint32_t *dst = out->data;
	dst += destination_rect->y0 * out->width + destination_rect->x0;
	const uint32_t *src = in->data;
	src += source_rect->y0 * in->width + source_rect->x0;
	uint32_t y;
	for (y = 0; y < h; y++)
	{
		memcpy(dst, src, w * 4);
		src += in->width;
		dst += out->width;
	}
	out->data_clear = 0;
	return VDP_STATUS_OK;
}

VdpStatus vdp_output_surface_render_bitmap_surface(VdpOutputSurface destination_surface, VdpRect const *destination_rect, VdpBitmapSurface source_surface, VdpRect const *source_rect, VdpColor const *colors, VdpOutputSurfaceRenderBlendState const *blend_state, uint32_t flags)
{
	output_surface_ctx_t *out = handle_get(destination_surface);
	if (!out)
		return VDP_STATUS_INVALID_HANDLE;

	VDPAU_DBG_ONCE("%s called but unimplemented!", __func__);



	return VDP_STATUS_OK;
}

VdpStatus vdp_output_surface_query_capabilities(VdpDevice device, VdpRGBAFormat surface_rgba_format, VdpBool *is_supported, uint32_t *max_width, uint32_t *max_height)
{
	if (!is_supported || !max_width || !max_height)
		return VDP_STATUS_INVALID_POINTER;

	device_ctx_t *dev = handle_get(device);
	if (!dev)
		return VDP_STATUS_INVALID_HANDLE;

	*is_supported = (surface_rgba_format == VDP_RGBA_FORMAT_R8G8B8A8 || surface_rgba_format == VDP_RGBA_FORMAT_B8G8R8A8);
	*max_width = 8192;
	*max_height = 8192;

	return VDP_STATUS_OK;
}

VdpStatus vdp_output_surface_query_get_put_bits_native_capabilities(VdpDevice device, VdpRGBAFormat surface_rgba_format, VdpBool *is_supported)
{
	if (!is_supported)
		return VDP_STATUS_INVALID_POINTER;

	device_ctx_t *dev = handle_get(device);
	if (!dev)
		return VDP_STATUS_INVALID_HANDLE;

	*is_supported = VDP_FALSE;

	return VDP_STATUS_OK;
}

VdpStatus vdp_output_surface_query_put_bits_indexed_capabilities(VdpDevice device, VdpRGBAFormat surface_rgba_format, VdpIndexedFormat bits_indexed_format, VdpColorTableFormat color_table_format, VdpBool *is_supported)
{
	if (!is_supported)
		return VDP_STATUS_INVALID_POINTER;

	device_ctx_t *dev = handle_get(device);
	if (!dev)
		return VDP_STATUS_INVALID_HANDLE;

	*is_supported = VDP_FALSE;

	return VDP_STATUS_OK;
}

VdpStatus vdp_output_surface_query_put_bits_y_cb_cr_capabilities(VdpDevice device, VdpRGBAFormat surface_rgba_format, VdpYCbCrFormat bits_ycbcr_format, VdpBool *is_supported)
{
	if (!is_supported)
		return VDP_STATUS_INVALID_POINTER;

	device_ctx_t *dev = handle_get(device);
	if (!dev)
		return VDP_STATUS_INVALID_HANDLE;

	*is_supported = VDP_FALSE;

	return VDP_STATUS_OK;
}
