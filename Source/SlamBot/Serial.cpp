#include "SlamBot.h"
#include "Serial.h"

DEFINE_LOG_CATEGORY(SerialLog);

ASerial::ASerial()
{
	PrimaryActorTick.bCanEverTick = true;

	IsConnected = false;
	HasBeenOpen = false;
	MessageCount = 0;
}

void ASerial::BeginPlay()
{
	Super::BeginPlay();
}

void ASerial::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ClosePort();
}

void ASerial::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	const int BUFFER_SIZE = 1024;
	char SerialData[BUFFER_SIZE] = "";

	int BytesRead = ReadData(SerialData, BUFFER_SIZE);

	for (int i = 0; i < BytesRead; i++)
	{
		char character = SerialData[i];

		if (character == '\r')
		{
			// just ignore it
		}
		else if (character == '\n')
		{
			MessageList.Add(MessageBuffer);

			UE_LOG(SerialLog, Log, TEXT("Got message: '%s'"), *MessageBuffer);

			MessageBuffer = "";
		}
		else
		{
			MessageBuffer += character;
		}
	}

	MessageCount = MessageList.Num();
}

bool ASerial::OpenPort(FString Name)
{
	PortName = Name;

	UE_LOG(SerialLog, Log, TEXT("Opening serial port '%s'"), *PortName);
	
	// reset connected state
	IsConnected = false;

	// support ports beyong the first 9
	FString OpenPortName = "\\\\.\\" + PortName;

	// try to connect to the given port
	SerialHandle = CreateFile(
		*OpenPortName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	// check whether the connection was successfull
	if (SerialHandle == INVALID_HANDLE_VALUE)
	{
		// display error if failed
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			SetError(FString::Printf(TEXT("Failed opening serial port '%s' - port not available"), *PortName));
		}
		else
		{
			SetError(FString::Printf(TEXT("Failed opening serial port '%s' - unknown reason"), *PortName));
		}

		return false;
	}

	// set the serial port parameters
	DCB dcbSerialParams = { 0 };

	//Try to get the current
	if (!GetCommState(SerialHandle, &dcbSerialParams))
	{
		SetError(FString::Printf(TEXT("Getting serial port '%s' parameters failed"), *PortName));
	
		return false;
	}

	// setup parameters
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

	// set the parameters
	if (!SetCommState(SerialHandle, &dcbSerialParams))
	{
		SetError(FString::Printf(TEXT("Setting serial port '%s' parameters failed"), *PortName));

		return false;
	}

	// flush any remaining characters in the buffers 
	PurgeComm(SerialHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);

	// we're now connected!
	IsConnected = true;
	HasBeenOpen = true;

	return true;
}

void ASerial::ClosePort()
{
	//Check if we are connected before trying to disconnect
	if (!IsConnected)
	{
		return;
	}

	// we're no longer connected
	IsConnected = false;

	// close the handle
	CloseHandle(SerialHandle);

	UE_LOG(SerialLog, Log, TEXT("Closed serial port"));
}

bool ASerial::IsPortOpen()
{
	return IsConnected;
}

FString ASerial::GetNextMessage()
{
	if (MessageList.Num() == 0)
	{
		return "";
	}

	FString Message = MessageList[0];

	MessageList.RemoveAt(0);

	MessageCount = MessageList.Num();

	return Message;
}

void ASerial::SendMessage(FString Message, bool IncludeNewline)
{
	FString MessageToSend = IncludeNewline ? Message + '\n' : Message;

	WriteData(TCHAR_TO_ANSI(*MessageToSend), MessageToSend.Len());
}

int ASerial::ReadData(char *Buffer, uint32 BufferSize)
{
	unsigned long BytesRead;
	uint32 BytesToRead;

	// get serial port status
	ClearCommError(SerialHandle, &SerialError, &SerialStatus);

	// check whether there is something to read
	if (SerialStatus.cbInQue > 0)
	{
		// if there is we check if there is enough data to read the required number
		// of characters, if not we'll read only the available characters to prevent
		// locking of the application.
		if (BufferSize > 0 && SerialStatus.cbInQue > BufferSize - 1)
		{
			BytesToRead = BufferSize - 1;
		}
		else
		{
			BytesToRead = SerialStatus.cbInQue;
		}

		// try to read the require number of chars, and return the number of read bytes on success
		if (ReadFile(SerialHandle, Buffer, BytesToRead, &BytesRead, NULL))
		{
			Buffer[BytesRead] = 0;

			return BytesRead;
		}
	}

	// if nothing has been read, or that an error was detected return 0
	return 0;
}

bool ASerial::WriteData(char *Buffer, unsigned int BufferSize, bool IsRetry)
{
	// attempt to open the connection if not open but name is available
	if (!IsConnected && HasBeenOpen && PortName.Len() > 0)
	{
		OpenPort(PortName);
	}

	// don't even try if not connected
	if (!IsConnected)
	{
		SetError("Writing to serial requested but connection is not open");

		return false;
	}

	unsigned long BytesSent;

	// try to send the buffer
	if (!WriteFile(SerialHandle, (void *)Buffer, BufferSize, &BytesSent, 0))
	{
		// in case it don't work get the error and return false
		ClearCommError(SerialHandle, &SerialError, &SerialStatus);

		ClosePort();

		// try to open the port and send again
		if (!IsRetry)
		{
			OpenPort(PortName);

			WriteData(Buffer, BufferSize, true);
		}
		else
		{
			SetError("Writing to serial requested but operation failed");
		}

		return false;
	}

	return true;
}

void ASerial::SetError(FString Message)
{
	ErrorMessage = Message;
	UE_LOG(SerialLog, Error, TEXT("ERROR: %s"), *ErrorMessage);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);
}