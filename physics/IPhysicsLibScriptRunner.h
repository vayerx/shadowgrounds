
#ifndef IPHYSICSLIBSCRIPTRUNNER_H
#define IPHYSICSLIBSCRIPTRUNNER_H

namespace frozenbyte
{
namespace physics
{

	class IPhysicsLibScriptRunner
	{
	public:
		// returns true if successful
		virtual bool runPhysicsLibScript(const char *scriptname, const char *subname) = 0;

		virtual ~IPhysicsLibScriptRunner() {};
	};

}
}

#endif

