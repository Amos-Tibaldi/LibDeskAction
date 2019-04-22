//-----------------------------------------------------------------------
//
// This file is part of the LibDeskAction Project
//
//  by Amos Tibaldi - tibaldi at users.sourceforge.net
//
// http://sourceforge.net/projects/libdeskaction/
//
// http://libdeskaction.sourceforge.net/
//
//
// COPYRIGHT: http://www.gnu.org/licenses/gpl.html
//            COPYRIGHT-gpl-3.0.txt
//
//     The LibDeskAction Project
//         Windows desktop automatic interaction library for windows 
//
//     Copyright (C) 2015 Amos Tibaldi
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------

#ifndef LIBDESKACTION_H
#define LIBDESKACTION_H

#include <Windows.h>
#include <stdio.h>
#include <d3d9.h>

namespace LibDeskActionNamespace
{
	extern "C" void GetCpuSerialNumber(unsigned char * buffer, char * prefix);

	class DesktopUtilities
	{
	private:
		UCHAR * bmData;
		BYTE *pData;
		int xsize, ysize;
		unsigned char ucr, ucg, ucb;
		HWND window_of_interest;
		int WOIoriginalClientWidth, WOIoriginalClientHeight;

		void WaitLongMouseClick();
		bool ActualPixelsCopy(HDC hDC, HBITMAP hBitmap, unsigned char * mypixels);
		void MovePointerAtAbsoluteXYScreenPositionAndClick(int x, int y);
		HWND GetUnderTheMouseWindowHandler();
		bool GetMouseAbsoluteScreenPosition(int * x, int * y);
		bool GetClientPixelsFromWindow(HWND hwnd, unsigned char * mb);
		bool GetWindowClassName(HWND hwnd, char * className, int bufSize);
		bool GetWindowTitleAndSize(HWND hwnd, char * title, int titleSize, int * xsize, int * ysize);
		HWND GetWindowParent(HWND hwnd);
		HWND GetAncestorWindow(HWND hwnd);

	public:
		void ClickRelativeWindowOfInterestAtXY(int x, int y);
		void GrabWindowOfInterest(int * xsize, int * ysize);
		bool CopyClientAreaOfWindowOfInterest(unsigned char * myb);
		void GetRGBAtXYOfWindowOfInterest(int x, int y, int * r, int * g, int * b);
		DesktopUtilities();
		~DesktopUtilities();

	};



#define DXWINDOW_BUFFER_SIZE 4

	LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	typedef struct {
		DWORD * Pixels[DXWINDOW_BUFFER_SIZE];
		CRITICAL_SECTION mutex;
		CONDITION_VARIABLE cond_not_full, cond_not_empty;
		int head, tail, size;
	} pixelbuffer_t;

	class DXWindow {

	public:
		DXWindow(char * theWindowTitle, int ww, int hh, bool WaitVSync);
		~DXWindow();
		LRESULT CALLBACK MyMsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void SetRGBAtXY(int r, int g, int b, int x, int y);
		void DisplayBuffer(unsigned char * bfr);

	private:
		static DWORD run(LPVOID arg);
		void CalculateExactRect(int cx, int cy, RECT& rect);
		void ResizeWindow(int width, int height);
		void PutFrontBufferOntoScreen();
		void CopyFrameToFrontBuffer();
		void ReduceWindowSize();
		void IncreaseWindowSize();

		int nCmdShow;
		HINSTANCE instance;
		WNDCLASS window_class;
		bool mycond;
		char class_name[200];
		bool is_app_fullscreen;
		int window_width;
		int window_height;
		int buffer_width;
		int buffer_height;
		float buffer_ratio;
		DWORD style;
		HWND p_window;
		RECT window_rect;

		pixelbuffer_t PixelBuffer;

		D3DLOCKED_RECT lr;
		IDirect3D9 *g_D3D;
		D3DFORMAT format;
		D3DPRESENT_PARAMETERS pp;
		IDirect3DDevice9 *p_device;
		HRESULT hr;
		LPDIRECT3DSURFACE9 FrontBuffer;
		LPDIRECT3DSURFACE9 BackBuffer;
		HRESULT Result;
	};

}

typedef void * LDADXWINDOWHANDLE;

#define LDASetARGBXYOfBuffer(buffer, bufxsize, x, y, r, g, b) \
{ \
	int offset = 4 * ((x) + (y) * (bufxsize)); \
	buffer[offset] = b; \
	buffer[offset+1] = g; \
	buffer[offset+2] = r; \
	buffer[offset+3] = 0; \
}

#define LDAGetRGBXYOfBuffer(buffer, bufxsize, x, y, r, g, b) \
{ \
	int offset = 3 * ((x) + (y) * (bufxsize)); \
	r = buffer[offset]; \
	g = buffer[offset+1]; \
	b = buffer[offset+2]; \
}

#ifdef __cplusplus
extern "C" {
	LDADXWINDOWHANDLE LDACreateDXWindow(char * title, int width, int height);
	void LDADXWindowDestroy(LDADXWINDOWHANDLE win);
	void LDADXWindowDisplayBuffer(LDADXWINDOWHANDLE win, unsigned char * bfr);
	void LDAInitializeLibDeskAction();
	void LDAFinalizeLibDeskAction();
	void LDAGrabWindowOfInterest(int * xsize, int * ysize);
	bool LDACopyRGBClientAreaOfWindowOfInterest(unsigned char * myRGBbuffer);
	void LDACallWhenIdle();
	bool LDAAppShouldExit();
	void LDAClickRelativeWindowOfInterestAtXY(int x, int y);
}
#endif




#endif


