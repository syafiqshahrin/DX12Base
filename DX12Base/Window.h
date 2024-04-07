#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


class Window
{
public:
	Window(LPCWSTR WinTitle, int w, int h, HINSTANCE hInstance);
	Window(const Window& w) = delete;
	~Window();
	int GetWidth();
	int GetHeight();
	HWND GetWindHandle();
	int ProcessMessages();
private:
	const LPCWSTR pClassName = L"WinApp";
	LPCWSTR name;
	int width;
	int height;
	HINSTANCE hInst;
	WNDCLASSEX wndClass;
	HWND hWnd;
	void InitialiseWinClass();
	void InitialiseWindow();
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wP, LPARAM lP);
};
