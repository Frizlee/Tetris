#ifndef IMAGE_HPP
#define IMAGE_HPP
#include "Prerequisites.hpp"
#include "ImageCodec.hpp"

enum class ColorFormat
{
	NONE,

	R8,
	RG8,
	RGB8,
	RGBA8,
	R3G3B2,
	R5G6B5,
	RGBA4,
	RGB5A1,
	RGB10A2,
	
	SRGB8,
	SRGB8A8,

	// Compressed formats.
	BC1_RGB,
	BC1_RGBA,
	BC2_RGBA,
	BC3_RGBA,
	BC4_R,
	BC4_SIGNED_R,
	BC5_RG,
	BC5_SIGNED_RG,

	// Aliases.
	R = R8,
	RG = RG8,
	RGB = RGB8,
	RGBA = RGBA8,
};


class Image
{
	friend class Texture;

public:
	Image();
	Image(std::string fileName, ImageCodec *codec);
	Image(unsigned int width, unsigned int height, ColorFormat format,
		std::vector<uint8_t> bytes);
	Image(unsigned int width, unsigned int height, ColorFormat format,
		uint8_t *bytes = nullptr);
	virtual ~Image();
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;

	void loadFromFile(std::string fileName, ImageCodec *codec);
	void loadFromMemory(std::vector<uint8_t> &memory, ImageCodec *codec, uint8_t level = 0);
	uint8_t getMaxMipmapLevel();
	std::shared_ptr<Image> getMipmap(uint8_t level);
	virtual void create(unsigned int width, unsigned int height, ColorFormat format,
		std::vector<uint8_t> bytes);
	virtual void create(unsigned int width, unsigned int height, ColorFormat format,
		uint8_t *bytes = nullptr);
	ColorFormat getColorFormat();
	unsigned int getWidth();
	unsigned int getHeight();
	const std::vector<uint8_t>& getBytes();
	void setPixel(unsigned int x, unsigned int y, uint8_t *bytes);
	void flipVerticaly();

protected:
	std::vector<uint8_t> mBytes;
	std::vector<std::shared_ptr<Image>> mMipmaps;
	uint8_t mLevels;
	unsigned int mWidth;
	unsigned int mHeight;
	ColorFormat mFormat;

	void flipCompressedVerticaly();
};


#endif // IMAGE_HPP

