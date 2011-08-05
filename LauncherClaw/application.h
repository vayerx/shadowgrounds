#ifndef INC_APPLICATION_H
#define INC_APPLICATION_H

namespace frozenbyte {

namespace launcher {



class ApplicationImpl;

	
class Application
{
public:
	Application();
	~Application();

	void run();

private:

	ApplicationImpl* impl;

};


} // end of namespace launcher
} // end of namespace frozenbyte


#endif