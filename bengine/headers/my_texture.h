#pragma once

#include <glad/glad.h>
#include "stb_image.h"
#include <iostream>

//a bad but proud texture class made by battak for loading textures easily

class Texture {
public:
	//texture id?
	unsigned int ID;
	GLenum globalTexType;

	Texture(GLenum texType, const char* imagePath, bool verticallyFlip, GLenum texUnit) {
		
		globalTexType = texType;


		//create the texture
		glGenTextures(1, &ID);
		glActiveTexture(GL_TEXTURE0 + texUnit);
		glBindTexture(texType, ID);

		//initializing
		glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(texType, GL_TEXTURE_WRAP_R, 0);
		glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_set_flip_vertically_on_load(verticallyFlip);

		int width, height, nrChannels;
		unsigned char* data = stbi_load(imagePath, &width, &height, &nrChannels, 0);

		//dynamically setting the image parameters so texture is loaded ideally
		if (data) {
			GLenum format = GL_RGB;

			if (nrChannels == 1) { format = GL_RED; glPixelStorei(GL_UNPACK_ALIGNMENT, 1); }
			else if (nrChannels == 3) { format = GL_RGB; glPixelStorei(GL_UNPACK_ALIGNMENT, 1); }
			else if (nrChannels == 4) { format = GL_RGBA; glPixelStorei(GL_UNPACK_ALIGNMENT, 4); }

			if (texType == GL_TEXTURE_2D) {
				glTexImage2D(texType, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(texType);
			}
			else {
				std::cout << "Texture Type(" << texType << ") currently unspported in my_texture.h" << std::endl;
			}


		}
		else {
			std::cout << "failed to load the texture image!" << std::endl;
		}
	}


	void wrapMode(GLenum texWrapModeS, GLenum texWrapModeT, GLenum texWrapModeR) {
		glTexParameteri(globalTexType, GL_TEXTURE_WRAP_S, texWrapModeS);
		glTexParameteri(globalTexType, GL_TEXTURE_WRAP_T, texWrapModeT);
		glTexParameteri(globalTexType, GL_TEXTURE_WRAP_R, texWrapModeR);
	}
	void filteringMode(GLenum texMinFilterMode, GLenum texMagFilterMode) {
		glTexParameteri(globalTexType, GL_TEXTURE_MIN_FILTER, texMinFilterMode);
		glTexParameteri(globalTexType, GL_TEXTURE_MAG_FILTER, texMagFilterMode);
	}


};