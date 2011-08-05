#ifndef IGIOS3D_H
#define IGIOS3D_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include "VertexFormats.h"
#include "c2_matrix.h"
#include "c2_aabb.h"
#include "igios.h"


// these DON'T match d3d definitions, they just have the same names
typedef enum {
	D3DDECLUSAGE_POSITION,
	D3DDECLUSAGE_TEXCOORD,
	D3DDECLUSAGE_NORMAL,
	D3DDECLUSAGE_COLOR
} d3dusage;

typedef enum {
	D3DDECLTYPE_FLOAT2,
	D3DDECLTYPE_FLOAT3,
	D3DDECLTYPE_FLOAT4,
	D3DDECLTYPE_D3DCOLOR
} d3dtype;


class Element
{
public:
	int stream, offset, index;
	d3dtype type;
	d3dusage usage;
	Element(int stream_, int offset_, d3dtype type_, d3dusage usage_, int index_ = 0);
};


class Storm3D;


class glTexWrapper {
private:
	GLuint handle;
	GLint width, height;
	GLenum fmt;

    // private constructor, use the factory methods
	glTexWrapper(GLuint w, GLuint h);

public:
	~glTexWrapper();

	static boost::shared_ptr<glTexWrapper> rgbaTexture(GLint w, GLint h);
	static boost::shared_ptr<glTexWrapper> depthStencilTexture(GLint w, GLint h);

	void bind();

	GLint getWidth() const { return width; }
	GLint getHeight() const { return height; }
	GLenum getFmt() const { return fmt; }

	friend class Framebuffer;
};


class Framebuffer
{
	struct renderBuffer {
		unsigned int w, h;
		GLuint rbo;
	};
	std::vector<struct renderBuffer> renderbuffers;
	GLuint fbo;

	static Framebuffer *activeFBO;
public:
	Framebuffer();
	~Framebuffer();

	void disable(); // return to original
	void activate();  // activate with last bound textures/renderbuffers

	void setRenderTarget(GLuint tex, GLuint width, GLuint height, GLenum textarget = GL_TEXTURE_2D);
	void setRenderTarget(boost::shared_ptr<glTexWrapper> tex, GLenum textarget = GL_TEXTURE_2D);
	void setRenderTarget(boost::shared_ptr<glTexWrapper> tex, boost::shared_ptr<glTexWrapper> depthstencil, GLenum textarget = GL_TEXTURE_2D);
	bool validate();
};


uintptr_t applyFVF(VxFormat fmt, uintptr_t size);
void renderUP(VxFormat fmt, GLenum type, int count, int size, char *vx);

void setVertexDeclaration(const std::vector<Element> &elements);
void setStreamSource(int stream_, GLuint vbo, int offs, int stride);

#ifndef M_PI
#define M_PI PI
#endif
#define D3DXToRadian(x) ((x) * (M_PI / 180.0))

struct D3DXPLANE {
	union {
		struct {
			double a, b, c, d;
		};
		double p[4];
	};

	operator const double* () const {
		return p;
	};
};


struct D3DMATRIX {
	union {
		struct {
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float m[4][4];
		float raw[16];
	};

	D3DMATRIX() :
		_11(1), _12(0), _13(0), _14(0),
		_21(0), _22(1), _23(0), _24(0),
		_31(0), _32(0), _33(1), _34(0),
		_41(0), _42(0), _43(0), _44(1)
	{};

	D3DMATRIX operator*(const D3DMATRIX &other) const {
		D3DMATRIX n;

		n._11 = _11 * other._11 + _12 * other._21 + _13 * other._31 + _14 * other._41;
		n._12 = _11 * other._12 + _12 * other._22 + _13 * other._32 + _14 * other._42;
		n._13 = _11 * other._13 + _12 * other._23 + _13 * other._33 + _14 * other._43;
		n._14 = _11 * other._14 + _12 * other._24 + _13 * other._34 + _14 * other._44;

		n._21 = _21 * other._11 + _22 * other._21 + _23 * other._31 + _24 * other._41;
		n._22 = _21 * other._12 + _22 * other._22 + _23 * other._32 + _24 * other._42;
		n._23 = _21 * other._13 + _22 * other._23 + _23 * other._33 + _24 * other._43;
		n._24 = _21 * other._14 + _22 * other._24 + _23 * other._34 + _24 * other._44;

		n._31 = _31 * other._11 + _32 * other._21 + _33 * other._31 + _34 * other._41;
		n._32 = _31 * other._12 + _32 * other._22 + _33 * other._32 + _34 * other._42;
		n._33 = _31 * other._13 + _32 * other._23 + _33 * other._33 + _34 * other._43;
		n._34 = _31 * other._14 + _32 * other._24 + _33 * other._34 + _34 * other._44;

		n._41 = _41 * other._11 + _42 * other._21 + _43 * other._31 + _44 * other._41;
		n._42 = _41 * other._12 + _42 * other._22 + _43 * other._32 + _44 * other._42;
		n._43 = _41 * other._13 + _42 * other._23 + _43 * other._33 + _44 * other._43;
		n._44 = _41 * other._14 + _42 * other._24 + _43 * other._34 + _44 * other._44;

		return n;
	}
};


struct D3DXMATRIX : public D3DMATRIX {

	D3DXMATRIX() {};

	D3DXMATRIX(float __11, float __12, float __13, float __14
			   ,float __21, float __22, float __23, float __24
			   ,float __31, float __32, float __33, float __34
			   ,float __41, float __42, float __43, float __44)
	{
		_11 = __11; _12 = __12; _13 = __13; _14 = __14;
		_21 = __21; _22 = __22; _23 = __23; _24 = __24;
		_31 = __31; _32 = __32; _33 = __33; _34 = __34;
		_41 = __41; _42 = __42; _43 = __43; _44 = __44;
	};

	D3DXMATRIX(const MAT &mat) {
        _11 = mat.Get(0); _12 = mat.Get(4); _13 = mat.Get(8);  _14 = mat.Get(12);
        _21 = mat.Get(1); _22 = mat.Get(5); _23 = mat.Get(9);  _24 = mat.Get(13);
        _31 = mat.Get(2); _32 = mat.Get(6); _33 = mat.Get(10); _34 = mat.Get(14);
        _41 = mat.Get(3); _42 = mat.Get(7); _43 = mat.Get(11); _44 = mat.Get(15);
	};

	float operator[](int i) const {
		int row = i / 4;
		int col = i % 4;
        return this->m[row][col];
	}

	float &operator()(int row, int col) {
		return m[row][col];
	}

	D3DXMATRIX operator*(const D3DXMATRIX &other) const {
		D3DXMATRIX n;

		n._11 = _11 * other._11 + _12 * other._21 + _13 * other._31 + _14 * other._41;
		n._12 = _11 * other._12 + _12 * other._22 + _13 * other._32 + _14 * other._42;
		n._13 = _11 * other._13 + _12 * other._23 + _13 * other._33 + _14 * other._43;
		n._14 = _11 * other._14 + _12 * other._24 + _13 * other._34 + _14 * other._44;

		n._21 = _21 * other._11 + _22 * other._21 + _23 * other._31 + _24 * other._41;
		n._22 = _21 * other._12 + _22 * other._22 + _23 * other._32 + _24 * other._42;
		n._23 = _21 * other._13 + _22 * other._23 + _23 * other._33 + _24 * other._43;
		n._24 = _21 * other._14 + _22 * other._24 + _23 * other._34 + _24 * other._44;

		n._31 = _31 * other._11 + _32 * other._21 + _33 * other._31 + _34 * other._41;
		n._32 = _31 * other._12 + _32 * other._22 + _33 * other._32 + _34 * other._42;
		n._33 = _31 * other._13 + _32 * other._23 + _33 * other._33 + _34 * other._43;
		n._34 = _31 * other._14 + _32 * other._24 + _33 * other._34 + _34 * other._44;

		n._41 = _41 * other._11 + _42 * other._21 + _43 * other._31 + _44 * other._41;
		n._42 = _41 * other._12 + _42 * other._22 + _43 * other._32 + _44 * other._42;
		n._43 = _41 * other._13 + _42 * other._23 + _43 * other._33 + _44 * other._43;
		n._44 = _41 * other._14 + _42 * other._24 + _43 * other._34 + _44 * other._44;

		return n;
	}
};

void D3DXMatrixLookAtLH(D3DXMATRIX &out, const VC3 &eye, const VC3 &at, const VC3 &up);
void D3DXMatrixPerspectiveFovLH(D3DXMATRIX &out, float fov, float aspect, float zNear, float zFar);
void D3DXMatrixInverse(D3DXMATRIX &out, float *det, const D3DXMATRIX &mat);
void D3DXMatrixTranspose(D3DXMATRIX &out, const D3DXMATRIX &mat);
void D3DXMatrixMultiply(D3DXMATRIX &out, const D3DXMATRIX &m1, const D3DXMATRIX &m2);
float D3DXMatrixDeterminant(const D3DXMATRIX &mat);
void D3DXMatrixRotationY(D3DXMATRIX &out, float angle);
void D3DXMatrixPerspectiveOffCenterLH(D3DXMATRIX &out, float l, float r, float b, float t, float zn, float zf);
void D3DXMatrixAffineTransformation(D3DXMATRIX &out, float scale, const VC3 &rotCenter, const QUAT &rot, const VC3 &trans);
void D3DXMatrixIdentity(D3DXMATRIX &out);
void D3DXMatrixOrthoOffCenterLH(D3DXMATRIX &out, float l, float r, float b, float t, float zn, float zf);

void D3DXVec3Transform(VC4 &out, const VC3 &in, const D3DXMATRIX &mat);
void D3DXVec3TransformCoord(VC3 &out, const VC3 &in, const D3DXMATRIX &mat);
void D3DXVec3TransformNormal(VC3 &out, const VC3 &in, const D3DXMATRIX &mat);
void D3DXVec4Transform(VC4 &out, const VC4 &in, const D3DXMATRIX &mat);

void D3DXPlaneNormalize(D3DXPLANE &out, const D3DXPLANE &in);
void D3DXPlaneTransform(D3DXPLANE &out, const D3DXPLANE &in, const D3DXMATRIX &mat);
void D3DXPlaneFromPoints(D3DXPLANE &out, const VC3 &p1, const VC3 &p2, const VC3 &p3);
void D3DXPlaneFromPointNormal(D3DXPLANE &out, const VC3 &point, const VC3 &normal);

void setTextureMatrix(int stage, const D3DMATRIX &mat_);

void dumpD3DXMatrix(const D3DXMATRIX &mat);


static inline void clamp(float &value) {
	float intval = floorf(value);
	if(fabsf(intval) > 1.f)
		value = fmodf(value, intval);
}

static inline bool contains2D(const AABB &area, const VC3 &position)
{
	if(position.x < area.mmin.x)
		return false;
	if(position.x > area.mmax.x)
		return false;

	if(position.z < area.mmin.z)
		return false;
	if(position.z > area.mmax.z)
		return false;

	return true;
}


#endif // IGIOS3D_H
