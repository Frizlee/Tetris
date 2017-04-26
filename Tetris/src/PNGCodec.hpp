#ifndef PNG_CODEC_HPP
#define PNG_CODEC_HPP
#include "Prerequisites.hpp"
#include "ImageCodec.hpp"

class PNGCodec : public ImageCodec
{
public:
	PNGCodec();
	~PNGCodec();
	
	uint8_t getMipmapLevels(std::vector<uint8_t> &in);
	bool shouldBeFlippedVerticaly();

	void decode(std::vector<uint8_t> &in,
		std::vector<uint8_t> *out, unsigned int *width,
		unsigned int *height, ColorFormat *format, uint8_t level);
	// void encode();
};

#endif // PNG_CODEC_HPP

