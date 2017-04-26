#ifndef IMAGE_CODEC_HPP
#define IMAGE_CODEC_HPP
#include "Prerequisites.hpp"

enum class ColorFormat;
class Image;

class ImageCodec
{
public:
	ImageCodec() {};
	virtual ~ImageCodec() {};
	
	virtual uint8_t getMipmapLevels(std::vector<uint8_t> &in) = 0;
	virtual bool shouldBeFlippedVerticaly() = 0;
	//virtual bool shouldBeFlippedHorizontaly() = 0;

	virtual void decode(std::vector<uint8_t> &in, 
		std::vector<uint8_t> *out, unsigned int *width,
		unsigned int *height, ColorFormat *format, uint8_t level) = 0;
	//virtual void encode() = 0;
};

#endif // IMAGE_CODEC_HPP

