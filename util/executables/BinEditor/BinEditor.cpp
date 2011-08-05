
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <Storm3D_UI.h>
#include <IStorm3D_terrain_renderer.h>
#include <keyb3.h>

#include "../system/Logger.h"

#include "../ogui/Ogui.h"
#include "../ogui/OguiStormDriver.h"

#include "../ui/uidefaults.h"
#include "../ui/cursordefs.h"
#include "../ui/LoadingMessage.h"
#include "../ui/DebugDataView.h"

#include "../util/BuildingMap.h"

#include "../game/scaledefs.h"

#include "../convert/str2int.h"

#include "../system/Timer.h"
#include "../system/SystemRandom.h"

#include "../util/assert.h"
#include "../util/Debug_MemoryManager.h"

#include "../filesystem/ifile_package.h"
#include "../filesystem/standard_package.h"
#include "../filesystem/zip_package.h"
#include "../filesystem/file_package_manager.h"
#include "../filesystem/file_list.h"


#pragma comment(lib, "storm3dv2.lib")

using namespace game;
using namespace ui;


#define GRID_AREA_SIZE 10


bool next_interface_generation_request = false;
bool apply_options_request = false;
IStorm3D_Scene *disposable_scene = NULL;

bool lostFocusPause = false;

char *binFilename = NULL;
char *modelFilename = NULL;
int modelAngleX = 0;
int modelAngle = 0;
int modelAngleZ = 0;
char *binFilenameUnrotated = NULL;

float zoom = 20.0f;
float camrot = 0;
float camBeta = 0;



int renderfunc(IStorm3D_Scene *scene)
{
  return scene->RenderScene();
}

LRESULT WINAPI customMessageProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	if(msg == WM_ACTIVATE)
	{
		int active = LOWORD(wParam);
		int minimized = HIWORD(wParam);

		if(active == WA_INACTIVE)
		{
			//ShowWindow(hWnd, SW_MINIMIZE);
		}
		else
		{
			//if(minimized)
			//	ShowWindow(hWnd, SW_RESTORE);
		}

		if(active == WA_INACTIVE || minimized)
			lostFocusPause = true;
		else 
			lostFocusPause = false;
	}

	return 0;
}

/* --------------------------------------------------------- */

void parse_commandline(const char *cmdline)
{
	if (cmdline != NULL)
	{
		int cmdlineLen = strlen(cmdline);
		char *parseBuf = new char[cmdlineLen + 1];
		strcpy(parseBuf, cmdline);
		//int i;

		/*
		for (i = 0; i < cmdlineLen; i++)
		{
			if (parseBuf[i] == ' ')
				parseBuf[i] = '\0';
		}
		*/

		/*
		for (i = 0; i < cmdlineLen; i++)
		{
			if (parseBuf[i] != '\0')
			{
				binFilename = new char[strlen(&parseBuf[i]) + 1];
				strcpy(binFilename, &parseBuf[i]);
			}
		}
		*/
		binFilename = new char[strlen(parseBuf) + 1];
		strcpy(binFilename, parseBuf);

		delete [] parseBuf;
	}

	if (binFilename != NULL)
	{
		if (strlen(binFilename) > 4
			&& strcmp(&binFilename[strlen(binFilename) - 4], ".bin") == 0) 
		{
			int cutpos = strlen(binFilename) - 4;
			for (int i = strlen(binFilename) - 3; i >= 0; i--)
			{
				if (binFilename[i] == '_' && binFilename[i + 1] == 'R' 
					&& binFilename[i + 2] == '_')
				{
					cutpos = i;
					modelAngle = atoi(&binFilename[i + 3]);

					char *tmpbuf = new char[strlen(binFilename) + 1];
					bool multipleRot = false;
					int cutposes[2] = { 0, 0 };
					int cutposnum = 0;
					for (int j = i+3; j < (int)strlen(binFilename); j++)
					{
						if (binFilename[j] == '_')
						{
							tmpbuf[j] = '\0';	
							assert(cutposnum < 2);
							if (cutposnum < 2)
							{
								cutposes[cutposnum] = j;
								cutposnum++;
							}
							multipleRot = true;
						} else {
							tmpbuf[j] = binFilename[j];
						}
					}
					if (multipleRot)
					{
						modelAngleX = atoi(&tmpbuf[cutpos + 3]);
						modelAngle = atoi(&tmpbuf[cutposes[0] + 1]);
						modelAngleZ = atoi(&tmpbuf[cutposes[1] + 1]);

//MessageBox(0, int2str(modelAngleX), "Angle X", MB_OK);
//MessageBox(0, int2str(modelAngle), "Angle Y", MB_OK);
//MessageBox(0, int2str(modelAngleZ), "Angle Z", MB_OK);

					}
					delete [] tmpbuf;
				}
			}
			binFilenameUnrotated = new char[strlen(binFilename) + 1];
			strcpy(binFilenameUnrotated, binFilename);
			binFilenameUnrotated[cutpos] = '\0';
			strcat(binFilenameUnrotated, ".bin");

			modelFilename = new char[strlen(binFilename) + 1];
			strcpy(modelFilename, binFilename);
			modelFilename[cutpos] = '\0';
			strcat(modelFilename, ".s3d");
		} else {			
			// oops? not a bin file?
			delete [] binFilename;
			binFilename = NULL;
		}
	}
}


/* --------------------------------------------------------- */

int WINAPI WinMain(HINSTANCE hInstance, 
  HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // initialize...

  int scr_width = 0;
  int scr_height = 0;

	char *cmdline = lpCmdLine;

	std::string fileName = cmdline;
	//MessageBox(0, cmdline, "Command line", MB_OK);

	std::string parsedLine;
	if(strlen(cmdline) > 2)
	{
		char buffer[1024] = { 0 };
		GetModuleFileName(GetModuleHandle(0), buffer, 1023);
		std::string dir = buffer;
		int end = dir.find_last_of('\\');
		dir = dir.substr(0, end);

//#ifdef NDEBUG
		if (strstr(dir.c_str(), "source") != NULL)
		{
			MessageBox(0, "NOTE: Seem to be running under source dir, so not changing dir.", "Running from source.", MB_OK);
		} else {
			//MessageBox(0, dir.c_str(), "Changing dir to", MB_OK);
			SetCurrentDirectory(dir.c_str());
		}
//#endif

		{
			int start = 0;
			if (fileName[0] == '\"')
				start++;
			int end = fileName.find_last_of('\"');
			fileName = fileName.substr(start, end - start);
		}
	}

	using namespace frozenbyte::filesystem;
	boost::shared_ptr<IFilePackage> standardPackage(new StandardPackage());
	//boost::shared_ptr<IFilePackage> zipPackage(new ZipPackage("data.zip"));

	FilePackageManager &manager = FilePackageManager::getInstance();
	manager.addPackage(standardPackage, 0);
	//manager.addPackage(zipPackage, 2);

	parse_commandline(fileName.c_str());

	if (binFilename == NULL)
	{
		return 1;
	}

	//MessageBox(0, binFilename, "Bin", MB_OK);
	//MessageBox(0, modelFilename, "Model", MB_OK);

  Timer::init();

	IStorm3D *s3d = IStorm3D::Create_Storm3D_Interface(true, NULL);
	s3d->SetUserWindowMessageProc(&customMessageProc);

	s3d->SetApplicationName("BinEditor", "BinEditor");

  scr_width = 1024;
  scr_height = 768;

	s3d->SetWindowedMode(scr_width, scr_height);
	ShowWindow(s3d->GetRenderWindow(), SW_MAXIMIZE);

  // keyb3 controller devices
	int ctrlinit = 0;
	ctrlinit |= KEYB3_CAPS_MOUSE;
  ctrlinit |= KEYB3_CAPS_KEYBOARD;
  Keyb3_Init(s3d->GetRenderWindow(), ctrlinit);
  Keyb3_SetActive(1);

	float mouse_sensitivity = 1.0f;
  
  Storm3D_SurfaceInfo screenInfo = s3d->GetScreenSize();
  Keyb3_SetMouseBorders((int)(screenInfo.width / mouse_sensitivity), 
    (int)(screenInfo.height / mouse_sensitivity));
  Keyb3_SetMousePos((int)(screenInfo.width / mouse_sensitivity) / 2, 
    (int)(screenInfo.height / mouse_sensitivity) / 2);

	Keyb3_UpdateDevices();

  // make a scene
  COL bgCol = COL(0.0f, 0.0f, 0.0f);

  disposable_scene = s3d->CreateNewScene();
  disposable_scene->SetBackgroundColor(bgCol);

  disposable_scene->GetCamera()->SetVisibilityRange(100.0f);

  // create and initialize ogui
  Ogui *ogui = new Ogui();
  OguiStormDriver *ogdrv = new OguiStormDriver(s3d, disposable_scene);
  ogui->SetDriver(ogdrv);
  ogui->SetScale(OGUI_SCALE_MULTIPLIER * scr_width / 1024, 
    OGUI_SCALE_MULTIPLIER * scr_height / 768); 
  ogui->SetMouseSensitivity(mouse_sensitivity, mouse_sensitivity);
  ogui->Init();

  // set default font
#ifdef LEGACY_FILES
  ogui->LoadDefaultFont("Data/Fonts/default.ogf");
#else
  ogui->LoadDefaultFont("data/gui/font/common/default.ogf");
#endif

  // set default ui (ogui and storm) for visual objects (pictures and models)
  ui::createUIDefaults(ogui);

	LoadingMessage::setManagers(s3d, disposable_scene, ogui);

  // create cursors
  ogui->SetCursorController(0, OGUI_CURSOR_CTRL_MOUSE);

  // cursors images for controller 0,1,2,3
  loadDHCursors(ogui, 0); 

  ogui->SetCursorImageState(0, DH_CURSOR_ARROW);

  // do the loop...

  Timer::update();
  DWORD startTime = Timer::getTime(); 
  DWORD curTime = startTime;
  DWORD movementTime = startTime;
  DWORD frameCountTime = startTime;
  DWORD lastOguiUpdateTime = startTime;
  bool quitRequested = false;

  int frames = 0;
  int polys = 0;
  int fps = 0;
  int polysps = 0;
	int precount = 0;

	VC3 campos = VC3(0,0,0);

	Keyb3_UpdateDevices();

	IStorm3D_Model *model = NULL;

	if (modelFilename != NULL)
	{
	  model = s3d->CreateNewModel();
		model->LoadS3D(modelFilename);
		disposable_scene->AddModel(model);

		/*
    QUAT rot = QUAT(
      UNIT_ANGLE_TO_RAD(modelAngleX), 
      UNIT_ANGLE_TO_RAD(modelAngle), 
      UNIT_ANGLE_TO_RAD(modelAngleZ));
		*/
		QUAT qx;
		qx.MakeFromAngles(modelAngleX*3.1415f/180.0f, 0, 0);
		QUAT qy;
		qy.MakeFromAngles(0, modelAngle*3.1415f/180.0f, 0);
		QUAT qz;
		qz.MakeFromAngles(0, modelAngleZ*3.1415f/180.0f, 0);
		QUAT rot = qz * qx * qy;

    model->SetRotation(rot);

		// ----------------------------
		// HIDE ROOF...

		Iterator<IStorm3D_Model_Object *> *objectIterator;
		for(objectIterator = model->ITObject->Begin(); !objectIterator->IsEnd(); objectIterator->Next())
		{
			IStorm3D_Model_Object *object = objectIterator->GetCurrent();
			const char *name = object->GetName();

			int namelen = strlen(name);

			if(namelen < 12)
				continue;

			// Test name tag
			for(int i = 0; i < namelen - 12 + 1; ++i)
			{
				if (strncmp(&name[i], "BuildingRoof", 12) == 0)
				{
					object->SetNoRender(true);
				}
			}

		}

		delete objectIterator;

		// END OF HIDE ROOF
		// ----------------------------

	}

	/*
	IStorm3D_Line *gridlines[GRID_AREA_SIZE *2+1][GRID_AREA_SIZE *2+1][4] = { 0 };
	*/
	IStorm3D_Model *gridmodels[GRID_AREA_SIZE *2+1][GRID_AREA_SIZE *2+1][3] = { 0 };
	for (int i = 0; i < GRID_AREA_SIZE *2+1; i++)
	{
		for (int j = 0; j < GRID_AREA_SIZE *2+1; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				gridmodels[i][j][k] = s3d->CreateNewModel();
#ifdef LEGACY_FILES
				if (k < 1)
					gridmodels[i][j][k]->LoadS3D("Data/Models/Pointers/grid_green.s3d");
				else if (k == 2)
					gridmodels[i][j][k]->LoadS3D("Data/Models/Pointers/grid_red.s3d");
				else
					gridmodels[i][j][k]->LoadS3D("Data/Models/Pointers/grid_blue.s3d");
#else
				if (k < 1)
					gridmodels[i][j][k]->LoadS3D("data/model/pointer/grid_green.s3d");
				else if (k == 2)
					gridmodels[i][j][k]->LoadS3D("data/model/pointer/grid_red.s3d");
				else
					gridmodels[i][j][k]->LoadS3D("data/model/pointer/grid_blue.s3d");
#endif
				disposable_scene->AddModel(gridmodels[i][j][k]);
			}
		}
	}


	VC3 targ = campos;
	VC3 pos = targ - VC3(zoom/2,-zoom,0);
	disposable_scene->GetCamera()->SetPosition(pos);
	disposable_scene->GetCamera()->SetTarget(targ);

	unsigned short buffer[32*32] = { 0 };
	IStorm3D_Terrain *terrain = s3d->CreateNewTerrain(32);
	terrain->setHeightMap(buffer, VC2I(32,32), VC3(500,.01f,500), 4, 0, 1, 1);
	//terrain->getRenderer().setRenderMode(IStorm3D_TerrainRenderer::TexturesOnly);
	terrain->getRenderer().setFloatValue(IStorm3D_TerrainRenderer::ForceAmbient, 0.5f);
	disposable_scene->AddTerrain(terrain);
	disposable_scene->SetAmbientLight(COL(0.5f,0.5f,0.5f));

	frozenbyte::BuildingMap *bmap = NULL;

	if (binFilename != NULL
		&& modelFilename != NULL)
	{
		bmap = new frozenbyte::BuildingMap(modelFilename, model, modelAngleX, modelAngle, modelAngleZ);
	}

	if (bmap == NULL)
	{
		return 2;
	}

	float resolution = bmap->getMapResolution();
	const std::vector<std::vector<unsigned char> > &obstmap = bmap->getObstacleMap();
	const std::vector<std::vector<unsigned char> > &heightmap = bmap->getHeightMap();
	const std::vector<std::vector<char> > &floormap = bmap->getFloorHeightMap();

	int lastcammapx = -1;
	int lastcammapz = -1;

	bool wireframe = false;
	bool collision = false;

  while (!quitRequested)
  {
    // read input
    
    Keyb3_UpdateDevices();

		if (Keyb3_IsKeyPressed(KEYCODE_ESC))
		{
			quitRequested = true;
		}

    Timer::update();

    Timer::update();
    curTime = Timer::getTime();

    if (curTime - movementTime > 0)
    {
      // VEEERY jerky... 
      // attempt to fix that...
      float delta;
      delta = 100.0f;
      if (fps > 0) delta = 1000.0f/fps;
      if (delta < 1.0f) delta = 1.0f;
      if (delta > 100.0f) delta = 100.0f;
      float camera_time_factor = 1.0f;
      delta = (delta * camera_time_factor);

			float movespeed = 0.01f;
			if (Keyb3_IsKeyDown(KEYCODE_SHIFT_RIGHT)
				|| Keyb3_IsKeyDown(KEYCODE_SHIFT_LEFT))
				movespeed = 0.03f;

			IStorm3D_TerrainRenderer &renderer = terrain->getRenderer();
			if (Keyb3_IsKeyPressed(KEYCODE_1))
			{
				wireframe = !wireframe;
				if (wireframe)
				{
					renderer.enableFeature(IStorm3D_TerrainRenderer::Wireframe, true);
				} else {
					renderer.enableFeature(IStorm3D_TerrainRenderer::Wireframe, false);
				}
			}
			if (Keyb3_IsKeyPressed(KEYCODE_2))
			{
				collision = !collision;
				if (collision)
				{
					renderer.enableFeature(IStorm3D_TerrainRenderer::Collision, true);
				} else {
					renderer.enableFeature(IStorm3D_TerrainRenderer::Collision, false);
				}
			}

			if (Keyb3_IsKeyDown(KEYCODE_Q))
			{
				camrot -= 0.1f * delta;
			}
			if (Keyb3_IsKeyDown(KEYCODE_E))
			{
				camrot += 0.1f * delta;
			}
			if (camrot >= 360.0f) camrot -= 360.0f;
			if (camrot < 0.0f) camrot += 360.0f;

			if (Keyb3_IsKeyDown(KEYCODE_UP_ARROW)
				|| Keyb3_IsKeyDown(KEYCODE_W))
			{
				campos.x += (movespeed * delta)* cosf(camrot / 180.0f * 3.1415f);
				campos.z += (movespeed * delta)* -sinf(camrot / 180.0f * 3.1415f);
			}
			if (Keyb3_IsKeyDown(KEYCODE_DOWN_ARROW)
				|| Keyb3_IsKeyDown(KEYCODE_S))
			{
				campos.x -= (movespeed * delta)* cosf(camrot / 180.0f * 3.1415f);
				campos.z -= (movespeed * delta)* -sinf(camrot / 180.0f * 3.1415f);
			}
			if (Keyb3_IsKeyDown(KEYCODE_LEFT_ARROW)
				|| Keyb3_IsKeyDown(KEYCODE_A))
			{
				campos.z += (movespeed * delta)* cosf(camrot / 180.0f * 3.1415f);
				campos.x += (movespeed * delta)* sinf(camrot / 180.0f * 3.1415f);
			}
			if (Keyb3_IsKeyDown(KEYCODE_RIGHT_ARROW)
				|| Keyb3_IsKeyDown(KEYCODE_D))
			{
				campos.z -= (movespeed * delta)* cosf(camrot / 180.0f * 3.1415f);
				campos.x -= (movespeed * delta)* sinf(camrot / 180.0f * 3.1415f);
			}
			if (Keyb3_IsKeyDown(KEYCODE_KEYPAD_PLUS))
			{
				zoom -= movespeed * delta;
				if (zoom < 1.5f)
					zoom = 1.5f;
			}
			if (Keyb3_IsKeyDown(KEYCODE_KEYPAD_MINUS))
			{
				zoom += movespeed * delta;
				if (zoom > 80.0f)
					zoom = 80.0f;
			}
			if (Keyb3_IsKeyDown(KEYCODE_KEYPAD_DIVIDE))
			{
				camBeta -= movespeed*2 * delta;
				if (camBeta < -45.0f)
					camBeta = -45.0f;
			}
			if (Keyb3_IsKeyDown(KEYCODE_KEYPAD_MULTIPLY))
			{
				camBeta += movespeed*2 * delta;
				if (camBeta > 45.0f)
					camBeta = 45.0f;
			}

			VC3 targ = campos;
			float dirx = zoom*cosf((camBeta+45)/ 180.0f * 3.1415f) * cosf(camrot / 180.0f * 3.1415f);
			float dirz = zoom*cosf((camBeta+45)/ 180.0f * 3.1415f) * -sinf(camrot / 180.0f * 3.1415f);
			VC3 pos = targ - VC3(dirx,-zoom*cosf((camBeta-45)/ 180.0f * 3.1415f),dirz);

			disposable_scene->GetCamera()->SetPosition(pos);
			disposable_scene->GetCamera()->SetTarget(targ);

      movementTime = curTime;
    }

		// height/floor/obstacle visualization...
		
		int cammapx = int(campos.x / resolution);
		int cammapz = int(campos.z / resolution);

		if (cammapx != lastcammapx || cammapz != lastcammapz)
		{
			lastcammapx = cammapx;
			lastcammapz = cammapz;
			for (int mz = -GRID_AREA_SIZE; mz < GRID_AREA_SIZE+1; mz++)
			{
				for (int mx = -GRID_AREA_SIZE; mx < GRID_AREA_SIZE+1; mx++)
				{
					int tx = heightmap.size() / 2 + cammapx + mx;
					int tz = heightmap[0].size() / 2 + cammapz + mz;
					for (int i = 0; i < 3; i++)
					{
						gridmodels[GRID_AREA_SIZE+mx][GRID_AREA_SIZE+mz][i]->SetPosition(VC3(0,0,0));
						//gridmodels[GRID_AREA_SIZE+mx][GRID_AREA_SIZE+mz][i]->SetPosition(VC3(0,-2000,0));
						/*
						if (gridlines[GRID_AREA_SIZE+mx][GRID_AREA_SIZE+mz][i] != NULL)
						{
							disposable_scene->RemoveLine(gridlines[GRID_AREA_SIZE+mx][GRID_AREA_SIZE+mz][i]);
							while (gridlines[GRID_AREA_SIZE+mx][GRID_AREA_SIZE+mz][i]->GetPointCount() > 0)
							{
								gridlines[GRID_AREA_SIZE+mx][GRID_AREA_SIZE+mz][i]->RemovePoint(0);
							}
							delete gridlines[GRID_AREA_SIZE+mx][GRID_AREA_SIZE+mz][i];
							gridlines[GRID_AREA_SIZE+mx][GRID_AREA_SIZE+mz][i] = NULL;
						}
						*/
					}
					if (tx >= 0 && tz >= 0
						&& tx < (int)heightmap.size() && tz < (int)heightmap[0].size())
					{
						for (int i = 0; i < 3; i++)
						{
							VC3 origin = VC3(cammapx*resolution,0,cammapz*resolution) + VC3(mx*resolution, 0, mz*resolution);

							if (floormap[tx][tz] != BUILDINGMAP_NO_FLOOR_BLOCK)
							{
								origin.y = (float)floormap[tx][tz] * bmap->getHeightScale() + 0.01f;
								if (!wireframe)
									origin.y += 0.05f;
							}

							if (i >= 2)
							{
								origin.y += (float)heightmap[tx][tz] * bmap->getHeightScale();
								origin.y += 0.02f;
								if (obstmap[tx][tz] == 0)
									origin = VC3(0,-2000,0);
							}
							//if (i == 1)
							//{
							//	gridmodels[GRID_AREA_SIZE+mx][GRID_AREA_SIZE+mz][i]->SetScale(VC3(0.5f,0.5f,0.5f));
							//}
							if (i == 1 && obstmap[tx][tz] == 0)
							{
								origin = VC3(0,-2000,0);
							}
							if (i == 0 && obstmap[tx][tz] != 0)
							{
								origin = VC3(0,-2000,0);
							}

							gridmodels[GRID_AREA_SIZE+mx][GRID_AREA_SIZE+mz][i]->SetPosition(origin);

							if (obstmap[tx][tz] == 2)
								gridmodels[GRID_AREA_SIZE+mx][GRID_AREA_SIZE+mz][i]->SetScale(VC3(0.8f,1,0.8f));
							else
								gridmodels[GRID_AREA_SIZE+mx][GRID_AREA_SIZE+mz][i]->SetScale(VC3(1,1,1));


							/*
							IStorm3D_Line *lineObject = s3d->CreateNewLine();
							unsigned int alpha = 0x30000000;
							unsigned int color = 0x0000ff00;
							float thickness = 0.03f;

							if (obstmap[tx][tz] != 0)
								color = 0x00ff0000;

							if (i < 2 || obstmap[tx][tz] != 0)
							{
								lineObject->SetThickness(thickness);
								lineObject->SetColor(color | alpha);

								VC3 origin = campos + VC3(mx*resolution, 0, mz*resolution);

								origin.y = floormap[tx][tz] * bmap->getHeightScale() + 0.001f;

								if (i >= 2)
									origin.y += (float)heightmap[tx][tz] * bmap->getHeightScale();

								lineObject->AddPoint(origin);
								VC3 line_endpoint;
								if ((i % 2) == 0)
									line_endpoint = origin + VC3(resolution,0,0);
								else
									line_endpoint = origin + VC3(0,0,resolution);
								lineObject->AddPoint(line_endpoint);
								disposable_scene->AddLine(lineObject, true);

								gridlines[GRID_AREA_SIZE+mx][GRID_AREA_SIZE+mz][i] = lineObject;
							}
							*/
						}
					}
				}
			}
		}

    // frame/poly counting
    frames++;
    {
      if (curTime - frameCountTime >= 200) 
      {
       float seconds = (curTime - frameCountTime) / 1000.0f;
       fps = (int)(frames / seconds);
       polysps = (int)(polys / seconds);
       frameCountTime = curTime;
       frames = 0;
       polys = 0;
      }
    }

    // run the gui
		int oguiTimeDelta = curTime - lastOguiUpdateTime;
		if (oguiTimeDelta > 200)
			oguiTimeDelta = 200;

    ogui->Run(oguiTimeDelta);

		lastOguiUpdateTime = curTime;

    // render stats

		bool show_polys = true;
		bool show_fps = true;
		// WARNING: unsafe cast!
		IStorm3D_Font *fpsFont = ((OguiStormFont *)ui::defaultIngameFont)->fnt;
    if (show_polys || show_fps)
    {
      float polyoffset = 0;
      float terroffset = 0;
      char polytextbuf[40];
      char polyframetextbuf[40];
      char fpstextbuf[40];
      if (show_fps)
      {
        polyoffset = 16;
				terroffset = 16;
        sprintf(fpstextbuf, "FPS:%d", fps);
        disposable_scene->Render2D_Text(fpsFont, VC2(0,0), VC2(16, 16), fpstextbuf);
      }
      if (show_polys)
      {
				terroffset += 32;
        sprintf(polytextbuf, "POLYS PER S:%d", polysps);
        int polyspf = 0;
        if (fps > 0) polyspf = polysps / fps;
        sprintf(polyframetextbuf, "POLYS PER F:%d", polyspf);
        disposable_scene->Render2D_Text(fpsFont, VC2(0,polyoffset), VC2(16, 16), polytextbuf);
        disposable_scene->Render2D_Text(fpsFont, VC2(0,polyoffset+16), VC2(16, 16), polyframetextbuf);
      }
    }

		int tx = heightmap.size() / 2 + cammapx;
		int tz = heightmap[0].size() / 2 + cammapz;
    char coordtextbuf[64];
    sprintf(coordtextbuf, "%d,%d", tx,tz);
    disposable_scene->Render2D_Text(fpsFont, VC2(0,768-16), VC2(16, 16), coordtextbuf);

    // Render scene
    polys += renderfunc(disposable_scene);

		//DebugDataView::getInstance(game)->run();

		// wait here if minimized...
		while (true)
		{
			// psd -- handle windows messages
			MSG msg = { 0 };
			while(PeekMessage(&msg, s3d->GetRenderWindow(), 0, 0, PM_NOREMOVE))
			{
				if(GetMessage(&msg, s3d->GetRenderWindow(), 0, 0) <= 0)
				{
					quitRequested = true;
				}
				else
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			if (lostFocusPause)
			{
				Sleep(500);
			} else {
				break;
			}
		}
  }

	if (bmap != NULL)
	{
		delete bmap;
		bmap = NULL;
	}

  // clean up

  unloadDHCursors(ogui, 0); 

  deleteUIDefaults();

  ogui->Uninit();
  delete ogui;
  delete ogdrv;

  Keyb3_Free();
  
  delete s3d;

	SystemRandom::cleanInstance();

	uninitLinkedListNodePool();

  Timer::uninit();

	Logger::cleanInstance();

  return 0;
}
 
