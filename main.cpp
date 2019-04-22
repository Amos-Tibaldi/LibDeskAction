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

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	LDAInitializeLibDeskAction();
	int winxsize, winysize;
	unsigned char r, g, b;
	LDAGrabWindowOfInterest(&winxsize, &winysize);
	unsigned char * RGBbuffer = new unsigned char [winxsize * winysize * 3];
	LDADXWINDOWHANDLE dxw = LDACreateDXWindow("DebugWindow", winxsize, winysize);
	unsigned char * dxwARGBbuffer = new unsigned char [winxsize * winysize * 4];
	while(!LDAAppShouldExit())
	{
		bool copyok = LDACopyRGBClientAreaOfWindowOfInterest(RGBbuffer);
		bool donefornow = false;
		memset(dxwARGBbuffer, 0, winxsize * winysize * 4);
		if(copyok)
			for(int i=0; i<winxsize; i++) 
				for(int j=0; j<winysize; j++)
				{
					LDAGetRGBXYOfBuffer(RGBbuffer, winxsize, i, j, r, g, b);
					LDASetARGBXYOfBuffer(dxwARGBbuffer, winxsize, i, j, r, g, b);
					if(i>100 && j>100 && r==255 && g==0 && b==0 && !donefornow)
					{
						LDAClickRelativeWindowOfInterestAtXY(i, j);
						donefornow = true;
					}
				}
		LDADXWindowDisplayBuffer(dxw, dxwARGBbuffer);
		LDACallWhenIdle();
		SwitchToThread();
		Sleep(1);
	}
	LDADXWindowDestroy(dxw);
	LDAFinalizeLibDeskAction();
	delete RGBbuffer;
	delete dxwARGBbuffer;
	return 0;
}
