
#ifndef IPOINTABLEOBJECT_H
#define IPOINTABLEOBJECT_H

namespace ui
{
  class IPointableObject
  {
  public:
		virtual ~IPointableObject() {}
		virtual const VC3 &getPointerPosition() const = 0;
		virtual const VC3 getPointerMiddleOffset() const = 0;
  };
}

#endif

