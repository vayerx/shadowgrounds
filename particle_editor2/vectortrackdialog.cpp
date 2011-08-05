// Copyright 2002-2004 Frozenbyte Ltd.

#include "../editor/string_conversions.h"
#include "../editor/parser.h"
#include "../editor/window.h"
#include "../editor/storm.h"
#include "../editor/dialog.h"
#include "../editor/dialog_utils.h"
#include "../editor/icommand.h"
#include "../editor/command_list.h"
#include "../editor/common_dialog.h"
#include "../editor/color_component.h"
#include "../editor/color_picker.h"

#include "vectortrackdialog.h"
#include "parseutil.h"
#include "resource.h"
#include <windowsx.h>

namespace frozenbyte
{
namespace particle
{

using namespace editor;

struct VectorTrackDialogData {


	struct DialogData {
		
		ParserGroup& pg;
		Dialog& dlg;
		int curKey;
		bool floatMode;

		class Key {
		public:
			Key() {}
			Key(float t) : time(t) { value = Vector(0.0f, 0.0f, 0.0f); }
			Key(const Key& other) { *this = other; }
			Key& operator=(const Key& other) { 
				time = other.time; 
				value = other.value;
				return *this; }
			float time;
			Vector value;
		};

		std::vector<Key> keys;
		bool noValueUpdate;

		DialogData(Dialog& dlg_, ParserGroup& pg_, bool floatMode_) : dlg(dlg_), pg(pg_), floatMode(floatMode_) {
		
			
			int numKeys = convertFromString<int>(pg.getValue("num_keys", ""), 0);
			keys.resize(numKeys);
			
			for(int i = 0; i < numKeys; i++) {
				
				std::string str = "key" + convertToString<int>(i);
				
				keys[i].time = convertFromString<float>(pg.getValue((str + ".time"), ""), 0);				
				
				if(!floatMode) {
					keys[i].value = convertVectorFromString(pg.getValue((str + ".value"), "0,0,0"));
				} else {
					keys[i].value.x = convertFromString<float>(pg.getValue((str + ".value"), ""), 0);
				}
			}

			updateDialog();
			
			if(numKeys > 0) {
				ListBox_SetCurSel(dlg.getItem(IDC_KEYSV), 0);
				selectKey();
			}
			

		}

		void updateKey() {
							
			if(curKey < 0)
				return;

			if(!noValueUpdate) {	
				keys[curKey].time = getDialogItemFloat(dlg, IDC_TIMEV);
				keys[curKey].value.x = getDialogItemFloat(dlg, IDC_VALUE_X);			
				if(!floatMode) {	
					keys[curKey].value.y = getDialogItemFloat(dlg, IDC_VALUE_Y);			
					keys[curKey].value.z = getDialogItemFloat(dlg, IDC_VALUE_Z);
				}
			}

		}
		
		void update() 
		{
			pg.setValue("num_keys", convertToString<int>(keys.size()));

			for(unsigned int i = 0; i < keys.size(); ++i) 
			{
				std::string str = "key" + convertToString<int>(i);
				pg.setValue((str + ".time"), convertToString<float>(keys[i].time));

				if(!floatMode)	
					pg.setValue((str + ".value"), convertVectorToString(keys[i].value));
				else
					pg.setValue((str + ".value"), convertToString<float>(keys[i].value.x));
			}
		}
		
		void updateDialog() 
		{	
			ListBox_ResetContent(dlg.getItem(IDC_KEYSV));

			for(unsigned int i = 0; i < keys.size(); i++) 
			{
				std::string str = "key" + convertToString<int>(i);
				ListBox_AddString(dlg.getItem(IDC_KEYSV), str.c_str());
			}
		
			selectKey();
		}

		void addKey() 
		{
			keys.push_back(Key(1.0f));
			updateDialog();
		}

		void removeKey() {
			
			if(curKey < 0)
				return;

			if(keys.size() > 1) {
				keys.erase(keys.begin()+curKey);			
			}
			if(keys.size()==1) {
				keys[0].time = 0.0f;
			}

			updateDialog();
		}

		void selectKey() {

			curKey = ListBox_GetCurSel(dlg.getItem(IDC_KEYSV));
			if(curKey < 0)
				return;
			
			noValueUpdate = true;
			
			setDialogItemFloat(dlg, IDC_VALUE_X, keys[curKey].value.x);
			setDialogItemFloat(dlg, IDC_TIMEV, keys[curKey].time);
			if(!floatMode) {	
				setDialogItemFloat(dlg, IDC_VALUE_Y, keys[curKey].value.y);
				setDialogItemFloat(dlg, IDC_VALUE_Z, keys[curKey].value.z);
			}

			noValueUpdate = false;
		}

	
	};
	
	class SelectKeyCommand : public ICommand {
		DialogData& data;
	public:
		SelectKeyCommand(DialogData& data_) : data(data_) {}
		void execute(int id) {
			data.selectKey();
		}
	};

	class AddKeyCommand : public ICommand {
		DialogData& data;
	public:
		AddKeyCommand(DialogData& data_) : data(data_) {}
		void execute(int id) {
			data.addKey();
		}
	};

	class RemoveKeyCommand : public ICommand {
		DialogData& data;
	public:
		RemoveKeyCommand(DialogData& data_) : data(data_) {}
		void execute(int id) {
			data.removeKey();
		}
	};

	class UpdateCommand : public ICommand {
		DialogData& data;
	public:
		UpdateCommand(DialogData& data_) : data(data_) {}
		void execute(int id) {
			data.updateKey();
		}
	};

	class OkCommand : public ICommand {
		Dialog& dlg;
		DialogData& data;
	public:
		OkCommand(Dialog& dlg_, DialogData& data_) : dlg(dlg_), data(data_) {}
		void execute(int id) {
			data.update();
			dlg.hide();
		}
	};

	class CancelCommand : public ICommand {
		Dialog& dlg;
	public:
		CancelCommand(Dialog& dlg_) : dlg(dlg_) {}
		void execute(int id) {
			dlg.hide();
		}
	};


	Dialog dialog;
	DialogData data;
	SelectKeyCommand selectKeyCommand;
	AddKeyCommand addKeyCommand;
	RemoveKeyCommand removeKeyCommand;
	OkCommand okCommand;
	CancelCommand cancelCommand;
	UpdateCommand updateCommand;

	VectorTrackDialogData(Dialog& parent, int id, ParserGroup& pg, bool floatMode) : dialog(id, parent.getWindowHandle()),
		data(dialog, pg, floatMode), selectKeyCommand(data),
		addKeyCommand(data), removeKeyCommand(data), okCommand(dialog, data), 
		cancelCommand(dialog), updateCommand(data) {
	
		dialog.getCommandList().addCommand(IDC_KEYSV, &selectKeyCommand);
		dialog.getCommandList().addCommand(IDC_ADD_KEYV, &addKeyCommand);
		dialog.getCommandList().addCommand(IDC_REMOVE_KEYV, &removeKeyCommand);
		dialog.getCommandList().addCommand(IDOK, &okCommand);
		dialog.getCommandList().addCommand(IDCANCEL, &cancelCommand);
		dialog.getCommandList().addCommand(IDC_VALUE_X, &updateCommand);
		dialog.getCommandList().addCommand(IDC_VALUE_Y, &updateCommand);
		dialog.getCommandList().addCommand(IDC_VALUE_Z, &updateCommand);
		dialog.getCommandList().addCommand(IDC_TIMEV, &updateCommand);

	}


};

VectorTrackDialog::VectorTrackDialog() {
}
	
VectorTrackDialog::~VectorTrackDialog() {
}


void VectorTrackDialog::open(Dialog& parent_, int id_, ParserGroup& pg_, bool floatMode) {
	boost::scoped_ptr<VectorTrackDialogData> p(new VectorTrackDialogData(parent_, id_, pg_, floatMode));
	data.swap(p);
}




} // particle
} // frozenbyte
