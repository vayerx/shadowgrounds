
#ifndef IOGUIIMAGE_H
#define IOGUIIMAGE_H

class IStorm3D_Texture;
class IStorm3D_Material;

class IOguiImage
{
public:
  virtual ~IOguiImage() {};

  virtual IStorm3D_Texture *getTexture() = 0;
	virtual IStorm3D_Material *getMaterial() = 0;
};

#endif

