#include <GL/glew.h>
#include <SDL.h>
#include "VertexFormats.h"
#include "igios.h"
#include "igios3D.h"
#include "storm3d.h"


//! Apply FVF vertex format
/*! buffer must be bound before this
 \param fmt vertex format descriptor
 \param size byte offset between consecutive vertices
 \param base base pointer (or null if vbo)
 */

uintptr_t applyFVF(VxFormat fmt, uintptr_t size) {
	std::vector<Element> fvfVertexDecl;

	uintptr_t ptr = 0;

	if (fmt & D3DFVF_XYZ) {
		fvfVertexDecl.push_back(Element(0, ptr, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION));
		ptr += 3 * sizeof(GLfloat);
		fmt &= (D3DFVF_XYZ ^ -1);
	} else if (fmt & D3DFVF_XYZRHW) {  // already transformed vertices
		fvfVertexDecl.push_back(Element(0, ptr, D3DDECLTYPE_FLOAT4, D3DDECLUSAGE_POSITION));
		ptr += 4 * sizeof(GLfloat);
		fmt &= (D3DFVF_XYZRHW ^ -1);
	} else {
		igiosWarning("strange: neither D3DFVF_XYZ nor D3DFVF_XYZRHW\n");
	}

	if (fmt & D3DFVF_NORMAL) {
		fvfVertexDecl.push_back(Element(0, ptr, D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_NORMAL));
		ptr += 3 * sizeof(GLfloat);
		fmt &= (D3DFVF_NORMAL ^ -1);
	}

	if (fmt & D3DFVF_DIFFUSE) {
		fvfVertexDecl.push_back(Element(0, ptr, D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR));
		ptr += 4 * sizeof(GLubyte);
		fmt &= (D3DFVF_DIFFUSE ^ -1);
	}

	int i;
	int numtexcoords = ((fmt & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT);
	for (i = 0; i < numtexcoords; i++) {
		fvfVertexDecl.push_back(Element(0, ptr, D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD, i));
		ptr += 2 * sizeof(GLfloat);
	}

	fmt &= (D3DFVF_TEXCOUNT_MASK ^ -1);
	if (fmt) {
		igiosWarning("strange: fmt=%08x\n", fmt);
		igios_backtrace();
	}

	if (ptr != size) {
#ifdef __x86_64
		igiosWarning("applyFVF: ptr(%ld) != size(%ld)\n", ptr, size);
#else
		igiosWarning("applyFVF: ptr(%d) != size(%d)\n", ptr, size);
#endif
		igios_backtrace();
	}

	setVertexDeclaration(fvfVertexDecl);

	return ptr;
}


//! Render something
/*!
 \param fmt
 \param type
 \param count
 \param vx
 */
void renderUP(VxFormat fmt, GLenum type, int count, int size, char *vx) {
	int vxCount = 0;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	switch (type) {
	case GL_TRIANGLE_STRIP:
		vxCount = count + 2;
		break;
	case GL_QUADS:
		vxCount = count * 4;
		break;
	case GL_TRIANGLES:
		vxCount = count * 3;
		break;
	default:
		igios_unimplemented();
		break;
	}

	intptr_t ptr = applyFVF(fmt, size);
	setStreamSource(0, 0, (intptr_t) vx, size);

	if (ptr != size) {
		igiosWarning("strange: ptr(%d) != size(%d)\n", ptr, size);
		igios_backtrace();
	}

	if (fmt & D3DFVF_XYZRHW) {  // already transformed vertices
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		SDL_Surface *screen = SDL_GetVideoSurface();
		glOrtho(0, screen->w, screen->h, 0, -1, 1);
	}

	glDrawArrays(type, 0, vxCount);
}


void setTextureMatrix(int stage, const D3DMATRIX &mat_) {
	glActiveTexture(GL_TEXTURE0 + stage);
	glMatrixMode(GL_TEXTURE);
	glLoadTransposeMatrixf(mat_.raw);
}


Element::Element(int stream_, int offset_, d3dtype type_, d3dusage usage_, int index_)
: stream(stream_),
  offset(offset_),
  index(index_),
  type(type_),
  usage(usage_)
{
}


static std::vector<Element> vertexDecl;

/* Set vertex declaration
 Enable specific arrays
 FIXME: make more efficient
 FIXME: more than 4 textures?
 */
void setVertexDeclaration(const std::vector<Element> &elements) {
	vertexDecl = elements;
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	for (int i = 0; i < 8; i++) {
		glClientActiveTexture(GL_TEXTURE0 + i);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	for (unsigned int i = 0; i < vertexDecl.size(); i++) {
		switch (vertexDecl[i].usage) {
		case D3DDECLUSAGE_POSITION:
			glEnableClientState(GL_VERTEX_ARRAY);
			break;
		case D3DDECLUSAGE_TEXCOORD:
			glClientActiveTexture(GL_TEXTURE0 + vertexDecl[i].index);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			break;
		case D3DDECLUSAGE_NORMAL:
			glEnableClientState(GL_NORMAL_ARRAY);
			break;
		case D3DDECLUSAGE_COLOR:
			glEnableClientState(GL_COLOR_ARRAY);
			break;
		}
	}
}

static void d3dTypeSize(d3dtype type, int &siz, GLenum &gltype) {
	switch (type) {
	case D3DDECLTYPE_FLOAT2:
		siz = 2;
		gltype = GL_FLOAT;
		break;
	case D3DDECLTYPE_FLOAT3:
		siz = 3;
		gltype = GL_FLOAT;
		break;
	case D3DDECLTYPE_FLOAT4:
		siz = 4;
		gltype = GL_FLOAT;
		break;
	case D3DDECLTYPE_D3DCOLOR:
		siz = 4;
		gltype = GL_UNSIGNED_BYTE;
		break;
	}
}

void setStreamSource(int stream_, GLuint vbo, int offs, int stride) {
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	for (unsigned int i = 0; i < vertexDecl.size(); i++) {
		if (vertexDecl[i].stream == stream_) {
			int siz = 3;
			GLenum typ = GL_FLOAT;
			GLvoid *offs_ = (GLvoid *) (offs + vertexDecl[i].offset);
			d3dTypeSize(vertexDecl[i].type, siz, typ);
			switch (vertexDecl[i].usage) {
			case D3DDECLUSAGE_POSITION:
				glVertexPointer(siz, typ, stride, offs_);
				break;
			case D3DDECLUSAGE_TEXCOORD:
				glClientActiveTexture(GL_TEXTURE0 + vertexDecl[i].index);
				glTexCoordPointer(siz, typ, stride, offs_);
				break;
			case D3DDECLUSAGE_NORMAL:
				glNormalPointer(typ, stride, offs_);
				break;
			case D3DDECLUSAGE_COLOR:
				glColorPointer(siz, typ, stride, offs_);
				break;
			}
		}
	}
}

void D3DXMatrixLookAtLH(D3DXMATRIX &out, const VC3 &eye, const VC3 &at, const VC3 &up) {
	VC3 zaxis = at - eye;
	zaxis.Normalize();

	VC3 xaxis = up.GetCrossWith(zaxis);
	xaxis.Normalize();
	VC3 yaxis = zaxis.GetCrossWith(xaxis);

	out._11 = xaxis.x;
	out._12 = yaxis.x;
	out._13 = zaxis.x;
	out._14 = 0;

	out._21 = xaxis.y;
	out._22 = yaxis.y;
	out._23 = zaxis.y;
	out._24 = 0;

	out._31 = xaxis.z;
	out._32 = yaxis.z;
	out._33 = zaxis.z;
	out._34 = 0;

	out._41 = -xaxis.GetDotWith(eye);
	out._42 = -yaxis.GetDotWith(eye);
	out._43 = -zaxis.GetDotWith(eye);
	out._44 = 1;
}

void D3DXMatrixPerspectiveFovLH(D3DXMATRIX &out, float fov, float aspect, float zNear, float zFar) {
	float yscale = 1.0f / (tan(fov/2));
	float xscale = yscale / aspect;

	out._11 = xscale;
	out._12 = 0;
	out._13 = 0;
	out._14 = 0;

	out._21 = 0;
	out._22 = -yscale;
	out._23 = 0;
	out._24 = 0;

	out._31 = 0;
	out._32 = 0;
	out._33 = zFar / (zFar - zNear);
	out._34 = 1;

	out._41 = 0;
	out._42 = 0;
	out._43 = -zNear * zFar / (zFar - zNear);
	out._44 = 0;
}

void D3DXMatrixInverse(D3DXMATRIX &out, float *_det, const D3DXMATRIX &mat) {
	D3DXMATRIX temp;

	float fA0 = mat._11 * mat._22 - mat._12 * mat._21;
    float fA1 = mat._11 * mat._23 - mat._13 * mat._21;
    float fA2 = mat._11 * mat._24 - mat._14 * mat._21;
    float fA3 = mat._12 * mat._23 - mat._13 * mat._22;
    float fA4 = mat._12 * mat._24 - mat._14 * mat._22;
    float fA5 = mat._13 * mat._24 - mat._14 * mat._23;
    float fB0 = mat._31 * mat._42 - mat._32 * mat._41;
    float fB1 = mat._31 * mat._43 - mat._33 * mat._41;
    float fB2 = mat._31 * mat._44 - mat._34 * mat._41;
    float fB3 = mat._32 * mat._43 - mat._33 * mat._42;
    float fB4 = mat._32 * mat._44 - mat._34 * mat._42;
    float fB5 = mat._33 * mat._44 - mat._34 * mat._43;

    float fDet = fA0*fB5-fA1*fB4+fA2*fB3+fA3*fB2-fA4*fB1+fA5*fB0;
	float fInvDet = ((float)1.0)/fDet;

    temp._11 = (+ mat._22*fB5 - mat._23*fB4 + mat._24*fB3) * fInvDet;
    temp._21 = (- mat._21*fB5 + mat._23*fB2 - mat._24*fB1) * fInvDet;
    temp._31 = (+ mat._21*fB4 - mat._22*fB2 + mat._24*fB0) * fInvDet;
    temp._41 = (- mat._21*fB3 + mat._22*fB1 - mat._23*fB0) * fInvDet;
    temp._12 = (- mat._12*fB5 + mat._13*fB4 - mat._14*fB3) * fInvDet;
    temp._22 = (+ mat._11*fB5 - mat._13*fB2 + mat._14*fB1) * fInvDet;
    temp._32 = (- mat._11*fB4 + mat._12*fB2 - mat._14*fB0) * fInvDet;
    temp._42 = (+ mat._11*fB3 - mat._12*fB1 + mat._13*fB0) * fInvDet;
    temp._13 = (+ mat._42*fA5 - mat._43*fA4 + mat._44*fA3) * fInvDet;
    temp._23 = (- mat._41*fA5 + mat._43*fA2 - mat._44*fA1) * fInvDet;
    temp._33 = (+ mat._41*fA4 - mat._42*fA2 + mat._44*fA0) * fInvDet;
    temp._43 = (- mat._41*fA3 + mat._42*fA1 - mat._43*fA0) * fInvDet;
    temp._14 = (- mat._32*fA5 + mat._33*fA4 - mat._34*fA3) * fInvDet;
    temp._24 = (+ mat._31*fA5 - mat._33*fA2 + mat._34*fA1) * fInvDet;
    temp._34 = (- mat._31*fA4 + mat._32*fA2 - mat._34*fA0) * fInvDet;
	temp._44 = (+ mat._31*fA3 - mat._32*fA1 + mat._33*fA0) * fInvDet;

	out = temp;
}

void D3DXMatrixTranspose(D3DXMATRIX &out, const D3DXMATRIX &mat) {
	D3DXMATRIX temp;
	temp._11 = mat._11;
	temp._12 = mat._21;
	temp._13 = mat._31;
	temp._14 = mat._41;

	temp._21 = mat._12;
	temp._22 = mat._22;
	temp._23 = mat._32;
	temp._24 = mat._42;

	temp._31 = mat._13;
	temp._32 = mat._23;
	temp._33 = mat._33;
	temp._34 = mat._43;

	temp._41 = mat._14;
	temp._42 = mat._24;
	temp._43 = mat._34;
	temp._44 = mat._44;
	out = temp;
}

void D3DXMatrixMultiply(D3DXMATRIX &out, const D3DXMATRIX &m1, const D3DXMATRIX &m2) {
	D3DXMATRIX temp = m1 * m2;

	out = temp;
}

float D3DXMatrixDeterminant(const D3DXMATRIX &mat) {
	float fA0 = mat._11 * mat._22 - mat._12 * mat._21;
    float fA1 = mat._11 * mat._23 - mat._13 * mat._21;
    float fA2 = mat._11 * mat._24 - mat._14 * mat._21;
    float fA3 = mat._12 * mat._23 - mat._13 * mat._22;
    float fA4 = mat._12 * mat._24 - mat._14 * mat._22;
    float fA5 = mat._13 * mat._24 - mat._14 * mat._23;
    float fB0 = mat._31 * mat._42 - mat._32 * mat._41;
    float fB1 = mat._31 * mat._43 - mat._33 * mat._41;
    float fB2 = mat._31 * mat._44 - mat._34 * mat._41;
    float fB3 = mat._32 * mat._43 - mat._33 * mat._42;
    float fB4 = mat._32 * mat._44 - mat._34 * mat._42;
    float fB5 = mat._33 * mat._44 - mat._34 * mat._43;

    return fA0*fB5-fA1*fB4+fA2*fB3+fA3*fB2-fA4*fB1+fA5*fB0;
}

void D3DXMatrixRotationY(D3DXMATRIX &out, float angle) {
	out._11 = cos(angle);
	out._12 = 0;
	out._13 = sin(angle);
	out._14 = 0;

	out._21 = 0;
	out._22 = 1;
	out._23 = 0;
	out._24 = 0;

	out._31 = -sin(angle);
	out._32 = 0;
	out._33 = cos(angle);
	out._34 = 0;

	out._41 = 0;
	out._42 = 0;
	out._43 = 0;
	out._44 = 1;
}

void D3DXMatrixPerspectiveOffCenterLH(D3DXMATRIX &out, float l, float r, float b, float t, float zn, float zf) {
	out._11 = 2 * zn / (r - 1.0f);
	out._12 = 0;
	out._13 = 0;
	out._14 = 0;

	out._21 = 0;
	out._22 = - 2 * zn / (t - b);
	out._23 = 0;
	out._24 = 0;

	out._31 = (1 + r) / (1 - r);
	out._32 = (t + b) / (b - t);
	out._33 = zf / (zf - zn);
	out._34 = 1;

	out._41 = 0;
	out._42 = 0;
	out._43 = zn * zf / (zn - zf);
	out._44 = 0;
}

void D3DXMatrixOrthoOffCenterLH(D3DXMATRIX &out, float l, float r, float b, float t, float zn, float zf) {
	out._11 = 2 / (r - l);
	out._12 = 0;
	out._13 = 0;
	out._14 = 0;

	out._21 = 0;
	out._22 = 2 / (t - b);
	out._23 = 0;
	out._24 = 0;

	out._31 = 0;
	out._32 = 0;
	out._33 = 1.0f / (zf - zn);
	out._34 = 0;

	out._41 = (l + r)/(l - r);
	out._42 = (t + b) / (b - t);
	out._43 = zn / (zn - zf);
	out._44 = l;
}

void D3DXMatrixAffineTransformation(D3DXMATRIX &out, float scale, const VC3 &rotCenter, const QUAT &rot, const VC3 &trans) {
	D3DXMATRIX s;
	s._11 = scale;
	s._22 = scale;
	s._33 = scale;

	D3DXMATRIX rc;
	rc._41 = rotCenter.x;
	rc._42 = rotCenter.y;
	rc._43 = rotCenter.z;

	D3DXMATRIX r;
	float xx = rot.x * rot.x;
	float xy = rot.x * rot.y;
	float xz = rot.x * rot.z;
	float xw = rot.x * rot.w;

	float yy = rot.y * rot.y;
	float yz = rot.y * rot.z;
	float yw = rot.y * rot.w;

	float zz = rot.z * rot.z;
	float zw = rot.z * rot.w;

	r._11 = 1 - 2 * (yy + zz);
	r._12 = 2 * (xy - zw);
	r._13 = 2 * (xz + yw);

	r._21 = 2 * (xy + zw);
	r._22 = 1 - 2 * (xx + zz);
	r._23 = 2 * (yz - xw);

	r._31 = 2 * (xz - yw);
	r._32 = 2 * (yz + xw);
	r._33 = 1 - 2 * (xx + yy);

	D3DXMATRIX t;
	t._41 = trans.x;
	t._42 = trans.y;
	t._43 = trans.z;

	D3DXMATRIX rci;
	D3DXMatrixInverse(rci, NULL, rc);

	out = s * rci * r * rc * t;
}

void D3DXMatrixIdentity(D3DXMATRIX &out) {
	out._11 = 1;
	out._12 = 0;
	out._13 = 0;
	out._14 = 0;

	out._21 = 0;
	out._22 = 1;
	out._23 = 0;
	out._24 = 0;

	out._31 = 0;
	out._32 = 0;
	out._33 = 1;
	out._34 = 0;

	out._41 = 0;
	out._42 = 0;
	out._43 = 0;
	out._44 = 1;
}

void D3DXVec3Transform(VC4 &out, const VC3 &in, const D3DXMATRIX &mat) {
	VC4 temp;
	temp.x = mat._11 * in.x + mat._12 * in.y + mat._13 * in.z + mat._14 * 1.0f;
	temp.y = mat._21 * in.x + mat._22 * in.y + mat._23 * in.z + mat._24 * 1.0f;
	temp.z = mat._31 * in.x + mat._32 * in.y + mat._33 * in.z + mat._34 * 1.0f;
	temp.w = mat._41 * in.x + mat._42 * in.y + mat._43 * in.z + mat._44 * 1.0f;
	out = temp;
}

void D3DXVec3TransformCoord(VC3 &out, const VC3 &in, const D3DXMATRIX &mat) {
	VC4 temp;
	temp.x = mat._11 * in.x + mat._12 * in.y + mat._13 * in.z + mat._14 * 1.0f;
	temp.y = mat._21 * in.x + mat._22 * in.y + mat._23 * in.z + mat._24 * 1.0f;
	temp.z = mat._31 * in.x + mat._32 * in.y + mat._33 * in.z + mat._34 * 1.0f;
	temp.w = mat._41 * in.x + mat._42 * in.y + mat._43 * in.z + mat._44 * 1.0f;
	out.x = temp.x / temp.w;
	out.y = temp.y / temp.w;
	out.z = temp.z / temp.w;
}

void D3DXVec3TransformNormal(VC3 &out, const VC3 &in, const D3DXMATRIX &mat) {
	VC3 temp;
	temp.x = mat._11 * in.x + mat._21 * in.y + mat._31 * in.z;
	temp.y = mat._12 * in.x + mat._22 * in.y + mat._32 * in.z;
	temp.z = mat._13 * in.x + mat._23 * in.y + mat._33 * in.z;
	out = temp;
}

void D3DXVec4Transform(VC4 &out, const VC4 &in, const D3DXMATRIX &mat) {
	VC4 temp;
	temp.x = mat._11 * in.x + mat._12 * in.y + mat._13 * in.z + mat._14 * in.w;
	temp.y = mat._21 * in.x + mat._22 * in.y + mat._23 * in.z + mat._24 * in.w;
	temp.z = mat._31 * in.x + mat._32 * in.y + mat._33 * in.z + mat._34 * in.w;
	temp.w = mat._41 * in.x + mat._42 * in.y + mat._43 * in.z + mat._44 * in.w;
	out = temp;
}

void D3DXPlaneFromPointNormal(D3DXPLANE &out, const VC3 &point, const VC3 &normal) {
	// FIXME: normalize?
	out.a = normal.x;
	out.b = normal.y;
	out.c = normal.z;
	out.d = -normal.x * point.x - normal.y * point.y - normal.z * point.z;
}

void D3DXPlaneNormalize(D3DXPLANE &out, const D3DXPLANE &in) {
	// Not actually used
	igios_unimplemented();
}

void D3DXPlaneTransform(D3DXPLANE &out, const D3DXPLANE &in, const D3DXMATRIX &mat) {
	// Not actually used
	/*
	out.a = mat._11 * in.a + mat._12 * in.b + mat._13 * in.c;
	out.b = mat._21 * in.a + mat._22 * in.b + mat._23 * in.c;
	out.c = mat._31 * in.a + mat._32 * in.b + mat._33 * in.c;
	out.d = in.d;
	*/
	igios_unimplemented();
}

void D3DXPlaneFromPoints(D3DXPLANE &out, const VC3 &p1, const VC3 &p2, const VC3 &p3) {
	// Not actually used
	igios_unimplemented();
}


void dumpD3DXMatrix(const D3DXMATRIX &mat) {
	for (unsigned int i = 0; i < 4; i++) {
        igiosWarning("%f\t%f\t%f\t%f\n", mat.m[i][0], mat.m[i][1], mat.m[i][2], mat.m[i][3]);
	}
}


Framebuffer *Framebuffer::activeFBO = NULL;

Framebuffer::Framebuffer() :
renderbuffers(),
fbo(0)
{
	glGenFramebuffersEXT(1, &fbo);

	glErrors();
}


Framebuffer::~Framebuffer() {
	if (activeFBO == this) {
		disable();
	}
	glDeleteFramebuffersEXT(1, &fbo);
	fbo = 0;

	for (vector<struct renderBuffer>::iterator it = renderbuffers.begin(); it < renderbuffers.end(); it++) {
		glDeleteRenderbuffersEXT(1, &(it->rbo));
		it->rbo = 0;
	}

}

//! return to normal fb
void Framebuffer::disable() {
	if (activeFBO == this) {
		// for unknown reasons this blow up on some ati cards
		// shitty drivers?
		/*
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, 0, 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);
		*/
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		activeFBO = NULL;
	}
}


//! activate with last bound textures/renderbuffers
void Framebuffer::activate() {
	if (activeFBO != this) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		glErrors();
		activeFBO = this;
	}
}


void Framebuffer::setRenderTarget(GLuint tex, GLuint width, GLuint height, GLenum textarget) {
	glErrors();

	if (activeFBO != this) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		glErrors();
		activeFBO = this;
	}

	GLuint depthstencil = 0;
	// see if we have right-sized depthstencil
	for (vector<struct renderBuffer>::iterator it = renderbuffers.begin(); it < renderbuffers.end(); it++) {
		if (it->w == width && it->h == height) {
			depthstencil = it->rbo;
            break;
		}
	}

    // no match?
	if (depthstencil == 0) {
		igiosWarning("create new depthstencil buffer size %dx%d\n", width, height);

		// create one
		struct renderBuffer rbo;
		glGenRenderbuffersEXT(1, &rbo.rbo);
		depthstencil = rbo.rbo;
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthstencil);
		rbo.w = width; rbo.h = height;

		// Set depthstencil size equal to the texture size
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_STENCIL_EXT, width, height);
		renderbuffers.push_back(rbo);
	}

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textarget, tex, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthstencil);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthstencil);

	glErrors();
}

void Framebuffer::setRenderTarget(boost::shared_ptr<glTexWrapper> tex, GLenum textarget) {
	glErrors();

	if (tex->fmt != GL_RGBA8) {
		igiosWarning("setRenderTarget: tex->fmt(0x%x) != GL_RGBA8\n", tex->fmt);
		igios_backtrace();
	}
	tex->bind();
	GLint tempW, tempH, tempFmt;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &tempFmt);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tempW);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tempH);

	if (tempW != tex->getWidth() || tempH != tex->getHeight()) {
		igiosWarning("Framebuffer::setRenderTarget: texture size has changed! Should be (%dx%d), is (%dx%d)\n", tex->getWidth(), tex->getHeight(), tempW, tempH);
		igios_backtrace();
	}
	if ((GLenum) tempFmt != tex->getFmt()) {
		igiosWarning("Framebuffer::setRenderTarget: texture format has changed! Should be %x, is %x\n", tex->getFmt(), tempFmt);
		igios_backtrace();
	}
	glBindTexture(textarget, 0);

	setRenderTarget(tex->handle, tex->width, tex->height, textarget);

	glErrors();
}

void Framebuffer::setRenderTarget(boost::shared_ptr<glTexWrapper> tex, boost::shared_ptr<glTexWrapper> depthstencil, GLenum textarget) {
	glErrors();

	if (tex->fmt != GL_RGBA8) {
		igiosWarning("setRenderTarget: tex->fmt(0x%x) != GL_RGBA8\n", tex->fmt);
		igios_backtrace();
	}

	if (depthstencil->fmt != GL_DEPTH24_STENCIL8_EXT) {
		igiosWarning("setRenderTarget: depthstencil->fmt(0x%x) != GL_DEPTH24_STENCIL8_EXT\n", depthstencil->fmt);
		igios_backtrace();
	}

	if (tex->getWidth() != depthstencil->getWidth() || tex->getHeight() != depthstencil->getHeight()) {
		igiosWarning("setRenderTarget: tex and depthstencil size mismatch: %dx%dx vs %dx%d\n", tex->getWidth(), tex->getHeight(), depthstencil->getWidth(), depthstencil->getHeight());
		igios_backtrace();
	}

	tex->bind();
	GLint tempFmt, tempH, tempW;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &tempFmt);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tempW);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tempH);
	if (tempW != tex->getWidth() || tempH != tex->getHeight()) {
		igiosWarning("Framebuffer::setRenderTarget: texture size has changed! Should be (%dx%d), is (%dx%d)\n", tex->getWidth(), tex->getHeight(), tempW, tempH);
		igios_backtrace();
	}
	if ((GLenum) tempFmt != tex->getFmt()) {
		igiosWarning("Framebuffer::setRenderTarget: texture format has changed! Should be %x, is %x\n", tex->getFmt(), tempFmt);
		igios_backtrace();
	}

	depthstencil->bind();
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &tempFmt);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tempW);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tempH);
	if (tempW != depthstencil->getWidth() || tempH != depthstencil->getHeight()) {
		igiosWarning("Framebuffer::setRenderTarget: depthstencil size has changed! Should be (%dx%d), is (%dx%d)\n", depthstencil->getWidth(), depthstencil->getHeight(), tempW, tempH);
		igios_backtrace();
	}

	if ((GLenum) tempFmt != depthstencil->getFmt()) {
		igiosWarning("Framebuffer::setRenderTarget: texture format has changed! Should be %x, is %x\n", depthstencil->getFmt(), tempFmt);
		igios_backtrace();
	}
	glBindTexture(textarget, 0);

	if (activeFBO != this) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		glErrors();
		activeFBO = this;
	}
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textarget, tex->handle, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, textarget, depthstencil->handle, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, textarget, depthstencil->handle, 0);
}

bool Framebuffer::validate() {
	if (activeFBO == this) {
		GLenum ret = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if (ret != GL_FRAMEBUFFER_COMPLETE_EXT) {
			igiosWarning("framebuffer not complete at %d: %x\n", __LINE__, ret);
			*((char *) NULL) = '\0';
			igios_backtrace();

			return false;
		}
	}
	return true;
}

glTexWrapper::glTexWrapper(GLuint w, GLuint h) :
handle(0),
width(w),
height(h),
fmt(0)
{
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
}

glTexWrapper::~glTexWrapper() {
	glDeleteTextures(1, &handle);
	handle = 0; fmt = 0;
}

boost::shared_ptr<glTexWrapper> glTexWrapper::rgbaTexture(GLint w, GLint h) {
	glErrors();

	boost::shared_ptr<glTexWrapper> tex(new glTexWrapper(w, h));

	tex->fmt = GL_RGBA8;
	glTexImage2D(GL_TEXTURE_2D, 0, tex->fmt, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	GLint tempFmt, tempH, tempW;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &tempFmt);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tempH);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tempW);
	if (tex->fmt != (GLenum) tempFmt || tempH != h || tempW != w) {
		igiosWarning("glTexWrapper::rgbaTexture: wanted %x, got %x\n", tex->fmt, tempFmt);
		igiosWarning("attempted size: %dx%d\ntexture size: %dx%d\n", w, h, tempH, tempW);
		tex->fmt = tempFmt;
	}

	glErrors();
	return tex;
}

boost::shared_ptr<glTexWrapper> glTexWrapper::depthStencilTexture(GLint w, GLint h) {
	glErrors();

	boost::shared_ptr<glTexWrapper> tex(new glTexWrapper(w, h));

	tex->fmt = GL_DEPTH24_STENCIL8_EXT;
	glTexImage2D(GL_TEXTURE_2D, 0, tex->fmt, w, h, 0, GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	GLint tempFmt;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &tempFmt);
	if (tex->fmt != (GLenum) tempFmt) {
		igiosWarning("glTexWrapper::depthStencilTexture: wanted %x, got %x\n", tex->fmt, tempFmt);
		tex->fmt = tempFmt;
	}

	glErrors();
	return tex;
}

void glTexWrapper::bind() {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, handle);

	// FIXME: something wrong here...
	GLint tempW, tempH;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tempW);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &tempH);

	if (tempW != width || tempH != height) {
		igiosWarning("glTexWrapper::bind(): texture size has changed! Should be (%dx%d), is (%dx%d)\n", width, height, tempW, tempH);
		igios_backtrace();
	}

	GLint tempFmt;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &tempFmt);

	if ((GLenum) tempFmt != fmt) {
		igiosWarning("glTexWrapper::bind(): texture format has changed! Should be %x, is %x\n", fmt, tempFmt);
		igios_backtrace();
	}

}
