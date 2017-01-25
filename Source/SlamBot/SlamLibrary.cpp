#include "SlamBot.h"
#include "SlamLibrary.h"

// generates typedef and local reference
#define DEFINE_DLL_METHOD(name, returnType, ...) typedef returnType (*_##name##)(__VA_ARGS__);\
_##name name##_ = NULL;

// defines import to local reference
#define IMPORT_DLL_METHOD(name) name##_ = (_##name##)FPlatformProcess::GetDllExport(dllHandle, L#name)

// imported dll handle
void *dllHandle;

// define imported function signatures and local references
DEFINE_DLL_METHOD(invertBool, bool, bool value);

// imports given dll and all methods from dll in project/Plugins/folder/name
bool USlamLibrary::loadSlamLibrary(FString folder, FString name)
{
	FString filePath = *FPaths::GamePluginsDir() + folder + "/" + name;

	if (!FPaths::FileExists(filePath))
	{
		return false;
	}

	dllHandle = FPlatformProcess::GetDllHandle(*filePath);

	if (dllHandle == NULL)
	{
		return false;
	}

	// import all methods
	IMPORT_DLL_METHOD(invertBool);

	return true;
}

// releases imported dll
void USlamLibrary::unloadSlamLibrary()
{
	if (dllHandle == NULL)
	{
		return;
	}

	// release dll handle
	FPlatformProcess::FreeDllHandle(dllHandle);
	dllHandle = NULL;
}

// method conversions
bool USlamLibrary::invertBool(bool value)
{
	return invertBool_(value);
}