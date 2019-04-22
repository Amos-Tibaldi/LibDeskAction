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

#include "LibDeskAction.h"

#include <intrin.h>
#include <stdio.h>

bool g_app_done;

#pragma comment(lib, "d3d9.lib")

namespace LibDeskActionNamespace
{
	void GetCpuSerialNumber(unsigned char * buffer, char * prefix)
	{
		int CPUInfo[4];
		CPUInfo[3] = 0;
		CPUInfo[3] = 1 << 18;
		__cpuid(CPUInfo, 1);
		// sprintf((char *)buffer, "%s%08x%08x", prefix, (int)CPUInfo[3], (int)CPUInfo[0]);
	}

	void DesktopUtilities::ClickRelativeWindowOfInterestAtXY(int x, int y)
	{
		POINT point;
		point.x = 0;
		point.y = 0;
		ClientToScreen(window_of_interest, &point);
		MovePointerAtAbsoluteXYScreenPositionAndClick(x + point.x, y + point.y);
	}

	void DesktopUtilities::MovePointerAtAbsoluteXYScreenPositionAndClick(int x, int y)
	{
		INPUT evt;
		evt.type = INPUT_MOUSE;
		evt.mi.dx = (x * 65535) / (GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1);
		evt.mi.dy = (y * 65535) / (GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1);
		evt.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN;
		evt.mi.dwExtraInfo = 0;
		evt.mi.mouseData = 0;
		evt.mi.time = 0;
		SendInput(1, &evt, sizeof(evt));
		evt.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTUP;
		SendInput(1, &evt, sizeof(evt));
	}

	HWND DesktopUtilities::GetUnderTheMouseWindowHandler()
	{
		int mousexposition, mouseyposition;
		if (GetMouseAbsoluteScreenPosition(&mousexposition, &mouseyposition))
		{
			POINT Point;
			Point.x = mousexposition;
			Point.y = mouseyposition;
			HWND rethwnd = WindowFromPoint(Point);
			return rethwnd;
		}
		else
		{
			return NULL;
		}
	}

	bool DesktopUtilities::GetMouseAbsoluteScreenPosition(int * x, int * y)
	{
		POINT thePoint;
		LPPOINT thePointPointer = &thePoint;
		if (!x || !y) return false;
		BOOL rb = GetCursorPos(thePointPointer);
		if (rb)
		{
			*x = thePoint.x;
			*y = thePoint.y;
			return true;
		}
		else
		{
			return false;
		}
	}

	void DesktopUtilities::WaitLongMouseClick()
	{
		while (1) { // wait until the left mouse button has been clicked and kept down for at least .5 sec
			Sleep(500);
			GetAsyncKeyState(VK_LBUTTON);// for first time, clear buffer
			if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
				break;
			}
		}
	}

	bool DesktopUtilities::ActualPixelsCopy(HDC hDC, HBITMAP hBitmap, unsigned char * mypixels)
	{
		BITMAPINFO BitInfo;
		ZeroMemory(&BitInfo, sizeof(BITMAPINFO));
		BitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		BitInfo.bmiHeader.biBitCount = 0;
		if (!::GetDIBits(hDC, hBitmap, 0, 0, NULL, &BitInfo, DIB_RGB_COLORS))
		{
			return (false);
		}
		BitInfo.bmiHeader.biCompression = 0;
		long h = BitInfo.bmiHeader.biHeight;
		long w = BitInfo.bmiHeader.biWidth;
		if (!pData) pData = new BYTE[BitInfo.bmiHeader.biSizeImage + 5];
		if (!::GetDIBits(hDC, hBitmap, 0, h, pData, &BitInfo, DIB_RGB_COLORS))
		{
			delete pData;
			return false;
		}
		BYTE rVal, gVal, bVal;
		int x, y, pdataoffset, mypixelsoffset;
		for (y = 0; y < h; y++)
		{
			for (x = 0; x < w; x++)
			{
				pdataoffset = (x + y*w) * 4;
				mypixelsoffset = 3 * (x + (h - 1 - y)*w);
				rVal = pData[pdataoffset + 2];
				gVal = pData[pdataoffset + 1];
				bVal = pData[pdataoffset];
				mypixels[mypixelsoffset] = rVal;
				mypixels[mypixelsoffset + 1] = gVal;
				mypixels[mypixelsoffset + 2] = bVal;
			}
		}
		return true;
	}

	bool DesktopUtilities::GetClientPixelsFromWindow(HWND hwnd, unsigned char * mb)
	{
		BITMAP bm;
		HDC windc = GetDC(hwnd);
		HDC cdc = CreateCompatibleDC(windc);
		if (!cdc)
		{
			ReleaseDC(hwnd, windc);
			return false;
		}
		RECT theRect;
		LPRECT lprect = &theRect;
		BOOL gottherect = GetClientRect(hwnd, lprect);
		if (!gottherect)
		{
			ReleaseDC(hwnd, windc);
			return false;
		}
		xsize = theRect.right - theRect.left;
		ysize = theRect.bottom - theRect.top;

		if (WOIoriginalClientHeight == -1)
		{
			WOIoriginalClientHeight = ysize;
			WOIoriginalClientWidth = xsize;
		}
		else
		{
			if ((xsize != WOIoriginalClientWidth) || (ysize != WOIoriginalClientHeight))
			{
				ReleaseDC(hwnd, windc);
				return false;
			}
		}

		HBITMAP cbm = CreateCompatibleBitmap(windc, xsize, ysize);
		HBITMAP oldbm = (HBITMAP)SelectObject(cdc, cbm);
		BitBlt(cdc, 0, 0, xsize, ysize, windc, 0, 0, SRCCOPY);
		GetObject(cbm, sizeof(BITMAP), (void*)&bm);

		if ((bm.bmHeight != ysize) || (bm.bmWidth != xsize))
		{
			ReleaseDC(hwnd, windc);
			DeleteObject(cbm);
			return false;
		}

		if (!bmData) bmData = (UCHAR*)malloc(3 * (bm.bmHeight * bm.bmWidth));
		bool actualpixelscopied = ActualPixelsCopy(cdc, cbm, bmData);
		DeleteObject(cbm);
		DeleteDC(cdc);
		ReleaseDC(hwnd, windc);
		if (actualpixelscopied) memcpy(mb, bmData, xsize * ysize * 3);
		return actualpixelscopied;
	}

	void DesktopUtilities::GetRGBAtXYOfWindowOfInterest(int x, int y, int * r, int * g, int * b)
	{
		if ((x < 0) || (y < 0) || (x >= xsize) || (y >= ysize))return;
		int offset = 3 * (x + y * xsize);
		ucr = bmData[offset];
		ucg = bmData[offset + 1];
		ucb = bmData[offset + 2];
		*r = (int)ucr;
		*g = (int)ucg;
		*b = (int)ucb;
	}

	DesktopUtilities::DesktopUtilities()
	{
		bmData = NULL;
		pData = NULL;
		xsize = ysize = -1;
		WOIoriginalClientWidth = WOIoriginalClientHeight = -1;
	}

	DesktopUtilities::~DesktopUtilities()
	{
		free(bmData);
		delete pData;
	}

	bool DesktopUtilities::GetWindowClassName(HWND hwnd, char * className, int bufSize)
	{
		LPTSTR lpcn = className;
		int charWritten = GetClassName(hwnd,
			lpcn,
			bufSize);
		if (charWritten != 0) return true; else return false;
	}

	bool DesktopUtilities::GetWindowTitleAndSize(HWND hwnd, char * title, int titleSize, int * xsiz, int * ysiz)
	{
		LPTSTR lpt = title;
		int charWritten = GetWindowText(hwnd, lpt, titleSize);
		RECT rct;
		GetClientRect(hwnd, &rct);
		*xsiz = rct.right - rct.left;
		*ysiz = rct.bottom - rct.top;
		if (charWritten != 0) return true; else return false;
	}

	HWND DesktopUtilities::GetWindowParent(HWND hwnd)
	{
		return GetParent(hwnd);
	}

	HWND DesktopUtilities::GetAncestorWindow(HWND hwnd)
	{
		HWND parentHwnd = hwnd, previousParent = 0;
		while (parentHwnd != 0)
		{
			previousParent = parentHwnd;
			parentHwnd = GetWindowParent(parentHwnd);
		}
		return previousParent;
	}

	void DesktopUtilities::GrabWindowOfInterest(int * xsize, int * ysize)
	{
		int x, y;
		HWND hwndson;
		char nomeWOI[200];

		WaitLongMouseClick();
		if (GetMouseAbsoluteScreenPosition(&x, &y))
		{
			hwndson = GetUnderTheMouseWindowHandler();
			window_of_interest = GetAncestorWindow(hwndson);
			GetWindowTitleAndSize(window_of_interest, nomeWOI, 200, xsize, ysize);
		}
	}

	bool DesktopUtilities::CopyClientAreaOfWindowOfInterest(unsigned char * myb)
	{
		return GetClientPixelsFromWindow(window_of_interest, myb);
	}

	DWORD DXWindow::run(LPVOID arg)
	{
		DXWindow * pDXW = (DXWindow *)arg;
		while (!g_app_done)
		{
			EnterCriticalSection(&pDXW->PixelBuffer.mutex);
			while (pDXW->PixelBuffer.size == 0)
			{
				SleepConditionVariableCS(&pDXW->PixelBuffer.cond_not_empty, &pDXW->PixelBuffer.mutex, INFINITE);
			}
			pDXW->CopyFrameToFrontBuffer();
			pDXW->PutFrontBufferOntoScreen();
			LeaveCriticalSection(&pDXW->PixelBuffer.mutex);
			WakeConditionVariable(&pDXW->PixelBuffer.cond_not_full);
		}
		return 0;
	}

	void DXWindow::SetRGBAtXY(int r, int g, int b, int x, int y)
	{
		if ((x < 0) || (y < 0) || (x >= buffer_width) || (y >= buffer_height)) return;
		DWORD theColor = (0x00FF0000 & (r << 16)) | (0x0000FF00 & (g << 8)) | (0x000000FF & b);
		int offset = (x + buffer_width*y);
		PixelBuffer.Pixels[PixelBuffer.head][offset] = theColor;
	}

	DXWindow::~DXWindow()
	{
		mycond = true;

		// thrd->join();

		if (g_D3D)
		{
			g_D3D->Release();
			g_D3D = NULL;
		}

		DestroyWindow(p_window);

		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		UnregisterClass(class_name, instance);

		for (int q = 0; q < DXWINDOW_BUFFER_SIZE; q++)
		{
			delete PixelBuffer.Pixels[q];
		}

		DeleteCriticalSection(&PixelBuffer.mutex);
	}

	DXWindow::DXWindow(char * theWindowTitle, int ww, int hh, bool WaitVSync)
	{
		InitializeCriticalSection(&PixelBuffer.mutex);
		InitializeConditionVariable(&PixelBuffer.cond_not_empty);
		InitializeConditionVariable(&PixelBuffer.cond_not_full);

		PixelBuffer.head = PixelBuffer.tail = PixelBuffer.size = 0;

		mycond = false;
		is_app_fullscreen = false;
		style = 0;
		p_window = 0;
		g_D3D = NULL;
		format = D3DFMT_X8R8G8B8;
		p_device = NULL;
		FrontBuffer = NULL;
		BackBuffer = NULL;

		nCmdShow = SW_SHOW;
		instance = GetModuleHandle(NULL);

		sprintf_s(class_name, 199, "CN%s", theWindowTitle);
		window_class.style = CS_OWNDC;
		window_class.cbClsExtra = 0;
		window_class.cbWndExtra = 0;
		window_class.hInstance = instance;
		window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		window_class.lpszMenuName = NULL;
		window_class.lpszClassName = class_name;
		window_class.lpfnWndProc = (WNDPROC)WndProc;

		if (!RegisterClass(&window_class)) {
			MessageBox(p_window, "Error during registration", "Error", MB_OK);
			return;
		}

		buffer_width = ww;  buffer_height = hh;
		buffer_ratio = (float)buffer_width / (float)buffer_height;

		for (int q = 0; q < DXWINDOW_BUFFER_SIZE; q++)
		{
			PixelBuffer.Pixels[q] = new DWORD[buffer_width * buffer_height];
		}

		if (is_app_fullscreen) {
			window_width = GetSystemMetrics(SM_CXSCREEN);
			window_height = GetSystemMetrics(SM_CYSCREEN);
			style = WS_POPUP | WS_VISIBLE;
		}
		else {
			window_width = ww;
			window_height = hh;
			style = WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE | WS_MINIMIZEBOX;
		}
		p_window = CreateWindow(class_name,   //name of our registered class
			theWindowTitle,  //Window name/title
			style,        //Style flags
			0,            //X position
			0,            //Y position
			window_width, //width of window
			window_height,//height of window
			NULL,         //Parent window
			NULL,         //Menu
			instance,     //application instance handle
			NULL);        //pointer to window-creation data

		if (!p_window) {
			MessageBox(NULL, "Cannot create window", NULL, MB_OK);
			exit(-1);
		}

		SetWindowLongPtr(p_window, GWLP_USERDATA, (long)this);

		if (is_app_fullscreen) {
			ShowCursor(FALSE);
		}

		g_D3D = Direct3DCreate9(D3D_SDK_VERSION);
		if (!g_D3D) {
			MessageBox(p_window, "Error creating Direct3DCreate9", "Error", MB_OK);
			exit(-1);
		}

		ZeroMemory(&pp, sizeof(D3DPRESENT_PARAMETERS));
		pp.BackBufferCount = 1;  //We only need a single back buffer
		pp.BackBufferWidth = buffer_width;
		pp.BackBufferHeight = buffer_height;
		pp.MultiSampleType = D3DMULTISAMPLE_NONE; //No multi-sampling
		pp.MultiSampleQuality = 0;                //No multi-sampling
		pp.SwapEffect = D3DSWAPEFFECT_DISCARD;  // Throw away previous frames, we don't need them
		pp.hDeviceWindow = p_window;  //This is our main (and only) window
		pp.Windowed = TRUE;
		pp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;            //No flags to set
		pp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT; //Default Refresh Rate
		pp.PresentationInterval =
			WaitVSync ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;
		pp.BackBufferFormat = format;      //Display format
		pp.EnableAutoDepthStencil = FALSE; //No depth/stencil buffer

		hr = g_D3D->CreateDevice(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			p_window,
			D3DCREATE_HARDWARE_VERTEXPROCESSING, // D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&pp,
			&p_device);
		if (FAILED(hr)) {
			MessageBox(p_window, "Error creating Direct3D device with hardware acceleration", "Error", MB_OK);
			exit(-1);
		}

		Result = p_device->CreateOffscreenPlainSurface(
			buffer_width,
			buffer_height,
			format,
			D3DPOOL_SYSTEMMEM,
			&FrontBuffer,
			NULL
			);

		if (FAILED(Result))
		{
			MessageBox(p_window, "FrontBuffer failed.", "Error", MB_OK);
			exit(-1);
		}

		p_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer);

		ResizeWindow(window_width, window_height);

		ShowWindow(p_window, nCmdShow);
		UpdateWindow(p_window);

		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&DXWindow::run, this, NULL, NULL);
	}

	void DXWindow::CalculateExactRect(int cx, int cy, RECT& rect)
	{
		RECT rcWindow;
		GetWindowRect(p_window, &rcWindow);
		RECT rcClient;
		GetClientRect(p_window, &rcClient);
		cx += (rcWindow.right - rcWindow.left) - rcClient.right;
		cy += (rcWindow.bottom - rcWindow.top) - rcClient.bottom;

		rect.left = rcWindow.left;
		rect.top = rcWindow.top;
		rect.right = rect.left + cx;
		rect.bottom = rect.top + cy;
	}

	void DXWindow::ReduceWindowSize()
	{
		if (window_width <= 150) return;
		window_width -= 20;
		window_height = (int)((float)window_width / (float)buffer_ratio);
		ResizeWindow(window_width, window_height);
	}

	void DXWindow::IncreaseWindowSize()
	{
		if (window_width >= 1000) return;
		window_width += 20;
		window_height = (int)((float)window_width / (float)buffer_ratio);
		ResizeWindow(window_width, window_height);
	}

	void DXWindow::ResizeWindow(int width, int height)
	{
		RECT rect = { 0 };
		CalculateExactRect(width, height, rect);
		MoveWindow(p_window,
			rect.left, rect.top,
			rect.right - rect.left,
			rect.bottom - rect.top,
			TRUE);
	}

	void DXWindow::DisplayBuffer(unsigned char * bfr)
	{
		EnterCriticalSection(&PixelBuffer.mutex);
		while (PixelBuffer.size >= DXWINDOW_BUFFER_SIZE)
		{
			SleepConditionVariableCS(&PixelBuffer.cond_not_full, &PixelBuffer.mutex, INFINITE);
		}

		memcpy(PixelBuffer.Pixels[PixelBuffer.head], bfr, buffer_width*buffer_height * 4);

		PixelBuffer.head++;
		PixelBuffer.head %= DXWINDOW_BUFFER_SIZE;
		PixelBuffer.size++;
		LeaveCriticalSection(&PixelBuffer.mutex);
		WakeConditionVariable(&PixelBuffer.cond_not_empty);
	}

	void DXWindow::CopyFrameToFrontBuffer()
	{
		FrontBuffer->LockRect(&lr, NULL, 0);
		CopyMemory(lr.pBits, PixelBuffer.Pixels[PixelBuffer.tail], buffer_width*buffer_height * 4);
		PixelBuffer.tail++;
		PixelBuffer.tail %= DXWINDOW_BUFFER_SIZE;
		PixelBuffer.size--;
		FrontBuffer->UnlockRect();
	}


	void DXWindow::PutFrontBufferOntoScreen()
	{
		hr = p_device->Clear(0,  //Number of rectangles to clear, we're clearing everything so set it to 0
			NULL, //Pointer to the rectangles to clear, NULL to clear whole display
			D3DCLEAR_TARGET,   //What to clear.  We don't have a Z Buffer or Stencil Buffer
			0x00000000, //Colour to clear to (AARRGGBB)
			1.0f,  //Value to clear ZBuffer to, doesn't matter since we don't have one
			0);   //Stencil clear value, again, we don't have one, this value doesn't matter
		if (FAILED(hr)) {
			return;
		}

		hr = p_device->BeginScene();
		if (FAILED(hr)) {
			return;
		}

		p_device->UpdateSurface(FrontBuffer, NULL, BackBuffer, NULL);

		p_device->EndScene();

		hr = p_device->Present(NULL,  //Source rectangle to display, NULL for all of it
			NULL,  //Destination rectangle, NULL to fill whole display
			NULL,  //Target window, if NULL uses device window set in CreateDevice
			NULL);//Dirty Region, set it to NULL

		if (FAILED(hr)) {
			return;
		}

	}


	LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		DXWindow* winptr = (DXWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (winptr == NULL)
		{
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
		else
		{
			return winptr->MyMsgProc(hwnd, message, wParam, lParam);
		}
	}

	LRESULT CALLBACK DXWindow::MyMsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_MOUSEWHEEL:
			if (wParam & 0x80000000)
				ReduceWindowSize();
			else
				IncreaseWindowSize();
			break;
		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_ADD:
			case VK_OEM_PLUS:
				IncreaseWindowSize();
				break;
			case VK_SUBTRACT:
			case VK_OEM_MINUS:
				ReduceWindowSize();
				break;
			case VK_ESCAPE:
				g_app_done = true;
				break;
			default:
				break;
			}
			break;
		case WM_CLOSE:
			g_app_done = true;
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			g_app_done = true;
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		return 0;
	}

	DesktopUtilities * g_dut;

}

using namespace LibDeskActionNamespace;

LDADXWINDOWHANDLE LDACreateDXWindow(char * title, int width, int height)
{
	DXWindow * p = new DXWindow(title, width, height, true);
	return (LDADXWINDOWHANDLE)p;
}

void LDADXWindowDestroy(LDADXWINDOWHANDLE win)
{
	DXWindow * p = (DXWindow *)win;
	delete p;
}

void LDADXWindowDisplayBuffer(LDADXWINDOWHANDLE win, unsigned char * bfr)
{
	DXWindow * p = (DXWindow *)win;
	p->DisplayBuffer(bfr);
}

void LDAInitializeLibDeskAction()
{
	// GetCpuSerialNumber(CpuSerialNumberString, "CPUIDIS");
	g_dut = new DesktopUtilities();
}

void LDAFinalizeLibDeskAction()
{
	delete g_dut;
}

void LDAGrabWindowOfInterest(int * xsize, int * ysize)
{
	g_dut->GrabWindowOfInterest(xsize, ysize);
}

bool LDACopyRGBClientAreaOfWindowOfInterest(unsigned char * myRGBbuffer)
{
	if (!myRGBbuffer) return false;
	bool bret = g_dut->CopyClientAreaOfWindowOfInterest(myRGBbuffer);
	if (!bret) g_app_done = true;
	return bret;
}

void LDACallWhenIdle()
{
	MSG msg;
	for (int i = 0; i < 20; i++)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) != 0)
		{
			if (GetMessage(&msg, NULL, 0, 0) > 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}

bool LDAAppShouldExit()
{
	return g_app_done;
}

void LDAClickRelativeWindowOfInterestAtXY(int x, int y)
{
	g_dut->ClickRelativeWindowOfInterestAtXY(x, y);
}
