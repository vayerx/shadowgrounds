// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_EDITOR_ICOMMAND_H
#define INCLUDED_EDITOR_ICOMMAND_H

namespace frozenbyte {
namespace editor {

class ICommand
{
public:
	virtual ~ICommand() {}
	virtual void execute(int id) = 0;
};

} // end of namespace editor
} // end of namespace frozenbyte

#endif
