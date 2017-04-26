#include "PNGCodec.hpp"
#include "Image.hpp"
#include <png.h>

const unsigned int SIGNATURE_LENGTH = 8;
void PNGReadCallback(png_structp PNG_ptr, png_bytep outBytes,
	png_size_t byteCountToRead);

struct PNGVectorStream
{
	std::vector<uint8_t> *vec;
	unsigned int offset;
};


PNGCodec::PNGCodec()
{
}

PNGCodec::~PNGCodec()
{
}

uint8_t PNGCodec::getMipmapLevels(std::vector<uint8_t> &in)
{
	return 0;
}

bool PNGCodec::shouldBeFlippedVerticaly()
{
	return true;
}

void PNGCodec::decode(std::vector<uint8_t> &in,
	std::vector<uint8_t> *out, unsigned int *width, 
	unsigned int *height, ColorFormat *format, uint8_t level)
{
	if (level != 0)
	{
		// TODO: Error handling.
		return;
	}

	if (png_check_sig(in.data(), SIGNATURE_LENGTH) == false)
	{
		// TODO: Error handling.
		return;
	}

	png_structp pngPtr = png_create_read_struct(
		PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

	if (pngPtr == nullptr)
	{
		// TODO: Error handling.
		return;
	}

	png_infop pngInfoPtr = png_create_info_struct(pngPtr);

	if (pngInfoPtr == nullptr)
	{
		// TODO: Error handling.
		png_destroy_read_struct(&pngPtr, nullptr, nullptr);
		return;
	}

	PNGVectorStream stream;
	stream.vec = &in;
	stream.offset = 0;

	png_set_read_fn(pngPtr, &stream, PNGReadCallback);
	png_read_info(pngPtr, pngInfoPtr);

	int bitDepth = 0;
	int colorType = -1;
	png_uint_32 retval = png_get_IHDR(pngPtr, pngInfoPtr,
		width,
		height,
		&bitDepth,
		&colorType,
		nullptr, nullptr, nullptr);

	if (retval != 1)
	{
		// TODO: Error handling.
		png_destroy_read_struct(&pngPtr, &pngInfoPtr, nullptr);
		return;
	}

	switch (colorType)
	{
	case PNG_COLOR_TYPE_RGB:
		*format = ColorFormat::RGB;
		break;

	case PNG_COLOR_TYPE_RGBA:
		*format = ColorFormat::RGBA;
		break;

	default:
		// TODO: Error handling.
		png_destroy_read_struct(&pngPtr, &pngInfoPtr, nullptr);
		return;
	}

	out->resize((*width) * (*height) * static_cast<unsigned int>(*format));

	for (unsigned int i = 0; i < *height; ++i)
		png_read_row(pngPtr, 
			&(*out)[i * (*width) * static_cast<unsigned int>(*format)], nullptr);

	png_destroy_read_struct(&pngPtr, &pngInfoPtr, nullptr);
}


void PNGReadCallback(png_structp PNGPtr, png_bytep outBytes,
	png_size_t byteCountToRead)
{
	PNGVectorStream *stream = reinterpret_cast<PNGVectorStream*>(png_get_io_ptr(PNGPtr));

	if (stream == nullptr)
	{
		// TODO: Error handling.
		outBytes = nullptr;
		return;
	}

	if (stream->offset + byteCountToRead > stream->vec->size())
	{
		// TODO: Error handling.
		stream->offset = stream->vec->size();
		outBytes = nullptr;
		return;
	}

	memcpy(outBytes, &(*stream->vec)[stream->offset], byteCountToRead);
	// outBytes = &(*stream->vec)[stream->offset];
	stream->offset += byteCountToRead;
}

