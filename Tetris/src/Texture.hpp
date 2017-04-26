#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include "Prerequisites.hpp"

class Image;

class Texture
{
public:
	Texture();
	~Texture();
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	void createFromImage(Image &img);
	void bind(unsigned int slot);
	static void Unbind(unsigned int slot);

private:
	GLuint mId;

	void createFromCompressedImage(Image &img);
};

#endif // TEXTURE_HPP

