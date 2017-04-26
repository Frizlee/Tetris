#include "Texture.hpp"
#include "Image.hpp"

bool CheckS3TCExt();

Texture::Texture()
{
	gl::GenTextures(1, &mId);
}

Texture::~Texture()
{
	gl::DeleteTextures(1, &mId);
}

void Texture::createFromImage(Image &img)
{
	GLint format;
	GLint internalFormat;
	GLint dataType;
	

	switch (img.getColorFormat())
	{
	case ColorFormat::R8:
		format = gl::RED;
		internalFormat = gl::R8;
		dataType = gl::UNSIGNED_BYTE;
		break;

	case ColorFormat::RG8:
		format = gl::RG;
		internalFormat = gl::RG8;
		dataType = gl::UNSIGNED_BYTE;
		break;

	case ColorFormat::RGB8:
		format = gl::RGB;
		internalFormat = gl::RGB8;
		dataType = gl::UNSIGNED_BYTE;
		break;

	case ColorFormat::RGBA8:
		format = gl::RGBA;
		internalFormat = gl::RGBA8;
		dataType = gl::UNSIGNED_BYTE;
		break;

	case ColorFormat::R3G3B2:
		format = gl::RGB;
		internalFormat = gl::R3_G3_B2;
		dataType = gl::UNSIGNED_BYTE_2_3_3_REV;
		break;

	case ColorFormat::R5G6B5:
		format = gl::RGB;
		internalFormat = gl::RGB8;
		dataType = gl::UNSIGNED_SHORT_5_6_5_REV;
		break;

	case ColorFormat::RGBA4:
		format = gl::RGBA;
		internalFormat = gl::RGBA4;
		dataType = gl::UNSIGNED_SHORT_4_4_4_4_REV;
		break;

	case ColorFormat::RGB5A1:
		format = gl::RGBA;
		internalFormat = gl::RGB5_A1;
		dataType = gl::UNSIGNED_SHORT_1_5_5_5_REV;
		break;

	case ColorFormat::RGB10A2:
		format = gl::RGBA;
		internalFormat = gl::RGB10_A2;
		dataType = gl::UNSIGNED_INT_2_10_10_10_REV;
		break;

	case ColorFormat::SRGB8:
		format = gl::RGB;
		internalFormat = gl::SRGB8;
		dataType = gl::UNSIGNED_BYTE;
		break;

	case ColorFormat::SRGB8A8:
		format = gl::RGBA;
		internalFormat = gl::SRGB8_ALPHA8;
		dataType = gl::UNSIGNED_BYTE;
		break;

	default:
		return createFromCompressedImage(img);
	}
	
	bind(0);
	gl::TexImage2D(gl::TEXTURE_2D, 0, internalFormat, img.getWidth(), img.getHeight(),
		0, format, dataType, img.getBytes().data());

	Unbind(0);
}

void Texture::bind(unsigned int slot)
{
	gl::ActiveTexture(gl::TEXTURE0 + slot);
	gl::BindTexture(gl::TEXTURE_2D, mId);
}

void Texture::Unbind(unsigned int slot)
{
	gl::ActiveTexture(gl::TEXTURE0 + slot);
	gl::BindTexture(gl::TEXTURE_2D, 0);
}

void Texture::createFromCompressedImage(Image &img)
{
	GLenum internalFormat;

	switch (img.getColorFormat())
	{
	case ColorFormat::BC1_RGB:
		if (!CheckS3TCExt())
			return;

		internalFormat = gl::COMPRESSED_RGB_S3TC_DXT1_EXT;
		break;

	case ColorFormat::BC1_RGBA:
		if (!CheckS3TCExt())
			return;

		internalFormat = gl::COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;

	case ColorFormat::BC2_RGBA:
		if (!CheckS3TCExt())
			return;

		internalFormat = gl::COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;

	case ColorFormat::BC3_RGBA:
		if (!CheckS3TCExt())
			return;

		internalFormat = gl::COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;

	case ColorFormat::BC4_R:
		internalFormat = gl::COMPRESSED_RED_RGTC1;
		break;

	case ColorFormat::BC4_SIGNED_R:
		internalFormat = gl::COMPRESSED_SIGNED_RED_RGTC1;
		break;

	case ColorFormat::BC5_RG:
		internalFormat = gl::COMPRESSED_RG_RGTC2;
		break;

	case ColorFormat::BC5_SIGNED_RG:
		internalFormat = gl::COMPRESSED_SIGNED_RG_RGTC2;
		break;

	default:
		// TODO: Error handling.
		return;
	}

	bind(0);
	gl::CompressedTexImage2D(gl::TEXTURE_2D, 0, internalFormat, img.getWidth(), 
		img.getHeight(), 0, img.getBytes().size(), img.getBytes().data());
	Unbind(0);
}

bool CheckS3TCExt()
{
	if (gl::exts::var_EXT_texture_compression_s3tc)
		return true;

	// TODO: Error handling
	return false;
}
