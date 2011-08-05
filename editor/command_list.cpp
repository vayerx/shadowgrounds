// Copyright 2002-2004 Frozenbyte Ltd.

#pragma warning(disable:4103)
#pragma warning(disable:4786)

#include "command_list.h"
#include "icommand.h"
#include <map>
#include <cassert>

namespace frozenbyte {
namespace editor {

struct CommandListData
{
	std::map<int, ICommand *> commands;

	CommandListData()
	{
	}

	~CommandListData()
	{
	}
};

CommandList::CommandList()
{
	boost::shared_ptr<CommandListData> tempData(new CommandListData());
	data.swap(tempData);
}

CommandList::~CommandList()
{
}

void CommandList::addCommand(int id, ICommand *command)
{
	assert(id);
	assert(command);

	data->commands[id] = command;
}

void CommandList::execute(int id)
{
	std::map<int, ICommand *>::iterator it = data->commands.find(id);
	if(it != data->commands.end())
		(*it).second->execute(id);
}

} // end of namespace editor
} // end of namespace frozenbyte
