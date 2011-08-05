
#ifndef GAMESTATEVARIABLE_H
#define GAMESTATEVARIABLE_H




// TODO
// preliminary design



namespace game
{
	/**
	 * Script variable defining the game state. 
	 * Game state can be used to flag all sorts of game events.
	 * A successfull mission may for example set a specific gamestate
	 * variable and later some other mission may appear different 
	 * based on that gamestate. Gamestate variables should be preserved
	 * by save and load.
	 * 
   * @version 1.0, 13.3.2003
   * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see GameStateVariable
	 */
	class GameStateVariable : public IScriptVariable
	{
		public:
			GameStateVariable();

			~GameStateVariable();

			virtual IScriptVariable::VARTYPE getVariableType();

			virtual void setIntValue(int value);

			virtual void setBooleanValue(bool value);

			virtual int getIntValue();

			virtual bool getBooleanValue();

			virtual bool isReadOnly();

		private:
			

	};
}

#endif


