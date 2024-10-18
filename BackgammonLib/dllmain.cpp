// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <stdlib.h>
#include <stdio.h>

char tmpstr[] = "Hello, world!";

// Exported function
extern "C"  __declspec(dllexport) char* get_string()
{
    //char* str = (char*)malloc(sizeof(char) * 20);
    //sprintf_s(str,20, "Hello, world!");
    //return str;
    return tmpstr;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

