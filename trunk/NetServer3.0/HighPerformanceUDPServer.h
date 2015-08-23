#pragma once

#ifndef UDP_HIGH_PERFORMANCE_SERVER_H_
#define UDP_HIGH_PERFORMANCE_SERVER_H_

//-----------------------------------------------------------------------------

#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>

//-----------------------------------------------------------------------------

// Forward declarations
class HighPerformanceUDPServerData;
class HighPerformanceUDPServer;

//-----------------------------------------------------------------------------

// This is a function pointer typedef that we will use to register a function
// to receive packets from clients sent to the server with in our main program.
typedef void (WINAPI * OnClientToServerFunc)(HighPerformanceUDPServer * server, sockaddr_in & address, UINT16 count, UINT8 * data);

//-----------------------------------------------------------------------------

// How many bytes the internal buffer size is
const int HPS_BUFFER_SIZE = 8192;

//-----------------------------------------------------------------------------

class HighPerformanceUDPServer
{
private:
	// Our PIMPL pattern in action
	HighPerformanceUDPServerData * internalData;

	// Explicitly disable assignment
	HighPerformanceUDPServer & operator = (HighPerformanceUDPServer & rhs);

	// Explicitly disable copy construction
	HighPerformanceUDPServer(const HighPerformanceUDPServer & rhs);

	// Allow the user to assign a user data field to this object
	void * userData;

public:
	// Returns true on success and false on failure. You only should call this
	// function once before using the library.
	static bool Initialize();

	// Cleans up the library. You should only call this function once at program
	// exit.
	static void Deinitialize();

public:
	HighPerformanceUDPServer();
	~HighPerformanceUDPServer();

	// Create the UDP server. Returns true on success and false on failure.
	// numberOfConcurrentThreads - How many active IOCP threads are allowed (0 - auto detect)
	// numberOfWorkerThreads - How many worker threads to create (0 - auto detect)
	// maxPendingRecvs - How many pending receives to post (0 - auto detect, 2500 - 32bit max, 32767 - 64bit max)
	// refillPendingRecvs - How many pending receives can be left before refilling the queue (0 - Use 1/2 of maxPendingRecvs)
	bool Create(UINT16 port, UINT32 numberOfConcurrentThreads = 0,  UINT32 numberOfWorkerThreads = 0, LONG maxPendingRecvs = 0, LONG refillPendingRecvs = 0);

	// Close down the server and free internally used resources.
	void Destroy();

	// Sets our callback function. If we wish to not have one, we may 
	// pass 0 to use a default internal function.
	void SetClientToServerCallback(OnClientToServerFunc function);

	// Send to a specific ip/port. Our function will cache the addresses
	// so multiple calls will not incur the same overhead that a first
	// call for a specific address might have.
	bool SendTo(const char * ip, USHORT port, UINT16 count, UINT8 * data);
	bool SendTo(const char * ip, USHORT port, DWORD count, LPWSABUF buffers);

	// Send to a specific address. We will use this most of the time as we
	// are given a sockaddr_in object in our packet processing function.
	bool SendTo(const sockaddr_in & address, UINT16 count, UINT8 * data);
	bool SendTo(const sockaddr_in & address, DWORD count, LPWSABUF buffers);

	// Gets/Sets user data for a custom context field for the object
	void * GetUserData() const;
	void SetUserData(void * data);
};

//-----------------------------------------------------------------------------

#endif
