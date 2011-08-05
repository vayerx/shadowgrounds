#include "string_dialog.h"
#include "dialog.h"
#include "dialog_utils.h"
#include "command_list.h"
#include "icommand.h"
#include "resource/resource.h"

namespace frozenbyte {
namespace editor {
namespace {

struct OkCommand: public ICommand
{
	Dialog &dialog;
	std::string &data;

	OkCommand(Dialog &dialog_, std::string &data_)
	:	dialog(dialog_),
		data(data_)
	{
	}

	void execute(int id)
	{
		data = getDialogItemText(dialog, IDC_STRING);
		dialog.hide();
	}
};

} // unnamed

struct StringDialog::Data
{
	std::string data;
};

StringDialog::StringDialog()
:	data(new Data())
{
}

StringDialog::~StringDialog()
{
}

std::string StringDialog::show(const std::string &title)
{
	Dialog dialog(IDD_STRING);
	OkCommand okCommand(dialog, data->data);

	dialog.getCommandList().addCommand(IDOK, &okCommand);
	dialog.show();

	return data->data;
}

} // editor
} // frozenbyte
