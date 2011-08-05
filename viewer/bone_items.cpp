// Copyright 2002-2004 Frozenbyte Ltd.

#include "bone_items.h"
#include "../editor/common_dialog.h"
#include "../editor/parser.h"
#include "../editor/storm.h"
#include "../editor/storm_model_utils.h"

#include <cassert>
#include <map>
#include <vector>
#include <windows.h>
#include "resource.h"

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <istorm3d.h>
#include <istorm3d_model.h>
#include <istorm3d_helper.h>
#include <istorm3d_bone.h>

namespace frozenbyte {
namespace viewer {	

struct BoneItemsData
{
	// [helper] - items
	std::map<std::string, std::vector<std::string> > items;
	std::vector<std::string> helpers;

	static BOOL CALLBACK DialogHandler(HWND windowHandle, UINT message,  WPARAM wParam, LPARAM lParam)
	{
		BoneItemsData *data = reinterpret_cast<BoneItemsData *> (GetWindowLong(windowHandle, GWL_USERDATA));
		static int listIndex = -1;

		if(message == WM_INITDIALOG)
		{
			SetWindowLong(windowHandle, GWL_USERDATA, lParam);
			BoneItemsData *data = reinterpret_cast<BoneItemsData *> (lParam);

			listIndex = -1;
			for(unsigned int i = 0; i < data->helpers.size(); ++i)
				SendDlgItemMessage(windowHandle, IDC_HELPERS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM> (data->helpers[i].c_str()));
		}
		else if(message == WM_COMMAND)
		{
			int command = LOWORD(wParam);
			if(command == WM_DESTROY)
				EndDialog(windowHandle, 0);

			if(command == IDC_HELPERS)
			{
				int index = SendDlgItemMessage(windowHandle, IDC_HELPERS, LB_GETCURSEL, 0, 0);
				if(listIndex == index)
					return 0;

				SendDlgItemMessage(windowHandle, IDC_ITEMS, LB_RESETCONTENT, 0, 0);
				if(index == -1)
				{
					EnableWindow(GetDlgItem(windowHandle, IDC_ITEM_INSERT), FALSE);
					EnableWindow(GetDlgItem(windowHandle, IDC_ITEM_REMOVE), FALSE);
					return 0;
				}

				if(index != -1)
				{
					std::string &helper = data->helpers[index];
					std::vector<std::string> files = data->items[helper];

					for(unsigned int i = 0; i < files.size(); ++i)
					{
						std::string string = editor::getFileName(files[i]);
						SendDlgItemMessage(windowHandle, IDC_ITEMS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM> (string.c_str()));
					}
				}

				EnableWindow(GetDlgItem(windowHandle, IDC_ITEM_INSERT), TRUE);
				EnableWindow(GetDlgItem(windowHandle, IDC_ITEM_REMOVE), TRUE);
				listIndex = index;
			}

			if(command == IDC_ITEM_INSERT)
			{
				std::string fileName = editor::getOpenFileName("s3d", "Data\\Models");
				if(!fileName.empty())
				{
					std::string &helper = data->helpers[listIndex];
					data->items[helper].push_back(fileName);

					std::string string = editor::getFileName(fileName);
					SendDlgItemMessage(windowHandle, IDC_ITEMS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM> (string.c_str()));
				}
			}

			if(command == IDC_ITEM_REMOVE)
			{
				int index = SendDlgItemMessage(windowHandle, IDC_ITEMS, LB_GETCURSEL, 0, 0);
				if(index == -1)
					return 0;

				std::string &helper = data->helpers[listIndex];
				data->items[helper].erase(data->items[helper].begin() + index);
				SendDlgItemMessage(windowHandle, IDC_ITEMS, LB_DELETESTRING, index, 0);
			}
		}

		return 0;
	}

	void getHelpers(IStorm3D_Model *model)
	{
		assert(model);
		helpers.clear();

		boost::scoped_ptr<Iterator<IStorm3D_Helper *> > helperIterator(model->ITHelper->Begin());
		for(; !helperIterator->IsEnd(); helperIterator->Next())
		{
			IStorm3D_Helper *helper = helperIterator->GetCurrent();
			if((!helper) || (helper->GetHelperType() != IStorm3D_Helper::HTYPE_CAMERA))
				continue;

			if(!helper->GetParentBone())
				continue;

			helpers.push_back(helper->GetName());
		}
	}

	void removeOld(IStorm3D_Model *model)
	{
		std::vector<IStorm3D_Model_Object *> removeList;
		
		boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > objectIterator(model->ITObject->Begin());
		for(; !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			if((object) && (object->GetParentBone()))
				removeList.push_back(object);
		}

		for(unsigned int i = 0; i < removeList.size(); ++i)
			model->Object_Delete(removeList[i]);
	}

	void apply(boost::shared_ptr<IStorm3D_Model> model, editor::Storm &storm)
	{
		assert(model);

		std::map<std::string, std::vector<std::string> >::iterator it = items.begin();
		for(; it != items.end(); ++it)
		{
			std::vector<std::string> &items = (*it).second;
			for(unsigned int i = 0; i < items.size(); ++i)
			{
				boost::shared_ptr<IStorm3D_Model> helperModel(storm.storm->CreateNewModel());
				helperModel->LoadS3D(items[i].c_str());

				editor::addCloneModel(helperModel, model, it->first);
			}
		}
	}
};

BoneItems::BoneItems()
{
	boost::scoped_ptr<BoneItemsData> tempData(new BoneItemsData());
	data.swap(tempData);
}

BoneItems::~BoneItems()
{
}

void BoneItems::showDialog(IStorm3D_Model *model)
{
	if(!model)
		return;

	data->getHelpers(model);
	DialogBoxParam(GetModuleHandle(0), MAKEINTRESOURCE(IDD_ITEMS), 0, BoneItemsData::DialogHandler, reinterpret_cast<LPARAM> (data.get()));
}

void BoneItems::applyToModel(boost::shared_ptr<IStorm3D_Model> model, editor::Storm &storm)
{
	if(!model)
		return;

	data->removeOld(model.get());
	data->getHelpers(model.get());
	data->apply(model, storm);
}

void BoneItems::load(const editor::Parser &parser)
{
	data->items.clear();
	data->helpers.clear();

	const editor::ParserGroup &group = parser.getGlobals().getSubGroup("Helpers");
	int helpers = boost::lexical_cast<int> (group.getValue("Amount", "0"));

	for(int i = 0; i < helpers; ++i)
	{
		const editor::ParserGroup &g = group.getSubGroup(std::string("Helper") + boost::lexical_cast<std::string> (i));
		
		const std::string &helper = g.getValue("Name");
		if(helper.empty())
			continue;

		for(int j = 0; j < g.getLineCount(); ++j)
			data->items[helper].push_back(g.getLine(j));
	}
}

void BoneItems::save(editor::Parser &parser)
{
	std::map<std::string, std::vector<std::string> >::iterator it = data->items.begin();
	int index = 0;

	for(; it != data->items.end(); ++it)
	{
		const std::string &helper = (*it).first;
		if(std::find(data->helpers.begin(), data->helpers.end(), helper) == data->helpers.end())
			continue;

		std::vector<std::string> &items = (*it).second;
		if(items.empty())
			continue;

		std::string groupName = std::string("Helper") + boost::lexical_cast<std::string> (index++);
		editor::ParserGroup &group = parser.getGlobals().getSubGroup("Helpers").getSubGroup(groupName);
		group.setValue("Name", helper);

		for(unsigned int i = 0; i < items.size(); ++i)
			group.addLine(items[i]);
	}

	parser.getGlobals().getSubGroup("Helpers").setValue("Amount", boost::lexical_cast<std::string> (index));
}

} // end of namespace viewer
} // end of namespace frozenbyte
