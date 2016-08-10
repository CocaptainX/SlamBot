// Priit Kallas <kallaspriit@gmail.com>

#include "SlamBot.h"
#include "SLAM.h"


// Sets default values
ASLAM::ASLAM()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASLAM::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASLAM::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}