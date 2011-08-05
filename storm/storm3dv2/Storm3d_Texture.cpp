// Copyright 2002-2004 Frozenbyte Ltd.

#ifdef _MSC_VER
#pragma warning(disable:4103)
#endif

//------------------------------------------------------------------
// Includes
//------------------------------------------------------------------
#include "storm3d.h"
#include "storm3d_adapter.h"
#include "storm3d_texture.h"
#include <IStorm3D_Logger.h>

#include <stdio.h>
#include <map>
#include <string>
#include "../../filesystem/ifile_list.h"
#include "../../filesystem/file_package_manager.h"
#include "../../filesystem/input_stream_wrapper.h"
#include "../../util/Debug_MemoryManager.h"

#include <boost/lexical_cast.hpp>

using namespace frozenbyte;

namespace {
	int id;

	int pick_min(int a, int b)
	{
		return (a < b) ? a : b;
	}

	bool fileExists(const char *name)
	{
		if(!name)
			return false;

		filesystem::FB_FILE *fp = filesystem::fb_fopen(name, "rb");
		if(fp == 0)
			return false;

		filesystem::fb_fclose(fp);
		return true;
	}
/*
	class FindFileWrapper
	{
	public:
		enum Type
		{
			File,
			Dir
		};

	private:
		Type type;

		long handle;
		int result;

		_finddata_t data;

		bool isValid() const
		{
			std::string file = data.name;
			if(file == ".")
				return false;
			if(file == "..")
				return false;

			if(type == Dir)
			{
				if(data.attrib & _A_SUBDIR)
					return true;
				else 
					return false;
			}
			else if(type == File)
			{
				if(data.attrib & _A_SUBDIR)
					return false;
				else 
					return true;
			}

			return false;
		}

	public:
		FindFileWrapper(const char *spec, Type type_)
		:	type(type_),
			handle(-1),
			result(0)
		{
			handle = _findfirst(spec, &data);
			if(!isValid())
				next();
		}

		~FindFileWrapper()
		{
			if(handle != -1)
				_findclose(handle);
		}

		void next()
		{
			while(!end())
			{
				result = _findnext(handle, &data);

				if(isValid())
					break;
			}
		}

		const char *getName() const
		{
			return data.name;
		}

		bool end() const
		{
			if(handle == -1)
				return true;
			if(result == -1)
				return true;

			return false;
		}
	};

	class FileSeeker
	{
		// filename -> fullpath
		std::map<std::string, std::string> fileNames;
		IStorm3D_Logger *logger;

		std::string removePath(const char *file_) const
		{
			assert(file_);
			std::string file = file_;

			int index = file.find_last_of("/\\");
			if(index == file.npos)
				index = 0;
			else
				index += 1;

			int size = file.size() - index;
			return file.substr(index, size);
		}

		void getFiles(const std::string &dir, const char *fileType)
		{
			std::string spec = dir;
			spec += "\\";
			spec += fileType;

			FindFileWrapper files(spec.c_str(), FindFileWrapper::File);
			for(; !files.end(); files.next())
			{
				std::string baseName = files.getName();
				std::string fileName = dir + std::string("\\") + files.getName();

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

		void iterateDir(const std::string &dir, const char *fileType)
		{
			getFiles(dir, fileType);

			std::string findString = dir;
			findString += "\\*.*";

			FindFileWrapper dirs(findString.c_str(), FindFileWrapper::Dir);
			for(; !dirs.end(); dirs.next())
			{
				std::string currentDir = dir;
				currentDir += "\\";
				currentDir += dirs.getName();

				iterateDir(currentDir, fileType);
			}
		}

	public:
		FileSeeker(IStorm3D_Logger *logger_)
		:	logger(logger_)
		{
		}

		void addFiles(const char *fileType)
		{
			iterateDir("Data\\Textures", fileType);
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
#pragma message ("****************************")
#pragma message ("** Old imp for now **")
#pragma message ("****************************")
*/

	struct FileSeeker
	{
		// filename -> fullpath
		std::map<std::string, std::string> fileNames;
		IStorm3D_Logger *logger;

		std::string removePath(const char *file_) const
		{
			assert(file_);
			std::string file = file_;

			std::string::size_type index = file.find_last_of("/\\");
			if(index == file.npos)
				index = 0;
			else
				index += 1;

			int size = file.size() - index;
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

void freeTextureBank()
{
	delete fileSeeker;
	fileSeeker = 0;
}

std::string findTexture(const char *name)
{
	if(fileExists(name))
		return name;

	assert(fileSeeker);
	return fileSeeker->findPath(name);
}

//------------------------------------------------------------------
// Storm3D_Texture::Storm3D_Texture
//------------------------------------------------------------------
Storm3D_Texture::Storm3D_Texture(Storm3D *s2,const char *_filename,DWORD _texloadflags,
								 DWORD _tex_identity, const void *data, size_t data_size) :
	decentAlpha(false),
	Storm3D2(s2),
	dx_tempbuf_sm(NULL),
	dx_handle(NULL),
	dx_handle_cube(NULL),
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

	if(data == NULL && !fileExists(_filename))
	{
		assert(fileSeeker);
		//std::string name = fileSeeker->findPath(_filename);
		//filename = new char[name.size() + 1];
		//strcpy(filename, name.c_str());

		resolvedName = fileSeeker->findPath(_filename);
	}
	else
	{
		// Create filename
		//filename=new char[strlen(_filename)+1];
		//strcpy(filename,_filename);
		resolvedName = _filename;
	}

	if(!s2->hasHighQualityTextures() && resolvedName.size() > 4)
	{
		std::string newName = resolvedName.substr(0, resolvedName.size() - 4);
		newName += "_lowdetail";
		newName += resolvedName.substr(resolvedName.size() - 4, 4);

		if(fileExists(newName.c_str()))
			resolvedName = newName;
	}

	filename = new char[resolvedName.size() + 1];
	strcpy(filename, resolvedName.c_str());

	/*
	OutputDebugString("Loading texture: ");
	OutputDebugString(filename);ogui.LoadOguiImage
	OutputDebugString("\r\n");
	*/
	std::vector<char> fileData;
	if(data == NULL)
	{
		filesystem::FB_FILE *file = filesystem::fb_fopen(filename, "rb");
		if(file)
		{
			fileData.resize(filesystem::fb_fsize(file));
			filesystem::fb_fread(&fileData[0], 1, fileData.size(), file);

			filesystem::fb_fclose(file);

			data = &fileData[0];
			data_size = fileData.size();
		}
	}

	// Pack non-DDS textures with DXT1
	// DDS textures are left unpacked (or prepacked with DDS-file's format)
	//D3DFORMAT df=D3DFMT_DXT1;
	// psd
	D3DFORMAT df=D3DFMT_DXT1;
	int l=strlen(filename);
	if (l>4)
	if ((filename[l-1]=='s'||filename[l-1]=='S')&&
		(filename[l-2]=='d'||filename[l-2]=='D')&&
		(filename[l-3]=='d'||filename[l-3]=='D')) df=D3DFMT_UNKNOWN;

	// Check if device supports cubemaps
	bool supports_cubes=(Storm3D2->adapters[Storm3D2->active_adapter].caps&Storm3D_Adapter::CAPS_TEX_CUBE)!=0;
	if (!supports_cubes) df=D3DFMT_DXT1;

	// Image info (filled by DX8's texture load routines)
	D3DXIMAGE_INFO img_info;

	// Try to create DDS file as a cubemap first
	if (df==D3DFMT_UNKNOWN)
	{
		D3DXIMAGE_INFO imageInfo = { 0 };
		D3DXGetImageInfoFromFileInMemory(data, data_size, &imageInfo);

		int mipmaps = D3DX_DEFAULT;
		if(imageInfo.MipLevels <= 1)
			mipmaps = 1;

		// Load cube texture
		//HRESULT hr=D3DXCreateCubeTextureFromFileExA(Storm3D2->D3DDevice,filename,D3DX_DEFAULT,
		//	D3DX_DEFAULT,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_FILTER_TRIANGLE|D3DX_FILTER_DITHER,
		//	D3DX_FILTER_BOX,0,NULL,NULL,&dx_handle_cube);
		HRESULT hr=D3DXCreateCubeTextureFromFileInMemoryEx(Storm3D2->D3DDevice,data,data_size,D3DX_DEFAULT,
			mipmaps,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,D3DX_FILTER_TRIANGLE|D3DX_FILTER_DITHER,
			D3DX_FILTER_BOX,0,NULL,NULL,&dx_handle_cube);

		// Test if succeeded
		if (!FAILED(hr))
		{
			// Set texture LOD level
			// -jpk
			if((texloadflags & TEXLOADFLAGS_NOLOD) == 0 && mipmaps <= 0) 
			{
				int lod = s2->GetTextureLODLevel();
				int lods = dx_handle_cube->GetLevelCount();
				if(lod >= lods)
					lod = lods - 1;
				if(lod < 0)
					lod = 0;

				dx_handle_cube->SetLOD(lod);
			}

			// Set values
			height=0;
			width=0;
			textype=TEXTYPE_CUBE;
		}
		else textype=TEXTYPE_BASIC;
	}

	// No compress? (prefer 32bit format)
	if (texloadflags&TEXLOADFLAGS_NOCOMPRESS) df=D3DFMT_A8R8G8B8;

	// BETA!! DXTC disabled
	//df=D3DFMT_A8R8G8B8;

	// If texture is not cubemap, load as a basic texture
	if (textype==TEXTYPE_BASIC)
	{
		// psd: Get some data
		D3DXIMAGE_INFO imageInfo = { 0 };
		//D3DXGetImageInfoFromFile(filename, &imageInfo);
		D3DXGetImageInfoFromFileInMemory(data, data_size, &imageInfo);

		//df = imageInfo.Format;
		//if (texloadflags&TEXLOADFLAGS_NOCOMPRESS) 
		//	df=D3DFMT_A8R8G8B8;

		df = imageInfo.Format;
		if(imageInfo.Format == D3DFMT_DXT5 || imageInfo.Format == D3DFMT_A8R8G8B8)
			decentAlpha = true;

		// Disable mipmaps for non-mipmapped files
		int mipmaps = D3DX_DEFAULT;
		if(imageInfo.MipLevels <= 1)
		{
#ifdef PROJECT_AOV
			// aov does mipmap.
#else
			mipmaps = 1;
#endif
		}

		if (texloadflags & TEXLOADFLAGS_NOCOMPRESS)
			df = D3DFMT_A8R8G8B8;

		// Don't compress unless dds
		if((strcmp(filename + strlen(filename) - 4, ".dds") != 0))
		{
#ifdef PROJECT_AOV
			// aov does compress.
#else
			df = D3DFMT_A8R8G8B8;
#endif
		}

#ifdef PROJECT_AOV
		// some new filename tags to define no compression / no mipmaps
		// only in use for AOV, others probably don't need this(?), so no need to add extra string parsing
		if(strstr(filename, "_nocompress") != NULL)
		{
			df = D3DFMT_A8R8G8B8;
		}
		if(strstr(filename, "_nomip") != NULL)
		{
			mipmaps = 1;
		}
		if(strstr(filename, "_alphatest") != NULL)
		{
			decentAlpha = false;
		}
#endif

	/*
	unsigned int width = imageInfo.Width;
	unsigned int height = imageInfo.Height;

	if((texloadflags & TEXLOADFLAGS_NOLOD) == 0 && mipmaps <= 0) 
	{
		int lod = s2->GetTextureLODLevel();
		if(lod)
		{
			width >>= lod;
			height >>= lod;
		}
	}

		// Load texture
	HRESULT hr = S_OK;
	if(mipmaps == 1 || (width == imageInfo.Width && height == imageInfo.Height))
	{
		hr=D3DXCreateTextureFromFileInMemoryEx(Storm3D2->D3DDevice,data,data_size, D3DX_DEFAULT,
			D3DX_DEFAULT,mipmaps,0,df,D3DPOOL_MANAGED,D3DX_FILTER_TRIANGLE|D3DX_FILTER_DITHER,
			D3DX_FILTER_BOX,0,&img_info,NULL,&dx_handle);
	}
	else
	{
		CComPtr<IDirect3DTexture9> temp_handle;
		hr = D3DXCreateTextureFromFileInMemoryEx(Storm3D2->D3DDevice,data,data_size, D3DX_DEFAULT,
			D3DX_DEFAULT,mipmaps,0,df,D3DPOOL_SYSTEMMEM,D3DX_FILTER_TRIANGLE|D3DX_FILTER_DITHER,
			D3DX_FILTER_BOX,0,&img_info,NULL,&temp_handle);

		if(SUCCEEDED(hr))
		{
			hr = D3DXCreateTexture(Storm3D2->D3DDevice, width, height, mipmaps, 0, df, D3DPOOL_MANAGED, &dx_handle);
			if(SUCCEEDED(hr))
			{
				int level = 0;
				int x = width;
				int y = height;
				int lod = s2->GetTextureLODLevel();

				while(x > 1 || y > 1)
				{
					CComPtr<IDirect3DSurface9> src;
					temp_handle->GetSurfaceLevel(level + lod, &src);
					CComPtr<IDirect3DSurface9> dst;
					dx_handle->GetSurfaceLevel(level, &dst);

					D3DXLoadSurfaceFromSurface(dst, 0, 0, src, 0, 0, D3DX_FILTER_NONE, 0);
					++level;
					x >>= 1;
					y >>= 1;
				}
			}
		}

	}

	this->width = width;
	this->height = height;
	*/

		// Skip mipmaps during load time instead
		DWORD mipFilter = D3DX_FILTER_BOX;
		if((texloadflags & TEXLOADFLAGS_NOLOD) == 0 && mipmaps <= 0) 
		{
			int lodLevel = s2->GetTextureLODLevel();
			if(lodLevel)
				mipFilter = D3DX_SKIP_DDS_MIP_LEVELS(lodLevel, D3DX_FILTER_BOX);
		}

		DWORD filter = D3DX_FILTER_TRIANGLE|D3DX_FILTER_DITHER;
		if (texloadflags&TEXLOADFLAGS_NOWRAP)
			filter |= D3DX_FILTER_MIRROR;

		HRESULT hr=D3DXCreateTextureFromFileInMemoryEx(Storm3D2->D3DDevice,data,data_size, D3DX_DEFAULT,
			D3DX_DEFAULT,mipmaps,0,df,D3DPOOL_MANAGED,filter,
			mipFilter,0,&img_info,NULL,&dx_handle);

		if (!FAILED(hr))
		{
			/*
			// Set texture LOD level
			// -jpk
			if((texloadflags & TEXLOADFLAGS_NOLOD) == 0 && mipmaps <= 0) 
			{
				int lod = s2->GetTextureLODLevel();
				int lods = dx_handle->GetLevelCount();
				if(lod >= lods)
					lod = lods - 1;
				if(lod < 0)
					lod = 0;

				dx_handle->SetLOD(lod);
			}
			*/

			// Set values
			this->height=img_info.Height;
			this->width=img_info.Width;
		}
		else // No DXT support
		{

			DWORD filter = D3DX_FILTER_TRIANGLE|D3DX_FILTER_DITHER;
			if (texloadflags&TEXLOADFLAGS_NOWRAP)
				filter |= D3DX_FILTER_MIRROR;

			// Load texture without DTXC compression
			HRESULT hr=D3DXCreateTextureFromFileInMemoryEx(Storm3D2->D3DDevice,data,data_size,D3DX_DEFAULT,
				D3DX_DEFAULT,D3DX_DEFAULT,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,filter,
				D3DX_FILTER_BOX,0,&img_info,NULL,&dx_handle);


			// Set texture LOD level
			// -jpk
			if(!FAILED(hr))
			{
				if ((texloadflags & TEXLOADFLAGS_NOLOD) == 0) 
				{
					int lod = s2->GetTextureLODLevel();
					int lods = dx_handle->GetLevelCount();
					if(lod >= lods)
						lod = lods - 1;
					if(lod < 0)
						lod = 0;

					dx_handle->SetLOD( lod );
				}
			}

			// Set values
			height=img_info.Height;
			width=img_info.Width;
		}
	}
	
	// scope kludge (just to be on the safe side)
	fileData.clear();

	storm3d_texture_allocs++;
	//dx_handle->SetLOD(1);
}



//------------------------------------------------------------------
// Storm3D_Texture::Storm3D_Texture
//------------------------------------------------------------------
Storm3D_Texture::Storm3D_Texture(Storm3D *s2,int _width,int _height,TEXTYPE ttype) :
	decentAlpha(true),
	Storm3D2(s2),
	dx_tempbuf_sm(NULL),
	dx_handle(NULL),
	dx_handle_cube(NULL),
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
	int mwidth,mheight;
	Storm3D_SurfaceInfo ss=Storm3D2->GetScreenSize();
	mwidth=Storm3D2->adapters[Storm3D2->active_adapter].maxtexsize.width;
	mheight=Storm3D2->adapters[Storm3D2->active_adapter].maxtexsize.height;
	if (width<1) width=1;
	if (height<1) height=1;
	if (width>mwidth) 
	{
		MessageBox( NULL, "GFX card doesn't support textures this big.", "***", MB_OK );
		width=mwidth;
	}
	if (height>mheight) 
	{
		MessageBox( NULL, "GFX card doesn't support textures this big.", "***", MB_OK );
		height=mheight;
	}
//	if (width>ss.width) width=ss.width;
//	if (height>ss.height) height=ss.height;

	if(textype != TEXTYPE_RAM && textype != TEXTYPE_DYNAMIC && textype != TEXTYPE_BASIC_RENDER && textype != TEXTYPE_DYNAMIC_LOCKABLE && textype != TEXTYPE_DYNAMIC_LOCKABLE_32)
	{
		// Make 2 pot
		if(width>=4096) width=4096;
		else if (width>=2048) width=2048;
		else if (width>=1024) width=1024;
		else if (width>=512) width=512;
		else if (width>=256) width=256;
		else if (width>=128) width=128;
		else if (width>=64) width=64;
		else if (width>=32) width=32;
		else if (width>=16) width=16;
		else if (width>=8) width=8;
		else if (width>=4) width=4;
		else if (width>=2) width=2;
		else if (width>=1) width=1;
		
		if (height>=4096) height=4096;
		else if (height>=2048) height=2048;
		else if (height>=1024) height=1024;
		else if (height>=512) height=512;
		else if (height>=256) height=256;
		else if (height>=128) height=128;
		else if (height>=64) height=64;
		else if (height>=32) height=32;
		else if (height>=16) height=16;
		else if (height>=8) height=8;
		else if (height>=4) height=4;
		else if (height>=2) height=2;
		else if (height>=1) height=1;
	}

	// Check if device supports cubemaps
	bool supports_cubes=(Storm3D2->adapters[Storm3D2->active_adapter].caps&Storm3D_Adapter::CAPS_TEX_CUBE)!=0;
	if (!supports_cubes)
	{
		if (textype==TEXTYPE_CUBE) 
			textype=TEXTYPE_BASIC;
		if (textype==TEXTYPE_CUBE_RENDER) 
			textype=TEXTYPE_BASIC_RENDER;
	}

	// NULL filename indicates, that this texture is not shared automatically.
	// If user wants other materials to use this texture also, he/she gives
	// the pointer of this texture instead of giving a filename.

	switch (textype)
	{
		case TEXTYPE_BASIC_RPOOL:
			{
				// Create (empty) texture to default pool (without mipmaps)
				/*HRESULT hr=*/D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
					0,Storm3D2->present_params.BackBufferFormat,D3DPOOL_DEFAULT,&dx_handle);
	
				//if (FAILED(hr)) MessageBox(NULL,"Failed to create dynamic texture","Storm3D Error",0);
			}
			break;

		case TEXTYPE_BASIC_RENDER:
			{
				// Create (empty) rendertarget texture (without mipmaps)

				//HRESULT hr=D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
				//	D3DUSAGE_RENDERTARGET,D3DFMT_A4R4G4B4,D3DPOOL_DEFAULT,&dx_handle);
				//HRESULT hr=D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
				//	D3DUSAGE_RENDERTARGET,D3DFMT_A1R5G5B5,D3DPOOL_DEFAULT,&dx_handle);
				HRESULT hr=D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
					D3DUSAGE_RENDERTARGET,D3DFMT_R5G6B5,D3DPOOL_DEFAULT,&dx_handle);

				if(FAILED(hr))
				{
					/*HRESULT hr=*/D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
						D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,&dx_handle);
				}

				/*
				if(FAILED(hr))
				{
					HRESULT hr=D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
						D3DUSAGE_RENDERTARGET,D3DFMT_A4R4G4B4,D3DPOOL_DEFAULT,&dx_handle);
				}

				if(FAILED(hr))
				{
					hr=D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
						D3DUSAGE_RENDERTARGET,Storm3D2->present_params.BackBufferFormat,D3DPOOL_DEFAULT,&dx_handle);
				}
				*/
	
				//if (FAILED(hr)) MessageBox(NULL,"Failed to create dynamic texture","Storm3D Error",0);
			}
			break;

		case TEXTYPE_CUBE_RENDER:
			{
				// Create (empty) rendertarget cubetexture (without mipmaps)
				/*HRESULT hr=*/D3DXCreateCubeTexture(Storm3D2->D3DDevice,width,1,
					D3DUSAGE_RENDERTARGET,Storm3D2->present_params.BackBufferFormat,D3DPOOL_DEFAULT,&dx_handle_cube);

				//if (FAILED(hr)) MessageBox(NULL,"Failed to create dynamic cube texture","Storm3D Error",0);
			}
			break;

		case TEXTYPE_BASIC:
			{
				// Create (empty) texture (without mipmaps)
				/*HRESULT hr=*/D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
					0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&dx_handle);
					//0,Storm3D2->present_params.BackBufferFormat,D3DPOOL_MANAGED,&dx_handle);
				
				//if (FAILED(hr)) MessageBox(NULL,"Failed to create dynamic texture","Storm3D Error",0);
			}
			break;

		case TEXTYPE_RAM:
			{
				HRESULT hr = Storm3D2->D3DDevice->CreateTexture(width, height, 1, 
					0, Storm3D2->present_params.BackBufferFormat, D3DPOOL_SYSTEMMEM, &dx_handle, 0);

				if(FAILED(hr))
				{
					/*HRESULT hr=*/D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
						0,Storm3D2->present_params.BackBufferFormat,D3DPOOL_SYSTEMMEM,&dx_handle);
				}
			}
			break;

		case TEXTYPE_DYNAMIC:
			{
				HRESULT hr = Storm3D2->D3DDevice->CreateTexture(width,height,1,
					0,Storm3D2->present_params.BackBufferFormat,D3DPOOL_DEFAULT,&dx_handle, 0);

				if(FAILED(hr))
				{
					/*HRESULT hr=*/D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
						0,Storm3D2->present_params.BackBufferFormat,D3DPOOL_DEFAULT,&dx_handle);
				}
			}
			break;

		case TEXTYPE_DYNAMIC_LOCKABLE:
			{
				//HRESULT hr = Storm3D2->D3DDevice->CreateTexture(width,height,1,
				//	D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&dx_handle, 0);
				HRESULT hr = Storm3D2->D3DDevice->CreateTexture(width,height,1,
					D3DUSAGE_DYNAMIC,D3DFMT_R5G6B5,D3DPOOL_DEFAULT,&dx_handle, 0);

				if(FAILED(hr))
				{
					//HRESULT hr=D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
					//	D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&dx_handle);
					/*HRESULT hr=*/D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
						D3DUSAGE_DYNAMIC,D3DFMT_R5G6B5,D3DPOOL_DEFAULT,&dx_handle);
				}
			}
			break;

		case TEXTYPE_DYNAMIC_LOCKABLE_32:
			{
				HRESULT hr = Storm3D2->D3DDevice->CreateTexture(width,height,1,
					D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&dx_handle, 0);

				if(FAILED(hr))
				{
					/*HRESULT hr=*/D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
						D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&dx_handle);
				}
			}
			break;

		case TEXTYPE_CUBE:
			{
				// Create (empty) cubetexture (without mipmaps)
				/*HRESULT hr=*/D3DXCreateCubeTexture(Storm3D2->D3DDevice,width,1,
					0,Storm3D2->present_params.BackBufferFormat,D3DPOOL_MANAGED,&dx_handle_cube);

				//if (FAILED(hr)) MessageBox(NULL,"Failed to create dynamic cube texture","Storm3D Error",0);
			}
			break;
	}

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

	storm3d_texture_allocs++;
}



//------------------------------------------------------------------
// Storm3D_Texture::Storm3D_Texture
//------------------------------------------------------------------
Storm3D_Texture::Storm3D_Texture(Storm3D *s2) :
	decentAlpha(false),
	Storm3D2(s2),
	dx_tempbuf_sm(NULL),
	dx_handle(NULL),
	dx_handle_cube(NULL),
	textype(TEXTYPE_BASIC),
	filename(NULL),
	tex_identity(0),
	texloadflags(0),
	refcount(1),
	width(0),
	height(0),
	auto_release(true)
	//zbuffer(NULL)
{
	// Only for videotextures.
	// Leave the DX-handle NULL.
	// DX-handle will me modified directly by the videotexture
	storm3d_texture_allocs++;
}



//------------------------------------------------------------------
// Storm3D_Texture::~Storm3D_Texture
//------------------------------------------------------------------
Storm3D_Texture::~Storm3D_Texture()
{
	storm3d_texture_allocs--;

	// Remove from Storm3D's list
	Storm3D2->Remove(this);

	// Delete DX-surfaces
	if (dx_handle) dx_handle->Release();
	if (dx_handle_cube) dx_handle_cube->Release();

	// Delete filename
	if (filename) delete[] filename;

	// Release temp sysmem buffer
	if (dx_tempbuf_sm) dx_tempbuf_sm->Release();

	// Release temp Z-buffer
	//if (zbuffer) zbuffer->Release();
}

void *Storm3D_Texture::classId()
{
	return &id;
}

void *Storm3D_Texture::getId() const
{
	return &id;
}

//------------------------------------------------------------------
// Storm3D_Texture::AddRef
//------------------------------------------------------------------
void Storm3D_Texture::AddRef()
{
	// Add referencecount
	refcount++;
}


void Storm3D_Texture::AddHighPriority()
{
	if(++prioritycount)
	{
		if(dx_handle)
		{
			dx_handle->SetPriority(10);
			dx_handle->PreLoad();
		}
	}
}

void Storm3D_Texture::RemoveHighPriority()
{
	if(--prioritycount <= 0)
	{
		if(dx_handle)
			dx_handle->SetPriority(0);
	}
}


//------------------------------------------------------------------
// Storm3D_Texture::Release
//------------------------------------------------------------------
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



//------------------------------------------------------------------
// Storm3D_Texture::IsIdenticalWith
//------------------------------------------------------------------
bool Storm3D_Texture::IsIdenticalWith(const char *_filename,DWORD _texloadflags,DWORD _tex_identity) const
{
	if ((filename==NULL)||(_filename==NULL)) return false;
	if (texloadflags!=_texloadflags) return false;
	if (tex_identity!=_tex_identity) return false;
	if (strcmp(_filename,filename)!=0) return false;
	return true;
}



//------------------------------------------------------------------
// Storm3D_Texture::VideoSetFrame
//------------------------------------------------------------------
void Storm3D_Texture::VideoSetFrame(int num)
{
	// Do nothing
}



//------------------------------------------------------------------
// Storm3D_Texture::VideoSetFrameChangeSpeed
//------------------------------------------------------------------
void Storm3D_Texture::VideoSetFrameChangeSpeed(int millisecs)
{
	// Do nothing
}



//------------------------------------------------------------------
// Storm3D_Texture::VideoSetLoopingParameters
//------------------------------------------------------------------
void Storm3D_Texture::VideoSetLoopingParameters(VIDEOLOOP params)
{
	// Do nothing
}



//------------------------------------------------------------------
// Storm3D_Texture::VideoGetFrameAmount
//------------------------------------------------------------------
int Storm3D_Texture::VideoGetFrameAmount()
{
	return 1;
}



//------------------------------------------------------------------
// Storm3D_Texture::VideoGetCurrentFrame
//------------------------------------------------------------------
int Storm3D_Texture::VideoGetCurrentFrame()
{
	return 0;
}



//------------------------------------------------------------------
// Storm3D_Texture_Video::AnimateVideo
//------------------------------------------------------------------
void Storm3D_Texture::AnimateVideo()
{
	// Do nothing
}



//------------------------------------------------------------------
// Storm3D_Texture::Apply
//------------------------------------------------------------------
void Storm3D_Texture::Apply(int stage)
{
	// Apply correct texturemap (cube or basic)
	if ((textype==TEXTYPE_BASIC)||(textype==TEXTYPE_BASIC_RENDER)||(textype==TEXTYPE_BASIC_RPOOL)||(textype==TEXTYPE_DYNAMIC)||(textype==TEXTYPE_DYNAMIC_LOCKABLE)||(textype==TEXTYPE_DYNAMIC_LOCKABLE_32))
		Storm3D2->D3DDevice->SetTexture(stage,dx_handle);
	else
		Storm3D2->D3DDevice->SetTexture(stage,dx_handle_cube);
}

//------------------------------------------------------------------
// Storm3D_Texture - Get values
//------------------------------------------------------------------
bool Storm3D_Texture::IsCube()
{
	if ((textype==TEXTYPE_CUBE)||(textype==TEXTYPE_CUBE_RENDER)) return true;
	return false;
}


bool Storm3D_Texture::IsRenderTarget()
{
	if ((textype==TEXTYPE_BASIC_RENDER)||(textype==TEXTYPE_CUBE_RENDER)) return true;
	return false;
}



//------------------------------------------------------------------
// Storm3D_Texture - Texturecopy
//------------------------------------------------------------------
void Storm3D_Texture::CopyTextureToAnother(IStorm3D_Texture *other)
{
	// Typecast
	Storm3D_Texture *other_i=(Storm3D_Texture*)other;

	// Copy
	if ((dx_handle)&&(other_i->dx_handle))
	{
		if(textype == TEXTYPE_RAM && other_i->textype == TEXTYPE_DYNAMIC)
		{
			other_i->dx_handle->AddDirtyRect(0);
			/*HRESULT hr = */Storm3D2->D3DDevice->UpdateTexture(dx_handle, other_i->dx_handle);
			/*if(FAILED(hr))
			{
				int a = 0;
			}*/
		}
		else
		{
			LPDIRECT3DSURFACE9 surf,surf2;
			dx_handle->GetSurfaceLevel(0,&surf);
			other_i->dx_handle->GetSurfaceLevel(0,&surf2);

			Storm3D2->D3DDevice->UpdateSurface(surf,0,surf2,0);
			surf2->Release();
			surf->Release();
		}
	}
}



void Storm3D_Texture::CopyTextureTo8BitSysMembuffer(BYTE *sysbuffer)
{
	// Copy texture to system memory, if it's rendertarget
	if (IsRenderTarget()) 
	{
		LPDIRECT3DSURFACE9 surf;
		dx_handle->GetSurfaceLevel(0,&surf);
		if (FAILED(Storm3D2->D3DDevice->UpdateSurface(surf,0,dx_tempbuf_sm,0)))
			MessageBox(NULL,"CopyRects error (vid to system8)","Storm3D Error",0);
		surf->Release();
	}
	else
	{
		// Otherwise don't, just make the sysmem pointer point to videomem surface (no copying needed)
		dx_handle->GetSurfaceLevel(0,&dx_tempbuf_sm);	
	}

	// Get surface desc
	D3DSURFACE_DESC sdesc;
	dx_tempbuf_sm->GetDesc(&sdesc);

	// Lock surface
	D3DLOCKED_RECT lrect;
	if (!FAILED(dx_tempbuf_sm->LockRect(&lrect,NULL,D3DLOCK_READONLY)))
	{	
		if ((sdesc.Format==D3DFMT_A8R8G8B8)||(sdesc.Format==D3DFMT_X8R8G8B8))
		{
			BYTE *dp=(BYTE*)lrect.pBits;
			BYTE *ep=(BYTE*)dp;
			int rp=0;
			for (int row=0;row<height;row++)
			{
				dp=(BYTE*)ep;
				ep=(BYTE*)dp+lrect.Pitch;
				for (int i=0;i<width;i++)
				{
					sysbuffer[i+rp]=255-(*dp);
					dp+=4;
				}
				rp+=width;
			}
		}
		else
		if ((sdesc.Format==D3DFMT_R8G8B8))
		{
			BYTE *dp=(BYTE*)lrect.pBits;
			BYTE *ep=(BYTE*)dp;
			int rp=0;
			for (int row=0;row<height;row++)
			{
				dp=(BYTE*)ep;
				ep=(BYTE*)dp+lrect.Pitch;
				for (int i=0;i<width;i++)
				{
					sysbuffer[i+rp]=255-(*dp);
					dp+=3;
				}
				rp+=width;
			}
		}
		else
		if ((sdesc.Format==D3DFMT_R5G6B5))
		{
			WORD *dp=(WORD*)lrect.pBits;
			BYTE *ep=(BYTE*)dp;
			int rp=0;
			for (int row=0;row<height;row++)
			{
				dp=(WORD*)ep;
				ep=(BYTE*)dp+lrect.Pitch;
				for (int i=0;i<width;i++)
				{
					sysbuffer[i+rp]=255-(((*dp)&0x07E0)>>3);
					dp++;
				}
				rp+=width;
			}
		}
		else
		if ((sdesc.Format==D3DFMT_X1R5G5B5)||(sdesc.Format==D3DFMT_A1R5G5B5))
		{
			WORD *dp=(WORD*)lrect.pBits;
			BYTE *ep=(BYTE*)dp;
			int rp=0;
			for (int row=0;row<height;row++)
			{
				dp=(WORD*)ep;
				ep=(BYTE*)dp+lrect.Pitch;
				for (int i=0;i<width;i++)
				{
					sysbuffer[i+rp]=255-(((*dp)&0x03E0)>>2);
					dp++;
				}
				rp+=width;
			}
		}
		else 
		{
			char s[80];
			sprintf(s,"Illegal videomode format (%d)",sdesc.Format);
			MessageBox(NULL,s,"Storm3D Error",0);	
		}

		// Unlock buffer
		dx_tempbuf_sm->UnlockRect();
	
	} 
	else MessageBox(NULL,"Cannot lock visbuffer","Storm3D Error",0);

	// Set systembuffer pointer back to NULL if not rendertarget (otherwise engine tries to delete buffer twice)
	if (!IsRenderTarget())
	{
		// Free temporary systembuffer handle (if not rendertarget)
		SAFE_RELEASE(dx_tempbuf_sm); 
	}
}



void Storm3D_Texture::Copy32BitSysMembufferToTexture(DWORD *sysbuffer)
{
	// If texture is not rendertarget, just make the sysmem pointer point to videomem surface (no copying needed)
	if (!IsRenderTarget())
	{
		dx_handle->GetSurfaceLevel(0,&dx_tempbuf_sm);	
	}

	// Get surface desc
	D3DSURFACE_DESC sdesc;
	dx_tempbuf_sm->GetDesc(&sdesc);

	// Lock surface
	D3DLOCKED_RECT lrect;
	HRESULT hr = dx_tempbuf_sm->LockRect(&lrect,NULL,0); 
	if (!FAILED(hr))
	{	
		if ((sdesc.Format==D3DFMT_A8R8G8B8)||(sdesc.Format==D3DFMT_X8R8G8B8))
		{
			BYTE *dp=(BYTE*)lrect.pBits;
			//if (lrect.Pitch!=width*4) MessageBox(NULL,"Error in SYSMEM (P)","Storm3D Error",0);
			//int siz=width*height;
			//memcpy(dp,sysbuffer,sizeof(DWORD)*siz);

			
			// sigh.
			// --psd
			for(int y = 0; y < height; ++y)
			{
				void *destination = &dp[y * lrect.Pitch];
				void *source = &sysbuffer[y * width];
				memcpy(destination, source, width * 4);
			}
		}
		else
		if ((sdesc.Format==D3DFMT_R8G8B8))
		{
			BYTE *dp=(BYTE*)lrect.pBits;
			//if (lrect.Pitch!=width*3) MessageBox(NULL,"Error in SYSMEM (P)","Storm3D Error",0);
			int siz=width*height;
			BYTE *sb=(BYTE*)sysbuffer;
			int ip=0;
			for (int i=0;i<siz;i++)
			{
				// ip+0 = alphachannel (ARGB)
				*dp++=sb[ip+2];
				*dp++=sb[ip+1];
				*dp++=sb[ip+0];
				ip+=4;
			}
		}
		else
		if ((sdesc.Format==D3DFMT_R5G6B5))
		{
			WORD *dp=(WORD*)lrect.pBits;
			//if (lrect.Pitch!=width*2) MessageBox(NULL,"Error in SYSMEM (P)","Storm3D Error",0);
			int siz=width*height;
			BYTE *sb=(BYTE*)sysbuffer;
			int ip=0;
			for (int i=0;i<siz;i++)
			{
				// ip+0 = alphachannel (ARGB)
				*dp++=((sb[ip+2]>>3)<<11)+((sb[ip+1]>>2)<<5)+(sb[ip+0]>>3);
				ip+=4;
			}
		}
		else
		if ((sdesc.Format==D3DFMT_X1R5G5B5)||(sdesc.Format==D3DFMT_A1R5G5B5))
		{
			WORD *dp=(WORD*)lrect.pBits;
			//if (lrect.Pitch!=width*2) MessageBox(NULL,"Error in SYSMEM (P)","Storm3D Error",0);
			int siz=width*height;
			BYTE *sb=(BYTE*)sysbuffer;
			int ip=0;
			for (int i=0;i<siz;i++)
			{
				// ip+0 = alphachannel (ARGB)
				*dp++=((sb[ip+3]>>7)<<15)+((sb[ip+2]>>3)<<10)+((sb[ip+1]>>3)<<5)+(sb[ip+0]>>3);
				ip+=4;
			}
		}
		else 
		{
			char s[80];
			sprintf(s,"Illegal videomode format (%d)",sdesc.Format);
			//MessageBox(NULL,s,"Storm3D Error",0);	
		}

		// Unlock buffer
		dx_tempbuf_sm->UnlockRect();
	
	} 
	//else MessageBox(NULL,"Cannot lock visbuffer","Storm3D Error",0);

	// Copy system memory to texture (if texture is rendertarget)
	if (IsRenderTarget()) 
	{
		LPDIRECT3DSURFACE9 surf;
		dx_handle->GetSurfaceLevel(0,&surf);
		if (FAILED(Storm3D2->D3DDevice->UpdateSurface(dx_tempbuf_sm,0,surf,0)))
			MessageBox(NULL,"CopyRects error (system32 to vid)","Storm3D Error",0);
		surf->Release();
	}
	else
	{
		// Free temporary systembuffer handle (if not rendertarget)
		SAFE_RELEASE(dx_tempbuf_sm); 
	}
}



void Storm3D_Texture::CopyTextureTo32BitSysMembuffer(DWORD *sysbuffer)
{
	// Copy texture to system memory, if it's rendertarget
	if (IsRenderTarget()) 
	{
		LPDIRECT3DSURFACE9 surf;
		dx_handle->GetSurfaceLevel(0,&surf);
		assert(dx_tempbuf_sm);	// Crashes if NULL.
//		if (FAILED(Storm3D2->D3DDevice->UpdateSurface(surf,0,dx_tempbuf_sm,0)))
//			MessageBox(NULL,"CopyRects error (vid to system32)","Storm3D Error",0);
		if(FAILED(Storm3D2->D3DDevice->GetRenderTargetData ( surf, dx_tempbuf_sm )))
			MessageBox(NULL,"CopyRects error (vid to system32)","Storm3D Error",0);
		//surf->Release();
	}
	else
	{
		// Otherwise don't, just make the sysmem pointer point to videomem surface (no copying needed)
		dx_handle->GetSurfaceLevel(0,&dx_tempbuf_sm);	
	}

	// Get surface desc
	D3DSURFACE_DESC sdesc;
	dx_tempbuf_sm->GetDesc(&sdesc);

	// Lock surface
	D3DLOCKED_RECT lrect;
	if (!FAILED(dx_tempbuf_sm->LockRect(&lrect,NULL,0)))
	{	
		if ((sdesc.Format==D3DFMT_A8R8G8B8)||(sdesc.Format==D3DFMT_X8R8G8B8))
		{
			BYTE *dp=(BYTE*)lrect.pBits;
			if (lrect.Pitch!=width*4) MessageBox(NULL,"Error in SYSMEM (P)","Storm3D Error",0);
			int siz=width*height;
			memcpy(sysbuffer,dp,sizeof(DWORD)*siz);
		}
		else
		if ((sdesc.Format==D3DFMT_R8G8B8))
		{
			BYTE *dp=(BYTE*)lrect.pBits;
			if (lrect.Pitch!=width*3) MessageBox(NULL,"Error in SYSMEM (P)","Storm3D Error",0);
			int siz=width*height;
			BYTE *sb=(BYTE*)sysbuffer;
			int ip=0;
			for (int i=0;i<siz;i++)
			{
				// ip+0 = alphachannel (ARGB)
				sb[ip+3]=0;
				sb[ip+2]=*dp++;
				sb[ip+1]=*dp++;
				sb[ip+0]=*dp++;
				ip+=4;
			}
		}
		else
		if ((sdesc.Format==D3DFMT_R5G6B5))
		{
			WORD *dp=(WORD*)lrect.pBits;
			if (lrect.Pitch!=width*2) MessageBox(NULL,"Error in SYSMEM (P)","Storm3D Error",0);
			int siz=width*height;
			BYTE *sb=(BYTE*)sysbuffer;
			int ip=0;
			for (int i=0;i<siz;i++)
			{
				// ip+0 = alphachannel (ARGB)
				WORD pix=*dp++;
				//((sb[ip+2]>>3)<<11)+((sb[ip+1]>>2)<<5)+(sb[ip+0]>>3);
				sb[ip+3]=0;
				sb[ip+2]=(pix>>11)<<3;
				sb[ip+1]=((pix>>5)&0x3f)<<2;
				sb[ip+0]=((pix)&0x1f)<<3;
				ip+=4;
			}
		}
		else
		if ((sdesc.Format==D3DFMT_X1R5G5B5)||(sdesc.Format==D3DFMT_A1R5G5B5))
		{
			WORD *dp=(WORD*)lrect.pBits;
			if (lrect.Pitch!=width*2) MessageBox(NULL,"Error in SYSMEM (P)","Storm3D Error",0);
			int siz=width*height;
			BYTE *sb=(BYTE*)sysbuffer;
			int ip=0;
			for (int i=0;i<siz;i++)
			{
				// ip+0 = alphachannel (ARGB)
				WORD pix=*dp++;
				//((sb[ip+2]>>3)<<10)+((sb[ip+1]>>3)<<5)+(sb[ip+0]>>3);
				sb[ip+3]=((pix>>15)&0x01)*255;
				sb[ip+2]=((pix>>10)&0x1f)<<3;
				sb[ip+1]=((pix>>5)&0x3f)<<2;
				sb[ip+0]=((pix)&0x1f)<<3;
				ip+=4;
			}
		}
		else 
		{
			char s[80];
			sprintf(s,"Illegal videomode format (%d)",sdesc.Format);
			MessageBox(NULL,s,"Storm3D Error",0);	
		}

		// Unlock buffer
		dx_tempbuf_sm->UnlockRect();
	
	} 
	else MessageBox(NULL,"Cannot lock visbuffer","Storm3D Error",0);

	// Set systembuffer pointer back to NULL if not rendertarget (otherwise engine tries to delete buffer twice)
	if (!IsRenderTarget())
	{
		// Free temporary systembuffer handle (if not rendertarget)
		SAFE_RELEASE(dx_tempbuf_sm); 
	}
}



//------------------------------------------------------------------
// Storm3D_Texture - Reset routines
//------------------------------------------------------------------
void Storm3D_Texture::ReleaseDynamicDXBuffers()
{
	if ((textype==TEXTYPE_BASIC_RENDER)||(textype==TEXTYPE_BASIC_RPOOL)) 
		SAFE_RELEASE(dx_handle);
	if (textype==TEXTYPE_CUBE_RENDER) 
		SAFE_RELEASE(dx_handle_cube);
	if(textype == TEXTYPE_DYNAMIC)
		SAFE_RELEASE(dx_handle);
	if(textype == TEXTYPE_DYNAMIC_LOCKABLE)
		SAFE_RELEASE(dx_handle);
	if(textype == TEXTYPE_DYNAMIC_LOCKABLE_32)
		SAFE_RELEASE(dx_handle);
}


void Storm3D_Texture::ReCreateDynamicDXBuffers()
{
	if (textype==TEXTYPE_BASIC_RPOOL)
	{
		// Create (empty) texture (without mipmaps)
		/*HRESULT hr=*/D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
			0,Storm3D2->present_params.BackBufferFormat,D3DPOOL_DEFAULT,&dx_handle);
	
		//if (FAILED(hr)) MessageBox(NULL,"Failed to create dynamic texture","Storm3D Error",0);
	}
	else
	if (textype==TEXTYPE_BASIC_RENDER)
	{
		//HRESULT hr=D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
		//	D3DUSAGE_RENDERTARGET,D3DFMT_A4R4G4B4,D3DPOOL_DEFAULT,&dx_handle);
		//HRESULT hr=D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
		//	D3DUSAGE_RENDERTARGET,D3DFMT_A1R5G5B5,D3DPOOL_DEFAULT,&dx_handle);
		HRESULT hr=D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
			D3DUSAGE_RENDERTARGET,D3DFMT_R5G6B5,D3DPOOL_DEFAULT,&dx_handle);

		if(FAILED(hr))
		{
			/*HRESULT hr=*/D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
				D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,&dx_handle);
		}

		// Create (empty) texture (without mipmaps)
		//HRESULT hr=D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
		//	D3DUSAGE_RENDERTARGET,Storm3D2->present_params.BackBufferFormat,D3DPOOL_DEFAULT,&dx_handle);

		/*
		HRESULT hr=D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
			D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&dx_handle);

		if(FAILED(hr))
		{
			HRESULT hr=D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
				D3DUSAGE_RENDERTARGET,D3DFMT_A4R4G4B4,D3DPOOL_DEFAULT,&dx_handle);
		}

		if(FAILED(hr))
		{
			hr=D3DXCreateTexture(Storm3D2->D3DDevice,width,height,1,
				D3DUSAGE_RENDERTARGET,Storm3D2->present_params.BackBufferFormat,D3DPOOL_DEFAULT,&dx_handle);
		}
		*/
		

		//if (FAILED(hr)) MessageBox(NULL,"Failed to create dynamic texture","Storm3D Error",0);
	}
	else
	if (textype==TEXTYPE_CUBE_RENDER)
	{
		// Create (empty) cubetexture (without mipmaps)
		/*HRESULT hr=*/D3DXCreateCubeTexture(Storm3D2->D3DDevice,width,1,
			D3DUSAGE_RENDERTARGET,Storm3D2->present_params.BackBufferFormat,D3DPOOL_DEFAULT,&dx_handle_cube);

		//if (FAILED(hr)) MessageBox(NULL,"Failed to create dynamic cube texture","Storm3D Error",0);
	}
	else
	if(textype == TEXTYPE_DYNAMIC)
	{
		/*HRESULT hr=*/Storm3D2->D3DDevice->CreateTexture(width,height,1,
			0,Storm3D2->present_params.BackBufferFormat,D3DPOOL_DEFAULT,&dx_handle, 0);
	}
	else
	if(textype == TEXTYPE_DYNAMIC_LOCKABLE)
	{
		//HRESULT hr=Storm3D2->D3DDevice->CreateTexture(width,height,1,
		//	D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&dx_handle, 0);
		/*HRESULT hr=*/Storm3D2->D3DDevice->CreateTexture(width,height,1,
			D3DUSAGE_DYNAMIC,D3DFMT_R5G6B5,D3DPOOL_DEFAULT,&dx_handle, 0);
	}
	else
	if(textype == TEXTYPE_DYNAMIC_LOCKABLE_32)
	{
		/*HRESULT hr=*/Storm3D2->D3DDevice->CreateTexture(width,height,1,
			D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&dx_handle, 0);
	}
}


//------------------------------------------------------------------
// Storm3D_Texture - Get parameters
//------------------------------------------------------------------
const char *Storm3D_Texture::GetFilename()
{
	return filename;
}


DWORD Storm3D_Texture::GetTexIdentity()
{
	return tex_identity;
}


//------------------------------------------------------------------
// Storm3D_Texture::GetSurfaceInfo
//------------------------------------------------------------------
Storm3D_SurfaceInfo Storm3D_Texture::GetSurfaceInfo()
{
	return Storm3D_SurfaceInfo(width,height,0);
}

void Storm3D_Texture::swapTexture(IStorm3D_Texture *otherI)
{
	Storm3D_Texture *other = static_cast<Storm3D_Texture *> (otherI);
	if(!other)
	{
		assert(!"Whoops");
		return;
	}

	/*
	if(other->getId() != classId())
	{
		assert(!"Whoops");
		return;
	}
	*/

	std::swap(dx_handle, other->dx_handle);
	std::swap(width, other->width);
	std::swap(height, other->height);
}

bool Storm3D_Texture::lock(D3DLOCKED_RECT &rect)
{
	if(textype != TEXTYPE_DYNAMIC_LOCKABLE && textype != TEXTYPE_DYNAMIC_LOCKABLE_32)
		return false;

	//if(SUCCEEDED(dx_handle->LockRect(0, &rect, 0, D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK)))
	if(SUCCEEDED(dx_handle->LockRect(0, &rect, 0, D3DLOCK_NOSYSLOCK)))
		return true;

	return false;
}

void Storm3D_Texture::unlock()
{
	dx_handle->UnlockRect(0);
}


void Storm3D_Texture::saveToFile( const char * filename ) 
{
	D3DXSaveTextureToFile( filename, D3DXIFF_PNG, dx_handle, NULL);
}

void Storm3D_Texture::loadFromFile( const char * filename )
{
	if(dx_handle)
	{
		dx_handle->Release();
	}
	dx_handle = NULL;
	D3DXCreateTextureFromFile( Storm3D2->GetD3DDevice(), filename, &dx_handle );
}



