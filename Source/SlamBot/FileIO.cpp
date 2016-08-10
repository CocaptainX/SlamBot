// Priit Kallas <kallaspriit@gmail.com>

#include "SlamBot.h"
#include "FileIO.h"


// Sets default values for this component's properties
UFileIO::UFileIO()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UFileIO::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UFileIO::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

bool UFileIO::WriteFile(FString Text, FString Filename)
{
	return FFileHelper::SaveStringToFile(Text, *(FPaths::GameDir() + Filename));
}