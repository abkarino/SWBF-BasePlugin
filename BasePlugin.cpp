// BasePlugin.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "BasePlugin.h"

#pragma comment(lib, "libMinHook.x86.lib")

using namespace BasePlugin;

BASEPLUGIN_API LUACALL  BasePlugin::luaL__gettop = 0;
BASEPLUGIN_API LUACALL2 BasePlugin::luaL__isnumber = 0;
BASEPLUGIN_API LUACALL2 BasePlugin::luaL__check_number = 0;
BASEPLUGIN_API LUACALL2 BasePlugin::luaL__push_number = 0;

BASEPLUGIN_API bool BasePlugin::Ready = false;

std::vector<BasePlugin::LuaCFunction*> registeredFns;
typedef void(*lua__register_type)(LPVOID, const char*, LPVOID) ;
lua__register_type lua__register;
LPVOID trampoline;

BASEPLUGIN_API GameVersion BasePlugin::DetectGameVersion() {
	BYTE fileName[MAX_PATH];
	DWORD len = GetModuleBaseNameA(GetCurrentProcess(), NULL, (LPSTR)fileName, MAX_PATH);
	std::string strFileName((LPSTR)fileName);
	if (strFileName == "SPTest.exe") return SPTEST;
	PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS nt_header = (PIMAGE_NT_HEADERS)((DWORD)dos_header + dos_header->e_lfanew);
	PIMAGE_OPTIONAL_HEADER32 optional_header = &nt_header->OptionalHeader;
	DWORD importDirectoryRVA = optional_header->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	PIMAGE_IMPORT_DESCRIPTOR importImageDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)dos_header + importDirectoryRVA);
	//DLL Imports
	for (; importImageDescriptor->Name != 0; importImageDescriptor++) {
		std::string dllName((LPSTR)((DWORD)dos_header + importImageDescriptor->Name));
		if (dllName == "steam_api.dll") return STEAM;
	}

	return GOG;
	//return NOTDEFINED;
}

BASEPLUGIN_API void BasePlugin::RegisterLUAFunctions(LuaCFunction* fns) {
	if (fns)
	{
		while (fns->name)
			registeredFns.push_back(fns++);
	}
}

void internalRegisterLUAFunctions(LPVOID luaState) {
	for (std::vector<LuaCFunction*>::iterator it = registeredFns.begin(); it != registeredFns.end(); ++it)
	{
		lua__register(luaState, (*it)->name, (*it)->fn);
	}
}

_declspec(naked) void hook() {
	__asm {
		push    edi;
		call    internalRegisterLUAFunctions;
		add     esp, 4
		jmp     trampoline;
	}
}

bool BasePlugin::init() {
	LPVOID base = GetModuleHandle(NULL);
	GameVersion version = DetectGameVersion();
	LPVOID dest;
	if (version == SPTEST) {
		dest					= (BYTE*)base + 0x17201;
		*(BYTE**)&lua__register = (BYTE*)base + 0x21dd1;
		luaL__gettop			= (LUACALL)((BYTE*)base + 0x12a260);
		luaL__isnumber			= (LUACALL2)((BYTE*)base + 0x12a1a0);
		luaL__check_number		= (LUACALL2)((BYTE*)base + 0x12a1a0);
		luaL__push_number		= (LUACALL2)((BYTE*)base + 0x12a6e0);
	}
	else if (version == STEAM) {
		dest					= (BYTE*)base + 0x175708;
		*(BYTE**)&lua__register = (BYTE*)base + 0x19547;
		luaL__gettop			= (LUACALL)((BYTE*)base + 0x128b9);
		luaL__isnumber			= (LUACALL2)((BYTE*)base + 0xd05d);
		luaL__check_number		= (LUACALL2)((BYTE*)base + 0x1aaf);//0x1536b returns int directly but we already do the conversion manually
		luaL__push_number		= (LUACALL2)((BYTE*)base + 0x11af4);
	}
	else if (version == GOG) {
		dest					= (BYTE*)base + 0x15b988;
		*(BYTE**)&lua__register = (BYTE*)base + 0x154d8;
		luaL__gettop			= (LUACALL)((BYTE*)base + 0xf86c);
		luaL__isnumber			= (LUACALL2)((BYTE*)base + 0x8e72);
		luaL__check_number		= (LUACALL2)((BYTE*)base + 0x172b);
		luaL__push_number		= (LUACALL2)((BYTE*)base + 0x13ab6);
	}
	else return FALSE;

	if (MH_Initialize() != MH_OK)
	{
		return false;
	}

	if (MH_CreateHook(dest, &hook, &trampoline) != MH_OK)
	{
		return false;
	}

	if (MH_EnableHook(dest) != MH_OK)
	{
		return false;
	}

	BasePlugin::Ready = true;
	return true;
}