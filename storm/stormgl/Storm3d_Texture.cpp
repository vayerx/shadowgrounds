// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include <stdio.h>
#include <map>
#include <string>
#include <iostream>
#include <boost/scoped_array.hpp>
#include <boost/lexical_cast.hpp>
#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>

#include "storm3d_terrain_utils.h"
#include "storm3d.h"
#include "storm3d_adapter.h"
#include "storm3d_texture.h"
#include <IStorm3D_Logger.h>

#include "../../filesystem/ifile_list.h"
#include "../../filesystem/file_package_manager.h"
#include "../../filesystem/input_stream_wrapper.h"
#include "../../util/Debug_MemoryManager.h"

using namespace frozenbyte;

namespace {

	bool s3dFileExists(const char *name)
	{
		if(!name)
			return false;

		filesystem::FB_FILE *fp = filesystem::fb_fopen(name, "rb");
		if(fp == 0)
			return false;

		filesystem::fb_fclose(fp);
		return true;
	}

	struct FileSeeker
	{
		// filename -> fullpath
		std::map<std::string, std::string> fileNames;
		IStorm3D_Logger *logger;

		std::string removePath(const char *file_) const
		{
			assert(file_);
			std::string file = file_;

			size_t index = file.find_last_of("/\\");
			if(index == file.npos)
				index = 0;
			else
				index += 1;

			size_t size = file.size() - index;
			return file.substr(index, size);
		}

		FileSeeker(IStorm3D_Logger *logger_)
		:	logger(logger_)
		{
		}

		void addFiles(const char *fileType)
		{
			filesystem::FilePackageManager &manager = filesystem::FilePackageManager::getInstance();
#ifdef LEGACY_FILES
			boost::shared_ptr<filesystem::IFileList> fileList = manager.findFiles("Data/Textures", fileType);
#else
			boost::shared_ptr<filesystem::IFileList> fileList = manager.findFiles("data/texture", fileType);
#endif
			//boost::shared_ptr<filesystem::IFileList> fileList = manager.findFiles("Data/Models", fileType);

			std::vector<std::string> files;
#ifdef LEGACY_FILES
			filesystem::getAllFiles(*fileList, "Data/Textures", files, true);
#else
			filesystem::getAllFiles(*fileList, "data/texture", files, true);
#endif
			//fileList.getAllFiles(files);

			for(unsigned int i = 0; i < files.size(); ++i)
			{
				std::string fileName = files[i];
				std::string baseName = removePath(fileName.c_str());

				for(unsigned int i = 0; i < baseName.size(); ++i)
				{
					if(baseName[i] == '\\')
						baseName[i] = '/';

					baseName[i] = tolower(baseName[i]);
				}

				for(unsigned int j = 0; j < fileName.size(); ++j)
				{
					if(fileName[j] == '\\')
						fileName[j] = '/';

					fileName[j] = tolower(fileName[j]);
				}

				if(!fileNames[baseName].empty() && logger)
				{
					std::string error("Ambiguous filename: ");
					error += baseName;

					logger->error(error.c_str());
				}

				fileNames[baseName] = fileName;
			}
		}

		std::string findPath(const char *file_) const
		{
			assert(file_);
			std::string file = removePath(file_);

			for(unsigned int i = 0; i < file.size(); ++i)
			{
				if(file[i] == '\\')
					file[i] = '/';

				file[i] = tolower(file[i]);
			}

			std::map<std::string, std::string>::const_iterator it = fileNames.find(file);
			if(it == fileNames.end())
				return file_;

			return it->second;
		}
	};

	FileSeeker *fileSeeker = 0;
}

int storm3d_texture_allocs = 0;

//! Initialize texture bank
/*!
	\param logger logger
*/
void initTextureBank(IStorm3D_Logger *logger)
{
	fileSeeker = new FileSeeker(logger);

	fileSeeker->addFiles("*.dds");
	fileSeeker->addFiles("*.tga");
	fileSeeker->addFiles("*.jpg");
	fileSeeker->addFiles("*.avi");
#ifndef LEGACY_FILES
	fileSeeker->addFiles("*.png");
#endif
}

//! Free texture bank
void freeTextureBank()
{
	delete fileSeeker;
	fileSeeker = 0;
}

//! Find texture
/*!
	\param name name of texture
	\return path of texture
*/
std::string findTexture(const char *name)
{
	if(s3dFileExists(name))
		return name;

	assert(fileSeeker);
	return fileSeeker->findPath(name);
}

//! Is number power of 2?
/*! http://graphics.stanford.edu/~seander/bithacks.html
  \param n number to test
 */
static inline bool isPowerOf2(int n) {
	return !(n & (n - 1)) && n;
}

string frameBufferStatusString()
{
  switch(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)) {
  case GL_FRAMEBUFFER_COMPLETE_EXT:
    return "complete";
  case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
    return "unsupported";
  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
    return "incomplete attachment";
  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
    return "missing attachment";
  case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
    return "incomplete dimensions";
  case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
    return "incomplete formats";
  case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
    return "incomplete draw buffer";
  case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
    return "incomplete read buffer";
  default:
    return "incomplete";
  }
}

//! Constructor
Storm3D_Texture::Storm3D_Texture(Storm3D *s2,const char *_filename,DWORD _texloadflags,
								 DWORD _tex_identity, const void *data, size_t data_size) :
	decentAlpha(false),
	Storm3D2(s2),
	glTextype(GL_TEXTURE_2D),
	texfmt(0),
	textype(TEXTYPE_BASIC),
	filename(NULL),
	tex_identity(_tex_identity),
	texloadflags(_texloadflags),
	refcount(1),
	width(0),
	height(0),
	prioritycount(0),
	auto_release(true)
{
	std::string resolvedName;

	if(data == NULL && !s3dFileExists(_filename))
	{
		assert(fileSeeker);
		resolvedName = fileSeeker->findPath(_filename);
	}
	else
	{
		// Create filename
		resolvedName = _filename;
	}

	if(!s2->hasHighQualityTextures() && resolvedName.size() > 4)
	{
		std::string newName = resolvedName.substr(0, resolvedName.size() - 4);
		newName += "_lowdetail";
		newName += resolvedName.substr(resolvedName.size() - 4, 4);

		if(s3dFileExists(newName.c_str()))
			resolvedName = newName;
	}

	filename = new char[resolvedName.size() + 1];
	strcpy(filename, resolvedName.c_str());

	/*
	OutputDebugString("Loading texture: ");
	OutputDebugString(filename);ogui.LoadOguiImage
	OutputDebugString("\r\n");
	*/
	boost::scoped_array<char> fileData(NULL);
	if (data == NULL)
	{
		filesystem::FB_FILE *file = filesystem::fb_fopen(filename, "rb");
		if(file)
		{
			data_size = filesystem::fb_fsize(file);
			fileData.reset(new char[data_size]);
			filesystem::fb_fread(fileData.get(), 1, data_size, file);

			filesystem::fb_fclose(file);
		} else {
			igiosErrorMessage("Failed to load texture %s", filename);
			return ;
		}
	} else {
		fileData.reset(new char[data_size]);
		memcpy(fileData.get(), data, data_size);
	}

	// Pack non-DDS textures with DXT1
	// DDS textures are left unpacked (or prepacked with DDS-file's format)
	int l = strlen(filename);
	if (l > 4) {
		if ((filename[l - 1] == 's' || filename[l - 1] == 'S') &&
			(filename[l - 2] == 'd' || filename[l - 2] == 'D') &&
			(filename[l - 3] == 'd' || filename[l - 3] == 'D')) {
				// some joker named tga as dds...
				// so fall through on error
				// this is completely FUCKED UP!
				if (loadDDS(filename, fileData.get()))
					return;
		}
	}

	class surfaceContainer {
		SDL_Surface *s;
	public:
		surfaceContainer(SDL_Surface *_s) :
			s(_s)
		{
		};

		~surfaceContainer() {
			SDL_FreeSurface(s);
		};

		SDL_Surface *operator->() { return s; };

	};


	SDL_RWops *rw = SDL_RWFromMem(fileData.get(), data_size);
	SDL_Surface *tmp;
	if (filename != NULL
		&& tolower(filename[l - 3]) == 't'
		&& tolower(filename[l - 2]) == 'g'
		&& tolower(filename[l - 1]) == 'a')
		tmp = IMG_LoadTGA_RW(rw);
	else {
		tmp = IMG_Load_RW(rw, false);
		if (tmp == NULL) {
			tmp = IMG_LoadTGA_RW(rw);
		}
	}

	SDL_FreeRW(rw); rw = NULL;

	if (tmp == NULL) {
		igiosErrorMessage("Failed to load texture %s: %s", filename, IMG_GetError());
		return ;
	}
	surfaceContainer img(tmp);

	glGenTextures(1, &texhandle);
	glBindTexture(GL_TEXTURE_2D, texhandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// const int fmt = (img->format->Bshift < img->format->Rshift) ? (img->format->Amask == 0 ? GL_RGB : GL_RGBA) : (img->format->Amask == 0 ? GL_BGR : GL_BGRA);
	GLenum fmt = GL_RGBA;
	GLenum internalfmt = GL_RGBA;
	boost::scoped_array<unsigned char> imageData;
	char *src = reinterpret_cast<char *>(img->pixels);

	int dw = img->w, dh = img->h;

	if (img->format->BytesPerPixel == 1) {
		internalfmt = fmt = GL_LUMINANCE;
	} else {
		// do nearest scaling before mipmapping for very small textures to prevent bleeding
		if (dw < 16) toNearestPow(dw);
		if (dh < 16) toNearestPow(dh);
		imageData.reset(new unsigned char[dw * dh * 4]);
		unsigned char *target = imageData.get();
		int pitch = dw * 4, aorr = img->format->Ashift ? 0 : 0xff;
    bool alpha = false, map = false;
		// UGLY HACK: if we're loading a map texture force compression off and set alpha channel to 0
		const char *mapname = "map_default.png";
		if (strcmp(filename + strlen(filename) - strlen(mapname), mapname) == 0) {
			alpha = true;
			map = true;
		}
		float sxd = (float)img->w / dw, syd = (float)img->h / dh;
		for (int y = 0; y < dh/*img->h*/; y++) {
			for (int x = 0; x < dw/*img->w*/; x++) {
				/*
				Uint8 *target = (Uint8 *) (imageData.get() + (y * img->w + x) * 4);
				SDL_GetRGBA(*((DWORD *) (src + (y * img->pitch + x * img->format->BytesPerPixel))), img->format, target, target + 1, target + 2, target + 3);
				*/
				int sx = int(sxd * x), sy = int(syd * y);
				unsigned char A = *(src + (sy * img->pitch + sx * img->format->BytesPerPixel + img->format->Ashift / 8)) | aorr;
				if (map) A = 0x00;
        if (A > 0 && A < 0xff) alpha = true;
				unsigned char R = *(src + (sy * img->pitch + sx * img->format->BytesPerPixel + img->format->Rshift / 8));
				unsigned char G = *(src + (sy * img->pitch + sx * img->format->BytesPerPixel + img->format->Gshift / 8));
				unsigned char B = *(src + (sy * img->pitch + sx * img->format->BytesPerPixel + img->format->Bshift / 8));
				*(target + (y * pitch + x * 4 + 0)) = R;
				*(target + (y * pitch + x * 4 + 1)) = G;
				*(target + (y * pitch + x * 4 + 2)) = B;
				*(target + (y * pitch + x * 4 + 3)) = A;
			}
		}

		if (!(texloadflags&TEXLOADFLAGS_NOCOMPRESS) & !alpha)
			internalfmt = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	}

	gluBuild2DMipmaps(GL_TEXTURE_2D, internalfmt, dw, dh, fmt, GL_UNSIGNED_BYTE, imageData.get() ? imageData.get() : img->pixels);
	texfmt = internalfmt;

  bool scaleToPowerOfTwo = false;
  // UGLY HACK: Since font rendering doesnt work with scaled textures, disable scaling to
  // nearest power of two when the texture path contains "fonts"
#ifdef _WIN32
  string flow(filename);
  for (unsigned int i=0; i < flow.length(); i++)
  {
	flow[i] = tolower(flow[i]);
  }
  if (flow.find("fonts") != flow.npos) scaleToPowerOfTwo = false;
#else
  const char *fonts = "fonts";
  if (strcasestr(filename,fonts) != 0) scaleToPowerOfTwo = false;
#endif
  if (scaleToPowerOfTwo) {
    int dw2 = dw,dh2 = dh;
		toNearestPow(dw2);
		toNearestPow(dh2);
    if (dw2 != dw || dh2 != dh) {
      GLuint fboHandle,rboHandle[2],fboTexture;
      // Setup fbo texture
      glGenTextures(1,&fboTexture);
      glBindTexture(GL_TEXTURE_2D,fboTexture);
      glTexImage2D(GL_TEXTURE_2D,0,fmt,dw2,dh2,0,fmt,GL_UNSIGNED_BYTE,0);
      glGenerateMipmapEXT(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D,0);
      // Set up render buffers
      glGenRenderbuffersEXT(2,rboHandle);
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,rboHandle[0]);
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,fmt,dw2,dh2);
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,rboHandle[1]);
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_DEPTH24_STENCIL8_EXT,dw2,dh2);
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,0);
      // Setup fbo
      glGenFramebuffersEXT(1,&fboHandle);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboHandle);
      glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_RENDERBUFFER_EXT,rboHandle[0]);
      glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT,rboHandle[1]);
      if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) {
        // cout << "Rendering texture " << filename << "(" << dw << "x" << dh << ") at (" << dw2 << "x" << dh2 << ")" << endl;
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,fboTexture,0);
        if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) {
          // Set up matrices
          glMatrixMode(GL_PROJECTION);
          glPushMatrix();
          glLoadIdentity();
          gluOrtho2D(0,1,0,1);
          glMatrixMode(GL_MODELVIEW);
          glPushMatrix();
          glLoadIdentity();
          // Render the current texture
          glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_POLYGON_BIT|GL_VIEWPORT_BIT);
          glDisable(GL_CULL_FACE);
          glDisable(GL_DEPTH_TEST);
          glDepthMask(GL_FALSE);
          glViewport(0,0,dw2,dh2);
          glClearColor(0,0,0,0);
          glClear(GL_COLOR_BUFFER_BIT);
          glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
          glEnable(GL_BLEND);
          glBindTexture(GL_TEXTURE_2D,texhandle);
          glEnable(GL_TEXTURE_2D);
          glBegin(GL_QUADS);
          glColor4f(1,1,1,1); glTexCoord2f(0,0); glVertex2f(0,0);
          glColor4f(1,1,1,1); glTexCoord2f(1,0); glVertex2f(1,0);
          glColor4f(1,1,1,1); glTexCoord2f(1,1); glVertex2f(1,1);
          glColor4f(1,1,1,1); glTexCoord2f(0,1); glVertex2f(0,1);
          glEnd();
          glDisable(GL_TEXTURE_2D);
          glBindTexture(GL_TEXTURE_2D,0);
          glPopAttrib();
          // Restore matrices
          glMatrixMode(GL_MODELVIEW);
          glPopMatrix();
          glMatrixMode(GL_PROJECTION);
          glPopMatrix();
          glMatrixMode(GL_MODELVIEW);
          // Update texture mip map 
          glBindTexture(GL_TEXTURE_2D,fboTexture);
          glGenerateMipmapEXT(GL_TEXTURE_2D);
          glBindTexture(GL_TEXTURE_2D,0);
        } else {
          // cout << "WARNING: Incomplete frame buffer after binding texture: " << frameBufferStatusString() << endl;
          // The texhandle texture is deleted at end of this method and replaced with the fbotexture.
          // Since we failed to produce a valid scaled image, swap the 2 to retain original texture image.
          swap(texhandle,fboTexture);
        }
      } else {
        // The texhandle texture is deleted at end of this method and replaced with the fbotexture.
        // Since we failed to produce a valid scaled image, swap the 2 to retain original texture image.
        swap(texhandle,fboTexture);
      }
      // Restore fbo
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
      // Delete render buffers and fbo
      glDeleteRenderbuffersEXT(2,rboHandle);
      glDeleteFramebuffersEXT(1,&fboHandle);
      // Release unused texture handles
      glDeleteTextures(1,&texhandle);
      texhandle = fboTexture;
    }
  }

	bool show = false;
	if (show) {
		glDisable(GL_FOG);
		//glDisable(GL_BLEND);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_ALPHA_TEST);
		//glEnable(GL_ALPHA_TEST);
		//glAlphaFunc(GL_ALWAYS, 0);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();				// Reset The Projection Matrix

		// Calculate The Aspect Ratio Of The Window
		gluPerspective(45.0f,(GLfloat)640.0/(GLfloat)480.0,0.1f,100.0f);

		glClearColor(1.0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(-1.5f,0.0f,-6.0f);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texhandle);

		float umax = 1.0, vmax = 1.0;
		glBegin(GL_QUADS);						// Draw A Quad
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 0.0f);				// Top Left
		glTexCoord2f(umax, 0.0f); glVertex3f( 3.0f, 1.0f, 0.0f);				// Top Right
		glTexCoord2f(umax, vmax); glVertex3f( 3.0f,-3.0f, 0.0f);				// Bottom Right
		glTexCoord2f(0.0f, vmax); glVertex3f(-1.0f,-3.0f, 0.0f);				// Bottom Left
		glEnd();							// Done Drawing The Quad
		glDisable(GL_TEXTURE_2D);

		glFlush();

		SDL_GL_SwapBuffers();

#ifdef WIN32
		Sleep(100);
#else
		sleep(10);
#endif
	}

	this->height = img->h;
	this->width = img->w;
	textype = TEXTYPE_BASIC;

	storm3d_texture_allocs++;
}

//! Load DDS texture
static const char *loadDDS_(bool compressed, int mipmaps, int w, int h, GLenum &fmt, const char *src, GLenum glTextype, int mult) {
	int offset = 0;
	if (compressed) {
		for (int i = 0; i < mipmaps; i++) {
			int size = max(1, w / 4) * max(1, h / 4) * mult;
			if (GLEW_EXT_texture_compression_s3tc) {
				glCompressedTexImage2D(glTextype, i, fmt, w, h, 0, size, src+offset);
			} else {
				igios_unimplemented(); // FIXME: uncompress manually
			}
			offset += size;
			w = max(1, w / 2); h = max(1, h / 2);
		}
	} else {
		boost::scoped_array<unsigned char> data(new unsigned char[w*h*4]);
		for (int i = 0; i < mipmaps; i++) {
			unsigned int destincr = 2;

			switch(mult) {
			case 1:
				fmt = GL_LUMINANCE;
				break;
			case 2:
				if (fmt == GL_DU8DV8_ATI) {
					if (!GLEW_ATI_envmap_bumpmap) {
						destincr = 4;
						fmt = GL_RGBA;
					}
				} else {
					igiosWarning("IGIOS_WARNING: Strange fmt-mult combination: %d %x\n", fmt, mult);
				}

				for(unsigned int s = 0, d = 0; s < (unsigned int)(w * h * 2); s += 2, d += destincr) {
					data[d+0] = src[offset+s+1] + 0x80;
					data[d+1] = src[offset+s+0] + 0x80;
					data[d+2] = 0xFF;
					data[d+3] = 0xFF;
				}

				break;
			case 3:
				for(int i=0;i<w*h*3;i+=3) {
					data[i+0] = src[offset+i+2];
					data[i+1] = src[offset+i+1];
					data[i+2] = src[offset+i+0];
				}
				break;
			case 4:
				for(int i=0;i<w*h*4;i+=4) {
					data[i+0] = src[offset+i+2];
					data[i+1] = src[offset+i+1];
					data[i+2] = src[offset+i+0];
					data[i+3] = src[offset+i+3];
				}
				break;
			default:
				igiosWarning("IGIOS_WARNING: Bad DDS texture bpp\n");
				break;
			};
			int size = w * h * mult;
			glTexImage2D(glTextype, i, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data.get());
			offset += size;
			w = max(1, w / 2); h = max(1, h / 2);
		}
	}
	return src+offset;
}


#include "../keyb3/keyb3.h"
//! Load DDS texture
/*!
	\param filename file
	\param fileData load destination
*/
bool Storm3D_Texture::loadDDS(const char *filename, const char *fileData) {
	if (memcmp(fileData, "DDS ", 4) != 0) {
		// igiosErrorMessage("invalid DDS file %s", filename);
		return false;
	}

	Uint32 surfaceDestLen = SDL_SwapLE32(*(Uint32 *) (fileData + 0x04));
	if (surfaceDestLen != 124) {
		// igiosErrorMessage("invalid DDS file %s: size mismatch (%d)", filename, surfaceDestLen);
		return false;
	}

	Uint32 caps  = SDL_SwapLE32(*(Uint32 *) (fileData + 0x08));
	// must have DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
	if ((caps & 0x1007) == 0) {
		igiosWarning("%s: strange caps: %08x\n", filename, caps);
		return false;
	}
	caps &= (0x1007 ^ -1); // mask them out

	this->height = SDL_SwapLE32(*(Uint32 *) (fileData + 0x0c));
	this->width  = SDL_SwapLE32(*(Uint32 *) (fileData + 0x10));
	Uint32 pixelFormat = SDL_SwapLE32(*(Uint32 *) (fileData + 0x50));
	Uint32 dwcaps2 = SDL_SwapLE32(*(Uint32 *) (fileData + 0x70));

	int mipmaps;
	if (caps & 0x20000) {
		mipmaps = SDL_SwapLE32(*(Uint32 *) (fileData + 0x1c));
		caps &= (0x20000 ^ -1);
	} else {
		mipmaps = 1;
	}

	const char *data = fileData + 0x80;

	if (pixelFormat & 1) { // alpha
		decentAlpha = true; pixelFormat &= (0x1 ^ -1);
	} else
		decentAlpha = false;

	glGenTextures(1, &texhandle);

	GLenum fmt;
	Uint32 fourcc;
	bool compressed = false;
	int mult = 0;
	switch (pixelFormat) {
	case 0x4: // DDPF_FOURCC
		fourcc = SDL_SwapLE32(*(Uint32 *) (fileData + 0x54));
		compressed = true;

		mult = 16;
		switch (fourcc) {
		case 0x31545844: // DXT1
			// alpha bit in pixelformat means nothing...
			fmt = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			mult = 8;
			break;
		case 0x33545844: // DXT3
			fmt = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			break;
		case 0x35545844: // DXT5
			fmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		default:
			igiosErrorMessage("unknown fourcc: %08x\n", fourcc);
			return false;
		}

		break;
	case 0x40: // DDPF_RGB
		if (decentAlpha) {
			fmt = GL_RGBA;
			mult = 4;
		} else {
			fmt = GL_RGB;
			mult = 3;
		}
		break;
	case 0x20000: // DDPF_LUMINANCE
		fmt = GL_LUMINANCE;
		mult = 1;
		break;
	case 0x80000:
		fmt = GL_DU8DV8_ATI;
		mult = 2;
		break;
	default:
		igiosErrorMessage("%s\nunknown pixelformat: %08x\n", filename, pixelFormat);
		return false;
	}

	int w = this->width, h = this->height;
	int pitch;
	int linearsize;
	if (caps & 0x80000) { // DDSD_LINEARSIZE
		caps &= (0x80000 ^ -1);
		linearsize = SDL_SwapLE32(*(Uint32 *) (fileData + 0x14));
		pitch = w * mult;
	} else {
		pitch = SDL_SwapLE32(*(Uint32 *) (fileData + 0x14));
		linearsize = w * h * mult;
	}

	bool show = false;
	if (dwcaps2 & 0xfe00) {
		textype = TEXTYPE_CUBE;
		glTextype = GL_TEXTURE_CUBE_MAP;
	} else {
		textype = TEXTYPE_BASIC;
		this->glTextype = GL_TEXTURE_2D;
	}

	glBindTexture(glTextype, texhandle);
	glTexParameteri(glTextype, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (mipmaps > 1)
		glTexParameteri(glTextype, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		glTexParameteri(glTextype, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (textype == TEXTYPE_CUBE) {  // cubemap
		for (int i = 0; i < 6; i++) {
			data = loadDDS_(compressed, mipmaps, w, h, fmt, data, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mult);
			texfmt = fmt;
		}
	} else {
		loadDDS_(compressed, mipmaps, w, h, fmt, data, glTextype, mult);
		texfmt = fmt;
	}
	
	storm3d_texture_allocs++;
	//dx_handle->SetLOD(1);

	show = false;
	// bool wait = false;
	if (show) {
		glDisable(GL_FOG);
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_DEPTH_TEST);
		frozenbyte::storm::VertexShader::disable();
		frozenbyte::storm::PixelShader::disable();
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glEnable(glTextype);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();							// Reset The Projection Matrix

		// Calculate The Aspect Ratio Of The Window
		gluPerspective(45.0f,(GLfloat)640.0/(GLfloat)480.0,0.1f,100.0f);

		glClearColor(1.0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(-1.5f,0.0f,-6.0f);
		float umax, vmax;
		umax = 1.0f; vmax = 1.0f;

		glEnable(glTextype);
		glBindTexture(glTextype, texhandle);
		glBegin(GL_QUADS);						// Draw A Quad
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 0.0f);				// Top Left
		glTexCoord2f(umax, 0.0f); glVertex3f( 3.0f, 1.0f, 0.0f);				// Top Right
		glTexCoord2f(umax, vmax); glVertex3f( 3.0f,-3.0f, 0.0f);				// Bottom Right
		glTexCoord2f(0.0f, vmax); glVertex3f(-1.0f,-3.0f, 0.0f);				// Bottom Left
		glEnd();							// Done Drawing The Quad

		glDisable(glTextype);

		glFlush();

		SDL_GL_SwapBuffers();
/*		if (wait) {
			while (true) {
				if (Keyb3_WaitKeypress() == KEYCODE_SPACE) break;
			}
		} */
	}

	return true;
}

//! Constructor
Storm3D_Texture::Storm3D_Texture(Storm3D *s2,int _width,int _height,TEXTYPE ttype) :
	decentAlpha(true),
	Storm3D2(s2),
	texfmt(0),
	textype(ttype),
	filename(NULL),
	tex_identity(0),
	texloadflags(0),
	refcount(1),
	width(_width),
	height(_height),
	prioritycount(0),
	auto_release(true)
{
	// Limit dimensions (v2.3 bugfix)
	int mwidth = Storm3D2->maxtexsize, mheight = Storm3D2->maxtexsize;
	Storm3D_SurfaceInfo ss=Storm3D2->GetScreenSize();
	if (width<1) width=1;
	if (height<1) height=1;
	if (width>mwidth) 
	{
		igiosErrorMessage("GFX card doesn't support textures this big.");
		width=mwidth;
	}
	if (height>mheight) 
	{
		igiosErrorMessage("GFX card doesn't support textures this big.");
		height=mheight;
	}
//	if (width>ss.width) width=ss.width;
//	if (height>ss.height) height=ss.height;

	if(textype != TEXTYPE_RAM && textype != TEXTYPE_DYNAMIC && textype != TEXTYPE_DYNAMIC_LOCKABLE && textype != TEXTYPE_DYNAMIC_LOCKABLE_32)
	{
		// Make 2 pot
		width *= 2;
		height *= 2;
		if(width>4096) width=4096;
		else if (width>2048) width=2048;
		else if (width>1024) width=1024;
		else if (width>512) width=512;
		else if (width>256) width=256;
		else if (width>128) width=128;
		else if (width>64) width=64;
		else if (width>32) width=32;
		else if (width>16) width=16;
		else if (width>8) width=8;
		else if (width>4) width=4;
		else if (width>2) width=2;
		else if (width>1) width=1;
		
		if (height>4096) height=4096;
		else if (height>2048) height=2048;
		else if (height>1024) height=1024;
		else if (height>512) height=512;
		else if (height>256) height=256;
		else if (height>128) height=128;
		else if (height>64) height=64;
		else if (height>32) height=32;
		else if (height>16) height=16;
		else if (height>8) height=8;
		else if (height>4) height=4;
		else if (height>2) height=2;
		else if (height>1) height=1;
	}

	// NULL filename indicates, that this texture is not shared automatically.
	// If user wants other materials to use this texture also, he/she gives
	// the pointer of this texture instead of giving a filename.

	glGenTextures(1, &texhandle);

	if (textype == TEXTYPE_CUBE || textype == TEXTYPE_CUBE_RENDER)
		glTextype = GL_TEXTURE_CUBE_MAP;
	else
		glTextype = GL_TEXTURE_2D;

	glBindTexture(glTextype, texhandle);
	// no mipmapping
	glTexParameteri(glTextype, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(glTextype, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glErrors();
	if (glTextype == GL_TEXTURE_CUBE_MAP) {
		for (int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			texfmt = GL_RGBA;
		}
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		texfmt = GL_RGBA;
	}
	glErrors();

/*
	D3DSURFACE_DESC ddsd;
	if(dx_handle)
	{
		ZeroMemory(&ddsd, sizeof(ddsd));
		dx_handle->GetLevelDesc(0, &ddsd);

		width = ddsd.Width;
		height = ddsd.Height;
	}


	// Create temp system memory buffer if needed (for copyrects. DX8 cannot lock rendertarget videomem buffers)
	if (IsRenderTarget() && dx_handle)
	{
		Storm3D2->D3DDevice->CreateOffscreenPlainSurface( width, height, ddsd.Format, D3DPOOL_SYSTEMMEM, &dx_tempbuf_sm, 0);
		if(!dx_tempbuf_sm)
		{
			MessageBox( NULL, "Storm error: Couldn't create offscreen plain surface for dynamic texture.", "Storm error", MB_OK);
		}
		if( ddsd.MultiSampleType != D3DMULTISAMPLE_NONE )
		{
			MessageBox( NULL, "Dynamic texture should not be multisampled.", "Storm error", MB_OK);
		}
	}

    */
	storm3d_texture_allocs++;
}

//! Constructor
Storm3D_Texture::Storm3D_Texture(Storm3D *s2) :
	decentAlpha(false),
	Storm3D2(s2),
	texhandle(0),
	texfmt(0),
	textype(TEXTYPE_BASIC),
	filename(NULL),
	tex_identity(0),
	texloadflags(0),
	refcount(1),
	width(0),
	height(0),
	auto_release(true)
{
	// Only for videotextures.
	// Leave the DX-handle NULL.
	// DX-handle will me modified directly by the videotexture
	storm3d_texture_allocs++;
}

//! Destructor
Storm3D_Texture::~Storm3D_Texture()
{
	storm3d_texture_allocs--;

	// Remove from Storm3D's list
	Storm3D2->Remove(this);

	// Delete filename
	if (filename) delete[] filename;

	if (texhandle != 0 && glIsTexture(texhandle)) {
		glDeleteTextures(1, &texhandle);
		texhandle = 0;
	}
}

//! Check if texture is identical with other texture
/*!
	\param _filename
	\param _texloadflags
	\param _tex_identity
	\return true if identical
*/
bool Storm3D_Texture::IsIdenticalWith(const char *_filename, DWORD _texloadflags, DWORD _tex_identity) const
{
	if ((filename==NULL)||(_filename==NULL)) return false;
	if (texloadflags!=_texloadflags) return false;
	if (tex_identity!=_tex_identity) return false;
	if (strcmp(_filename,filename)!=0) return false;
	return true;
}

//! Get filename of texture
/*!
	\return file name
*/
const char *Storm3D_Texture::GetFilename()
{
	return filename;
}

//! Get texture identity
/*!
	\return identity
*/
DWORD Storm3D_Texture::GetTexIdentity()
{
	return tex_identity;
}

//! Get surface info
/*!
	\return surface info
*/
Storm3D_SurfaceInfo Storm3D_Texture::GetSurfaceInfo()
{
	return Storm3D_SurfaceInfo(width,height,0);
}

void Storm3D_Texture::CopyTextureToAnother(IStorm3D_Texture *other)
{
	// Not used anywhere
	igios_unimplemented();
}

//! Copy 32 bit sysmem buffer to texture
/*!
	\param sysbuffer source memory buffer
*/
void Storm3D_Texture::Copy32BitSysMembufferToTexture(DWORD *sysbuffer)
{
	glBindTexture(GL_TEXTURE_2D, texhandle);
	glTexImage2D(GL_TEXTURE_2D, 0, texfmt, width, height, 0, texfmt, GL_UNSIGNED_BYTE, sysbuffer);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//! Copy texture to 32 bit sysmem buffer
/*!
	\param sysbuffer source memory buffer
*/
void Storm3D_Texture::CopyTextureTo32BitSysMembuffer(DWORD *sysbuffer)
{
	glBindTexture(GL_TEXTURE_2D, texhandle);
	glGetTexImage(GL_TEXTURE_2D, 0, texfmt, GL_UNSIGNED_BYTE, sysbuffer);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Storm3D_Texture::CopyTextureTo8BitSysMembuffer(BYTE *sysbuffer)
{
	// Not used anywhere
	igios_unimplemented();
}

//! Adds high priority status to texture
void Storm3D_Texture::AddHighPriority()
{
	if(++prioritycount)
	{
		const GLclampf p = 1;
		glPrioritizeTextures(1, &texhandle, &p);
	}
}

//! Removes high priority status from texture
void Storm3D_Texture::RemoveHighPriority()
{
	if(--prioritycount <= 0)
	{
		const GLclampf p = 0;
		glPrioritizeTextures(1, &texhandle, &p);
	}
}

//! Add reference to texture
void Storm3D_Texture::AddRef()
{
	// Add referencecount
	refcount++;
}

//! Release reference to texture
/*!
	\return true if that was the last reference
*/
bool Storm3D_Texture::Release()
{
	// Subtract reference count
	refcount--;

	// If the are no references left. This will be deleted
	if (refcount<1)
	{
		delete this;
		return true;
	} else return false;
}

void Storm3D_Texture::AnimateVideo()
{
	// Do nothing
}

void Storm3D_Texture::VideoSetFrame(int num)
{
	// Do nothing
}

void Storm3D_Texture::VideoSetFrameChangeSpeed(int millisecs)
{
	// Do nothing
}

void Storm3D_Texture::VideoSetLoopingParameters(VIDEOLOOP params)
{
	// Do nothing
}

int Storm3D_Texture::VideoGetFrameAmount()
{
	return 1;
}

int Storm3D_Texture::VideoGetCurrentFrame()
{
	return 0;
}

//! Swap texture to other texture
/*!
	\param other texture
*/
void Storm3D_Texture::swapTexture(IStorm3D_Texture *otherI)
{
	Storm3D_Texture *other = static_cast<Storm3D_Texture *> (otherI);
	if(!other)
	{
		assert(!"Whoops");
		return;
	}

	std::swap(texhandle, other->texhandle);
	std::swap(glTextype, other->glTextype);
	std::swap(texfmt, other->texfmt);
	std::swap(width, other->width);
	std::swap(height, other->height);
}

//! Apply texture to stage
/*!
	\param stage
*/
void Storm3D_Texture::Apply(int stage)
{
	glActiveTexture(GL_TEXTURE0 + stage);
	glClientActiveTexture(GL_TEXTURE0 + stage);

	// Apply correct texturemap (cube or basic)
	if ((textype==TEXTYPE_BASIC)||(textype==TEXTYPE_BASIC_RENDER)||(textype==TEXTYPE_BASIC_RPOOL)||(textype==TEXTYPE_DYNAMIC)||(textype==TEXTYPE_DYNAMIC_LOCKABLE)||(textype==TEXTYPE_DYNAMIC_LOCKABLE_32)) {
		glDisable(GL_TEXTURE_CUBE_MAP);
		glDisable(GL_TEXTURE_3D);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texhandle);
	} else {
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_3D);
		glEnable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texhandle);
	}
}

//! Is this cube map texture
/*!
	\return true if cubemap
*/
bool Storm3D_Texture::IsCube()
{
	if ((textype==TEXTYPE_CUBE)||(textype==TEXTYPE_CUBE_RENDER)) return true;
	return false;
}

//! Is this render target
/*!
	\return true if render target
*/
bool Storm3D_Texture::IsRenderTarget()
{
	if ((textype==TEXTYPE_BASIC_RENDER)||(textype==TEXTYPE_CUBE_RENDER)) return true;
	return false;
}

void Storm3D_Texture::saveToFile( const char * filename ) 
{
	igios_unimplemented();
}

void Storm3D_Texture::loadFromFile( const char * filename )
{
	igios_unimplemented();
}
