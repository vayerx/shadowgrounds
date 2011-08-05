#ifndef GRAPH_H
#define GRAPH_H

#define WM_GRAPH_CHANGED 0x0411 // returned by color graph when closed

struct GraphData;
class Graph {
	boost::scoped_ptr<GraphData> m;
	bool mRegistered;
	std::string mClassName;
	HWND mHandle;
public:
	Graph(int nChannels=1);
	~Graph();
	int getNumChannels();
	void setChannelColor(int i, int r, int g, int b);
	void enableChannel(int i, bool value);
	bool channelEnabled(int i);
	bool channelChanged(int i);
	void setNumKeys(int c, int n);
	int getNumKeys(int c);
	void linkChannels(int i, int j);
	float getX(int c, int i);
	float getY(int c, int i);
	void setX(int c, int i, float f);
	void setY(int c, int i, float f);
	void open(HWND parent);
	bool isOpen();
	void close();
	void setWindow(float _xmin, float _ymin, float _xmax, float _ymax, 
		float _xscl, float _yscl);
	bool msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); 
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};


#endif
