
#ifndef IGAMEPHYSICSSCRIPTRUNNER_H
#define IGAMEPHYSICSSCRIPTRUNNER_H

namespace game
{
	class IGamePhysicsScriptRunner
	{
	public:
		// returns true on success.
		virtual bool runGamePhysicsScript(const char *scriptname, const char *subname) = 0;

		virtual void *getGamePhysicsScriptRunnerImplementation() = 0;

		virtual ~IGamePhysicsScriptRunner() {};
	};
}

#endif

