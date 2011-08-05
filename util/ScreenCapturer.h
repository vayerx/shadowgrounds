
#ifndef SCREENCAPTURER_H
#define SCREENCAPTURER_H

class IStorm3D;

namespace util
{
  
  class ScreenCapturer
  {
  public:
    static void captureScreen(IStorm3D *storm3d);
		// uses same filename as last call to captureScreen, nothing more to it :)
		static void captureScreenWithLastName(IStorm3D *storm3d, const char *extra_extension);
  };

}

#endif

