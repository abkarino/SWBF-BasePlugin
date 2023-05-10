// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the BASEPLUGIN_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// BASEPLUGIN_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef BASEPLUGIN_EXPORTS
#define BASEPLUGIN_API __declspec(dllexport)
#else
#define BASEPLUGIN_API __declspec(dllimport)
#endif

typedef LPVOID(*LUACALL)(LPVOID);
typedef LPVOID(*LUACALL2)(LPVOID, LPVOID);

namespace BasePlugin {

	extern BASEPLUGIN_API LUACALL  luaL__gettop;
	/*
	* Numbers Functions
	*/
	extern BASEPLUGIN_API LUACALL2 luaL__isnumber;
	extern BASEPLUGIN_API LUACALL2 luaL__check_number;
	extern BASEPLUGIN_API LUACALL2 luaL__push_number;

	extern BASEPLUGIN_API bool Ready;

	enum GameVersion
	{
		NOTDEFINED,
		SPTEST,
		GOG,
		STEAM
	};

	struct LuaCFunction {
		const char* name;
		void* fn;
	};

	BASEPLUGIN_API GameVersion DetectGameVersion();
	BASEPLUGIN_API void RegisterLUAFunctions(LuaCFunction* fns);

	bool init();
}