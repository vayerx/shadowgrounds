
#ifndef IPOINTABLEOBJECT_H
#define IPOINTABLEOBJECT_H

namespace ui
{
  class IPointableObject
  {
  public:
		virtual ~IPointableObject() {}
		virtual const VC3 &getPointerPosition() = 0;
		virtual const VC3 getPointerMiddleOffset() = 0;
  };
}

#endif

