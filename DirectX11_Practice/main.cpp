////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <Windows.h>
#include "systemclass.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	SystemClass* System;
	bool result;


	// 시스템 객체를 생성합니다.
	System = new SystemClass;

	// 시스템 객체를 초기화하고 실행합니다.
	result = System->Initialize();
	if (result)
	{
		System->Run();
	}

	// 시스템 객체를 종료하고 해제합니다.
	System->Shutdown();
	delete System;
	System = 0;

	return 0;
}