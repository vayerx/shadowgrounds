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
#include "paramUI.h"
#include "resource.h"

#include <map>
#include <windowsx.h>
#include "parseutil.h"

namespace frozenbyte {
namespace particle {

using namespace editor;

ParamDesc::ParamDesc(const std::string _name, int _id, PARAM_TYPE _type) :
name(_name), type(_type), id1(_id) 
{	
}

ParamDesc::ParamDesc(const std::string _name, int _id, PARAM_TYPE _type, const std::string &default_) :
name(_name), type(_type), id1(_id), defaultValue(default_)
{	
}

ParamDesc::ParamDesc(const std::string _name, int _id1, int _id2, int _id3, PARAM_TYPE _type) 
:	name(_name), 
	type(_type), 
	id1(_id1), 
	id2(_id2), 
	id3(_id3)
{
}

ParamDesc::ParamDesc(const std::string name_, int id1_, int id2_, const std::string& ext_, const std::string& path_,PARAM_TYPE type_) 
:	name(name_), 
	id1(id1_), 
	id2(id2_), 
	ext(ext_), 
	path(path_), 
	type(type_) 
{
}

ParamDesc::ParamDesc(const std::string name_, int id1_, const std::string& selections_, PARAM_TYPE type_) 
:	name(name_), 
	id1(id1_), 
	type(type_) 
{	
	if(type == PARAM_SELECTION) 
	{
		std::string str;	
		for(unsigned int i = 0; i < selections_.size(); i++) 
		{
			if(selections_[i] == ',') 
			{
				selections.push_back(str);
				str.clear();
			} 
			else
				str += selections_[i];
		}

		selections.push_back(str);
	} 
	else if(type == PARAM_STRING) 
	{		
	}
}

ParamDesc::ParamDesc(const ParamDesc &rhs) 
{
	*this = rhs;
}
		
ParamDesc& ParamDesc::operator=(const ParamDesc& rhs) 
{
	ext = rhs.ext;
	id1 = rhs.id1;
	id2 = rhs.id2;
	id3 = rhs.id3;
	name = rhs.name;
	defaultValue = rhs.defaultValue;
	path = rhs.path;
	selections = rhs.selections;
	type = rhs.type;

	return *this;
}


namespace
{
	
class AnimatedFloatCommand : public ICommand {
	VectorTrackDialog floatTrackDialog;
	Dialog& parent;
	ParserGroup& pg;
public:
	AnimatedFloatCommand(Dialog& parent_,
		ParserGroup& pg_) : parent(parent_), pg(pg_) {
	}
	void execute(int id) {
		floatTrackDialog.open(parent, IDD_VECTOR_TRACK, pg, true);	
	}
};

class AnimatedVectorCommand : public ICommand {
	Dialog& parent;
	VectorTrackDialog vectorTrackDialog;
	ParserGroup& pg;
public:
	AnimatedVectorCommand(Dialog& parent_, ParserGroup& pg_) : parent(parent_), pg(pg_) {}
	void execute(int id) {
		vectorTrackDialog.open(parent, IDD_VECTOR_TRACK, pg, false);
	}
};

class FileCommand : public ICommand {
	std::string fileName;
	std::string ext;
	std::string path;
	int viewID;
	Dialog& dlg;
public:
	FileCommand(const std::string& oldFileName,
		const std::string& ext_, const std::string& path_, Dialog& dlg_, int id) : ext(ext_),
		path(path_), dlg(dlg_), viewID(id) {
		fileName = oldFileName;
		setDialogItemText(dlg, viewID, fileName);
	}
	
	void execute(int id) {
		fileName = getOpenFileName(ext, path, false);
		setDialogItemText(dlg, viewID, fileName);
	}
	
	std::string getFileName() {
		return fileName;
	}
};
	

struct SharedData {
		
	Dialog& dlg;	
	const std::vector<ParamDesc>& pd;
	ParserGroup& parser;
	std::map< std::string, boost::shared_ptr<ICommand> >& cmds;
		
	SharedData(ParserGroup& _parser, Dialog& _dlg, const std::vector<ParamDesc>& _pd,
		std::map< std::string, boost::shared_ptr<ICommand> >& cmds_) :
	parser(_parser), dlg(_dlg), pd(_pd), cmds(cmds_) {
		
	}
	
	void update() {
		
		for(unsigned int i = 0; i < pd.size(); ++i) 
		{
			if(pd[i].type == PARAM_INT) 
			{
				int value = getDialogItemInt(dlg, pd[i].id1);
				parser.setValue(pd[i].name, convertToString<int>(value));
			}
			else if(pd[i].type == PARAM_BOOL) 
			{
				bool value = isCheckEnabled(dlg, pd[i].id1);
				parser.setValue(pd[i].name, convertToString<int>(value));
			}
			else if(pd[i].type == PARAM_FLOAT) 
			{
				float value = getDialogItemFloat(dlg, pd[i].id1);
				parser.setValue(pd[i].name, convertToString<float>(value));
			}
			else if(pd[i].type == PARAM_VECTOR) 
			{
				
				float value1 = getDialogItemFloat(dlg, pd[i].id1);
				float value2 = getDialogItemFloat(dlg, pd[i].id2);
				float value3 = getDialogItemFloat(dlg, pd[i].id3);
				
				parser.setValue(pd[i].name, convertVectorToString(Vector(value1, value2, value3)));
			}
			else if(pd[i].type == PARAM_ANIMATED_FLOAT) 
			{
				
				//ParserGroup& pg = parser.getSubGroup(pd[i].name);
				//pg = reinterpret_cast<AnimatedFloatCommand*>(cmds[pd[i].name].get())->getParserGroup();
				
			}
			else if(pd[i].type == PARAM_ANIMATED_VECTOR) 
			{
				
				//ParserGroup& pg = parser.getSubGroup(pd[i].name);
				//pg = reinterpret_cast<AnimatedVectorCommand*>(cmds[pd[i].name].get())->getParserGroup();
				
			}
			else if(pd[i].type == PARAM_STRING) 
			{
				std::string str = getDialogItemText(dlg, pd[i].id1);	
				parser.setValue(pd[i].name, str);
				
			}
			else if(pd[i].type == PARAM_FILE) {
				
				parser.setValue(pd[i].name,
					reinterpret_cast<FileCommand*>(cmds[pd[i].name].get())->getFileName());
				
			}
			else if(pd[i].type == PARAM_SELECTION) 
			{	
				int sel = ComboBox_GetCurSel(dlg.getItem(pd[i].id1));

				//psd
				if(sel == -1)
					continue;

				assert(sel >= 0 && sel < int(pd[i].selections.size()));
				parser.setValue(pd[i].name, pd[i].selections[sel]);

			}
			else 
			{
				assert(0);
			}
		}		
	}
};

	
class UpdateCommand : public ICommand {
	SharedData& data;
public:
	UpdateCommand(SharedData& _data) 
	:	 data(_data) 
	{
	}
	
	void execute(int id) 
	{
		data.update();
	}
};

} // unnamed namespace

	
struct ParamUIData
{

	Dialog dlg;
	SharedData data;
	UpdateCommand updateCommand;
	std::map< std::string, boost::shared_ptr<ICommand> > cmds;

	ParamUIData(Dialog& parent, int id, ParserGroup& parser, const std::vector<ParamDesc>& pd) 
	:	dlg(id, parent.getWindowHandle()), 
		data(parser, dlg, pd, cmds), 
		updateCommand(data)   
	{
		for(unsigned int i = 0; i < pd.size(); i++) 
		{		
			if(pd[i].type == PARAM_INT) {
				int value = convertFromString<int>(parser.getValue(pd[i].name, ""), convertFromString<int>(pd[i].defaultValue, 0));
				setDialogItemInt(dlg, pd[i].id1, value);
			}
			if(pd[i].type == PARAM_BOOL) {
				bool value = convertFromString<bool>(parser.getValue(pd[i].name, ""), convertFromString<bool>(pd[i].defaultValue, false));
				enableCheck(dlg, pd[i].id1, value);
			}
			if(pd[i].type == PARAM_FLOAT) {
				float value = convertFromString<float>(parser.getValue(pd[i].name, ""), convertFromString<float>(pd[i].defaultValue, 0.f));
				setDialogItemFloat(dlg, pd[i].id1, value);
			}
			if(pd[i].type == PARAM_VECTOR) {
				Vector value = convertVectorFromString(parser.getValue(pd[i].name, "0,0,0"));
				setDialogItemFloat(dlg, pd[i].id1, value.x);
				setDialogItemFloat(dlg, pd[i].id2, value.y);
				setDialogItemFloat(dlg, pd[i].id3, value.z);
			}
			if(pd[i].type == PARAM_ANIMATED_FLOAT) {
				boost::shared_ptr<ICommand> c(new AnimatedFloatCommand(dlg, parser.getSubGroup(pd[i].name)));
				cmds[pd[i].name] = c;
				dlg.getCommandList().addCommand(pd[i].id1, c.get());
			}
			if(pd[i].type == PARAM_ANIMATED_VECTOR) {
				boost::shared_ptr<ICommand> c(new AnimatedVectorCommand(dlg, parser.getSubGroup(pd[i].name)));
				cmds[pd[i].name] = c;
				dlg.getCommandList().addCommand(pd[i].id1, c.get());
			}
			if(pd[i].type == PARAM_STRING) {
				std::string str = parser.getValue(pd[i].name, pd[i].defaultValue);
				setDialogItemText(dlg, pd[i].id1, str); 
			}
			if(pd[i].type == PARAM_FILE) {
				std::string str = parser.getValue(pd[i].name, "");				
				boost::shared_ptr<ICommand> c(new FileCommand(str, pd[i].ext, pd[i].path, dlg, pd[i].id2));
				cmds[pd[i].name] = c;
				dlg.getCommandList().addCommand(pd[i].id1, c.get());
			}
			if(pd[i].type == PARAM_SELECTION) 
			{
				ComboBox_ResetContent(dlg.getItem(pd[i].id1));
				std::string sel = parser.getValue(pd[i].name, "");

				bool found = false;
				for(unsigned int j = 0; j < pd[i].selections.size(); j++) 
				{
					ComboBox_AddString(dlg.getItem(pd[i].id1), pd[i].selections[j].c_str());
					if(pd[i].selections[j] == sel)
					{
						ComboBox_SetCurSel(dlg.getItem(pd[i].id1), j);
						found = true;
					}
				}

				if(!found)
					ComboBox_SetCurSel(dlg.getItem(pd[i].id1), 0);
			}
			
		}
		
		dlg.getCommandList().addCommand(IDC_UPDATE, &updateCommand);
		
	}
};

ParamUI::ParamUI(Dialog& parent, int id, ParserGroup& parser, const std::vector<ParamDesc>& pd) 
{
	boost::scoped_ptr<ParamUIData> p(new ParamUIData(parent, id, parser, pd));
	data.swap(p);
}

ParamUI::~ParamUI() {
}

} // particle
} // frozenbyte
	

