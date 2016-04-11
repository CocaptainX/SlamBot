#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "GameFramework/Actor.h"
#include "Serial.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(SerialLog, Log, All);

// based on http://playground.arduino.cc/Interfacing/CPPWindows
UCLASS()
class SLAMBOT_API ASerial : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASerial();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when object is destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Opens a serial COM port
	UFUNCTION(BlueprintCallable, Category = "Setup")
	void OpenPort(FString PortName);

	// Closes opened a serial COM port
	UFUNCTION(BlueprintCallable, Category = "IO")
	void ClosePort();

	// Returns the next message, unshifting it from the queue
	UFUNCTION(BlueprintCallable, Category = "IO")
	FString GetNextMessage();

	// Send a message
	UFUNCTION(BlueprintCallable, Category = "IO")
	void SendMessage(FString Message, bool IncludeNewline = false);

protected:

	// Is the connection valid
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Info", meta = (BlueprintProtected = "true"))
	bool IsConnected = false;

	// Serial port name
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Info", meta = (BlueprintProtected = "true"))
	FString PortName = "";

	// Is the connection valid
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Info", meta = (BlueprintProtected = "true"))
	FString ErrorMessage = "";

	// Number of availble messages in the queue, waiting to be read
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Info", meta = (BlueprintProtected = "true"))
	int32 MessageCount = 0;

private:

	// Read data in a buffer, if nbChar is greater than the
	// maximum number of bytes available, it will return only the
	// bytes available. The function return -1 when nothing could
	// be read, the number of bytes actually read.
	int ReadData(char *Buffer, uint32 BufferSize = 1024);

	// Writes data from a buffer through the Serial connection
	// return true on success.
	bool WriteData(char *Buffer, unsigned int BufferSize, bool IsRetry = false);

	// Sets the last error message
	void SetError(FString Message);

	// Serial com port handle
	HANDLE SerialHandle;

	// Get various information about the connection
	COMSTAT SerialStatus;

	// Keep track of last error
	unsigned long SerialError;

	// List of received messages
	TArray<FString> MessageList;

	// Current received message buffer
	FString MessageBuffer;

	// Has the connection ever been opened
	bool HasBeenOpen;
	
};
