#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SlamLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SLAMBOT_API USlamLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	// loads dll
	UFUNCTION(BlueprintCallable, Category = "Slam library")
	static bool loadSlamLibrary(FString folder = "SlamLibrary", FString name = "SlamLibrary.dll");

	// unloads dll
	UFUNCTION(BlueprintCallable, Category = "Slam library")
	static void unloadSlamLibrary();

	// imported methods
	UFUNCTION(BlueprintCallable, Category = "Slam library")
	static bool invertBool(bool value);
	
	
};
