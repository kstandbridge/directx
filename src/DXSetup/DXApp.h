#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

class DXApp
{
public:
	DXApp(HINSTANCE hInstance);
	virtual ~DXApp();

	// MAIN APPLICATION LOOP
	int Run();

	// FRAMEWORK METHODS
	virtual bool Init();
	virtual void Update(float dt) = 0;
	virtual void Render(float dt) = 0;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:

	// WIN32 ATTRIBUTES
	HWND			m_hAppWnd;
	HINSTANCE		m_hAppInstance;
	UINT			m_ClientWidth;
	UINT			m_ClientHeight;
	std::string		m_AppTitle;
	DWORD			m_WndStyle;

	// INITIALIZE WIN32 WINDOW
	bool InitWindow();

};

