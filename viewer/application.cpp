// Copyright 2002-2004 Frozenbyte Ltd.

//#define TEKES

#include "application.h"
#include "model.h"
#include "../editor/window.h"
#include "../editor/storm.h"
#include "../editor/dialog.h"
#include "../editor/dialog_utils.h"
#include "../editor/camera.h"
#include "../editor/mouse.h"
#include "../editor/icommand.h"
#include "../editor/command_list.h"
#include "../editor/common_dialog.h"
#include "../editor/color_component.h"
#include "../editor/color_picker.h"
#include "../filesystem/ifile_package.h"
#include "../filesystem/standard_package.h"
#include "../filesystem/zip_package.h"
#include "../filesystem/file_package_manager.h"
#include "../sound/wavereader.h"
#include "../sound/soundlib.h"
#include "../sound/lipsyncmanager.h"
#include "../sound/lipsyncproperties.h"
#include <stdio.h>
#include <istorm3d.h>
#include <istorm3d_scene.h>
#include <istorm3d_mesh.h>
#include <istorm3d_videostreamer.h>
#include "resource.h"
#include <istorm3d_terrain_renderer.h>

#pragma comment(lib, "fmodvc.lib")
#pragma comment(lib, "storm3dv2.lib")

namespace frozenbyte {
namespace viewer {

using namespace frozenbyte::filesystem;
using namespace frozenbyte::editor;
extern int BLEND_TIME;

namespace {
	struct SharedData
	{
		editor::Storm &storm;
		Model &model;

		editor::Dialog &dialog;
		editor::Dialog &renderDialog;
		editor::Camera &camera;
		editor::ColorComponent &colorComponent;

		int animationState;
		int color;

		boost::scoped_ptr<sfx::LipsyncManager> lipsync;
		sfx::SoundLib lib;
		boost::scoped_ptr<sfx::Sound> sound;
		int soundId;

		IStorm3D_Font *font;
		IStorm3D_Font *font2;

		SharedData(editor::Storm &storm_, Model &model_, editor::Dialog &dialog_, editor::Dialog &renderDialog_, editor::Camera &camera_, editor::ColorComponent &colorComponent_)
		:	storm(storm_),
			model(model_),
			
			dialog(dialog_),
			renderDialog(renderDialog_),
			camera(camera_),
			colorComponent(colorComponent_),
			soundId(-1),
			font(0),
			font2(0)
		{
			animationState = 0;
			color = RGB(70, 70, 70);
			//CheckDlgButton(dialog.getWindowHandle(), IDC_DRAW_BONES, BST_CHECKED);
			updateDialog();

			CheckDlgButton(dialog.getWindowHandle(), IDC_BASE, BST_CHECKED);
			CheckDlgButton(dialog.getWindowHandle(), IDC_BLEND, BST_UNCHECKED);
			colorComponent.setColor(color);

#ifdef TEKES
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_GEOMETRY), FALSE);
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_ADD), FALSE);
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_BONES), FALSE);
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_ATTACH), FALSE);
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_SAVE), FALSE);
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_GLOW), FALSE);

	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_BASE), FALSE);
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_BLEND), FALSE);
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_INSERT), FALSE);
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_REMOVE), FALSE);
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_PLAY), FALSE);
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_LOOP), FALSE);
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_STOP), FALSE);
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_MODEL_SCALE), FALSE);
	EnableWindow(GetDlgItem(dialog.getWindowHandle(), IDC_RELOAD), FALSE);
	setSliderValue(dialog, IDC_MODEL_SCALE, 10);
#endif
			setDialogItemText(dialog, IDC_BLENDTIME, "200");
			lib.initialize(0);
		}

		void reload()
		{
			if(soundId >= 0)
			{
				lib.stopSound(soundId);
				soundId = -1;
			}

			lipsync.reset(new sfx::LipsyncManager(storm.storm));
			const sfx::LipsyncProperties &prop = lipsync->getProperties();

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_IDLES, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPRESSIONS, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_IDLES, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("(none)"));
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPRESSIONS, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ("(none)"));

			int i = 0;
			for(i = 0; i < prop.getIdleAnimationAmount(); ++i)
			{
				const std::string &name = prop.getIdleAnimationName(i);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_IDLES, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (name.c_str()));
			}
			for(i = 0; i < prop.getExpressionAnimationAmount(); ++i)
			{
				const std::string &name = prop.getExpressionAnimationName(i);
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPRESSIONS, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> (name.c_str()));
			}

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_IDLES, CB_SETCURSEL, 0, 0);
			SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPRESSIONS, CB_SETCURSEL, 0, 0);

			/*
			{
				IStorm3D_Texture *texture = storm.storm->CreateNewTexture("Data/Fonts/font2.dds");
				font = 0; 

				if(texture)
				{
					std::string buff;
					buff += '\t';
					buff += '!';
					buff += '"';
					buff += "#$%@'()*+.-,/0123456789:;<=>?\nabcdefghijklmnopqrstuvwxyz[\\]\t_";

					std::vector<unsigned char> widths;
					for(unsigned int i = 0; i < buff.size(); ++i)
						widths.push_back(64);

					font = storm.storm->CreateNewFont();
					font->AddTexture(texture);
					font->SetTextureRowsAndColums(8, 8);
					font->SetCharacters(buff.c_str(), &widths[0]);
					font->SetColor(COL(1.0f, 1.0f, 1.0f));	
				}
			}
			*/

			font = storm.storm->CreateNewFont();
			font->SetFont("Times New Roman", 0, 20, false, false);
			font->SetColor(COL(1.0f, 1.0f, 1.0f));	

			font2 = storm.storm->CreateNewFont();
			font2->SetFont("Times New Roman", 0, 15, false, false);
			font2->SetColor(COL(1.0f, 1.0f, 1.0f));	
		}

		void play(const std::string &name)
		{
			stop();

			IStorm3D_Model *m = model.getModel();
			if(!m)
				return;

			sound.reset(new sfx::Sound(name.c_str(), FSOUND_2D));
			soundId = lib.createSound(sound.get(), 1);

			if(soundId >= 0)
			{
				boost::shared_ptr<sfx::AmplitudeArray> amplitudes = lipsync->getAmplitudeBuffer(name);
				unsigned int currentTime = timeGetTime();

				lib.playSound(soundId);
				lipsync->play(m, amplitudes, currentTime);
			}
		}

		void updateSound(int delta, int currentTime)
		{
			//IStorm3D_Model *m = model.getModel();
			//if(soundId >= 0 && m)
			//	lipsync->setPlayTime(m, lib.getSoundTime(soundId));

			//lipsync->update(delta, currentTime + 150);
			lipsync->update(delta, currentTime);
		}

		void stop()
		{
			if(soundId >= 0)
			{
				lib.stopSound(soundId);
			}
		}

		void applyIdle()
		{
			IStorm3D_Model *m = model.getModel();
			if(m)
			{
				int i = SendDlgItemMessage(dialog.getWindowHandle(), IDC_IDLES, CB_GETCURSEL, 0, 0);
				if(i > 0)
				{
					const std::string &name = lipsync->getProperties().getIdleAnimationName(i - 1);
					lipsync->setIdle(m, name);
				}
			}
		}

		void applyExpr()
		{
			IStorm3D_Model *m = model.getModel();
			if(m)
			{
				int i = SendDlgItemMessage(dialog.getWindowHandle(), IDC_EXPRESSIONS, CB_GETCURSEL, 0, 0);
				if(i > 0)
				{
					const std::string &name = lipsync->getProperties().getExpressionAnimationName(i - 1);
					lipsync->setExpression(m, name);
				}
			}
		}

		void setBackGround()
		{
			float r = GetRValue(color) / 255.f;
			float g = GetGValue(color) / 255.f;
			float b = GetBValue(color) / 255.f;

			storm.scene->SetBackgroundColor(Color(r, g, b));
		}

		void enableDialogs(bool enable)
		{

#ifndef TEKES
			editor::enableDialogItem(dialog, IDC_ADD, enable);
			editor::enableDialogItem(dialog, IDC_BONES, enable);
			editor::enableDialogItem(dialog, IDC_ATTACH, enable);
#endif

		}

		void updateDialog()
		{
			if(model.hasModel())
				enableDialogs(true);
			else
				enableDialogs(false);

			SendDlgItemMessage(dialog.getWindowHandle(), IDC_ANIMATIONS, LB_RESETCONTENT, 0, 0);
			for(int i = 0; i < model.getAnimationCount(animationState); ++i)
			{
				std::string name = editor::getFileName(model.getAnimation(animationState, i));
				SendDlgItemMessage(dialog.getWindowHandle(), IDC_ANIMATIONS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM> (name.c_str()));
			}

		}

		void updateBoneDrawing(editor::Dialog &dialog)
		{
			int state = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_DRAW_BONES);
			if(state == BST_CHECKED)
				storm.scene->DrawBones(true);
			else
				storm.scene->DrawBones(false);
		}

		int getSelection()
		{
			return SendMessage(GetDlgItem(dialog.getWindowHandle(), IDC_ANIMATIONS), LB_GETCURSEL, 0, 0);
		}
	};

	class LoadGeometryCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		LoadGeometryCommand(SharedData &sharedData_, editor::Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_GEOMETRY, this);
		}

		void execute(int id)
		{
			std::vector<std::string> fileNames = editor::getMultipleOpenFileName("s3d", "Data\\Models");
			if(fileNames.empty())
				return;

			execute(fileNames);
		}

		void execute(std::vector<std::string> fileNames)
		{
			sharedData.lipsync->reset();
			sharedData.model.loadGeometry(fileNames[0]);

			for(unsigned int i = 1; i < fileNames.size(); ++i)
				sharedData.model.addGeometry(fileNames[i]);

			sharedData.camera.setToOrigo();
			sharedData.updateDialog();

			sharedData.applyIdle();
			sharedData.applyExpr();
		}
	};

	class AddGeometryCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		AddGeometryCommand(SharedData &sharedData_, editor::Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_ADD, this);
		}

		void execute(int id)
		{
			std::vector<std::string> fileNames = editor::getMultipleOpenFileName("s3d", "Data\\Models");
			if(!fileNames.empty())
				sharedData.lipsync->reset();

			for(unsigned int i = 0; i < fileNames.size(); ++i)
				sharedData.model.addGeometry(fileNames[i]);

			sharedData.applyIdle();
			sharedData.applyExpr();
		}
	};

	class LoadBonesCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		LoadBonesCommand(SharedData &sharedData_, editor::Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_BONES, this);
		}

		void execute(int id)
		{
			std::string fileName = editor::getOpenFileName("b3d", "Data\\Models");
			if(!fileName.empty())
			{
				sharedData.model.loadBones(fileName);
				sharedData.updateDialog();
			}
		}
	};

	class AttachItemsCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		AttachItemsCommand(SharedData &sharedData_, editor::Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_ATTACH, this);
		}

		void execute(int id)
		{
			sharedData.model.attachItems();
		}
	};

	class DrawBoneCommand: public editor::ICommand
	{
		SharedData &sharedData;
		editor::Dialog &dialog;

	public:
		DrawBoneCommand(SharedData &sharedData_, editor::Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			dialog.getCommandList().addCommand(IDC_DRAW_BONES, this);
		}

		void execute(int id)
		{
			sharedData.updateBoneDrawing(dialog);
		}
	};

	class BaseCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		BaseCommand(SharedData &sharedData_, editor::Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_BASE, this);
		}

		void execute(int id)
		{
			if(sharedData.animationState == 0)
				return;

			sharedData.animationState = 0;
			sharedData.updateDialog();
		}
	};

	class BlendCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		BlendCommand(SharedData &sharedData_, editor::Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_BLEND, this);
		}

		void execute(int id)
		{
			if(sharedData.animationState == 1)
				return;

			sharedData.animationState = 1;
			sharedData.updateDialog();
		}
	};

	class InsertAnimationCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		InsertAnimationCommand(SharedData &sharedData_, editor::Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_INSERT, this);
		}

		void execute(int id)
		{
			std::vector<std::string> fileNames = editor::getMultipleOpenFileName("anm", "Data\\Animations");
			if(fileNames.empty())
				return;

			for(unsigned int i = 0; i < fileNames.size(); ++i)
				sharedData.model.addAnimation(sharedData.animationState, fileNames[i]);

			sharedData.updateDialog();
		}
	};

	class RemoveAnimationCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		RemoveAnimationCommand(SharedData &sharedData_, editor::Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_REMOVE, this);
		}

		void execute(int id)
		{
			int index = sharedData.getSelection();
			if(index >= 0)
			{
				sharedData.model.removeAnimation(sharedData.animationState, index);
				sharedData.updateDialog();
			}
		}
	};

	class PlayAnimationCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		PlayAnimationCommand(SharedData &sharedData_, editor::Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_PLAY, this);
		}

		void execute(int id)
		{
			int index = sharedData.getSelection();
			if(index >= 0)
				sharedData.model.playAnimation(sharedData.animationState, index, false);
		}
	};

	class LoopAnimationCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		LoopAnimationCommand(SharedData &sharedData_, editor::Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_LOOP, this);
		}

		void execute(int id)
		{
			int index = sharedData.getSelection();
			if(index >= 0)
				sharedData.model.playAnimation(sharedData.animationState, index, true);
		}
	};

	class StopAnimationCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		StopAnimationCommand(SharedData &sharedData_, editor::Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_STOP, this);
		}

		void execute(int id)
		{
			int index = sharedData.getSelection();
			if(index >= 0)
				sharedData.model.stopAnimation(sharedData.animationState, index);
		}
	};

	class SaveCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		SaveCommand(SharedData &sharedData_, editor::Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_SAVE, this);
		}

		void execute(int id)
		{
			std::string fileName = editor::getSaveFileName("fbv", "Editor\\Viewer");
			if(!fileName.empty())
				sharedData.model.save(fileName);
		}
	};

	class LoadCommand: public editor::ICommand
	{
		SharedData &sharedData;
		editor::Dialog &dialog;

	public:
		LoadCommand(SharedData &sharedData_, editor::Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			dialog.getCommandList().addCommand(IDC_LOAD, this);
		}

		void execute(int id)
		{
			std::string fileName = editor::getOpenFileName("fbv", "Editor\\Viewer");
			if(fileName.empty())
				return;

			sharedData.lipsync->reset();

			sharedData.model.load(fileName);
			sharedData.camera.setToOrigo();
			sharedData.updateDialog();

#ifdef TEKES
	setComboIndex(dialog, IDC_IDLES, 1);
	setComboIndex(dialog, IDC_EXPRESSIONS, 2);
	sharedData.applyIdle();
	sharedData.applyExpr();
#endif

		}
	};

	class ReloadCommand: public editor::ICommand
	{
		SharedData &sharedData;
		editor::Dialog &dialog;

	public:
		ReloadCommand(SharedData &sharedData_, editor::Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			dialog.getCommandList().addCommand(IDC_RELOAD, this);
		}

		void execute(int id)
		{
			sharedData.model.reload(sharedData.renderDialog);
			if(!sharedData.storm.storm)
				return;

			sharedData.updateBoneDrawing(dialog);
			sharedData.setBackGround();
			sharedData.reload();

			//sharedData.storm.storm->setGlobalTimeFactor(0.2f);
		}
	};

	class ColorCommand: public editor::ICommand
	{
		SharedData &sharedData;

	public:
		ColorCommand(SharedData &sharedData_, editor::Dialog &dialog)
		:	sharedData(sharedData_)
		{
			dialog.getCommandList().addCommand(IDC_COLOR, this);
		}

		void execute(int id)
		{
			editor::ColorPicker colorPicker;
			if(colorPicker.run(sharedData.color))
			{
				sharedData.color = colorPicker.getColor();
				sharedData.colorComponent.setColor(sharedData.color);
				sharedData.setBackGround();
			}
		}
	};

	class ModeCommand: public editor::ICommand
	{
		SharedData &sharedData;
		editor::Dialog &dialog;

	public:
		ModeCommand(SharedData &sharedData_, editor::Dialog &dialog_)
		:	sharedData(sharedData_),
			dialog(dialog_)
		{
			//dialog.getCommandList().addCommand(IDC_RENDER_MODE, this);
			dialog.getCommandList().addCommand(IDC_WIREFRAME, this);
		}

		void execute(int id)
		{
			/*
			int index = SendDlgItemMessage(dialog.getWindowHandle(), IDC_RENDER_MODE, CB_GETCURSEL, 0, 0);
			IStorm3D_TerrainRenderer &renderer = sharedData.storm.terrain->getRenderer();

			if(index == 0)
				renderer.enableFeature(IStorm3D_TerrainRenderer::Wireframe, false);
			else if(index == 1)
				renderer.enableFeature(IStorm3D_TerrainRenderer::Wireframe, true);
			*/
			bool enable = IsDlgButtonChecked(dialog.getWindowHandle(), IDC_WIREFRAME) == BST_CHECKED;
			IStorm3D_TerrainRenderer &renderer = sharedData.storm.terrain->getRenderer();
			renderer.enableFeature(IStorm3D_TerrainRenderer::Wireframe, enable);
		}
	};

	class ResetCameraCommand : public editor::ICommand
	{
		SharedData &sharedData;
		editor::Camera &camera;;

	public:
		ResetCameraCommand(SharedData &sharedData_, editor::Dialog &dialog, editor::Camera &camera_)
		:	sharedData(sharedData_),
			camera(camera_)
		{
			dialog.getCommandList().addCommand(IDC_RESET_CAMERA, this);
		}

		void execute(int id)
		{
			camera.setToOrigo();
		}
	};

	class ModelScaleCommand : public editor::ICommand
	{
		SharedData &data;

	public:
		ModelScaleCommand(SharedData &data_)
		:	data(data_)
		{
			CommandList &cs = data.dialog.getCommandList(WM_NOTIFY);
			cs.addCommand(IDC_MODEL_SCALE, this);
		}

		float getScale() const
		{
			return float(getSliderValue(data.dialog, IDC_MODEL_SCALE));
		}

		void execute(int id)
		{
			data.model.setScale(getScale());
		}
	};

	class WavCommand : public editor::ICommand
	{
		SharedData &data;

	public:
		WavCommand(SharedData &data_)
		:	data(data_)
		{
			data.dialog.getCommandList().addCommand(IDC_WAV, this);
		}

		void execute(int id)
		{
			std::string fileName = editor::getOpenFileName("wav", "Data\\Sounds\\Vocals");
			if(fileName.empty())
				return;

			setDialogItemText(data.dialog, IDC_WAV, fileName);
		}
	};

	class PlayCommand : public editor::ICommand
	{
		SharedData &data;

	public:
		PlayCommand(SharedData &data_)
		:	data(data_)
		{
			data.dialog.getCommandList().addCommand(IDC_PLAYW, this);
		}

		void execute(int id)
		{
			data.play(getDialogItemText(data.dialog, IDC_WAV));
		}
	};

	class IdleCommand : public editor::ICommand
	{
		SharedData &data;

	public:
		IdleCommand(SharedData &data_)
		:	data(data_)
		{
			data.dialog.getCommandList().addCommand(IDC_IDLES, this);
		}

		void execute(int id)
		{
			data.applyIdle();
		}
	};

	class ExprCommand : public editor::ICommand
	{
		SharedData &data;

	public:
		ExprCommand(SharedData &data_)
		:	data(data_)
		{
			data.dialog.getCommandList().addCommand(IDC_EXPRESSIONS, this);
		}

		void execute(int id)
		{
			data.applyExpr();
		}
	};

} // end of unnamed namespace

struct ApplicationData
{
	editor::Window window;
	editor::Dialog mainDialog;
	editor::Dialog renderDialog;

	editor::Storm storm;
	bool mustQuit;

	editor::Camera camera;
	editor::Mouse mouse;
	Model model;

	editor::ColorComponent colorComponent;
	SharedData sharedData;
	
	LoadGeometryCommand geometryCommand;
	LoadBonesCommand bonesCommand;
	AddGeometryCommand addGeometryCommand;
	AttachItemsCommand attachItemsCommand;
	DrawBoneCommand drawBoneCommand;

	BaseCommand baseCommand;
	BlendCommand blendCommand;
	InsertAnimationCommand insertCommand;
	RemoveAnimationCommand removeCommand;
	
	PlayAnimationCommand playCommand;
	LoopAnimationCommand loopCommand;
	StopAnimationCommand stopCommand;

	SaveCommand saveCommand;
	LoadCommand loadCommand;
	ReloadCommand reloadCommand;

	ColorCommand colorCommand;
	ModeCommand modeCommand;
	ResetCameraCommand resetCameraCommand;
	ModelScaleCommand modelScaleCommand;
	WavCommand wavCommand;
	PlayCommand playWavCommand;
	IdleCommand idleCommand;
	ExprCommand exprCommand;

	ApplicationData()
	//:	window("w,a,s,d,q,e,+,-,wheel,arrows. Reload fixes stretching & updates resources & stops animations(!)", IDI_ICON1, false, false),
	:	window("Frozenbyte Viewer", IDI_ICON1, false, false),
		mainDialog(IDD_MAIN, window, editor::Dialog::ATTACH_BOTTOM),
		renderDialog(IDD_RENDER, window, editor::Dialog::ATTACH_ALL),

		storm(renderDialog.getWindowHandle()),
		mustQuit(false),

		camera(storm),
		model(storm),

		colorComponent(mainDialog.getWindowHandle(), 105, 500, 100, 24),
		sharedData(storm, model, mainDialog, renderDialog, camera, colorComponent),
		
		geometryCommand(sharedData, mainDialog),
		bonesCommand(sharedData, mainDialog),
		addGeometryCommand(sharedData, mainDialog),
		attachItemsCommand(sharedData, mainDialog),
		drawBoneCommand(sharedData, mainDialog),

		baseCommand(sharedData, mainDialog),
		blendCommand(sharedData, mainDialog),
		insertCommand(sharedData, mainDialog),
		removeCommand(sharedData, mainDialog),
		
		playCommand(sharedData, mainDialog),
		loopCommand(sharedData, mainDialog),
		stopCommand(sharedData, mainDialog),

		saveCommand(sharedData, mainDialog),
		loadCommand(sharedData, mainDialog),
		reloadCommand(sharedData, mainDialog),

		colorCommand(sharedData, mainDialog),
		modeCommand(sharedData, mainDialog),
		resetCameraCommand(sharedData, mainDialog, camera),
		modelScaleCommand(sharedData),
		wavCommand(sharedData),
		playWavCommand(sharedData),
		idleCommand(sharedData),
		exprCommand(sharedData)
	{
		mainDialog.setPosition(0, 0);
		renderDialog.setPosition(230, 0);

		window.setMouse(mouse);
		renderDialog.setMouse(mouse);
		mouse.setTrackWindow(renderDialog.getWindowHandle());

		HWND window = mainDialog.getWindowHandle();
		setSliderRange(mainDialog, IDC_MODEL_SCALE, 1, 10);
		setSliderValue(mainDialog, IDC_MODEL_SCALE, 1);
		setSliderRange(mainDialog, IDC_POINT_LIGHT, 0, 10);
		setSliderValue(mainDialog, IDC_POINT_LIGHT, 5);
		setSliderRange(mainDialog, IDC_AMBIENT, 0, 10);
		setSliderValue(mainDialog, IDC_AMBIENT, 5);
		enableCheck(mainDialog, IDC_GLOW, true);

		setSliderRange(mainDialog, IDC_GLOW_FACTOR, 1, 20);
		setSliderValue(mainDialog, IDC_GLOW_FACTOR, 20);

#ifdef TEKES
		setSliderValue(mainDialog, IDC_MODEL_SCALE, 10);
		enableCheck(mainDialog, IDC_GLOW, false);
#endif
	}

	bool ignoreMessage(MSG message)
	{
		// Ignore keys that are used as controls
		if((message.message == WM_KEYDOWN) || (message.message == WM_KEYUP))
		{
			switch(message.wParam)
			{
				case 'W':
				case 'A':
				case 'S':
				case 'D':
				case 'Q':
				case 'E':
				case VK_ADD:
				case VK_SUBTRACT:
				case VK_LEFT:
				case VK_RIGHT:
				case VK_UP:
				case VK_DOWN:
					return true;
			}
		}

		return false;
	}

	void handleMessage()
	{
		MSG windowsMessage = { 0 };
		
		if(GetMessage(&windowsMessage, 0, 0, 0) <= 0)
		{
			mustQuit = true;
			return;
		}

		if(windowsMessage.message == WM_MOUSELAST+1)
			windowsMessage.hwnd = window.getWindowHandle();

		if(ignoreMessage(windowsMessage))
			return;

		TranslateMessage(&windowsMessage);
		DispatchMessage(&windowsMessage);
	}

	void windowsMessages()
	{
		if(!window.isActive())
			handleMessage();

		MSG windowsMessage = { 0 };
		if(PeekMessage(&windowsMessage, 0, 0, 0, PM_NOREMOVE)) 
			handleMessage();

		Sleep(0);
	}

	void tick()
	{
		mouse.update();
		camera.update(mouse, true);
	}
};

Application::Application()
{
	FilePackageManager &manager = FilePackageManager::getInstance();
	boost::shared_ptr<IFilePackage> standardPackage(new StandardPackage());
	manager.addPackage(standardPackage, 0);
	//boost::shared_ptr<IFilePackage> zipPackage(new ZipPackage("Data.zip"));
	//manager.addPackage(zipPackage, 3);

	boost::scoped_ptr<ApplicationData> tempData(new ApplicationData());
	data.swap(tempData);
}

Application::~Application()
{
}

void Application::run(const std::string &fileName)
{
	if(!data->storm.storm)
		return;

	data->storm.scene->DrawBones(true);
	data->camera.setToOrigo();
	data->reloadCommand.execute(0);

	if(!data->storm.storm)
		return;

	int polygonCounter = 0;
	int frameCount = 0;
	int timeValue = 0;

	if(!fileName.empty())
	{
		std::vector<std::string> files;
		files.push_back(fileName);

		data->geometryCommand.execute(files);
	}

/*
// hax
#pragma message("**********************")
#pragma message("** remove **")
#pragma message("**********************")
IStorm3D_Scene *sc = data->storm.storm->CreateNewScene();
sc->SetAmbientLight(COL());
sc->GetCamera()->SetPosition(VC3(0.f,1.f,-6.f));
sc->GetCamera()->SetTarget(VC3(0,1.f,0));
sc->GetCamera()->SetUpVec(VC3(0,1.f,0));
sc->GetCamera()->SetVisibilityRange(100.f);

IStorm3D_Texture *tex = data->storm.storm->getRenderTarget(0);
IStorm3D_Material *m = data->storm.storm->CreateNewMaterial("..");
m->SetAlphaType(IStorm3D_Material::ATYPE_USE_TEXTRANSPARENCY);
m->SetBaseTexture(tex);
*/

	char fpsBuffer[128] =  { 0 };
	int oldTime = timeGetTime();

	//boost::scoped_ptr<IStorm3D_VideoStreamer> streamer;
	//boost::scoped_ptr<IStorm3D_VideoStreamer> streamer2;

//IStorm3D_Texture *cube = data->storm.storm->CreateNewTexture(512, 512, IStorm3D_Texture::TEXTYPE_CUBE_RENDER);
//IStorm3D_Texture *cube2 = data->storm.storm->CreateNewTexture("LobbyCube.dds");
//IStorm3D_Texture *target = data->storm.storm->CreateNewTexture(786, 755, IStorm3D_Texture::TEXTYPE_BASIC_RENDER);
//IStorm3D_Texture *target = data->storm.storm->CreateNewTexture(512, 512, IStorm3D_Texture::TEXTYPE_BASIC_RENDER);
//IStorm3D_Texture *target = data->storm.storm->CreateNewTexture(1024, 1024, IStorm3D_Texture::TEXTYPE_BASIC_RENDER);
//IStorm3D_Material *m = data->storm.storm->CreateNewMaterial("..");
//m->SetBaseTexture(target);

	float dist = 5.f;
	float rad = 9.f;
	float h = 1.5f;
	
	//int l1 = data->storm.terrain->addLight(VC3(0, h, -dist), 0.01f, COL(0, 0, 0));
	//int l2 = data->storm.terrain->addLight(VC3(0, h, -dist), 0.01f, COL(0, 0, 0));
	//int l3 = data->storm.terrain->addLight(VC3(0, h, -dist), 0.01f, COL(0, 0, 0));
	//int l4 = data->storm.terrain->addLight(VC3(0, h, -dist), 0.01f, COL(0, 0, 0));

	//int l1 = data->storm.terrain->addLight(VC3(-dist, h, 0), rad, COL(1.f, 0, 0));
	//int l2 = data->storm.terrain->addLight(VC3( dist, h, 0), rad, COL(0, 1.f, 0));
	//int l3 = data->storm.terrain->addLight(VC3(0, h, -dist), rad, COL(0, 0, 1.f));
	//int l4 = data->storm.terrain->addLight(VC3(0, h,  dist), rad, COL(0, 0.5f, 1.f));

	int l1 = data->storm.terrain->addLight(VC3(0, h, -dist), rad, COL(1.f, 1.f, 1.f));

	while(!data->mustQuit)
	{
		data->tick();
		data->windowsMessages();

		if(!data->window.isActive())
			continue;

		BLEND_TIME = getDialogItemInt(data->sharedData.dialog, IDC_BLENDTIME);

		if(data->storm.terrain)
		{
			IStorm3D_Model *m = data->sharedData.model.getModel();
			/*
			float ambient = 0.f;
			if(isCheckEnabled(data->mainDialog, IDC_LIGHT))
			{
				ambient = .3f;
				if(m)
				{
					float c = 0.3f;
					c /= data->modelScaleCommand.getScale();

					m->SetLighting(VC3(-2.5, 5, -10), COL(c,c,c));
				}
			}
			else
			{
				ambient = .5f;
				if(m)
					m->SetLighting(VC3(), COL());
			}

			if(!isCheckEnabled(data->mainDialog, IDC_AMBIENT))
				ambient = 0.f;
			*/

			float ambient = getSliderValue(data->mainDialog, IDC_AMBIENT) / 20.f;
			if(m)
			{
				float c = getSliderValue(data->mainDialog, IDC_POINT_LIGHT) / 20.f;
				c /= data->modelScaleCommand.getScale();
				//m->SetLighting(0, VC3(-2.5, 5, -10), COL(c,c,c), 50.f);

				m->SetLighting(0, l1);
				//m->SetLighting(1, l2);
				//m->SetLighting(2, l3);
				//m->SetLighting(3, l4);
			}

			data->storm.terrain->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::ForceAmbient, ambient);
			data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Glow, isCheckEnabled(data->mainDialog, IDC_GLOW));
			data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::BetterGlowSampling, isCheckEnabled(data->mainDialog, IDC_IMPROVED_GLOW));
			//data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Reflection, isCheckEnabled(data->mainDialog, IDC_GLOW_DEBUG));
			data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Distortion, isCheckEnabled(data->mainDialog, IDC_DISTORTION));

			if(isCheckEnabled(data->mainDialog, IDC_ANISO))
				data->storm.scene->SetAnisotropicFilteringLevel(8);
			else
				data->storm.scene->SetAnisotropicFilteringLevel(1);

			if(isCheckEnabled(data->mainDialog, IDC_HALF_SPEED))
				data->storm.storm->setGlobalTimeFactor(0.1f);
			//	data->storm.storm->setGlobalTimeFactor(0.4f);
			else
				data->storm.storm->setGlobalTimeFactor(1.0f);

			if(isCheckEnabled(data->mainDialog, IDC_POP_MODE))
			{
				data->storm.terrain->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::GlowFactor, 0.4f);
				data->storm.terrain->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::GlowTransparencyFactor, 0.5f);
				data->storm.terrain->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::GlowAdditiveFactor, 0.0f);
			}
			else
			{
				float glow = getSliderValue(data->mainDialog, IDC_GLOW_FACTOR) / 20.f;
				data->storm.terrain->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::GlowFactor, 0.f);
				data->storm.terrain->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::GlowTransparencyFactor, 0.f);
				data->storm.terrain->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::GlowAdditiveFactor, glow);
			}

			data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::HalfRendering, isCheckEnabled(data->mainDialog, IDC_HALF_RENDERING));
		}
/*
IStorm3D_Model *mod = data->sharedData.model.getModel();
if(mod)
{
	static bool init = false;
	if(!init)
	{
		sc->AddModel(mod);
		init = true;
	}
	sc->SetBackgroundColor(COL(0.2f, 0.2f, 0.2f));
	sc->GetCamera()->SetPosition(data->storm.scene->GetCamera()->GetPosition());
	sc->GetCamera()->SetTarget(data->storm.scene->GetCamera()->GetTarget());
}
sc->RenderSceneToDynamicTexture(tex);
//data->storm.scene->RenderSceneToDynamicTexture(tex);
*/

		int newTime = timeGetTime();
		int delta = newTime - oldTime;
		data->sharedData.updateSound(delta, newTime);

		oldTime = newTime;

		float yAngleDelta = data->mouse.getWheelDelta() / 2000.f;
		yAngleDelta += data->camera.getHorizontal();
		float xAngleDelta = data->camera.getVertical();
		data->model.rotateModel(yAngleDelta, xAngleDelta);

{
	IStorm3D_Model *model = data->model.getModel();
	static bool init = false;
	if(model && !init)
	{
		init = true;

		boost::scoped_ptr<Iterator<IStorm3D_Model_Object *> > object_iterator(model->ITObject->Begin());
		for(; !object_iterator->IsEnd(); object_iterator->Next())
		{
			IStorm3D_Model_Object *o = object_iterator->GetCurrent();
			if(!o)
				continue;
			IStorm3D_Mesh *mesh = o->GetMesh();
			if(!mesh)
				continue;
			IStorm3D_Material *material = mesh->GetMaterial();
			if(!material)
				continue;

			if(!strstr(material->GetName(), "EffectTexture"))
				continue;

			/*
			IStorm3D_Material *newMat = data->storm.storm->CreateNewMaterial(material->GetName());
			newMat->SetBaseTexture(material->GetBaseTexture());
			newMat->SetLocalReflection(true, 0.5f);
			mesh->UseMaterial(newMat);
			*/

			material->SetLocalReflection(true, 0.25f);
			//material->SetLocalReflection(true, 1.f);
		}
	}
}

#ifndef TEKES
		char *stormInfo = data->storm.storm->GetPrintableStatusInfo();
		if(data->sharedData.font2)
			data->storm.scene->Render2D_Text(data->sharedData.font2, VC2(10,40), VC2(10, 18), stormInfo);
		//if(GetKeyState('P') & 0x80)
		//	OutputDebugString(stormInfo);
#endif

#ifndef TEKES
		++frameCount;
		if(newTime - timeValue > 1000)
		{
			int fps = frameCount / ((newTime - timeValue) / 1000);

			timeValue = newTime;
			frameCount = 0;

			sprintf(fpsBuffer, "fps %d, polygons: %d", fps, polygonCounter);
			//SetWindowText(data->window.getWindowHandle(), buffer);
		}

		if(data->sharedData.font)
			data->storm.scene->Render2D_Text(data->sharedData.font, VC2(10,10), VC2(10, 18), fpsBuffer);
#endif


//data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::BlackAndWhite, true);
//data->storm.scene->Render2D_Picture(m, VC2(), VC2(384,384), 1.f);
//data->storm.scene->Render2D_Picture(0, VC2(), VC2(384,384), 1.f, 0, 0,0,2,2);
//data->storm.scene->Render2D_Picture(m, VC2(196,30), VC2(128,128), 1.f);

/*
if(data->storm.storm)
{
	if(!streamer)
		streamer.reset(data->storm.storm->CreateVideoStreamer("Data\\Videos\\introduction_final.wmv", 1.f, true));
	if(!streamer2)
		streamer2.reset(data->storm.storm->CreateVideoStreamer("Data\\Videos\\test.wmv", 1.f, true));

	if(streamer)
	{
		streamer->setPosition(VC2(0.f,200.f), VC2(300.f, 250.f));
		streamer->render(data->storm.scene);
	}
}
*/

		int index = data->sharedData.getSelection();
		BOOL success = FALSE;
		int anim_stop_time = GetDlgItemInt(data->sharedData.dialog.getWindowHandle(), IDC_ANIMTIMER, &success, FALSE);
		if(success &&
			data->sharedData.model.getAnimationTime(data->sharedData.animationState, index) > anim_stop_time)
		{
			data->sharedData.model.setAnimationPaused(true);
		}

		if(data->storm.storm)
		{
			//data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Glow, false);
			//data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Distortion, false);
			//data->storm.scene->RenderSceneToDynamicTexture(target);
			//data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Glow, isCheckEnabled(data->mainDialog, IDC_GLOW));
			//data->storm.terrain->getRenderer().enableFeature(IStorm3D_TerrainRenderer::Distortion, isCheckEnabled(data->mainDialog, IDC_DISTORTION));
		}
		polygonCounter = data->storm.scene->RenderScene();

#ifndef TEKES
		data->storm.storm->DeletePrintableStatusInfo(stormInfo);
#endif
	}

	//delete m;
	delete data->sharedData.font;
	delete data->sharedData.font2;
	//target->Release();
}

} // end of namespace viewer
} // end of namespace frozenbyte
