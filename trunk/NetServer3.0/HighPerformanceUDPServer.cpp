#include "stdafx.h"

#include "HighPerformanceUDPServer.h"
#include "HighPerformanceUDPServerData.h"
#include <boost/pool/singleton_pool.hpp>

#pragma comment(lib, "ws2_32.lib")

//-----------------------------------------------------------------------------

typedef boost::singleton_pool<UDPOverlappedEvent, sizeof(UDPOverlappedEvent)> Pool_UDPOverlappedEvent;
typedef boost::singleton_pool<APCEvent, sizeof(APCEvent)> Pool_APCEvent;

//-----------------------------------------------------------------------------

// Default function place holder
void WINAPI DefaultOnClientToServer(HighPerformanceUDPServer * server, sockaddr_in & address, UINT16 count, UINT8 * data)
{
	UNREFERENCED_PARAMETER(address);
	UNREFERENCED_PARAMETER(count);
	UNREFERENCED_PARAMETER(data);
	UNREFERENCED_PARAMETER(server);
}

//-----------------------------------------------------------------------------

// The default ctor will setup our internal data object
HighPerformanceUDPServer::HighPerformanceUDPServer()
{
	internalData = new HighPerformanceUDPServerData(this);
}

//-----------------------------------------------------------------------------

// The default dtor will cleanup our internal data object
HighPerformanceUDPServer::~HighPerformanceUDPServer()
{
	delete internalData;
}

//-----------------------------------------------------------------------------

// Create the UDP server. Returns true on success and false on failure.
// numberOfConcurrentThreads - How many active IOCP threads are allowed (0 - auto detect)
// numberOfWorkerThreads - How many worker threads to create (0 - auto detect)
// maxPendingRecvs - How many pending receives to post (0 - auto detect, 2500 - 32bit max, 32767 - 64bit max)
// refillPendingRecvs - How many pending receives can be left before refilling the queue (0 - Use 1/2 of maxPendingRecvs)
bool HighPerformanceUDPServer::Create(UINT16 port, UINT32 numberOfConcurrentThreads, UINT32 numberOfWorkerThreads, LONG maxPendingRecvs, LONG refillPendingRecvs)
{
	return internalData->Create(port, numberOfConcurrentThreads, numberOfWorkerThreads, maxPendingRecvs, refillPendingRecvs);
}

//-----------------------------------------------------------------------------

// Close down the server and free internally used resources.
void HighPerformanceUDPServer::Destroy()
{
	return internalData->Destroy();
}

//-----------------------------------------------------------------------------

// Sets our callback function. If we wish to not have one, we may 
// pass 0 to use a default internal function.
void HighPerformanceUDPServer::SetClientToServerCallback(OnClientToServerFunc function)
{
	internalData->OnClientToServer = function;
	if(internalData->OnClientToServer == NULL)
	{
		internalData->OnClientToServer = DefaultOnClientToServer;
	}
}

//-----------------------------------------------------------------------------

// Send to a specific ip/port. Our function will cache the addresses
// so multiple calls will not incur the same overhead that a first
// call for a specific address might have.
bool HighPerformanceUDPServer::SendTo(const char * ip, USHORT port, UINT16 count, UINT8 * data)
{
	return internalData->SendTo(ip, port, count, data);
}

//-----------------------------------------------------------------------------

// Send to a specific address. We will use this most of the time as we
// are given a sockaddr_in object in our packet processing function.
bool HighPerformanceUDPServer::SendTo(const sockaddr_in & address, UINT16 count, UINT8 * data)
{
	return internalData->SendTo(address, count, data);
}

//-----------------------------------------------------------------------------

// Send to a specific ip/port. Our function will cache the addresses
// so multiple calls will not incur the same overhead that a first
// call for a specific address might have.
bool HighPerformanceUDPServer::SendTo(const char * ip, USHORT port, DWORD count, LPWSABUF buffers)
{
	return internalData->SendTo(ip, port, count, buffers);
}

//-----------------------------------------------------------------------------

// Send to a specific address. We will use this most of the time as we
// are given a sockaddr_in object in our packet processing function.
bool HighPerformanceUDPServer::SendTo(const sockaddr_in & address, DWORD count, LPWSABUF buffers)
{
	return internalData->SendTo(address, count, buffers);
}

//-----------------------------------------------------------------------------

// Gets/Sets user data for a custom context field for the object
void * HighPerformanceUDPServer::GetUserData() const
{
	return userData;
}

//-----------------------------------------------------------------------------

// Gets/Sets user data for a custom context field for the object
void HighPerformanceUDPServer::SetUserData(void * data)
{
	userData = data;
}

//-----------------------------------------------------------------------------

// Returns true if Winsock was setup or false if it failed
bool HighPerformanceUDPServer::Initialize()
{
	WSADATA wd = { 0 };
	if(WSAStartup(MAKEWORD(2, 2), &wd) != 0)
	{
		LOG("WSAStartup failed.");
		return false;
	}
	if(LOBYTE(wd.wVersion) != 2 || HIBYTE(wd.wVersion) != 2)
	{
		WSACleanup();
		LOG("WSAStartup failed to return the requested Winsock version.");
		return false;
	}
	Log::Setup();
	return true;
}

//-----------------------------------------------------------------------------

// Cleans up Winsock
void HighPerformanceUDPServer::Deinitialize()
{
	// Cleanup Winsock
	WSACleanup();

	// We have to have all threads cleaned up and terminated before we 
	// clean up the memory pools!
	Pool_UDPOverlappedEvent::purge_memory();
	Pool_APCEvent::purge_memory();

	// Cleanup the logging system
	Log::Cleanup();
}

//-----------------------------------------------------------------------------
