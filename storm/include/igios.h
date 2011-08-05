#ifndef IGIOS_H
#define IGIOS_H

#ifdef WIN32
#include <windows.h>
#endif
#include <string>


#ifndef __GNUC__
// non-gcc compilers don't like attribute
#define __attribute__(x)
#endif


#ifdef FINAL_RELEASE_BUILD

#define igios_unimplemented()
#define igiosWarning(fmt, args...)

#else

#include <stdio.h>

#include <stdio.h>

#ifdef __GNUC__
#define igios_unimplemented() { static bool firsttime_ = true; if (firsttime_) { igios_unimplemented_(__PRETTY_FUNCTION__, __FILE__, __LINE__); firsttime_ = false; } }
#define igiosWarning(fmt, args...) fprintf(stderr, fmt, ## args)
#else
#define igios_unimplemented() { static bool firsttime_ = true; if (firsttime_) { igios_unimplemented_("Unknown function",  __FILE__, __LINE__); firsttime_ = false; } }
void igiosWarning(const char *fmt, ...);
#endif

#endif  //FINAL_RELEASE_BUILD


#ifdef __GLIBC__
void igios_backtrace(void) __attribute__((no_instrument_function, externally_visible, visibility("default")));
#else
static inline void igios_backtrace(void) {};
#endif


static inline void igios_unimplemented_(const char *function, const char *file, int line);
static inline void igios_unimplemented_(const char *function, const char *file, int line) {
	igiosWarning("unimplemented function %s at %s:%d\n", function, file, line);

#ifdef __GLIBC__
    igios_backtrace();
#endif

}

void igiosErrorMessage(const char *msg, ...) __attribute__ ((format (printf, 1, 2))) __attribute__((no_instrument_function));

void igios_setsighandler(void);

std::string igios_getFontDirectory();

#define RGBI(r,g,b)          ((unsigned long)(((unsigned char)(r)|((unsigned short)((unsigned char)(g))<<8))|(((unsigned long)(unsigned char)(b))<<16)))
#define DegToRadian(degree) ((degree) * (PI / 180.0f))

enum CULLMODE {
	CULL_NONE,
	CULL_CCW,
	CULL_CW
};

#define glErrors() igiosCheckGlErrors(__FILE__, __LINE__)

bool igiosCheckGlErrors(const char *file, int line) __attribute__((no_instrument_function));


// there really should be a better place to put this...
#include "c2_quat.h"
#include "c2_matrix.h"
template <typename A> Vec3<A> Quat<A>::getEulerAngles() const
{
	Quat<A> quat = this->GetInverse();
	TMatrix<A> tm;
	tm.CreateRotationMatrix(quat);

	float heading = 0.f;
	float attitude = 0.f;
	float bank = 0.f;

	float m00 = tm.Get(0);
	float m02 = tm.Get(2);
	float m10 = tm.Get(4);
	float m11 = tm.Get(5);
	float m12 = tm.Get(6);
	float m20 = tm.Get(8);
	float m22 = tm.Get(10);

	if(m10 > 0.998f)
	{
		heading = atan2f(m02, m22);
		attitude = PI/2.f;
		bank = 0.f;
	}
	else if(m10 < -0.998f)
	{
		heading = atan2f(m02, m22);
		attitude = -PI/2;
		bank = 0.f;
	}
	else
	{
		heading = atan2f(-m20, m00);
		attitude = asinf(m10);
		bank = atan2f(-m12, m11);
	}

	return Vec3<A>(bank, heading, attitude);
}


struct NullDeleter
{
	void operator() (void *)
	{
	}
};


#endif // IGIOS_H
