#include <windows.h>
#include <windowsx.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <stdio.h>
#include <vector>
#include <string>
#include <commctrl.h>
#include <math.h>
#include <wingdi.h>

#include "graph.h"

struct GraphData {
public:

	int id;
	bool bDrag;
	float treshold;
	float gStep;
	int nChannels;
	
	float yMin, yMax;
	float xMin, xMax;
	float xScl, yScl;

	bool bClosed;

	struct Key {
		Key() {}
		Key(float _x, float _y) : x(_x), y(_y) {}
		float x,y;
	};

	struct Channel {

		int mLockedKey;
		std::vector<Key*> keys;
		DWORD color;
		std::vector<Channel*> linkedChannels;


		~Channel() {
			for(int i = 0; i < keys.size(); i++)
				delete keys[i];
		}

		void linkChannel(Channel* channel) {
			linkedChannels.push_back(channel);
			channel->linkedChannels.push_back(this);
		}
		
		void clearKeys() {
			for(int i = 0; i < keys.size(); i++) {
				delete keys[i];
			}
			keys.clear();
		}
		
		void setNumKeys(int n) {
			clearKeys();
			keys.resize(n);
			for(int i = 0; i < n; i++) {
				keys[i] = new Key;
			}
		}

		void insertKey(float x) {
			std::vector<Key*>::iterator it = keys.begin() + 1;
			for(it; it != keys.end(); it++) {
				Key* prev = *(it-1);
				Key* next = *it;
				if(next->x > x) {
					float t = (next->x - x) / (next->x - prev->x);
					Key* key = new Key(x, prev->y * (1.0f - t) + (next->y * t));
					keys.insert(it, key);
					break;
				}
			}						
			for(int i = 0; i < keys.size(); i++) {
				char buffer[256];
				sprintf(buffer, "%f, %f\n", keys[i]->x, keys[i]->y);
//				OutputDebugString(buffer);
			}
		}
		
		void insertKey(Key* key) {
			std::vector<Key*>::iterator it = keys.begin();
			for(it; it != keys.end(); it++) {
				if((*it)->x < key->x) {
					keys.insert(it, key);
				}
			}
		}
		
		int getNumKeys() {
			return keys.size();
		}

		Key* getKey(int i) {
			return keys[i];
		}

	float lenght(float dx, float dy) {
		return (float)sqrt(dx*dx + dy*dy);
	}

		bool lockKey(float x, float y, float xtresh, float ytresh) {
			for(int i = 0; i < keys.size(); i++) {
				Key* k = keys[i];
				float dx = x - k->x;
				if(dx < 0.0f)
					dx *= -1.0f;
				float dy = y - k->y;
				if(dy < 0.0f)
					dy *= -1.0f;

				if((dx < xtresh) && (dy < ytresh)) {
					mLockedKey = i;
					//MessageBox(0, "locked", "hee", MB_OK);
					return true;
				}
/*
				if(lenght(dx, dy)<treshold) {
					mLockedKey = i;
					//MessageBox(0, "locked", "hee", MB_OK);
					return true;
				}
*/				
			}
			return false;
		}

		void moveKey(float dx, float dy, float xtresh, float ymin, float ymax) {
			
			Key* key = keys[mLockedKey];
			if((key == keys.front()) || (key == keys.back())) {
				key->y = dy;
				if(key->y > ymax) 
					key->y = ymax;
				if(key->y < ymin) 
					key->y = ymin;
			} else {
				
				key->x = dx;
				key->y = dy;
				
				Key* next = keys[mLockedKey+1];
				Key* prev = keys[mLockedKey-1];
				
				float minX = prev->x + xtresh;
				float maxX = next->x - xtresh;

				if(key->x > maxX) key->x = maxX;
				if(key->x < minX) key->x = minX;
				if(key->y < ymin) key->y = ymin;
				if(key->y > ymax) key->y = ymax;

				for(int i = 0; i < linkedChannels.size(); i++) {
					linkedChannels[i]->keys[mLockedKey]->x = key->x;
				}

			}
			

		}

		void setColor(int r, int g, int b) {
			color = RGB(r, g, b);
		}

		DWORD getColor() {
			return color;
		}
		
	};

	std::vector<Channel> mChannels;
	
	int mLockedChannel;

	bool mLocked;

	float mOldX;
	float mOldY;

	float mXTreshold;
	float mYTreshold;

	GraphData(int nChannels) {

		mChannels.resize(nChannels);
/*		for(int i = 0; i < nChannels; i++) {

			mChannels[i].setNumKeys(2);
			
			mChannels[i].getKey(0)->x = 0.0f;
			mChannels[i].getKey(0)->y = 0.0f;

			mChannels[i].getKey(1)->x = 1.0f;
			mChannels[i].getKey(1)->y = 1.0f;

		}
*/
		mLocked = false;
	}

	void linkChannels(int i, int j) {
		mChannels[i].linkChannel(&mChannels[j]);
	}

	void setChannelColor(int i, int r, int g, int b) {
		mChannels[i].setColor(r, g, b);
	}

	Channel& getChannel(int i) {
		return mChannels[i];
	}

	int getNumChannels() {
		return mChannels.size();
	}

	bool channelEnabled(int i) {
		return false;
	}
	
	bool channelChanged(int i) {
		return false;
	}

	int getNumKeys(int i) {
		return mChannels[i].keys.size();
	}

	void setNumKeys(int c, int n) {
		mChannels[c].setNumKeys(n);
	}

	void setWindow(float _xmin, float _ymin, float _xmax, float _ymax, 
		float _xscl, float _yscl) {

		xmin = _xmin;
		ymin = _ymin;
		xmax = _xmax;
		ymax = _ymax;
		xscl = _xscl;
		yscl = _yscl;

	}

	int graphWidth;
	int graphHeight;
	float xmin, xmax;
	float ymin, ymax;
	float yscl, xscl;
	RECT graphRect;

	float toGraphX(int x) {
		x -= graphRect.left;
		float aspect = (xmax - xmin) / (float)graphWidth;
		return xmin + (float)x * aspect;
	}
	
	float toGraphY(int y) {
		y -= graphRect.top;
		float aspect = (ymax - ymin) / (float)graphHeight;
		return ymin + (float)y * aspect;
	}

	int toScreenX(float x) {
		float aspect = (float)graphWidth / (xmax - xmin);
		return graphRect.left + (int)((x * aspect) - xmin);
	}

	int toScreenY(float y) {
		float aspect = (float)graphHeight / (ymax - ymin);
		return graphRect.bottom -  (int)((y * aspect) - (ymin * aspect));
	}

	void updateBounds(HWND hwnd) {
		
		RECT rc;
		GetClientRect(hwnd, &rc);
		rc.bottom -= 16;
		rc.top += 16;
		rc.right -= 16;
		rc.left += 16;
		memcpy(&graphRect, &rc, sizeof(RECT));
		graphWidth = rc.right - rc.left;
		graphHeight = rc.bottom - rc.top;

		mYTreshold = ((ymax - ymin) / (float)graphHeight) + 0.01585f;
		mXTreshold = ((xmax - xmin) / (float)graphWidth) + 0.01585f;
	
	}

	void clearGraph(HWND graph, HDC hdc) {

		// fill the whole are with white:

		RECT r;
		GetClientRect(graph, &r);
		HBRUSH whiteBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, whiteBrush);
		Rectangle(hdc, 0, 0, (int)r.right, (int)r.bottom);			
		SelectObject(hdc, oldBrush);
		DeleteObject(whiteBrush);

	}

	void drawAxes(HWND hwnd, HDC hdc) {

		RECT r;
		GetClientRect(hwnd, &r);

		HPEN pen1 = CreatePen(PS_SOLID, 3, RGB(64, 64, 64));
		HPEN oldPen = (HPEN)SelectObject(hdc, pen1);
		POINT pt;
		
		// x-axis
		MoveToEx(hdc, r.left, toScreenY(0), &pt);
		LineTo(hdc, r.right, toScreenY(0));			
		
		// y-axis		
		MoveToEx(hdc, toScreenX(0), r.top, &pt);
		LineTo(hdc, toScreenX(0), r.bottom);			

		SelectObject(hdc, oldPen);
		DeleteObject(pen1);

	}

	void drawGrid(HWND hwnd, HDC hdc) {

		HFONT hf = CreateFont(8, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
		
		HFONT oldFont = SelectFont(hdc, hf);
		
		HPEN pen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);
				
		POINT pt;

		RECT rc;
		GetClientRect(hwnd, &rc);
		
		float y = ymin;
		while(y <= ymax) {
			char buffer[256];
			sprintf(buffer, "%1.3f", y);
			RECT r;
			r.left = 0;
			r.right = 100;
			r.bottom = toScreenY(y); 
			r.top = r.bottom - 16;
			DrawText(hdc, buffer, strlen(buffer), &r, DT_LEFT);
			MoveToEx(hdc, rc.left, toScreenY(y), &pt);
			LineTo(hdc, rc.right, toScreenY(y));
			y += yscl;
		} 

		DeleteObject(pen);
		SelectFont(hdc, oldFont);
		DeleteObject(hf);
				
	}

	void drawGraph(HWND hwnd, HDC hdc) {
	
		HPEN oldPen = NULL;
		
		POINT pt;
		
		for(int i = 0; i < mChannels.size(); i++) {

			Channel* c = &mChannels[i];

			HPEN pen = CreatePen(PS_SOLID, 2, c->getColor());
			if(i == 0) {
				oldPen = (HPEN)SelectObject(hdc, pen);
			} else {
				SelectObject(hdc, pen);
			}

			for(int j = 1; j < c->getNumKeys(); j++) {
				Key* k1 = c->getKey(j-1);
				Key* k2 = c->getKey(j);
				int x1 = toScreenX(k1->x);
				int y1 = toScreenY(k1->y);
				int x2 = toScreenX(k2->x);
				int y2 = toScreenY(k2->y);
				MoveToEx(hdc, x1, y1, &pt);
				LineTo(hdc, x2, y2);
				Rectangle(hdc, x1-3, y1-3, x1+3, y1+3);
				Rectangle(hdc, x2-3, y2-3, x2+3, y2+3);
			}

			DeleteObject(pen);
	
		}

		SelectObject(hdc, oldPen);

		RECT r;
		GetClientRect(hwnd, &r);
		ValidateRect(hwnd, &r);

	}

	
	void updateGraph(HWND hwnd) {

		HDC hdc = (HDC)GetDC(hwnd);
		
		clearGraph(hwnd, hdc);

		drawAxes(hwnd, hdc);

		drawGrid(hwnd, hdc);

		drawGraph(hwnd, hdc);
		
		ReleaseDC(hwnd, hdc);
		
	}

	bool msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
									
		if(msg == WM_SIZE) {

			updateBounds(hwnd);

		}
		
		if(msg == WM_PAINT) {

			updateBounds(hwnd);

			updateGraph(hwnd);

		}

		if(msg == WM_DESTROY) {
			//SendMessage(GetParent(hwnd), WM_GRAPH_CHANGED, 0, 0);		
			bClosed = true;
		}
		
		if(msg == WM_CREATE) {

			updateGraph(hwnd);

		}

				
		if(msg == WM_LBUTTONDBLCLK) {

//			POINT point;
			//GetCursorPos(&point);


			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);
			
			RECT rect;
			GetClientRect(hwnd, &rect);
			
			if((xPos >= rect.left) && (xPos <= rect.right) && (yPos >= rect.top) && 
				(yPos <= rect.bottom)) 
			{

				float x = toGraphX(xPos);
				float y = toGraphY(rect.bottom - yPos);
				
				for(int i = 0; i < mChannels.size(); i++) {
					Channel* c = &mChannels[i];
					c->insertKey(x);
				}
			}

			updateGraph(hwnd);
		
		}

		if((msg == WM_MOUSEMOVE) && mLocked) {
									
			if(wParam != MK_LBUTTON) {
			
				mLocked = false;
						
			} else {
		
				POINT point;
				
				point.x = LOWORD(lParam);
				point.y = HIWORD(lParam);
				
/*				//GetCursorPos(&point);
								
				RECT rect;
				GetClientRect(hwnd, &rect);
				
				float x = (float)(point.x - rect.left) / (float)(rect.right - rect.left);
				float y = 1.0f - (float)(point.y - rect.top) / (float)(rect.bottom - rect.top);
				float dx = x - mOldX;
				float dy = y - mOldY;
*/				
			RECT rc;
			GetClientRect(hwnd, &rc);

				float x = toGraphX(point.x);
				float y = toGraphY(rc.bottom - point.y);
				
				Channel* c = &mChannels[mLockedChannel];
				
				c->moveKey(x,y,mXTreshold, ymin, ymax);
			
				mOldX = x;
				mOldY = y;

			}

			updateGraph(hwnd);

		}
		

		if((msg == WM_LBUTTONDOWN) & !mLocked) {
		
			POINT point;
			//GetCursorPos(&point);
								
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);

			RECT rc;
			GetClientRect(hwnd, &rc);
				
//			float x = (float)(point.x - rect.left) / (float)(rect.right - rect.left);
//			float y = 1.0f - (float)(point.y - rect.top) / (float)(rect.bottom - rect.top);

				float x = toGraphX(point.x);
				float y = toGraphY(rc.bottom - point.y);


			for(int i = 0; i < mChannels.size(); i++) {
				Channel* c = &mChannels[i];
				if(c->lockKey(x, y, mXTreshold, mYTreshold)) {
					mLockedChannel = i;
					mLocked = true;
					mOldX = x;
					mOldY = y;
					break;
				}
			}
		
		}
		
		return false;
	}
};




Graph::Graph(int nChannels) : mRegistered(false), mHandle(0) {

	boost::scoped_ptr<GraphData> temp(new GraphData(nChannels));
	m.swap(temp);

}

Graph::~Graph() {

	if(mRegistered) {
		UnregisterClass(mClassName.c_str(), GetModuleHandle(0));		
	}
}

bool Graph::msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return m->msgProc(hwnd, msg, wParam, lParam);
}
		
LRESULT CALLBACK Graph::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	static Graph* graph = NULL;
	
	graph = (Graph*)GetWindowLong(hwnd, GWL_USERDATA); 
	
	if(graph) {
		graph->msgProc(hwnd, msg, wParam, lParam);
	}
	
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


void Graph::open(HWND parent) {

	m->bClosed = false;
	
	mClassName = "Test Class";
	
	HINSTANCE hInst = GetModuleHandle(NULL);

	if(!mRegistered) {

		WNDCLASS wc;
		memset(&wc, 0, sizeof(wc));
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.hCursor = (HCURSOR)LoadCursor(0, IDC_ARROW);
		wc.hIcon = NULL;
		wc.hInstance = hInst;
		wc.lpfnWndProc = WindowProc;
		wc.lpszClassName = mClassName.c_str();
		wc.lpszMenuName = NULL;
		wc.style = CS_CLASSDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

		RegisterClass(&wc);

		mRegistered = true;	
	}

	RECT r;
	r.bottom = 300;
	r.left = 0;
	r.right = 400;
	r.top = 0;
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, 0);

	mHandle = CreateWindow(mClassName.c_str(), "Heehee", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		r.right - r.left, r.bottom - r.top, parent, NULL, hInst, NULL);

	SetWindowLong(mHandle, GWL_USERDATA, (LONG)this);

	ShowWindow(mHandle, SW_SHOWNORMAL);
		
	UpdateWindow(mHandle);



}


bool Graph::isOpen() {
	return !m->bClosed;
}

void Graph::close() {
	if(mHandle)	{
		DestroyWindow(mHandle);
		mHandle = NULL;
	}
}



void Graph::linkChannels(int i, int j) {
	m->linkChannels(i, j);
}

void Graph::setChannelColor(int i, int r, int g, int b) {
	m->setChannelColor(i, r, g, b);
}

int Graph::getNumChannels() {
	return m->getNumChannels();
}

bool Graph::channelEnabled(int i) {
	return m->channelEnabled(i);
}

bool Graph::channelChanged(int i) {
	return m->channelChanged(i);
}

int Graph::getNumKeys(int c) {
	return m->getNumKeys(c);
}

void Graph::setNumKeys(int c, int n) {
	m->setNumKeys(c, n);
}

float Graph::getX(int c, int i) {
	return m->getChannel(c).getKey(i)->x;
}

float Graph::getY(int c, int i) {
	return m->getChannel(c).getKey(i)->y;
}

void Graph::setX(int c, int i, float f) {
	m->getChannel(c).getKey(i)->x = f;
}

void Graph::setY(int c, int i, float f) {
	m->getChannel(c).getKey(i)->y = f;
}

void Graph::setWindow(float _xmin, float _ymin, float _xmax, float _ymax, 
					  float _xscl, float _yscl) {
	m->setWindow(_xmin, _ymin, _xmax, _ymax, _xscl, _yscl);
}

