// Priit Kallas <kallaspriit@gmail.com>

#pragma once

#include "Components/ActorComponent.h"
#include "FileIO.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLAMBOT_API UFileIO : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFileIO();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	UFUNCTION(BlueprintCallable, Category = "IO")
	bool WriteFile(FString Text, FString Filename);
	
};
