#include "Image.hpp"

static const uint8_t BASE_LEVEL = 0;

Image::Image() : mBytes(), mLevels(0), mMipmaps()
{
}

Image::Image(std::string fileName, ImageCodec *codec) : mBytes(), mLevels(0), mMipmaps()
{
	loadFromFile(fileName, codec);
}

Image::Image(unsigned int width, unsigned int height, ColorFormat format, std::vector<uint8_t> bytes)
	: mBytes(), mLevels(0), mMipmaps()
{
	create(width, height, format, bytes);
}

Image::Image(unsigned int width, unsigned int height, ColorFormat format, uint8_t *bytes)
	: mBytes(), mLevels(0), mMipmaps()
{
	create(width, height, format, bytes);
}

Image::~Image()
{
}

void Image::loadFromFile(std::string fileName, ImageCodec *codec)
{
	std::ifstream in(fileName, std::ifstream::binary);

	if (in.is_open() == false)
	{
		// TODO: Error handling.
		return;
	}

	std::vector<uint8_t> data;

	in.seekg(std::ios_base::end);
	data.reserve((unsigned int)in.tellg());
	in.seekg(std::ios_base::beg);

	data.assign(std::istreambuf_iterator<char>(in),
		std::istreambuf_iterator<char>());

	in.close();

	return loadFromMemory(data, codec);
}

void Image::loadFromMemory(std::vector<uint8_t> &memory, ImageCodec *codec, uint8_t level)
{
	bool shouldFlip = codec->shouldBeFlippedVerticaly();

	if (level == BASE_LEVEL)
	{
		mLevels = codec->getMipmapLevels(memory);

		if (mLevels > BASE_LEVEL)
		{
			mMipmaps.resize(mLevels);

			for (uint8_t i = 0; i < mLevels; ++i)
			{
				std::shared_ptr<Image> mipmap = std::make_shared<Image>();
				mipmap->loadFromMemory(memory, codec, i + 1);

				if (shouldFlip)
					mipmap->flipVerticaly();

				mMipmaps.at(i) = mipmap;
			}
		}
	}

	codec->decode(memory, &mBytes, &mWidth, &mHeight, &mFormat, level);

	if (shouldFlip)
		flipVerticaly();
}

uint8_t Image::getMaxMipmapLevel()
{
	return mLevels;
}

std::shared_ptr<Image> Image::getMipmap(uint8_t level)
{
	//if (level == 0)
		//return std::make_shared<Image>(this);

	return mMipmaps.at(level - 1);
}

void Image::create(unsigned int width, unsigned int height, ColorFormat format, std::vector<uint8_t> bytes)
{
	mWidth = width;
	mHeight = height;
	mFormat = format;
	mBytes = bytes;
}

void Image::create(unsigned int width, unsigned int height, ColorFormat format, uint8_t *bytes)
{
	mWidth = width;
	mHeight = height;
	mFormat = format;
	mBytes.resize(mWidth * mHeight * static_cast<unsigned int>(mFormat));

	if (bytes != nullptr)
		memcpy(mBytes.data(), bytes, mBytes.size());
}

ColorFormat Image::getColorFormat()
{
	return mFormat;
}

unsigned int Image::getWidth()
{
	return mWidth;
}

unsigned int Image::getHeight()
{
	return mHeight;
}

const std::vector<uint8_t>& Image::getBytes()
{
	return mBytes;
}

void Image::setPixel(unsigned int x, unsigned int y, uint8_t *bytes)
{
	unsigned int row = y * mWidth * static_cast<unsigned int>(mFormat);
	unsigned int col = x * static_cast<unsigned int>(mFormat);

	for (char i = 0; i < static_cast<char>(mFormat); ++i)
		mBytes.at(row + col + i) = bytes[i];
}

void Image::flipVerticaly()
{
	uint8_t bytesPerPixel;
	uint8_t flipByte;

	switch (mFormat)
	{
	case ColorFormat::R8:
	case ColorFormat::R3G3B2:
		bytesPerPixel = 1;
		break;

	case ColorFormat::RG8:
	case ColorFormat::R5G6B5:
	case ColorFormat::RGB5A1:
	case ColorFormat::RGBA4:
		bytesPerPixel = 2;
		break;

	case ColorFormat::RGB8:
	case ColorFormat::SRGB8:
		bytesPerPixel = 3;
		break;

	case ColorFormat::RGBA8:
	case ColorFormat::RGB10A2:
	case ColorFormat::SRGB8A8:
		bytesPerPixel = 4;
		break;

	case ColorFormat::NONE:
		return;

	default:
		return flipCompressedVerticaly();
	}

	for (unsigned int v = 0; v < mHeight / 2; ++v)
	{
		unsigned int row1 = v * mWidth * bytesPerPixel;
		unsigned int row2 = (mHeight - v - 1) * mWidth * bytesPerPixel;

		for (unsigned int u = 0; u < mWidth; ++u)
		{
			unsigned int pixel1 = row1 + u * bytesPerPixel;
			unsigned int pixel2 = row2 + u * bytesPerPixel;

			for (uint8_t i = 0; i < bytesPerPixel; ++i)
			{
				flipByte = mBytes.at(pixel1 + i);
				mBytes.at(pixel1 + i) = mBytes.at(pixel2 + i);
				mBytes.at(pixel2 + i) = flipByte;
			}
		}
	}
}

void Image::flipCompressedVerticaly()
{
	// TODO: void Image::flipCompressedVerticaly()
	return;
}

