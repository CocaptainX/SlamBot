// Priit Kallas <kallaspriit@gmail.com>

#pragma once

#include "GameFramework/Actor.h"
#include "SLAM.generated.h"

UCLASS()
class SLAMBOT_API ASLAM : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLAM();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	
};
