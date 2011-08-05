// Copyright 2002-2004 Frozenbyte Ltd.


//#include "DatatypeDef.h"
#include <Storm3D_Ui.h>

//#include "..\ui\particlesystem.h"
//#include "..\ui\particlemanager.h"

#pragma comment(lib, "storm3dv2.lib")
//using namespace ui;


#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <fstream>
#include <time.h>
#include "..\editor\string_conversions.h"
#include "..\editor\parser.h"
#include "parseutil.h"
#include "particleeffect.h"
#include <tchar.h>


using namespace frozenbyte;
using namespace particle;
using namespace frozenbyte::editor;


bool spawn = false;
bool reload = false;
bool moveEmitter = true;


LRESULT CALLBACK MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch(msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	case WM_KEYDOWN:
		if((int)wParam == VK_SPACE) {
			spawn = true;
		}
		if((int)wParam == VK_RETURN) {
			reload = true;
		}
		if((int)wParam == VK_RETURN) {
			//moveEmitter = !moveEmitter;
		}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


inline Vector getCrossWith(const Vector& v1, const Vector& v2) {
	Vector v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}


void calcFacingMatrix(Matrix& m, const Vector& d) {

	Vector r = d;
	r.Normalize();
	Vector v = r.GetCrossWith(Vector(0.0f, 1.0f, 0.0f));
	float angle = v.GetDotWith(v);
	if(angle<0.00001f) {
		v = r.GetCrossWith(Vector(1.0f, 0.0f, 0.0f));			
	}
	Vector s = r.GetCrossWith(v);
	Vector t = r.GetCrossWith(s);
	m.CreateBaseChangeMatrix(Vector(s.x, r.x, t.x), Vector(s.y, r.y, t.y), Vector(s.z, r.z, t.z));

}


/*
void calcFacingMatrix(Matrix& m, const Vector& velocity) {

	Vector dir = Vector(0.0f, 0.0f, -1.0f);
	dir.Normalize();
	
	Vector zAxis(0.0f, 1.0f, 0.0f);
	
	// check if this up vector is ok!
	Vector v = dir.GetCrossWith(zAxis);
	if(v.GetDotWith(v)<0.0001f) {
		zAxis = Vector(0.0f, 0.0f, 1.0f);	
	}
	
	Vector xAxis;
	
	xAxis = dir.GetCrossWith(zAxis);
	xAxis.Normalize();
	
	zAxis = dir.GetCrossWith(xAxis);
	zAxis.Normalize();
	
	xAxis = 

	m.CreateBaseChangeMatrix(xAxis, dir, zAxis);
}
*/

class OrbitCamera {
	Vector m_position;
	Vector m_target;
	float m_fov;
public:
	void setPosition(const Vector& v) {
		m_position = v;
	}
	void setTarget(const Vector& v) {
		m_target = v;
	}
	void pan(const Vector& v) {
		m_position += v;
		m_target += v;
	}
	void truck(float amount) {
		if(amount == 0.0f)
			return;
		Vector dir = m_target - m_position;
		if(dir.GetLength() <= amount) {
			amount = dir.GetLength() - 0.0001f;
		}
		dir.Normalize();
		m_position += dir * amount;
	}
	void orbit(float dx, float dy) {
		Vector up(0.0f, 1.0f, 0.0f);
		Vector dir = m_target - m_position;
		dir.Normalize();
		Vector left = dir.GetCrossWith(up);
		left.Normalize();
		QUAT q1(up, dx);
		QUAT q2(left, dy);
		Matrix rx, ry;
		rx.CreateRotationMatrix(q1);
		ry.CreateRotationMatrix(q2);
		Vector temp = m_position - m_target;
		rx.RotateVector(temp);
		ry.RotateVector(temp);
		m_position = m_target + temp;
	}
	void setFov(float fov) {
		m_fov = fov; // fov in degrees!!!!
	}
	void apply(IStorm3D_Camera* camera) {
		camera->SetPosition(m_position);
		camera->SetTarget(m_target);
		camera->SetFieldOfView(m_fov);
		camera->SetVisibilityRange(10000.0f);	
	}
};


class ParticleViewer {
	
	IStorm3D_Scene* m_scene;
	IStorm3D* m_s3d;
	IStorm3D_Font* m_font;
	HWND m_hwnd;
	OrbitCamera m_camera;
	HINSTANCE m_hInst;
	ParticleEffectManager* m_particleEffectManager;
	bool m_keys[256];
	std::string m_fileName;
	boost::shared_ptr<IParticleEffect> m_effect;
	int m_movementType;
	IStorm3D_Model* m_model;
	bool m_active;
	bool m_followObject;
	Vector m_launchPosition;
	Vector m_launchVelocity;
	Vector m_effectPosition;
	Vector m_effectVelocity;
	float m_effectLenght;
	int m_effectID;
		
	void createWindow() {

		WNDCLASS wc;
		memset(&wc, 0, sizeof(wc));
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hIcon = NULL;
		wc.hInstance = m_hInst;
		wc.lpfnWndProc = msgProc;
		wc.lpszClassName = "ParticleEditor";
		wc.lpszMenuName = NULL;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		
		RegisterClass(&wc);
		
		m_hwnd = CreateWindow("ParticleEditor", "ParticleViewer", WS_OVERLAPPEDWINDOW, 0, 0, 1024, 768,
			NULL, NULL, m_hInst, NULL);
				
		SetWindowLong(m_hwnd, GWL_USERDATA, (LONG)this);
				
		ShowWindow(m_hwnd, SW_SHOWNORMAL);
	
	}
	
	void createFont() {

		m_font = m_s3d->CreateNewFont();
		
		std::string buff;
		buff += '\t';
		buff += '!';
		buff += '"';
		buff += "#$%@'()*+.-,/0123456789:;<=>?\nabcdefghijklmnopqrstuvwxyz[\]\t_";
		
		std::vector<unsigned char> widths;
		for(int i = 0; i < buff.size(); i++) {
			widths.push_back(64);
		}
		
		//IStorm3D_Texture* ftex = m_s3d->CreateNewTexture("font2.dds");
#ifdef LEGACY_FILES
		IStorm3D_Texture* ftex = m_s3d->CreateNewTexture("Data/Fonts/fpsfont.dds");
#else
		IStorm3D_Texture* ftex = m_s3d->CreateNewTexture("data/gui/font/fpsfont.dds");
#endif

		m_font->AddTexture(ftex);
		m_font->SetTextureRowsAndColums(8, 8);
		m_font->SetCharacters(buff.c_str(), &widths[0]);
		m_font->SetColor(COL(1.0f, 1.0f, 1.0f));	

	}

	void createStorm() {
		
		m_s3d = IStorm3D::Create_Storm3D_Interface(true);
		
		//m_s3d->SetUserWindowMessageProc(MsgProc);
		
		m_s3d->SetWindowedMode(m_hwnd);
		
		COL bgCol = COL(0.3f, 0.3f, 0.3f);
		COL ambLight = COL(1.0f, 1.0f, 1.0f);
		
		m_scene = m_s3d->CreateNewScene();
		m_scene->SetBackgroundColor(bgCol);

	}

	void freeStorm() {

		m_s3d->Empty();
		
		delete m_s3d;

	}

	void updateCamera() {
		
		float dx = 0.0f;
		float dy = 0.0f;
		float zoom = 0.0f;
		Vector pan = Vector(0.0f, 0.0f, 0.0f);
		if(m_keys[VK_LEFT]) {
			dx += -0.01f;
		}
		if(m_keys[VK_RIGHT]) {
			dx += 0.01f;
		}
		if(m_keys[VK_UP]) {
			dy += -0.01f;
		}
		if(m_keys[VK_DOWN]) {
			dy += 0.01f;
		}
		if(m_keys[VK_HOME]) {
			zoom += 1.0f;
		}
		if(m_keys[VK_END]) {
			zoom -= 1.0f;
		}
/*
		if(m_keys['A']) {
			pan.x -= 0.01f;
		}
		if(m_keys['D']) {
			pan.x += 0.01f;
		}
		if(m_keys['W']) {
			pan.y += 0.01f;
		}
		if(m_keys['S']) {
			pan.y -= 0.01f;
		}
		if(m_keys['R']) {
			pan.z += 0.01f;
		}
		if(m_keys['F']) {
			pan.z -= 0.01f;
		}
*/			

		m_camera.orbit(dx,dy);
		m_camera.truck(zoom);
//		m_camera.pan(pan);
		
		m_camera.apply(m_scene->GetCamera());
	}

	
	void loadEffect() {
		
		TCHAR curDir[255];

		GetCurrentDirectory(sizeof(curDir), curDir);
		
		OPENFILENAME ofn;
		TCHAR filename[255] = _T("");
		memset(&ofn, 0, sizeof(OPENFILENAME));
/*		
		fileStruct.lpstrFilter = createFilter(extension);
		fileStruct.lpstrFile = createFileName();
		fileStruct.nMaxFile = sizeof(fileName);
		fileStruct.lpstrInitialDir = defaultDir.c_str();
*/
		ofn.lStructSize = sizeof(OPENFILENAME);
#ifdef LEGACY_FILES
		ofn.lpstrFilter = _T("Text Files(*.txt)\0*.txt\0FB ParticleFX file(*.pfx)\0*.pfx\0\0");
#else
		ofn.lpstrFilter = _T("FB ParticleFX file(*.pfx)\0*.pfx\0Text Files(*.txt)\0*.txt\0\0");
#endif
		ofn.lpstrFile = filename;
		ofn.nMaxFile = sizeof(filename) / sizeof(TCHAR);
#ifdef LEGACY_FILES
		ofn.lpstrInitialDir = _T("data/particles/");
#else
		ofn.lpstrInitialDir = _T("data/effect/particle/");
#endif
/*
		ofn.hwndOwner = m_hwnd;
		ofn.hInstance = GetModuleHandle(NULL);
		ofn.nFilterIndex = 1;
		ofn.lpstrTitle   = _T("Choose Input");
		ofn.lpstrFileTitle =_T("*.pfx\0");
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = _T("data/particles/\0");
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;// | OFN_NOCHANGEDIR;
		ofn.nFileExtension = 3;
		ofn.lpstrDefExt = _T("pfx\0");
*/		
		if(GetOpenFileName(&ofn)) {
			SetCurrentDirectory(curDir);

			reloadEffect(filename, 0);			
/*
			reloadEffect("data/particles/fire_explosion.txt", 0);
//			reloadEffect("data/particles/fire_explosion.txt", 1);
//			reloadEffect("data/particles/fire_explosion.txt", 2);

			reloadEffect("data/particles/smoke.txt", 1);
			reloadEffect("data/particles/smoke.txt", 2);
			reloadEffect("data/particles/smoke.txt", 3);
			reloadEffect("data/particles/spark1.txt", 4);
			reloadEffect("data/particles/spark2.txt", 5);
			reloadEffect("data/particles/fire_explosion.txt", 6);
*/
			//			reloadEffect("data/particles/fire_explosion.txt", 4);
///			reloadEffect("data/particles/fire_explosion.txt", 5);
//			reloadEffect("data/particles/fire_explosion.txt", 6);

		}
		
	}

	void loadModel() {

		TCHAR curDir[255];

		GetCurrentDirectory(sizeof(curDir), curDir);
		
		OPENFILENAME ofn;
		TCHAR filename[255] = _T("");
		memset(&ofn, 0, sizeof(OPENFILENAME));
		

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFilter = _T("S3D Files\0*.s3d\0\0");
		ofn.lpstrFile = filename;
		ofn.nMaxFile = sizeof(filename) / sizeof(TCHAR);
#ifdef LEGACY_FILES
		ofn.lpstrInitialDir = _T("data/particles/");
#else
		ofn.lpstrInitialDir = _T("data/effect/particle/");
#endif

		if(GetOpenFileName(&ofn)) {
			SetCurrentDirectory(curDir);
			m_model = m_s3d->CreateNewModel();
			if(!m_model->LoadS3D(filename)) {

			} else {
				m_scene->AddModel(m_model);
				m_model->SetPosition(Vector(0.0f, 0.0f, 0.0f));
			}			
		}

	}

	void reloadEffect(const char* fileName, int i = 0) {

		if(fileName != NULL)
			m_fileName = fileName;

		std::ifstream file;
		file.open(m_fileName.c_str());
		if(file.bad()) {
			std::string str = "Failed to open ";
			str += fileName;
			MessageBox(0, str.c_str(), "Error", MB_OK | MB_ICONERROR);
		}
		
		editor::Parser parser;
		file >> parser;
		//m_particleEffectManager->loadParticleEffect(i, parser);
		m_effectID = m_particleEffectManager->loadParticleEffect(parser);
//		assert(m_effectID != -1);

		const editor::ParserGroup& g = parser.getGlobals();
		m_followObject = static_cast<bool>(convertFromString<int>(g.getValue("follow_object", ""), 0));
		m_launchPosition = convertVectorFromString(g.getValue("launch_position", "0,0,0"));
		m_launchVelocity = convertVectorFromString(g.getValue("launch_velocity", "0,0,0"));
		m_effectLenght = convertFromString<float>(g.getValue("effect_len", ""), 0.0f);
		
	}
	
	void createObjects() {
		
		m_particleEffectManager = new ParticleEffectManager(m_s3d, m_scene);

		m_camera.setPosition(Vector(0.0f, 80.0f, -80.0f));
		m_camera.setTarget(Vector(0.0f, 0.0f, 0.0f));
		m_camera.setFov(120.0f);

#ifdef LEGACY_FILES
		m_fileName = "data/particles/explosion2.txt";
#else
		m_fileName = "data/effect/particle/explosion2.txt";
#endif

	}

	void setMovementType(int type) {
		m_movementType = type;
	}

	void freeObjects() {

		delete m_particleEffectManager;

	}
	
	void spawnEffect() {

		// kill previous effect first
/*
		if(m_effect.get() != NULL) {
			if(!m_effect->isDead()) {
				m_effect->kill();
			}
		}
*/		
		
		m_effect = m_particleEffectManager->addEffectToScene(m_effectID);		
		
		m_effectPosition = m_launchPosition;
		m_effectVelocity = m_launchVelocity;

		m_effect->setPosition(m_effectPosition);
		m_effect->setVelocity(m_effectVelocity);
		m_effect->tick();
	
	}
	
	void tick() {
/*
		static float angle = 0.0f;
		static Vector oldPosition(0.0f, 0.0f, 0.0f);

		if(m_followObject) {

			angle += 0.01f;

			m_effectPosition.x = sin(angle) * 10.0f;
			m_effectPosition.z = cos(angle) * 10.0f + 10.0f;
			m_effectPosition.y = (sin(angle) + cos(angle)) * 5.0f;
			
			m_effectVelocity = m_effectPosition - oldPosition;
			
			oldPosition = m_effectPosition;
		
		} else {
			
			m_effectPosition += m_effectVelocity;

		}

		if(m_effect.get()) {
			
			m_effect->setPosition(m_effectPosition);
			m_effect->setVelocity(m_effectVelocity);
			m_effect->setLenght(m_effectLenght);
		
			m_effect->tick();
		}
*/				
		m_particleEffectManager->tick();		

	}

	void render() {

		char buffer[256];
		const ParticleEffectManager::Stats& stats = m_particleEffectManager->getStats();
		
		sprintf(buffer, "particles %d", stats.numParticles);		
		m_scene->Render2D_Text(m_font, VC2(8,0), VC2(10.0f, 16), buffer);
		
		sprintf(buffer, "max particles %d", stats.maxParticles);		
		m_scene->Render2D_Text(m_font, VC2(8,16), VC2(10.0f, 16), buffer);
				
		sprintf(buffer, "launch velocity, (%1.3f, %1.3f, %1.3f)", m_launchVelocity.x,
			m_launchVelocity.y, m_launchVelocity.z);
				
		m_scene->Render2D_Text(m_font, VC2(8,32), VC2(10.0f, 16), buffer);

		sprintf(buffer, "launch position, (%1.3f, %1.3f, %1.3f)", m_launchPosition.x,
			m_launchPosition.y, m_launchPosition.z);

		m_scene->Render2D_Text(m_font, VC2(8,48), VC2(10.0f, 16), buffer);

		m_particleEffectManager->render();

		m_scene->RenderScene();

	}

	void activate(bool val) {
		m_active = val;
	}

	static LRESULT CALLBACK msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		
		ParticleViewer* viewer = (ParticleViewer*)GetWindowLong(hwnd, GWL_USERDATA);

		switch(msg) {
		case WM_ACTIVATE: 
			{
			
				if(viewer != NULL) {
		
					if(LOWORD(wParam) != WA_INACTIVE)
						viewer->activate(true);
						else
						viewer->activate(false);
					
					
					// Minimized?
					if(HIWORD(wParam) == TRUE)
						viewer->activate(false);
				}
					
			} break; 
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
			break;
		case WM_KEYDOWN:
			{
				if(viewer)
				switch((int)wParam) {
				case VK_F1:
					{
						viewer->loadEffect();
					} break;
				case VK_F2:
					{
						viewer->reloadEffect(NULL);
					} break;
				case VK_F3:
					{
						viewer->loadModel();
					} break;
				case VK_SPACE:
					{
						viewer->spawnEffect();
					} break;
				
				default: viewer->m_keys[(int)wParam] = true;
				}

			} break;
		case WM_KEYUP:
			{
				if(viewer)
					viewer->m_keys[(int)wParam] = false;
			} break;
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}


public:
	bool init(HINSTANCE hInst) {
		m_hInst = hInst;
		createWindow();
		createStorm();
		createFont();
		createObjects();
		reloadEffect(NULL);
		m_active = true;
		for(int i = 0; i < 256; i++) {
			m_keys[i] = false;
		}
		return true;
	}
	int run() {
		MSG msg;
		int timeCount = 0;
		bool mustQuit = false;
		int startTime = timeGetTime();
		int lastTime = startTime;
		GetMessage(&msg, 0, 0, 0);
		while(!mustQuit) {
			
			if(!m_active) {
			
				while(!m_active) {
					
					if(GetMessage(&msg, 0, 0, 0) <= 0) {
						
						mustQuit = true;						
						break;
					}
					
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			
			} else {

				if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
					if(msg.message == WM_QUIT) {
						mustQuit = true;
					} else {
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				} else {
					updateCamera();
					int time = timeGetTime();
					int deltaTime = time - lastTime;
					timeCount += deltaTime;
					lastTime = time;
					while(timeCount > 10) {
						timeCount-=10;
						tick(); // move
					}
					render();
				}

			}
						
		}
		freeObjects();
		freeStorm();
		return msg.wParam;
	}
};
/*

class IParticleSystem {
public:
	virtual boost::shared_ptr<IParticleSystem> clone()=0;
	virtual bool isDead()=0;
	virtual void kill()=0;
	virtual void tick()=0;
	virtual void render()=0;
	virtual void parseFrom(const editor::ParserGroup& pg)=0;
};

class GenParticleSystemEditables {
public:

};

class GenParticleSystem : public IParticleSystem {
protected:

	void defaultTick(IStorm3D_Scene* scene, const GenParticleSystemEditables& eds);
	void defaultRender(IStorm3D_Scene* scene, const GenParticleSystemEditables& eds);
	void moveAndAnimateSystem(const GenParticleSystemEditables& eds);
	void emitParticles(const GenParticleSystemEditables& eds);
	void moveAndExpireParticles(const GenParticleSystemEditables& eds);
	void applyForces();
	void defaultParseFrom(const editor::ParserGroup& pg, GenParticleSystemEditables& eds);

	bool m_alive;
	bool m_shutdown;

	std::vector<boost::shared_ptr<IParticleForce> > m_forces;

public:

	virtual void setParticlePosition(Vector& pos)=0;
	virtual void setParticleVelocity(Vector& vel, float speed)=0;

};
*/


int WINAPI WinMain(HINSTANCE hInstance, 
  HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ParticleViewer viewer;
	viewer.init(hInstance);
	return viewer.run();
}


/*
int WINAPI WinMain(HINSTANCE hInstance, 
  HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = NULL;
	wc.hInstance = hInstance;
	wc.lpfnWndProc = MsgProc;
	wc.lpszClassName = "ParticleEditor";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);

	HWND hwnd = CreateWindow("ParticleEditor", "ParticleViewer", WS_OVERLAPPEDWINDOW, 0, 0, 640, 480,
		NULL, NULL, hInstance, NULL);
	
	ShowWindow(hwnd, SW_SHOWNORMAL);

	IStorm3D *s3d = IStorm3D::Create_Storm3D_Interface(true);

	s3d->SetUserWindowMessageProc(MsgProc);
	
	s3d->SetWindowedMode(hwnd);

	COL bgCol = COL(0.3f, 0.3f, 0.3f);
	COL ambLight = COL(1.0f, 1.0f, 1.0f);

	IStorm3D_Scene *scene = s3d->CreateNewScene();
	scene->SetBackgroundColor(bgCol);
 
	IStorm3D_Font* font = s3d->CreateNewFont();
	
	std::string buff;
	buff += '\t';
	buff += '!';
	buff += '"';
	buff += "#$%@'()*+.-,/0123456789:;<=>?\nabcdefghijklmnopqrstuvwxyz[\]\t_";
	
	std::vector<unsigned char> widths;
	for(int i = 0; i < buff.size(); i++) {
		widths.push_back(64);
	}

	IStorm3D_Texture* ftex = s3d->CreateNewTexture("font2.dds");
	font->AddTexture(ftex);
	font->SetTextureRowsAndColums(8, 8);
	font->SetCharacters(buff.c_str(), &widths[0]);
	font->SetColor(COL(1.0f, 1.0f, 1.0f));	

	
	scene->GetCamera()->SetPosition(Vector(10.0f, 20.0f, 40.0f));
	scene->GetCamera()->SetTarget(Vector(10.0f, 0.0f, 0.0f));
//	scene->GetCamera()->SetUpVec(Vector(0.0f, 1.0f, 0.0f));
	scene->GetCamera()->SetFieldOfView(45.0f);
	scene->GetCamera()->SetVisibilityRange(10000.0f);
	
	ParticleSystemManager* psMgr = new ParticleSystemManager(s3d, scene);
	ParticleEffectManager* efMgr = new ParticleEffectManager(s3d, scene, psMgr);
	
	psMgr->registerSystem(getSprayParticleSystemClassDesc());
	psMgr->registerSystem(getPointArrayParticleSystemClassDesc());
	psMgr->registerSystem(getCloudParticleSystemClassDesc());

	psMgr->registerForce(getParticleGravityClassDesc());
	psMgr->registerForce(getDragParticleForceClassDesc());

	
	std::ifstream file("effect.txt");
	editor::Parser parser;
	file >> parser;
	file.close();
	efMgr->loadParticleEffect(0, parser);
	

	int startTime = timeGetTime();
	int lastTime = timeGetTime();

	bool firstTime = true;

	boost::shared_ptr<ParticleEffect> effect;
	spawn = true;

	Vector emitterPos(0.0f, 0.0f, 0.0f);
	Vector emitterVelocity(0.0f, 0.0f, 0.0f);
	Vector oldEmitterPos;
	
	float angle = 0.0f;
	int timeCount = 0;

	srand(time(NULL));

	MSG msg;
	GetMessage(&msg, 0, 0, 0);
	while(msg.message != WM_QUIT) {
		if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {

			if(!firstTime) {				
				if(moveEmitter) {
					angle += 0.001f;
					emitterPos.x = sin(angle) * 5.0f;
					emitterPos.y = cos(angle) * 5.0f;
					emitterPos.z = cos(angle) * 5.0f;
					emitterVelocity = emitterPos - oldEmitterPos;
					oldEmitterPos = emitterPos;
				} else {
					emitterPos = Vector(0.0f, 0.0f, 0.0f);
					emitterVelocity = Vector(0.0f, 0.0f, 0.0f);
				}
			}
			
			if(reload) {
				editor::Parser parser2;
				std::ifstream file2("effect.txt");
				file2 >> parser2;
				file2.close();
				efMgr->loadParticleEffect(0, parser2);
				reload = false;
			}
			
			if(spawn) {
				if(firstTime) {
					effect = efMgr->addParticleEffect(0);		
					firstTime = false;
				} else if(effect->isDead()) {
					effect = efMgr->addParticleEffect(0);
				}
				Matrix tm;
				tm.CreateTranslationMatrix(Vector(10.0f, 5.0f, 0.0f));
				effect->setTM(tm);
				effect->setVelocity(Vector(0.0f, 0.0f, 0.0f));

				spawn = false;
			}
			Matrix tm;
			//if(moveEmitter)
			//	calcFacingMatrix(tm, Vector(0.0f, 1.0f, 0.0f));
			//else
				tm.CreateTranslationMatrix(Vector(0.0f, 0.0f, 0.0f));
			//tm.Set(12, emitterPos.x);
			//tm.Set(13, emitterPos.y);
			//tm.Set(14, emitterPos.z);
			//effect->setTM(tm);
			//effect->setVelocity(Vector(0.0f, 0.0f, 0.0f));

			// animate at 100 hz like the game
						
			int time = timeGetTime();
			timeCount += (time - lastTime);
			while(timeCount > 10) {
				timeCount-=10;
				psMgr->tick();
			}
			lastTime = time;

			psMgr->render();
			
			char buffer[256];
			const ParticleSystemManager::Stats& stats = ParticleSystemManager::getSingleton().getStats();

			sprintf(buffer, "particles %d", stats.numParticles);		
			scene->Render2D_Text(font, VC2(8,32), VC2(strlen(buffer), 16), buffer);
			
			sprintf(buffer, "max particles %d", stats.maxParticles);		
			scene->Render2D_Text(font, VC2(8,16), VC2(strlen(buffer), 16), buffer);

	//		sprintf(buffer, "emitter movement %s", );		
	//		scene->Render2D_Text(font, VC2(8,16), VC2(strlen(buffer), 16), buffer);

			scene->RenderScene();
		}
	}

	delete psMgr;
	delete efMgr;
	
	s3d->Empty();
	
	delete s3d;

	return 0;
}



*/