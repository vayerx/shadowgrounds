#ifndef IGIOS3D_H
#define IGIOS3D_H

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glew.h>

struct VertexBuffer{
	VertexBuffer()
		:	buffer(NULL),
			refs(1)
	{
	}

	~VertexBuffer()
	{
		if(buffer)
			delete[] buffer;
	}

	void AddRef()
	{
		refs++;
	}

	void Release()
	{
		refs--;
		if(buffer && refs <= 0)
			delete[] buffer;
	}

	GLfloat *buffer;
	int refs;
};

struct IndexBuffer{
	IndexBuffer()
		:	buffer(NULL),
			refs(1)
	{
	}

	~IndexBuffer()
	{
		if(buffer)
			delete[] buffer;
	}

	void AddRef()
	{
		refs++;
	}

	void Release()
	{
		refs--;
		if(buffer && refs <= 0)
			delete[] buffer;
	}

	GLuint *buffer;
	int refs;
};

#endif // IGIOS3D_H
