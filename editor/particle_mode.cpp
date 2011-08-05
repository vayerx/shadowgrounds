
#include <windows.h>
#include <windowsx.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <string>
#include <map>
#include <commctrl.h>
#include <fstream>

#include "particle_mode.h"
#include "icommand.h"
#include "command_list.h"
#include "idlg_handler.h"
#include "dialog.h"
#include "dialog_utils.h"
#include "window.h"
#include "common_dialog.h"
#include "color_component.h"
#include "color_picker.h"
//#include "terrain_mode.h"
#include "storm.h"
#include "gui.h"
#include "mouse.h"
#include "graph.h"
#include "particlefx/particle_typedef.h"
#include "particlefx/vector_track.h"
#include "particlefx/float_track.h"
#include "particlefx/particle_desc.h"
#include "particlefx/emitter_desc.h"
#include "particlefx/particle.h"
#include "particlefx/particle_system.h"
#include "particlefx/particle_system_manager.h"

#include "name_dialog.h"

#include "resource/resource.h"

namespace frozenbyte {
namespace editor {
namespace {

const int WM_UPDATE_DIALOG = WM_USER + 2;

void setGraphFromTrack(Graph& graph, VectorTrack& track) {
	for(int j = 0; j < 3; j++) {	
		graph.setNumKeys(j, track.getNumKeys());
		for(int i = 0; i < track.getNumKeys(); i++) {
			graph.setX(j, i, track.getKeyTime(i));
			if(j == 0) {
				graph.setY(j, i, track.getKeyValue(i).x);
			}
			if(j == 1) {
				graph.setY(j, i, track.getKeyValue(i).y);
			}
			if(j == 2) {
				graph.setY(j, i, track.getKeyValue(i).z);
			}
		}
	}
}

void setGraphFromTrack(Graph& graph, FloatTrack& track) {
	graph.setNumKeys(0, track.getNumKeys());
	for(int i = 0; i < track.getNumKeys(); i++) {
		graph.setX(0, i, track.getKeyTime(i));
		graph.setY(0, i, track.getKeyValue(i));
	}
}

void setTrackFromGraph(VectorTrack& track, Graph& graph) {
	track.setNumKeys(graph.getNumKeys(0));
	for(int i = 0; i < graph.getNumKeys(0); i++) {
		track.setKey(i, graph.getX(0, i), 
			Vector(graph.getY(0, i), graph.getY(1, i), graph.getY(2, i)));
	}
}

void setTrackFromGraph(FloatTrack& track, Graph& graph) {
	track.setNumKeys(graph.getNumKeys(0));
	for(int i = 0; i < graph.getNumKeys(0); i++) {
		track.setKey(i, graph.getX(0, i), graph.getY(0, i));
	}
}








class PSysDialog {
public:

	struct DialogData {
		Dialog& dlg;
		SharedPtr<ParticleSystem> ps;
		DialogData(Dialog& d) : dlg(d) {}
		bool noValueUpdate;
		
		void setPS(SharedPtr<ParticleSystem> _ps)  {
			ps = _ps;
			noValueUpdate = true;
			setDialogItemInt(dlg, IDC_MAX_PARTICLES, ps->getMaxParticles());
			noValueUpdate = false;
		}
		void update() {
			
			if(!noValueUpdate) {
				ps->setMaxParticles(getDialogItemInt(dlg, IDC_MAX_PARTICLES));
			}
		}
	};
	
	class HideCommand : public ICommand {
		Dialog& dlg;
	public:
		HideCommand(Dialog& d) : dlg(d) {}
		void execute() {
			dlg.hide();
		}
	};
	
	class UpdateCommand : public ICommand {
		DialogData& data;
	public:
		UpdateCommand(DialogData& d) : data(d) { }
		void execute() {
			data.update();
		}
	};
	
	Dialog dlg;
	DialogData data;
	HideCommand hideC;
	UpdateCommand updateC;
	
	PSysDialog(Dialog& d) : dlg(IDD_PSYS, d.getWindowHandle()), data(dlg), hideC(dlg), updateC(data) {
		dlg.getCommandList().addCommand(IDC_MAX_PARTICLES, &updateC);
		dlg.getCommandList().addCommand(IDC_PSYS_HIDE, &hideC);
	}
	
	void setPS(SharedPtr<ParticleSystem> ps) {
		data.setPS(ps);
	}
	
	void show() {
		dlg.show();
	}
	void hide() {
		dlg.hide();	
	}
};


class TextureDialog {
public:

	struct TextureDialogData {

		SharedPtr<ParticleDesc> pd;
		bool noValueUpdate;
		Dialog& dlg;
		Storm& storm;

		TextureDialogData(Dialog& d, Storm& s3d) : dlg(d), storm(s3d) {}

		void setParticleDesc(SharedPtr<ParticleDesc> _pd) {

			pd = _pd;

			noValueUpdate = true;
			
			ParticleDesc::TextureInfo& info = pd->texInfo;
			
			HWND hAlpha = GetDlgItem(dlg.getWindowHandle(), IDC_P_ALPHA_TYPE);
			ComboBox_SetCurSel(hAlpha, info.alphaType);
			
			HWND hAnim = GetDlgItem(dlg.getWindowHandle(), IDC_P_ANIM_TYPE);
			ComboBox_SetCurSel(hAnim, info.animType);
/*			
<<<<<<< particle_mode.cpp
			dlg.setItemInt(IDC_P_NUM_FRAMES, info.nFrames);
			dlg.setItemInt(IDC_P_FRAME_WIDTH, info.frameWidth);
			dlg.setItemInt(IDC_P_FRAME_HEIGHT, info.frameHeight);
			dlg.setItemFloat(IDC_P_FPS, info.fps);
			dlg.setItemFloat(IDC_P_START_FRAME, info.startFrame);
		
			dlg.setItemText(IDC_P_TEX_FILE_NAME, info.name);
=======
			//dlg.setItemInt(IDC_P_NUM_FRAMES, info.nFrames);
			//dlg.setItemInt(IDC_P_FRAME_WIDTH, info.frameWidth);
			//dlg.setItemInt(IDC_P_FRAME_HEIGHT, info.frameHeight);
			//dlg.setItemFloat(IDC_P_FPS, info.fps);
			//dlg.setItemFloat(IDC_P_FPS, info.startFrame);
		
			//dlg.setItemText(IDC_P_TEX_FILE_NAME, info.name);
*/
			setDialogItemInt(dlg, IDC_P_NUM_FRAMES, info.nFrames);
			setDialogItemInt(dlg, IDC_P_FRAME_WIDTH, info.columns);
			setDialogItemInt(dlg, IDC_P_FRAME_HEIGHT, info.rows);
			setDialogItemFloat(dlg, IDC_P_FPS, info.fps); // same id? --  psd
			setDialogItemFloat(dlg, IDC_P_START_FRAME, info.startFrame);
			setDialogItemText(dlg, IDC_P_TEX_FILE_NAME, info.name);
//>>>>>>> 1.9

			noValueUpdate = false;

		}

		void update() {
			if(!noValueUpdate)
				updateValues();
		}

		void updateValues() {
			
			ParticleDesc::TextureInfo& info = pd->texInfo;

			//info.frameWidth = dlg.getItemInt(IDC_P_FRAME_WIDTH);
			//info.frameHeight = dlg.getItemInt(IDC_P_FRAME_HEIGHT);
			//info.fps = dlg.getItemFloat(IDC_P_FPS);
			//info.startFrame = dlg.getItemFloat(IDC_P_START_FRAME);

			info.columns = getDialogItemInt(dlg, IDC_P_FRAME_WIDTH);
			info.rows = getDialogItemInt(dlg, IDC_P_FRAME_HEIGHT);
			info.fps = getDialogItemFloat(dlg, IDC_P_FPS);
			info.startFrame = getDialogItemFloat(dlg, IDC_P_START_FRAME);
			info.nFrames = getDialogItemInt(dlg, IDC_P_NUM_FRAMES);
			
			info.animType = ComboBox_GetCurSel(GetDlgItem(dlg.getWindowHandle(), IDC_P_ANIM_TYPE));
			int atype = ComboBox_GetCurSel(GetDlgItem(dlg.getWindowHandle(), IDC_P_ALPHA_TYPE));		

			pd->setAlphaType((IStorm3D_Material::ATYPE)atype);

		}

		void loadTexture() {
			std::string fileName = getOpenFileName("jpg", "data/particles");
			
			//dlg.setItemText(IDC_P_TEX_FILE_NAME, fileName);
			setDialogItemText(dlg, IDC_P_TEX_FILE_NAME, fileName);
			
			if(!fileName.empty())	
				pd->loadTexture(storm.storm, "data/particles/", fileName);
		}

	};

	class LoadTextureCommand : public ICommand {
		TextureDialogData& data;
	public:
		LoadTextureCommand(TextureDialogData& d) : data(d) { }
		void execute() {
			data.loadTexture();
		}
	};

	
	class HideCommand : public ICommand {
		Dialog& dlg;
	public:
		HideCommand(Dialog& d) : dlg(d) {}
		void execute() {
			dlg.hide();
		}
	};
	
	class UpdateCommand : public ICommand {
		TextureDialogData& data;
	public:
		UpdateCommand(TextureDialogData& d) : data(d) { }
		void execute() {
			data.update();
		}
	};

	Dialog dlg;
	LoadTextureCommand loadCommand;
	HideCommand hideCommand;
	TextureDialogData data;
	UpdateCommand updateCommand;

	TextureDialog(Dialog& parent, Storm& storm) : dlg(IDD_PTEX, parent.getWindowHandle()), data(dlg, storm),
		loadCommand(data), hideCommand(dlg), updateCommand(data) {

		HWND hAlpha = GetDlgItem(dlg.getWindowHandle(), IDC_P_ALPHA_TYPE);
		ComboBox_ResetContent(hAlpha);	
		ComboBox_AddString(hAlpha, "none");
		ComboBox_AddString(hAlpha, "use transparency");
		ComboBox_AddString(hAlpha, "use texture transparency");
		ComboBox_AddString(hAlpha, "add");
		ComboBox_AddString(hAlpha, "mul");
		ComboBox_AddString(hAlpha, "mul 2x");
	
		HWND hAnim = GetDlgItem(dlg.getWindowHandle(), IDC_P_ANIM_TYPE);
		ComboBox_ResetContent(hAnim);
		ComboBox_AddString(hAnim, "loop");
		ComboBox_AddString(hAnim, "life time");

		dlg.getCommandList().addCommand(IDC_P_TEXTURE_LOAD, &loadCommand);
		dlg.getCommandList().addCommand(IDC_P_TEXDLG_HIDE, &hideCommand);
		dlg.getCommandList().addCommand(IDC_P_NUM_FRAMES, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_FRAME_WIDTH, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_FRAME_HEIGHT, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_START_FRAME, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_FPS, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_ALPHA_TYPE, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_ANIM_TYPE, &updateCommand);
	
	}

	void setParticleDesc(SharedPtr<ParticleDesc> _pd) {
		data.setParticleDesc(_pd);
	}

	void show() {
		dlg.show();
	}

	void hide() {
		dlg.hide();
	}

};




class ParticleDialog {
public:
	
	
	struct ParticleDialogData {
			
		Dialog& dlg;
		Graph mColorGraph;
		Graph mAlphaGraph;
		Graph mSizeGraph;

		bool bColorGraphChanged;
		bool bSizeGraphChanged;
		bool bAlphaGraphChanged;

		bool noValueUpdate;

		SharedPtr<ParticleDesc> pd;

		Storm& mStorm;

		TextureDialog texDlg;

		ParticleDialogData(Dialog& d, Storm& storm) : dlg(d), mStorm(storm), bColorGraphChanged(true),
			bAlphaGraphChanged(true), bSizeGraphChanged(true), noValueUpdate(true),
			mColorGraph(3), texDlg(dlg, storm) {
		
			texDlg.hide();

			mColorGraph.setChannelColor(0, 255, 0, 0);
			mColorGraph.setChannelColor(1, 0, 255, 0);
			mColorGraph.setChannelColor(2, 0, 0, 255);
			mColorGraph.linkChannels(0, 1);
			mColorGraph.linkChannels(0, 2);
			mColorGraph.linkChannels(1, 2);
			mColorGraph.setWindow(0.0f, 0.0f, 1.0f, 1.0f, 0.1f, 0.1f);
			
			mAlphaGraph.setChannelColor(0, 64, 64, 64);
			mAlphaGraph.setWindow(0.0f, 0.0f, 1.0f, 1.0f, 0.1f, 0.1f);
			
			mSizeGraph.setChannelColor(0, 64, 64, 64);
			mSizeGraph.setWindow(0.0f, 0.0f, 1.0f, 30.0f, 5.0f, 5.0f);
		
		
		}
		
		void showTextureDialog() {

			texDlg.setParticleDesc(pd);
			texDlg.show();

		}

		void setParticleDesc(SharedPtr<ParticleDesc> _pd) {

			pd = _pd;

			noValueUpdate = true;
			
			setGraphFromTrack(mColorGraph, pd->colorTrack);			
			setGraphFromTrack(mSizeGraph, pd->sizeTrack);
			setGraphFromTrack(mAlphaGraph, pd->alphaTrack);
			
			/*
			dlg.setItemText(IDC_P_NAME, pd->getName());
			dlg.setItemFloat(IDC_P_MIN_LIFE, pd->minLife);
			dlg.setItemFloat(IDC_P_MAX_LIFE, pd->maxLife);
			dlg.setItemFloat(IDC_P_MIN_ANGLE, pd->minAngle);
			dlg.setItemFloat(IDC_P_MAX_ANGLE, pd->maxAngle);
			dlg.setItemFloat(IDC_P_MIN_SPIN, pd->minSpin);
			dlg.setItemFloat(IDC_P_MAX_SPIN, pd->maxSpin);
			dlg.setItemFloat(IDC_P_GRAVITY_MULTIPLIER, pd->gravityMultiplier);
			dlg.setItemFloat(IDC_P_BOUNCE, pd->bounce);
			dlg.setItemFloat(IDC_P_DRAG_FACTOR, pd->dragFactor);
			*/
			
			setDialogItemText(dlg, IDC_P_NAME, pd->getName());
			setDialogItemFloat(dlg, IDC_P_MIN_LIFE, pd->minLife);
			setDialogItemFloat(dlg, IDC_P_MAX_LIFE, pd->maxLife);
			setDialogItemFloat(dlg, IDC_P_MIN_ANGLE, pd->minAngle*180/PI);
			setDialogItemFloat(dlg, IDC_P_MAX_ANGLE, pd->maxAngle*180/PI);
			setDialogItemFloat(dlg, IDC_P_MIN_SPIN, pd->minSpin*180/PI);
			setDialogItemFloat(dlg, IDC_P_MAX_SPIN, pd->maxSpin*180/PI);
			setDialogItemFloat(dlg, IDC_P_GRAVITY_MULTIPLIER, pd->gravityMultiplier);
			setDialogItemFloat(dlg, IDC_P_BOUNCE, pd->bounce);
			setDialogItemFloat(dlg, IDC_P_DRAG_FACTOR, pd->dragFactor);

			HWND hCollision = GetDlgItem(dlg.getWindowHandle(), IDC_P_COLLISION_TYPE);
			ComboBox_SetCurSel(hCollision, pd->collisionType);
			
			HWND hDraw = GetDlgItem(dlg.getWindowHandle(), IDC_P_DRAW_STYLE);
			ComboBox_SetCurSel(hDraw, pd->drawStyle);
			
			HWND hDragByVelocity = GetDlgItem(dlg.getWindowHandle(), IDC_P_DRAG_BY_VELOCITY);
			ComboBox_SetCurSel(hDragByVelocity, pd->dragFuncByVelocity);
			
			HWND hDragBySize = GetDlgItem(dlg.getWindowHandle(), IDC_P_DRAG_BY_SIZE);
			ComboBox_SetCurSel(hDragBySize, pd->dragFuncBySize);

			noValueUpdate = false;
		}

		void editColorGraph() {
		
			bColorGraphChanged = true;
			mColorGraph.open(dlg.getWindowHandle());
		
		}

		void editSizeGraph() {

			bSizeGraphChanged = true;
			mSizeGraph.open(dlg.getWindowHandle());

		}

		void editAlphaGraph() {

			bAlphaGraphChanged = true;
			mAlphaGraph.open(dlg.getWindowHandle());

		}
		
		void update() {
			
			if(!noValueUpdate) {
				updateValues();
			}

		}

		void updateGraphs() {

			if(bColorGraphChanged && !mColorGraph.isOpen())
				setTrackFromGraph(pd->colorTrack, mColorGraph);
			
			if(bAlphaGraphChanged && !mAlphaGraph.isOpen())
				setTrackFromGraph(pd->alphaTrack, mAlphaGraph);
			
			if(bSizeGraphChanged && !mSizeGraph.isOpen())
				setTrackFromGraph(pd->sizeTrack, mSizeGraph);

			bColorGraphChanged = false;
			bAlphaGraphChanged = false;
			bSizeGraphChanged = false;

		}

		void updateValues() {
			
			updateGraphs();

			/*
			pd->minLife				= dlg.getItemFloat(IDC_P_MIN_LIFE);
			pd->maxLife				= dlg.getItemFloat(IDC_P_MAX_LIFE);
			pd->minAngle			= dlg.getItemFloat(IDC_P_MIN_ANGLE);
			pd->maxAngle			= dlg.getItemFloat(IDC_P_MAX_ANGLE);
			pd->minSpin				= dlg.getItemFloat(IDC_P_MIN_SPIN);
			pd->maxSpin				= dlg.getItemFloat(IDC_P_MAX_SPIN);
			pd->gravityMultiplier	= dlg.getItemFloat(IDC_P_GRAVITY_MULTIPLIER);
			pd->dragFactor			= dlg.getItemFloat(IDC_P_DRAG_FACTOR);
			pd->bounce				= dlg.getItemFloat(IDC_P_BOUNCE);
			*/

			pd->minLife	= getDialogItemFloat(dlg, IDC_P_MIN_LIFE);
			pd->maxLife	= getDialogItemFloat(dlg, IDC_P_MAX_LIFE);
			pd->minAngle = getDialogItemFloat(dlg, IDC_P_MIN_ANGLE)/180*PI;
			pd->maxAngle = getDialogItemFloat(dlg, IDC_P_MAX_ANGLE)/180*PI;
			pd->minSpin	= getDialogItemFloat(dlg, IDC_P_MIN_SPIN)/180*PI;
			pd->maxSpin	= getDialogItemFloat(dlg, IDC_P_MAX_SPIN)/180*PI;
			pd->gravityMultiplier = getDialogItemFloat(dlg, IDC_P_GRAVITY_MULTIPLIER);
			pd->dragFactor = getDialogItemFloat(dlg, IDC_P_DRAG_FACTOR);
			pd->bounce = getDialogItemFloat(dlg, IDC_P_BOUNCE);

			HWND hCollision = GetDlgItem(dlg.getWindowHandle(), IDC_P_COLLISION_TYPE);
			HWND hDraw = GetDlgItem(dlg.getWindowHandle(), IDC_P_DRAW_STYLE);
			HWND hDragByVelocity = GetDlgItem(dlg.getWindowHandle(), IDC_P_DRAG_BY_VELOCITY);
			HWND hDragBySize = GetDlgItem(dlg.getWindowHandle(), IDC_P_DRAG_BY_SIZE);

			pd->collisionType = ComboBox_GetCurSel(hCollision);
			pd->drawStyle = ComboBox_GetCurSel(hDraw);
			pd->dragFuncByVelocity = ComboBox_GetCurSel(hDragByVelocity);
			pd->dragFuncBySize = ComboBox_GetCurSel(hDragBySize);
		
		}

		void setName(const std::string& str) {

			pd->setName(str);
			
			//dlg.setItemText(IDC_P_NAME, pd->getName());
			setDialogItemText(dlg, IDC_P_NAME, pd->getName());
		}

		const std::string& getName() {
			return pd->getName();
		}

	};
	
	class UpdateCommand : public ICommand {
		ParticleDialogData& data;
	public:
		UpdateCommand(ParticleDialogData& d) : data(d) { }
		void execute() {
			data.update();
		}
	};

	class TextureCommand : public ICommand {
		ParticleDialogData& data;
	public:
		TextureCommand(ParticleDialogData& d) : data(d) {}
		void execute() {
			data.showTextureDialog();
		}
	};

	class ColorGraphCommand : public ICommand {
		ParticleDialogData& data;
	public:
		ColorGraphCommand(ParticleDialogData& d) : data(d) {}
		void execute() {
			data.editColorGraph();
		}
	};
	
	class SizeGraphCommand : public ICommand {
		ParticleDialogData& data;
	public:
		SizeGraphCommand(ParticleDialogData& d) : data(d) {}
		void execute() {
			data.editSizeGraph();
		}
	};

	class AlphaGraphCommand : public ICommand {
		ParticleDialogData& data;
	public:
		AlphaGraphCommand(ParticleDialogData& d) : data(d) {}
		void execute() {
			data.editAlphaGraph();
		}
	};

	class HideCommand : public ICommand {
		Dialog& dlg;
		ParticleDialogData& data;
	public:
		HideCommand(Dialog& d, ParticleDialogData& dat) : dlg(d), data(dat) {}
		void execute() {
			// purkka!!! fix this with more elegant solution
			data.updateGraphs();
			dlg.hide();
		}
	};
	
	class RenameCommand : public ICommand {
		ParticleDialogData& data;
	public:
		RenameCommand(ParticleDialogData& d) : data(d) {}
		void execute() {
			NameDialog dlg;
			dlg.setName(data.getName());
			if(dlg.doModal(NULL, IDD_NAME))
				data.setName(dlg.getName());		
		}
	};

	class UpdateGraphsCommand : public ICommand {
		ParticleDialogData& data;
	public:
		UpdateGraphsCommand(ParticleDialogData& d) : data(d) {}
		void execute() {
			data.updateGraphs();
		}
	};

	Dialog dlg;
	ParticleDialogData data;
	UpdateCommand updateCommand;
	ColorGraphCommand colorCommand;
	SizeGraphCommand sizeCommand;
	AlphaGraphCommand alphaCommand;
	HideCommand hideCommand;
	TextureCommand textureCommand;
	RenameCommand renCommand;
	UpdateGraphsCommand updateGraphs;

	void setParticleDesc(SharedPtr<ParticleDesc> pd) {
		data.setParticleDesc(pd);
	}

	ParticleDialog(Dialog& parent, Storm& storm) : dlg(IDD_PARTICLE, parent.getWindowHandle()), 
		data(dlg, storm), 
		updateCommand(data), colorCommand(data), hideCommand(dlg, data), textureCommand(data),
		renCommand(data), sizeCommand(data), alphaCommand(data), updateGraphs(data) {
	
		HWND hCollision = GetDlgItem(dlg.getWindowHandle(), IDC_P_COLLISION_TYPE);
		ComboBox_ResetContent(hCollision);
		ComboBox_AddString(hCollision, "none");
		ComboBox_AddString(hCollision, "die");
		ComboBox_AddString(hCollision, "bounce");
		
		HWND hDraw = GetDlgItem(dlg.getWindowHandle(), IDC_P_DRAW_STYLE);
		ComboBox_ResetContent(hDraw);
		ComboBox_AddString(hDraw, "point");
		ComboBox_AddString(hDraw, "quad");
		ComboBox_AddString(hDraw, "line");
				
		HWND hDragByVelocity = GetDlgItem(dlg.getWindowHandle(), IDC_P_DRAG_BY_VELOCITY);
		ComboBox_ResetContent(hDragByVelocity);
		ComboBox_AddString(hDragByVelocity, "none");
		ComboBox_AddString(hDragByVelocity, "linear");
		ComboBox_AddString(hDragByVelocity, "quadratic");
			
		HWND hDragBySize = GetDlgItem(dlg.getWindowHandle(), IDC_P_DRAG_BY_SIZE);
		ComboBox_ResetContent(hDragBySize);
		ComboBox_AddString(hDragBySize, "none");
		ComboBox_AddString(hDragBySize, "linear");
		ComboBox_AddString(hDragBySize, "quadratic");
		ComboBox_AddString(hDragBySize, "cubic");

		
		dlg.getCommandList().addCommand(IDC_P_MIN_LIFE, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_MAX_LIFE, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_MIN_SPIN, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_MAX_SPIN, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_GRAVITY_MULTIPLIER, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_DRAG_FACTOR, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_BOUNCE, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_COLLISION_TYPE, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_DRAW_STYLE, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_DRAG_BY_SIZE, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_DRAG_BY_VELOCITY, &updateCommand);
		dlg.getCommandList().addCommand(IDC_P_COLOR, &colorCommand);
		dlg.getCommandList().addCommand(IDC_P_HIDE, &hideCommand);
		dlg.getCommandList().addCommand(IDC_P_TEXTURE, &textureCommand);
//		dlg.getCommandList().addCommand(IDC_P_RENAME, &renCommand);
		dlg.getCommandList().addCommand(IDC_P_SIZE, &sizeCommand);
		dlg.getCommandList().addCommand(IDC_P_ALPHA, &alphaCommand);

	}

	void hide() {
		dlg.hide();
	}

	void show() {
		dlg.show();
	}

	
};


class CloudEmitterDialog {
public:

	class DialogData {
		Dialog& dlg;
		Storm& storm;
		CloudEmitterDesc* ed;
		bool noValueUpdate;
	public:
		
		DialogData(Dialog& d, Storm& s3d) : dlg(d), storm(s3d), ed(0) {}

		void setEmitterDesc(CloudEmitterDesc* _ed) {

			ed = _ed;

			noValueUpdate = true;


			setDialogItemFloat(dlg, IDC_CE_SPHERE_INNER, ed->sInnerRadius);
			setDialogItemFloat(dlg, IDC_CE_SPHERE_OUTER, ed->sOuterRadius);

			setDialogItemFloat(dlg, IDC_CE_CYL_RADIUS, ed->cRadius);
			setDialogItemFloat(dlg, IDC_CE_CYL_HEIGHT, ed->cHeight);

			setDialogItemFloat(dlg, IDC_CE_BOX_MIN_X, ed->bmin.x);
			setDialogItemFloat(dlg, IDC_CE_BOX_MIN_Y, ed->bmin.y);
			setDialogItemFloat(dlg, IDC_CE_BOX_MIN_Z, ed->bmin.z);

			setDialogItemFloat(dlg, IDC_CE_BOX_MAX_X, ed->bmax.x);
			setDialogItemFloat(dlg, IDC_CE_BOX_MAX_Y, ed->bmax.y);
			setDialogItemFloat(dlg, IDC_CE_BOX_MAX_Z, ed->bmax.z);

			setDialogItemFloat(dlg, IDC_CE_DIRECTION_X, ed->direction.x);
			setDialogItemFloat(dlg, IDC_CE_DIRECTION_Y, ed->direction.y);
			setDialogItemFloat(dlg, IDC_CE_DIRECTION_Z, ed->direction.z);

			Button_SetCheck(GetDlgItem(dlg.getWindowHandle(), IDC_CE_RANDOM_DIRECTION),
				ed->randomDirection ? BST_CHECKED : BST_UNCHECKED);
			
			ComboBox_SetCurSel(GetDlgItem(dlg.getWindowHandle(), IDC_CE_SHAPE), ed->shapeType);

			noValueUpdate = false;

			setShape();		
		}

		void update() {
			if(!noValueUpdate)
				updateValues();
		}

		void updateValues() {

			if(ed == NULL)
				return;

			/*
			ed->cRadius = dlg.getItemFloat(IDC_CE_CYL_RADIUS);
			ed->cHeight = dlg.getItemFloat(IDC_CE_CYL_HEIGHT);

			ed->sInnerRadius = dlg.getItemFloat(IDC_CE_SPHERE_INNER);
			ed->sOuterRadius = dlg.getItemFloat(IDC_CE_SPHERE_OUTER);

			ed->bmin.x = dlg.getItemFloat(IDC_CE_BOX_MIN_X);
			ed->bmin.y = dlg.getItemFloat(IDC_CE_BOX_MIN_Y);
			ed->bmin.z = dlg.getItemFloat(IDC_CE_BOX_MIN_Z);
			
			ed->bmax.x = dlg.getItemFloat(IDC_CE_BOX_MAX_X);
			ed->bmax.y = dlg.getItemFloat(IDC_CE_BOX_MAX_Y);
			ed->bmax.z = dlg.getItemFloat(IDC_CE_BOX_MAX_Z);
			*/

			ed->cRadius = getDialogItemFloat(dlg, IDC_CE_CYL_RADIUS);
			ed->cHeight = getDialogItemFloat(dlg, IDC_CE_CYL_HEIGHT);

			ed->sInnerRadius = getDialogItemFloat(dlg, IDC_CE_SPHERE_INNER);
			ed->sOuterRadius = getDialogItemFloat(dlg, IDC_CE_SPHERE_OUTER);

			ed->bmin.x = getDialogItemFloat(dlg, IDC_CE_BOX_MIN_X);
			ed->bmin.y = getDialogItemFloat(dlg, IDC_CE_BOX_MIN_Y);
			ed->bmin.z = getDialogItemFloat(dlg, IDC_CE_BOX_MIN_Z);

			ed->bmax.x = getDialogItemFloat(dlg, IDC_CE_BOX_MAX_X);
			ed->bmax.y = getDialogItemFloat(dlg, IDC_CE_BOX_MAX_Y);
			ed->bmax.z = getDialogItemFloat(dlg, IDC_CE_BOX_MAX_Z);

			ed->minSpeed = getDialogItemFloat(dlg, IDC_CE_MIN_SPEED);
			ed->maxSpeed = getDialogItemFloat(dlg, IDC_CE_MAX_SPEED);

			ed->randomDirection = 
				(Button_GetCheck(GetDlgItem(dlg.getWindowHandle(), IDC_CE_RANDOM_DIRECTION)) == BST_CHECKED) ?
				true : false;

			//ed->shapeType = ComboBox_GetCurSel(GetDlgItem(dlg.getWindowHandle(), IDC_CE_SHAPE));
			setShape();

		}

		void enableBox() {

			/*
			dlg.enableItem(IDC_CE_BOX_MIN_X);
			dlg.enableItem(IDC_CE_BOX_MIN_Y);
			dlg.enableItem(IDC_CE_BOX_MIN_Z);

			dlg.enableItem(IDC_CE_BOX_MAX_X);
			dlg.enableItem(IDC_CE_BOX_MAX_Y);
			dlg.enableItem(IDC_CE_BOX_MAX_Z);
			*/
			enableDialogItem(dlg, IDC_CE_BOX_MIN_X, true);
			enableDialogItem(dlg, IDC_CE_BOX_MIN_Y, true);
			enableDialogItem(dlg, IDC_CE_BOX_MIN_Z, true);

			enableDialogItem(dlg, IDC_CE_BOX_MAX_X, true);
			enableDialogItem(dlg, IDC_CE_BOX_MAX_Y, true);
			enableDialogItem(dlg, IDC_CE_BOX_MAX_Z, true);		
		}

		void disableBox() {
			/*
			dlg.disableItem(IDC_CE_BOX_MIN_X);
			dlg.disableItem(IDC_CE_BOX_MIN_Y);
			dlg.disableItem(IDC_CE_BOX_MIN_Z);

			dlg.disableItem(IDC_CE_BOX_MAX_X);
			dlg.disableItem(IDC_CE_BOX_MAX_Y);
			dlg.disableItem(IDC_CE_BOX_MAX_Z);
			*/
			enableDialogItem(dlg, IDC_CE_BOX_MIN_X, false);
			enableDialogItem(dlg, IDC_CE_BOX_MIN_Y, false);
			enableDialogItem(dlg, IDC_CE_BOX_MIN_Z, false);

			enableDialogItem(dlg, IDC_CE_BOX_MAX_X, false);
			enableDialogItem(dlg, IDC_CE_BOX_MAX_Y, false);
			enableDialogItem(dlg, IDC_CE_BOX_MAX_Z, false);		
		}

		void enableSphere() {
		

			enableDialogItem(dlg, IDC_CE_SPHERE_INNER, true);
			enableDialogItem(dlg, IDC_CE_SPHERE_OUTER, true);
		}

		void disableSphere() {


			enableDialogItem(dlg, IDC_CE_SPHERE_INNER, false);
			enableDialogItem(dlg, IDC_CE_SPHERE_OUTER, false);
		}

		void enableCyl() {

			enableDialogItem(dlg, IDC_CE_CYL_RADIUS, true);
			enableDialogItem(dlg, IDC_CE_CYL_HEIGHT, true);
		}

		void disableCyl() {

			enableDialogItem(dlg, IDC_CE_CYL_RADIUS, false);
			enableDialogItem(dlg, IDC_CE_CYL_HEIGHT, false);
		}

		void setShape() {
			
			if(noValueUpdate)
				return;

			if(!ed)
				return;

			ed->shapeType = ComboBox_GetCurSel(GetDlgItem(dlg.getWindowHandle(), IDC_CE_SHAPE));
			switch(ed->shapeType) {
			case CloudEmitterDesc::SHAPE_BOX:
				{
					enableBox();
					disableCyl();
					disableSphere();
				} break;
			case CloudEmitterDesc::SHAPE_CYLINDER:
				{
					enableCyl();
					disableBox();
					disableSphere();
				} break;
			case CloudEmitterDesc::SHAPE_SPHERE:
				{
					enableSphere();
					disableBox();
					disableCyl();
				} break;
			}

		}

	};

	class UpdateCommand : public ICommand {
		DialogData& data;
	public:
		UpdateCommand(DialogData& d) : data(d) {}
		void execute() {
			data.update();
		}
	};

	class HideCommand : public ICommand {
		Dialog& dlg;
	public:
		HideCommand(Dialog& d) : dlg(d) {}
		void execute() {
			dlg.hide();
		}
	};

	class ShapeCommand : public ICommand {
		DialogData& data;
	public:
		ShapeCommand(DialogData& d) : data(d) {}
		void execute() {
			data.setShape();
		}
	};


	UpdateCommand updateCommand;
	DialogData data;
	Dialog dlg;
	HideCommand hideCommand;
	ShapeCommand shapeCommand;

	CloudEmitterDialog(Dialog& parent, Storm& storm) : dlg(IDD_CLOUD_EMITTER, parent.getWindowHandle()), 
		data(dlg, storm), updateCommand(data), hideCommand(dlg), shapeCommand(data) {
		
		HWND hShape = GetDlgItem(dlg.getWindowHandle(), IDC_CE_SHAPE);
		ComboBox_ResetContent(hShape);
		ComboBox_AddString(hShape, "box");
		ComboBox_AddString(hShape, "sphere");
		ComboBox_AddString(hShape, "cylinder");
		
		dlg.getCommandList().addCommand(IDC_CE_DIRECTION_X, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_DIRECTION_Y, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_DIRECTION_Z, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_BOX_MIN_X, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_BOX_MIN_Y, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_BOX_MIN_Z, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_BOX_MAX_X, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_BOX_MAX_Y, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_BOX_MAX_Z, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_SPHERE_INNER, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_SPHERE_OUTER, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_CYL_RADIUS, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_CYL_HEIGHT, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_RANDOM_DIRECTION, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_MIN_SPEED, &updateCommand);
		dlg.getCommandList().addCommand(IDC_CE_MAX_SPEED, &updateCommand);

		dlg.getCommandList().addCommand(IDC_CE_SHAPE, &shapeCommand);
		dlg.getCommandList().addCommand(IDC_CE_HIDE, &hideCommand);
		
	}

	void open(CloudEmitterDesc* _ed) {
		
		data.setEmitterDesc(_ed);

		show();

	}

	void hide() {
		dlg.hide();
	}

	void show() {
		dlg.show();
	}


};


class PointArrayEmitterDialog {
public:

	class DialogData {
		Dialog& dlg;
		Storm& storm;
		PointArrayEmitterDesc* ed;
		bool noValueUpdate;
	public:
		
		DialogData(Dialog& d, Storm& s3d) : dlg(d), storm(s3d), ed(0) {}

		void setEmitterDesc(PointArrayEmitterDesc* _ed) {

			ed = _ed;

			noValueUpdate = true;

			/*
			dlg.setItemInt(IDC_PA_RANGE_START, ed->rangeStart);
			dlg.setItemInt(IDC_PA_RANGE_END, ed->rangeEnd);
			
			dlg.setItemFloat(IDC_PA_MIN_SPEED, ed->minSpeed);
			dlg.setItemFloat(IDC_PA_MAX_SPEED, ed->maxSpeed);
			*/
			
			setDialogItemInt(dlg, IDC_PA_RANGE_START, ed->rangeStart);
			setDialogItemInt(dlg, IDC_PA_RANGE_END, ed->rangeEnd);

			setDialogItemInt(dlg, IDC_PA_MIN_SPEED, ed->minSpeed);
			setDialogItemInt(dlg, IDC_PA_MAX_SPEED, ed->maxSpeed);

//			Button_SetText(GetDlgItem(dlg.getWindowHandle(), IDC_PA_MODEL), ed->getModelName());

			noValueUpdate = false;
		}

		void update() {
			if(!noValueUpdate)
				updateValues();
		}

		void updateValues() {

			if(ed == NULL)
				return;

			/*
			ed->minSpeed = dlg.getItemFloat(IDC_PA_MIN_SPEED);
			ed->maxSpeed = dlg.getItemFloat(IDC_PA_MAX_SPEED);
			
			ed->rangeStart = dlg.getItemInt(IDC_PA_RANGE_START);
			ed->rangeEnd = dlg.getItemInt(IDC_PA_RANGE_END);
			*/

			ed->minSpeed = getDialogItemFloat(dlg, IDC_PA_MIN_SPEED);
			ed->maxSpeed = getDialogItemFloat(dlg, IDC_PA_MAX_SPEED);
			
			ed->rangeStart = getDialogItemInt(dlg, IDC_PA_RANGE_START);
			ed->rangeEnd = getDialogItemInt(dlg, IDC_PA_RANGE_END);
		}

		void loadModel() {
			std::string fileName = getOpenFileName("s3d", "data/models/");
			ed->loadModel(storm.storm, fileName);
		}

	};

	class UpdateCommand : public ICommand {
		DialogData& data;
	public:
		UpdateCommand(DialogData& d) : data(d) {}
		void execute() {
			data.update();
		}
	};

	class HideCommand : public ICommand {
		Dialog& dlg;
	public:
		HideCommand(Dialog& d) : dlg(d) {}
		void execute() {
			dlg.hide();
		}
	};

	class LoadCommand : public ICommand {
		DialogData& data;
	public:
		LoadCommand(DialogData& d) : data(d) {}
		void execute() {
			data.loadModel();
		}
	};


	UpdateCommand updateCommand;
	DialogData data;
	Dialog dlg;
	HideCommand hideCommand;
	LoadCommand loadCommand;

	PointArrayEmitterDialog(Dialog& parent, Storm& storm) : dlg(IDD_POINT_ARRAY, parent.getWindowHandle()), 
		data(dlg, storm), updateCommand(data), hideCommand(dlg), loadCommand(data) {

		dlg.getCommandList().addCommand(IDC_PA_MIN_SPEED, &updateCommand);
		dlg.getCommandList().addCommand(IDC_PA_MAX_SPEED, &updateCommand);

		dlg.getCommandList().addCommand(IDC_PA_RANGE_START, &updateCommand);
		dlg.getCommandList().addCommand(IDC_PA_RANGE_END, &updateCommand);

		dlg.getCommandList().addCommand(IDC_PA_HIDE, &hideCommand);
		dlg.getCommandList().addCommand(IDC_PA_MODEL, &loadCommand);

	}

	void open(PointArrayEmitterDesc* _ed) {
		
		data.setEmitterDesc(_ed);

		show();

	}

	void hide() {
		dlg.hide();
	}

	void show() {
		dlg.show();
	}


};


class SprayEmitterDialog {
public:

	class SprayEmitterDialogData {
		Dialog& dlg;
		SprayEmitterDesc* ed;
		bool noValueUpdate;
	public:
		
		SprayEmitterDialogData(Dialog& d) : dlg(d), ed(0) {}

		void setEmitterDesc(SprayEmitterDesc* _ed) {

			ed = _ed;

			noValueUpdate = true;

			/*
			dlg.setItemFloat(IDC_SE_MIN_SPEED, ed->minSpeed);
			dlg.setItemFloat(IDC_SE_MAX_SPEED, ed->maxSpeed);

			dlg.setItemFloat(IDC_SE_SPREAD1, ed->spread1);
			dlg.setItemFloat(IDC_SE_SPREAD2, ed->spread2);
			*/
			setDialogItemFloat(dlg, IDC_SE_MIN_SPEED, ed->minSpeed);
			setDialogItemFloat(dlg, IDC_SE_MAX_SPEED, ed->maxSpeed);

			setDialogItemFloat(dlg, IDC_SE_SPREAD1, ed->spread1);
			setDialogItemFloat(dlg, IDC_SE_SPREAD2, ed->spread2);

			noValueUpdate = false;
		}

		void update() {
			if(!noValueUpdate)
				updateValues();
		}

		void updateValues() {

			if(ed == NULL)
				return;

			/*
			ed->minSpeed = dlg.getItemFloat(IDC_SE_MIN_SPEED);
			ed->maxSpeed = dlg.getItemFloat(IDC_SE_MAX_SPEED);

			ed->spread1 = dlg.getItemFloat(IDC_SE_SPREAD1);
			ed->spread2 = dlg.getItemFloat(IDC_SE_SPREAD2);
			*/

			ed->minSpeed = getDialogItemFloat(dlg, IDC_SE_MIN_SPEED);
			ed->maxSpeed = getDialogItemFloat(dlg, IDC_SE_MAX_SPEED);

			ed->spread1 = getDialogItemFloat(dlg, IDC_SE_SPREAD1);
			ed->spread2 = getDialogItemFloat(dlg, IDC_SE_SPREAD2);		
		}

	};

	class UpdateCommand : public ICommand {
		SprayEmitterDialogData& data;
	public:
		UpdateCommand(SprayEmitterDialogData& d) : data(d) {}
		void execute() {
			data.update();
		}
	};

	class HideCommand : public ICommand {
		Dialog& dlg;
	public:
		HideCommand(Dialog& d) : dlg(d) {}
		void execute() {
			dlg.hide();
		}
	};


	UpdateCommand updateCommand;
	SprayEmitterDialogData data;
	Dialog dlg;
	HideCommand hideCommand;

	SprayEmitterDialog(Dialog& parent) : dlg(IDD_SPRAY_EMITTER, parent.getWindowHandle()), 
		data(dlg), updateCommand(data), hideCommand(dlg) {

		dlg.getCommandList().addCommand(IDC_SE_MIN_SPEED, &updateCommand);
		dlg.getCommandList().addCommand(IDC_SE_MAX_SPEED, &updateCommand);

		dlg.getCommandList().addCommand(IDC_SE_SPREAD1, &updateCommand);
		dlg.getCommandList().addCommand(IDC_SE_SPREAD2, &updateCommand);

		dlg.getCommandList().addCommand(IDC_SE_HIDE, &hideCommand);

	}

	void open(SprayEmitterDesc* _ed) {
		
		data.setEmitterDesc(_ed);

		show();

	}

	void hide() {
		dlg.hide();
	}

	void show() {
		dlg.show();
	}

};



class EmitterDialog {
public:

	class ExtendedData {
		SprayEmitterDialog seDlg;
		PointArrayEmitterDialog paDlg;
		CloudEmitterDialog ceDlg;
		SharedPtr<EmitterDesc> ed;
	public:
		
		ExtendedData(Dialog& parent, Storm& s3d) 
			: seDlg(parent) , paDlg(parent, s3d), ceDlg(parent, s3d) {
			seDlg.hide();
			paDlg.hide();
			ceDlg.hide();
		}
		
		void setEmitterDesc(SharedPtr<EmitterDesc> _ed) {
			ed = _ed;
		}
		
		void open() {
			switch(ed->getType()) {
			case ED_SPRAY:
				seDlg.open((SprayEmitterDesc*)ed.get());
				break;
			case ED_POINT_ARRAY:
				paDlg.open((PointArrayEmitterDesc*)ed.get());
				break;
			case ED_CLOUD:
				ceDlg.open((CloudEmitterDesc*)ed.get());
				break;
			}
		}

	};
		
	struct Data {
		
		SharedPtr<EmitterDesc> ed;
		Dialog& dlg;
		Graph mEmitRateGraph;
		bool noValueUpdate;
				
		Data(Dialog& plah) : dlg(plah), noValueUpdate(true) {

			mEmitRateGraph.setChannelColor(0, 64, 64, 64);
			mEmitRateGraph.setWindow(0.0f, 0.0f, 1.0f, 100.0f, 10.0f, 10.0f);

		}

		const std::string& getName() {
			return ed->getName();
		}

		void setName(const std::string& str) {
			
			ed->setName(str);

			setEmitterDesc(ed);


		}
		
		void openGraph() {
			mEmitRateGraph.open(dlg.getWindowHandle());
		}
		
		void setEmitterDesc(SharedPtr<EmitterDesc> _ed) {
			
			ed = _ed;
		
			noValueUpdate = true;
			
			setGraphFromTrack(mEmitRateGraph, ed->emitRateTrack);
			
			/*
			dlg.setItemFloat(IDC_E_MIN_EMIT_TIME, ed->minEmitTime);
			dlg.setItemFloat(IDC_E_MAX_EMIT_TIME, ed->maxEmitTime);
			dlg.setItemText(IDC_E_NAME, ed->getName());
			*/

			setDialogItemFloat(dlg, IDC_E_MIN_EMIT_TIME, ed->minEmitTime);
			setDialogItemFloat(dlg, IDC_E_MAX_EMIT_TIME, ed->maxEmitTime);
			setDialogItemText(dlg, IDC_E_NAME, ed->getName());

			Button_SetCheck(GetDlgItem(dlg.getWindowHandle(), IDC_E_DIE_AFTER_EMISSION),
				ed->dieAfterEmission ? BST_CHECKED : BST_UNCHECKED);
			
			noValueUpdate = false;			
		}

		void update() {

			if(!noValueUpdate)
				updateValues();


		}

		void updateValues() {

				
			ed->dieAfterEmission = 
				(Button_GetCheck(GetDlgItem(dlg.getWindowHandle(), IDC_E_DIE_AFTER_EMISSION)) == BST_CHECKED) ?
				true : false;
			
			//ed->minEmitTime = dlg.getItemFloat(IDC_E_MIN_EMIT_TIME);
			//ed->maxEmitTime = dlg.getItemFloat(IDC_E_MAX_EMIT_TIME);
			ed->minEmitTime = getDialogItemFloat(dlg, IDC_E_MIN_EMIT_TIME);
			ed->maxEmitTime = getDialogItemFloat(dlg, IDC_E_MAX_EMIT_TIME);
		}

		void updateGraphs() {

			setTrackFromGraph(ed->emitRateTrack, mEmitRateGraph);

		}
	};

	class UpdateCommand : public ICommand {
		Data& data;
	public:
		UpdateCommand(Data& d) : data(d) {}
		void execute() {
			data.update();
		}
	};

	class RenameCommand : public ICommand {
		Data& data;
	public:
		RenameCommand(Data& d) : data(d) {}
		void execute() {
			NameDialog dlg;
			dlg.setName(data.getName());
			if(dlg.doModal(NULL, IDD_NAME)) {
				data.setName(dlg.getName());		
			}
		}
	};
	
	class ExtendedCommand : public ICommand {
		ExtendedData& data;
	public:
		ExtendedCommand(ExtendedData& d) : data(d) {}
		void execute() {
			data.open();	
		}
	};

	class GraphCommand : public ICommand {
		Data& data;
	public:
		GraphCommand(Data& d) : data(d) {}
		void execute() {
			data.openGraph();
		}
	};

	class HideCommand : public ICommand {
		Dialog& dlg;
		Data& data;
	public:
		HideCommand(Dialog& d, Data& dat) : dlg(d), data(dat) {}
		void execute() {
			dlg.hide();
			data.updateGraphs();
		}
	};

	
	Data data;
	Dialog dlg;
	UpdateCommand updateCommand;
	RenameCommand renameCommand;
	ExtendedData eData;
	ExtendedCommand eCommand;
	GraphCommand graphCommand;
	HideCommand hideCommand;

	EmitterDialog(Dialog& parent, Storm& storm) : 
		dlg(IDD_EMITTER, parent.getWindowHandle()), data(dlg), updateCommand(data),
		eData(dlg, storm), eCommand(eData), renameCommand(data), graphCommand(data), 
		hideCommand(dlg, data) {

		dlg.getCommandList().addCommand(IDC_E_MIN_EMIT_TIME, &updateCommand);
		dlg.getCommandList().addCommand(IDC_E_MAX_EMIT_TIME, &updateCommand);
		dlg.getCommandList().addCommand(IDOK, &updateCommand);
		dlg.getCommandList().addCommand(IDC_E_MORE, &eCommand);
//		dlg.getCommandList().addCommand(IDC_E_RENAME, &renameCommand);
		dlg.getCommandList().addCommand(IDC_EMITTER_EMITRATE, &graphCommand);
		dlg.getCommandList().addCommand(IDC_E_HIDE, &hideCommand);

		dlg.hide();
	}

	void setEmitterDesc(SharedPtr<EmitterDesc> _ed) {
		data.setEmitterDesc(_ed);
		eData.setEmitterDesc(_ed);
	}

	void hide() {
		dlg.hide();
	}

	void show() {
		dlg.show();
	}

};


class SelDialog {

	std::vector< SharedPtr<EmitterDesc> >& eds;
	std::vector< SharedPtr<ParticleDesc> >& pds;
	int ed, pd;

public:
	SelDialog(std::vector< SharedPtr<EmitterDesc> >& _eds,
		std::vector< SharedPtr<ParticleDesc> >& _pds) : eds(_eds), pds(_pds) {

		}

	int getED() {
		return ed;
	}

	int getPD() {
		return pd;
	}

	static BOOL CALLBACK func(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		static SelDialog* dlg = NULL;
		switch(msg) {
		case WM_INITDIALOG:
			{
				dlg = (SelDialog*)lParam;
				HWND h1 = GetDlgItem(hwnd, IDC_SELECTION1);
				HWND h2 = GetDlgItem(hwnd, IDC_SELECTION2);	
				ComboBox_ResetContent(h1);
				ComboBox_ResetContent(h2);
				int i;
				for(i = 0; i < dlg->eds.size(); i++) {
					ComboBox_AddString(h1, dlg->eds[i]->getName().c_str());
				}
				for(i = 0; i < dlg->pds.size(); i++) {
					ComboBox_AddString(h2, dlg->pds[i]->getName().c_str());
				}
				ComboBox_SetCurSel(h1, 0);
				ComboBox_SetCurSel(h2, 0);
				return TRUE;
			} break;
		case WM_COMMAND:
			{
				if(LOWORD(wParam)==IDOK) {
					HWND h1 = GetDlgItem(hwnd, IDC_SELECTION1);
					HWND h2 = GetDlgItem(hwnd, IDC_SELECTION2);	
					dlg->ed = ComboBox_GetCurSel(h1);
					dlg->pd = ComboBox_GetCurSel(h2);
					EndDialog(hwnd, IDOK);
					return TRUE;
				}
				if(LOWORD(wParam)==IDCANCEL) {
					EndDialog(hwnd, IDCANCEL);
					return TRUE;
				}
			} break;
		}
		return FALSE;
	}

	bool doModal(HWND parent) {

		if(IDOK == DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SELECTION), parent,
			func, (LPARAM)this))
			return true;

		return false;
	}
	

};


class SingleSelDialog {
	std::vector<std::string> sels;
	int sel;
public:
	void addSel(const std::string& str) { sels.push_back(str); }
	const std::string& getText() {
		return sels[sel];
	}
	int getSel() {
		return sel;
	}
	
	static BOOL CALLBACK msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		static SingleSelDialog* dlg = NULL;
		switch(msg) {
		case WM_INITDIALOG:
			{
				dlg = (SingleSelDialog*)lParam;
				HWND h1 = GetDlgItem(hwnd, IDC_SEL);
				ComboBox_ResetContent(h1);
				int i;
				for(i = 0; i < dlg->sels.size(); i++) {
					ComboBox_AddString(h1, dlg->sels[i].c_str());
				}
				ComboBox_SetCurSel(h1, dlg->sel);
				return TRUE;
			} break;
		case WM_COMMAND:
			{
				if(LOWORD(wParam)==IDOK) {
					HWND h1 = GetDlgItem(hwnd, IDC_SEL);
					dlg->sel = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_SEL));
					EndDialog(hwnd, IDOK);
					return TRUE;
				}
				if(LOWORD(wParam)==IDCANCEL) {
					EndDialog(hwnd, IDCANCEL);
					return TRUE;
				}
			} break;
		}
		return FALSE;
	}
	
	bool doModal(HWND parent) {

		sel = 0;

		if(IDOK == DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SINGLE_SEL), parent,
			msgProc, (LPARAM)this))
			return true;

		return false;
	}
};


HTREEITEM tvAddChildItem(HWND hTree, HTREEITEM parent, const std::string& name) {

	TV_INSERTSTRUCT tvinsert;   
	
	tvinsert.hParent=parent;		
	tvinsert.hInsertAfter=TVI_LAST;
	tvinsert.item.mask=TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tvinsert.item.pszText=const_cast<char*>(name.c_str());
	tvinsert.item.iImage=0;
	tvinsert.item.iSelectedImage=1;
	
	return TreeView_InsertItem(hTree, &tvinsert);
}

HTREEITEM tvAddRootItem(HWND hTree, const std::string& name) {
	
	TV_INSERTSTRUCT tvinsert;   
	
	tvinsert.hParent=NULL;		
	tvinsert.hInsertAfter=TVI_ROOT;
	tvinsert.item.mask=TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	tvinsert.item.pszText=const_cast<char*>(name.c_str());
	tvinsert.item.iImage=0;
	tvinsert.item.iSelectedImage=1;
	
	return TreeView_InsertItem(hTree, &tvinsert);
}



class ParticleSystemDialog {
public:


	class Item {
	public:
				
		virtual ~Item() {}
		virtual int getType() { return 0; }
				
		virtual bool newChildItem(std::map<HTREEITEM, Item*>& imap, HWND hTree, HTREEITEM self)=0;
		virtual bool delItem(std::map<HTREEITEM, Item*>& imap, HWND hTree, HTREEITEM self)=0;
		virtual void editItem(std::map<HTREEITEM, Item*>& imap, HWND hTree, HTREEITEM self)=0;
				
	};

	typedef std::map<HTREEITEM, Item*> ItemMap;	

	struct SharedData {
		ParticleSystemManager& mgr;
		Storm& storm;
		std::vector< SharedPtr<ParticleSystem> > temps;
		std::vector< SharedPtr<EmitterDesc> > emitters;
		std::vector< SharedPtr<ParticleDesc> > particles;
		ParticleDialog particleDialog;
		EmitterDialog emitterDialog;
		PSysDialog psysDialog;
		int selectedTemplate;
		
		SharedData(Dialog& dlg, ParticleSystemManager& _mgr, Storm& s) : 
			storm(s), mgr(_mgr), particleDialog(dlg, storm), emitterDialog(dlg, storm),
			selectedTemplate(-1), psysDialog(dlg) {
			
				emitterDialog.hide();
				particleDialog.hide();
				psysDialog.hide();
		}
		
		std::string getNewTemplateName(const std::string& name) {
			
			int n = 0;
			std::string str;
			while(1) {
				str = name;
				str += boost::lexical_cast<std::string>(n);
				for(int i = 0; i < temps.size(); i++) {
					if(temps[i]->getTemplateName()==str) {
						break;
					}
				}
				if(i != temps.size()) {
					n++;
				} else {
					break;
				}
			}

			return str;
		}

		

	};
	
	class EDItem : public Item {
		int id;
		SharedData& sd;
	public:
		EDItem(SharedData& data, int _id) : sd(data), id(_id) {}
		bool newChildItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			return false;
		}
		bool delItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			sd.emitters.erase(sd.emitters.begin()+id);
			return true;
		}
		void editItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			sd.emitterDialog.setEmitterDesc(sd.emitters[id]);
			sd.emitterDialog.show();
		}
	};

	class PDItem : public Item {
		int id;
		SharedData& sd;
	public:
		PDItem(SharedData& data, int _id) : sd(data), id(_id) {}
		bool newChildItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			return false;
		}
		bool delItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			sd.particles.erase(sd.particles.begin()+id);
			return true;
		}
		void editItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			sd.particleDialog.setParticleDesc(sd.particles[id]);
			sd.particleDialog.show();
		}
	};
	
	class EmitterListItem : public Item {
		SharedData& sd;
		HWND hDlg;
	public:
		EmitterListItem(SharedData& data, HWND dlg) : sd(data), hDlg(dlg) {

		}
		bool newChildItem(ItemMap& imap, HWND hTree, HTREEITEM self) {

			int n = 0;
			std::string str;
			while(1) {
				str = "emitter";
				str += boost::lexical_cast<std::string>(n);
				for(int i = 0; i < sd.emitters.size(); i++) {
					if(sd.emitters[i]->getName()==str) {
						break;
					}
				}
				if(i != sd.emitters.size()) {
					n++;
				} else {
					break;
				}
			}
			
			NameDialog ndlg;
			ndlg.setName(str);
			if(ndlg.doModal(hDlg, IDD_NAME)) {
				str = ndlg.getName();
			} else {
				return false;
			}

			EmitterDesc* _ed = NULL;
			SingleSelDialog selDlg;
			selDlg.addSel("spray emitter");
			selDlg.addSel("point array emitter");
			selDlg.addSel("cloud emitter");
			if(selDlg.doModal(hDlg)) {
				if(selDlg.getSel()==0)
					_ed = new SprayEmitterDesc();
				if(selDlg.getSel()==1)
					_ed = new PointArrayEmitterDesc();
				if(selDlg.getSel()==2)
					_ed = new CloudEmitterDesc();
			} else {
				return false;
			}
						
			SharedPtr<EmitterDesc> ed(_ed);
			ed->setName(str);
			sd.emitters.push_back(ed);

			HTREEITEM id = tvAddChildItem(hTree, self, str);
			EDItem* it = new EDItem(sd, sd.emitters.size()-1);
			imap[id] = it;
			return true;
		}
		bool delItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			return false; // cant delete root
		}
		void editItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
		
		}
	};

	class ParticleListItem : public Item {
		SharedData& sd;
		HWND hDlg;
	public:
		ParticleListItem(SharedData& data, HWND dlg) : sd(data), hDlg(dlg) {
			
		}
		bool newChildItem(ItemMap& imap, HWND hTree, HTREEITEM self) {

			int n = 0;
			std::string str;
			while(1) {
				str = "particle";
				str += boost::lexical_cast<std::string>(n);
				for(int i = 0; i < sd.particles.size(); i++) {
					if(sd.particles[i]->getName()==str) {
						break;
					}
				}
				if(i != sd.particles.size()) {
					n++;
				} else {
					break;
				}
			}

			NameDialog ndlg;
			ndlg.setName(str);
			if(ndlg.doModal(hDlg, IDD_NAME)) {
				str = ndlg.getName();
			} else {
				return false;
			}
			
			
			SharedPtr<ParticleDesc> pd(new ParticleDesc());
			pd->setName(str);
			sd.particles.push_back(pd);

			HTREEITEM id = tvAddChildItem(hTree, self, str);
			PDItem* it = new PDItem(sd, sd.particles.size()-1);
			imap[id] = it;
			return true;
		}
		bool delItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			return false; // cant delete root
		}
		void editItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			
		}
	};
	
	class TemplateListItem : public Item {
		SharedData& sd;
		HWND hDlg;
	public:
		TemplateListItem(SharedData& data, HWND dlg) : sd(data), hDlg(dlg) {}
		
		
		bool newChildItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			
			std::string str = sd.getNewTemplateName("template");
			
			NameDialog ndlg;
			ndlg.setName(str);
			if(ndlg.doModal(hDlg, IDD_NAME)) {
				str = ndlg.getName();
			} else {
				return false;
			}
					
			SharedPtr<ParticleSystem> ps = sd.mgr.addTemplate(str);
			sd.temps.push_back(ps);
			
			
			HTREEITEM id = tvAddChildItem(hTree, self, str);
			TemplateItem* temp = new TemplateItem(sd, sd.temps.size()-1, ps, hDlg);
			imap[id] = temp;
			ComboBox_AddString(GetDlgItem(hDlg, IDC_SELECTED_TEMPLATE), str.c_str());
			int sel = ComboBox_FindString(GetDlgItem(hDlg, IDC_SELECTED_TEMPLATE), 0, str.c_str());
			ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_SELECTED_TEMPLATE), sel);
			return true;
		}
		bool delItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			return false; // cant delete root
		}
		void editItem(ItemMap& imap, HWND hTree, HTREEITEM self) {

		}
		
	};

	class TemplateItem : public Item {
		SharedData& sd;
		HWND hDlg;
		int id;
	public:
		SharedPtr<ParticleSystem> ps;

		TemplateItem(SharedData& data, int _id, SharedPtr<ParticleSystem> _ps, HWND dlg) : sd(data),
			ps(_ps), hDlg(dlg), id(_id) {}
		
		int getType() { return 1; }

		bool newChildItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			SelDialog dlg(sd.emitters, sd.particles);
			if(dlg.doModal(hDlg)) {
				int sel1 = dlg.getED();
				int sel2 = dlg.getPD();
				ps->addEmitter(sd.emitters[sel1], sd.particles[sel2]);

				TV_INSERTSTRUCT tvinsert;   
				
				std::string str = sd.emitters[sel1]->getName();
				str += "/";
				str += sd.particles[sel2]->getName();
							
				HTREEITEM id = tvAddChildItem(hTree, self, str);
				EmitterItem* eItem = new EmitterItem(ps, ps->getNumEmitters()-1);
				imap[id] = eItem;
				return true;
			}
			return false;
		}
		bool delItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			
			sd.mgr.removeTemplate(ps->getTemplateName());
			
			sd.temps.erase(sd.temps.begin()+id);
			
			int sel = ComboBox_FindString(GetDlgItem(hDlg, IDC_SELECTED_TEMPLATE), 0,
				ps->getTemplateName().c_str());
			ComboBox_DeleteString(GetDlgItem(hDlg, IDC_SELECTED_TEMPLATE), sel);
			ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_SELECTED_TEMPLATE), 0);
			
			return true;
		}
		void editItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			
			sd.psysDialog.setPS(sd.temps[id]);
			sd.psysDialog.show();

		}


	};

	class EmitterItem : public Item {
		int id;
		SharedPtr<ParticleSystem> ps;
	public:
		EmitterItem(SharedPtr<ParticleSystem> _ps, int _id) : ps(_ps), id(_id) {}
		bool newChildItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			return false;
		}
		bool delItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			ps->removeEmitter(id);
			return true;
		}
		void editItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
		}
	};


	class PSItem : public Item {
	public:
		SharedPtr<ParticleSystem> ps;
		HTREEITEM hItem;
		PSItem(SharedPtr<ParticleSystem> _ps, HTREEITEM i) : ps(_ps), hItem(i) {}

		bool newChildItem(ItemMap& imap, HWND hTree, HTREEITEM self) {			
			return false;
		}

		bool delItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
			ps->kill();
			return false;
		}
		void editItem(ItemMap& imap, HWND hTree, HTREEITEM self) {
		}		
	};
	
	class MyHandler : public IDlgHandler {
		
		HTREEITEM hCurItem;
		HTREEITEM hEmitterList;
		HTREEITEM hParticleList;
		HTREEITEM hTemplateList;
		HTREEITEM hPSList;

		SharedData data;
		TemplateListItem tempList;
		EmitterListItem emitterList;
		ParticleListItem particleList;
		Dialog& dlg;
		HIMAGELIST hImageList;
		HBITMAP hBitMap;
		ItemMap itemMap;
		std::vector< PSItem* > psItems;
		ParticleSystemManager& mgr;
		Item* curItem;

	public:

		
		void launch(const Vector& pos, const Vector& vel) {

			char buffer[256];
			ComboBox_GetText(dlg.getItem(IDC_SELECTED_TEMPLATE), buffer, 256);
			std::string name = buffer;
			if(!name.empty()) {
				SharedPtr<ParticleSystem> ps = mgr.spawnParticleSystem(name);
				Matrix tm;
				tm.CreateTranslationMatrix(pos);
				ps->setTM(tm);
				ps->setVelocity(vel);
											
				HTREEITEM id = tvAddChildItem(GetDlgItem(dlg.getWindowHandle(), IDC_TREE1), 
					hPSList, ps->getTemplateName());
				
				//TreeView_SelectItem(GetDialogItem(dlg.getWindowHandle(), IDC_TREE1), id);
				PSItem* i = new PSItem(ps, id);
				itemMap[id] = i;
				psItems.push_back(i);
			
			}

		
		}
		
		void update() {
		
			if(mgr.getNumParticleSystems() != psItems.size()) {
				for(int i = 0; i < psItems.size(); i++) {
					if(psItems[i]->ps->isDead()) {
						TreeView_DeleteItem(dlg.getItem(IDC_TREE1), psItems[i]->hItem);
						ItemMap::iterator it = itemMap.find(psItems[i]->hItem);
						if(it != itemMap.end()) {
							delete it->second;
							itemMap.erase(it);
						}
					}
					psItems.erase(psItems.begin()+i);
				}
			}
			
		}
		
		MyHandler(Dialog& _dlg, ParticleSystemManager& _mgr, Storm& storm) :
		dlg(_dlg),
		mgr(_mgr),
		data(_dlg, _mgr, storm),
		curItem(NULL), hCurItem(NULL),
		tempList(data, _dlg.getWindowHandle()), 
		emitterList(data, _dlg.getWindowHandle()), 
		particleList(data, _dlg.getWindowHandle()) {
		
			
			InitCommonControls();	    // make our tree control to work
			HWND hTree=dlg.getItem(IDC_TREE1);
			// creating image list and put it into the tree control
			//====================================================//
			hImageList=ImageList_Create(16,16,ILC_COLOR16,2,10);					  // Macro: 16x16:16bit with 2 pics [array]
			hBitMap=LoadBitmap(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_BITMAP1));					  // load the picture from the resource
			ImageList_Add(hImageList,hBitMap,NULL);								      // Macro: Attach the image, to the image list
			DeleteObject(hBitMap);													  // no need it after loading the bitmap
			SendDlgItemMessage(dlg.getWindowHandle(),IDC_TREE1,TVM_SETIMAGELIST,0,(LPARAM)hImageList); // put it onto the tree control
						
			hTemplateList = tvAddRootItem(hTree, "templates");
			itemMap[hTemplateList] = &tempList;
					
			hEmitterList = tvAddRootItem(hTree, "emitters");
			itemMap[hEmitterList] = &emitterList;
			
			hParticleList = tvAddRootItem(hTree, "particles");
			itemMap[hParticleList] = &particleList;
		
			hPSList = tvAddRootItem(hTree, "particle systems in scene");

		}
				
		BOOL handleMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
									
			HWND hTree = GetDlgItem(hwnd, IDC_TREE1);
			
			switch(msg) {
				
			case WM_COMMAND:
				{
					
					switch(LOWORD(wParam)) {
					case IDM_NEW:
						{
							if(curItem != NULL)
								curItem->newChildItem(itemMap, hTree, hCurItem);											
						} break;
					case IDM_SAVE:
						{
							//curItem->saveItem(itemMap, hTree, hCurItem);
						} break;
					case IDM_EDIT:
						{
							if(curItem != NULL)
								curItem->editItem(itemMap, hTree, hCurItem);
						} break;
					case IDM_DELETE:
						{
							if(curItem != NULL)
							if(curItem->delItem(itemMap, hTree, hCurItem)) {
								delete curItem;
								curItem = NULL;
								TreeView_DeleteItem(hTree, hCurItem);
								ItemMap::iterator it = itemMap.find(hCurItem);
								if(it != itemMap.end()) {
									itemMap.erase(it);
								}
							}												
						} break;
					case IDC_EXPORT_TEMPLATE:
						{


							char buffer[256];
							ComboBox_GetText(dlg.getItem(IDC_SELECTED_TEMPLATE), buffer, 256);
							std::string name = buffer;
							if(!name.empty()) {

								std::string fileName = getSaveFileName("fbp", "data/particles");
								if(fileName.empty())
									return false;

								std::ofstream os;
								os.open(fileName.c_str());
								SharedPtr<ParticleSystem> ps = mgr.getTemplate(name);
								os << *ps;
								os.close();
							}
								

						} break;
					case IDC_IMPORT_TEMPLATE:
						{

							std::string tstr = data.getNewTemplateName("template");
							
							NameDialog ndlg;
							ndlg.setName(tstr);
							if(ndlg.doModal(hwnd, IDD_NAME)) {
								tstr = ndlg.getName();
							} else {
								return false;
							}

							std::string fileName = getOpenFileName("fbp", "data/particles");
							if(fileName.empty())
								return false;
							
							SharedPtr<ParticleSystem> ps = mgr.addTemplate(tstr);

							std::ifstream file;
							file.open(fileName.c_str());
							file >> (*ps.get());
							file.close();
						
							
							data.temps.push_back(ps);
							TemplateItem* temp = new TemplateItem(data, data.temps.size()-1, ps, hwnd);
							HTREEITEM hParent = tvAddChildItem(hTree, hTemplateList, ps->getTemplateName());
							itemMap[hParent] = temp;
							
							for(int i = 0; i < ps->getNumEmitters(); i++) {
								
								data.emitters.push_back(ps->getEmitterDesc(i));
								data.particles.push_back(ps->getParticleDesc(i));
								
								HTREEITEM id;
								
								EDItem* ed = new EDItem(data, data.emitters.size()-1);
								id = tvAddChildItem(hTree, hEmitterList, ps->getEmitterDesc(i)->getName());
								itemMap[id] = ed;
								
								PDItem* pd = new PDItem(data, data.particles.size()-1);
								id = tvAddChildItem(hTree, hParticleList, ps->getParticleDesc(i)->getName());
								itemMap[id] = pd;
								
								EmitterItem* ei = new EmitterItem(ps, i);
								std::string str = ps->getEmitterDesc(i)->getName();
								str += "/";
								str += ps->getParticleDesc(i)->getName();
								id = tvAddChildItem(hTree, hParent, str);
								itemMap[id] = ei;
								
							}
							
							ComboBox_AddString(GetDlgItem(hwnd, IDC_SELECTED_TEMPLATE), tstr.c_str());
							int sel = ComboBox_FindString(GetDlgItem(hwnd, IDC_SELECTED_TEMPLATE), 0, tstr.c_str());
							ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_SELECTED_TEMPLATE), sel);
							
						} break;
					}
				} break;
			
			case WM_NOTIFY:
			{
			case IDC_TREE1:
				
				if(((LPNMHDR)lParam)->code == NM_DBLCLK) 
				{				
					hCurItem = TreeView_GetSelection(hTree);
					if(hCurItem != NULL) {
						TreeView_EnsureVisible(hwnd, hCurItem);
						TreeView_SelectItem(hwnd, hCurItem);
						std::map< HTREEITEM, Item* >::iterator it = itemMap.find(hCurItem);
						if(it != itemMap.end()) {
							curItem = it->second;
						} else {
							curItem = NULL;
						}
					}
				}
								
				if(((LPNMHDR)lParam)->code == NM_RCLICK) 
				{

					hCurItem = TreeView_GetSelection(hTree);
					if(hCurItem != NULL) {
						TreeView_EnsureVisible(hwnd, hCurItem);
						TreeView_SelectItem(hwnd, hCurItem);
						std::map< HTREEITEM, Item* >::iterator it = itemMap.find(hCurItem);
						if(it != itemMap.end()) {
							curItem = it->second;

								
								POINT p;
								GetCursorPos(&p);
								TrackPopupMenuEx( GetSubMenu( LoadMenu( 0, MAKEINTRESOURCE(IDR_POPUP) ), 0 ),   
									TPM_VERTICAL, p.x, p.y, hwnd, NULL );
							
							
							
							

						} else {
							curItem = NULL;
						}
					}
				}

			} break;

			}
			
			return FALSE;
			
		}


	};
	
	Dialog& dlg;
	MyHandler handler;
	ParticleSystemManager& mgr;

	ParticleSystemDialog(Dialog& _dlg, ParticleSystemManager& _mgr, Storm& storm) :
	dlg(_dlg), handler(_dlg, _mgr, storm), mgr(_mgr) {
	
		dlg.setCustomHandler(&handler);
		
	}

	void update() {
		handler.update();
	}
	
	void launch(const Vector& pos, const Vector& vel) {
		handler.launch(pos, vel);
	}



};


class AddPSCommand : public ICommand {
public:
	bool launch;

	AddPSCommand() : launch(false) {}
	void execute() {
		launch = !launch;
	}
};



}


struct ParticleModeData
{

	ParticleSystemManager mManager;
	AddPSCommand addPSCommand;

	
	ParticleSystemDialog sysDlg;

	boost::shared_ptr<IStorm3D_Model> mModel;
	Storm& mStorm;
	Gui& gui;

	Vector mLaunchPos;
	Vector mLaunchVel;


	ParticleModeData(Dialog& dialog, Gui& _gui, Storm& storm) : mStorm(storm), gui(_gui), 
		mManager(storm.storm, storm.scene), sysDlg(dialog, mManager, storm) {
		
		dialog.getCommandList().addCommand(IDC_ADD_PS, &addPSCommand);
		
		loadModel();
	}

	~ParticleModeData() {

	}

	bool handleDialogs(MSG& msg) {
		
		if(IsDialogMessage(gui.getParticleDialog().getWindowHandle(), &msg))
			return true;

		return false;
	}

	void loadModel() {

		mModel = SharedPtr<IStorm3D_Model> (mStorm.storm->CreateNewModel());
		mModel->LoadS3D("data/models/pointers/cone.s3d");
		
	}

	void update() {
	}
	
	void tick() {

		static int nClicks = 0;
		static DWORD oldTime = timeGetTime();

		sysDlg.update();
		
		mStorm.scene->RemoveModel(mModel.get());
		
		Mouse &mouse = gui.getMouse();
		if(!mouse.isInsideWindow())
			return;
		
		if(!mModel)
			return;
		
		Vector p, d;
		VC2I screen(mouse.getX(), mouse.getY());
		
		IStorm3D_Scene *s = mStorm.scene;
		s->GetEyeVectors(screen, p, d);
		
		Storm3D_CollisionInfo ci;
		ObstacleCollisionInfo oi;
		
		ci.hit = false;
		oi.hit = false;

		mLaunchVel = Vector(0.0f, 0.0f, 0.0f);
		
		IStorm3D_Terrain* terrain = mStorm.terrain;
		if(terrain) {

			terrain->RayTrace(p, d, 200.f, ci, oi, true);
			
			if(!ci.hit)
				return;
			
			ci.position.y = terrain->GetHeightAt(VC2(ci.position.x, ci.position.z));
			
			mLaunchPos = ci.position;
			mLaunchPos.y += 0.5f;
			
			mModel->SetPosition(ci.position);
			mStorm.scene->AddModel(mModel.get());

		} else {

			mLaunchPos = Vector(0.0f, 100.0f, 0.0f);			

		}
				
		if(mouse.hasLeftClicked() && addPSCommand.launch)
		{
/*		
			// launch ps
			if(nClicks == 0) {
				// set position				
				mLaunchPos = ci.position;
				mLaunchPos.y += 10.0f;
			}

			if(nClicks == 1) {
				// set velocity
				mLaunchVel = Vector(0.0f, 1.0f, 0.0f);
			}

			if(nClicks == 2) {
				// launch
				if(mEditor.mPS) {
					boost::shared_ptr<ParticleSystem> temp(new ParticleSystem("blah", mStorm.storm, mStorm.scene));
					*temp = *mEditor.mPS;
					temp->setPosition(mLaunchPos);
					temp->setVelocity(mLaunchVel);
					mManager->addParticleSystem(temp);					
				}
				addPSCommand.execute();
				nClicks = 0;
			}

			nClicks++;
*/
		
			//SharedPtr<ParticleSystem> temp(new ParticleSystem("blah", mStorm.storm, mStorm.scene));
			//*temp = *mEditor.mPS;
			//temp->setPosition(mLaunchPos);
			//temp->setVelocity(mLaunchVel);
			//mManager->addParticleSystem(temp);					

			//mEditor.launchPS(mLaunchPos, mLaunchVel);
			sysDlg.launch(mLaunchPos, mLaunchVel);

			addPSCommand.execute();

		}
		
		DWORD time = timeGetTime();
		int step = time - oldTime;
		oldTime = time;

		mManager.tick(step);

		mManager.render();

	}

};




ParticleMode::ParticleMode(Gui &gui, Storm &storm)
{

	Dialog &d = gui.getParticleDialog();
	boost::scoped_ptr<ParticleModeData> tempData(new ParticleModeData(d, gui, storm));
	data.swap(tempData);
	
}


ParticleMode::~ParticleMode()
{

}

bool ParticleMode::handleDialogs(MSG& msg) {
	
	return data->handleDialogs(msg);
	
//	return false;
}

void ParticleMode::tick()
{
	data->tick();
}

void ParticleMode::update()
{
	if(!data->mModel)
		data->loadModel();

	data->update();	
}

void ParticleMode::reset()
{
	// Make sure all storm resources are freed here
	// --psd

	if(data->mModel)
		data->mModel.reset();
}

void ParticleMode::export(Exporter &exporter) const
{
}

filesystem::OutputStream &ParticleMode::writeStream(filesystem::OutputStream &stream) const
{
	return stream;
}

filesystem::InputStream &ParticleMode::readStream(filesystem::InputStream &stream)
{
	return stream;
}


} // end of namespace editor
} // end of namespace frozenbyte
