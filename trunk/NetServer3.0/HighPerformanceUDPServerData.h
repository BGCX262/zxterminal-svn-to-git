#pragma once

#ifndef UDP_HIGH_PERFORMANCE_SERVER_DATA_H_
#define UDP_HIGH_PERFORMANCE_SERVER_DATA_H_

//-----------------------------------------------------------------------------

#include "HighPerformanceUDPServer.h"
#include <string>
#include <map>

//-----------------------------------------------------------------------------

// Flags for the UDPOverlappedEvent operation. Since we use the 
// UDPOverlappedEvent object for multiple tasks, this allows
// us to know how to handle a pointer to one.
const BYTE HPS_OPERATION_READ = 0; // WSARecvFrom event
const BYTE HPS_OPERATION_WRITE = 1; // WSASendTo event

// Max bytes for the data buffers. UDP sockets are generally limited to
// being able to send no more than 65kb at a time. That is not to
// confuse how much data may be received by the socket. There is no way
// to obtain that value. See SO_MAX_MSG_SIZE for more information.
const int HPS_OVERLAPPED_BUFFER_SIZE = HPS_BUFFER_SIZE;

//-----------------------------------------------------------------------------

// This structure is our UDP event object that will hold the necessary
// information required for UDP based interactions. This is a generic
// object and will not hold any client context specific data.
struct UDPOverlappedEvent
{
	// A common mistake is to reuse an OVERLAPPED structure before the previous 
	// asynchronous operation has been completed. You should use a separate 
	// structure for each request. You should also create an event object for 
	// each thread that processes data.
	// http://msdn.microsoft.com/en-us/library/ms684342(VS.85).aspx
	OVERLAPPED overlapped;

	// The values indicated by lpFrom and lpFromlen are not updated until 
	// completion is itself indicated. Applications must not use or disturb 
	// these values until they have been updated, therefore the application 
	// must not use automatic (that is, stack-based) variables for 
	// these parameters.
	// http://msdn.microsoft.com/en-us/library/ms741686(VS.85).aspx
	sockaddr_in address;
	int addressLength;

	// A flag for which operation this event represents.
	BYTE operation;

	// Data we wish to send or receive as well as the byte count. Since
	// UDP is a packet based protocol, we do not have to worry about
	// split packets from the transport layer.
	UINT16 dataBufferSize;
	UINT8 dataBufferData[HPS_OVERLAPPED_BUFFER_SIZE];
};

//-----------------------------------------------------------------------------

// This structure is passed to our APC function to handle the event
// processing logic.
class HighPerformanceUDPServerData; // Forward declare
struct APCEvent
{
	HighPerformanceUDPServerData * server;
	UDPOverlappedEvent * event;
};

//-----------------------------------------------------------------------------

// Simple structure to hold worker thread information. We
// need to make sure we terminate all threads before the
// program exits, so this structure helps us with that.
struct WorkerThreadData
{
	HANDLE hThread;
	DWORD dwThreadId;
};

//-----------------------------------------------------------------------------

class HighPerformanceUDPServerData
{
private:
	// Which port this server will run on
	UINT16 port;

	// Our IOCP handle
	HANDLE completionPort;

	// IOCP settings that allow us to scale the server based on
	// the hardware we have. By default, we will allocate one
	// thread per processor and allow 2 worker threads per
	// processor.
	UINT32 numberOfConcurrentThreads;
	UINT32 numberOfWorkerThreads;

	// Our UDP socket
	SOCKET socket_;

	// The address the server runs on
	sockaddr_in internetAddr;

	// These variables control how many posted overlapped 
	// receives we can have at a time.
	LONG maxPendingRecvs;
	LONG curPendingRecvs;
	LONG refillPendingRecvs;

	// An event to signal when we should refill the posted
	// pending overlapped receive queue.
	HANDLE hRefillEvent;

	// Our event processing thread handle
	HANDLE hEventProcessingThread;

	// An array of worker thread handles to help us
	// clean up when the server needs to exit
	WorkerThreadData * workerThreadHandles;

	// The HighPerformanceUDPServer object that owns this object
	HighPerformanceUDPServer * parent;

	// A cache of addresses to make sending to an address more
	// efficient in the long run.
	std::map<std::string, hostent *> addressCache;

public:
	// This is our function pointer to an external function that
	// will handle client to server data. We only make this public
	// so we don't have to 'friend' the HighPerformanceUDPServer
	// class to this class. Since this class is not exposed to
	// anyone else, it is 'ok'.
	OnClientToServerFunc OnClientToServer;

public:
	HighPerformanceUDPServerData(HighPerformanceUDPServer * parent_);
	~HighPerformanceUDPServerData();

	// Create the UDP server. Returns true on success and false on failure.
	bool Create(UINT16 port_, UINT32 numberOfConcurrentThreads_, UINT32 numberOfWorkerThreads_, LONG maxPendingRecvs_, LONG refillPendingRecvs_);

	// Close down the server and free internally used resources.
	void Destroy();

	// Dispatches the UDP event (one buffer at a time)
	bool Send(UDPOverlappedEvent * event);

	// Dispatches the UDP event (multiple buffers at a time)
	bool Send(UDPOverlappedEvent * event, DWORD count, LPWSABUF buffers);

	// Returns a hostent object with the information about a host
	hostent * GetHost(const char * host);

	// Send to a specific ip/port. Our function will cache the addresses
	// so multiple calls will not incur the same overhead that a first
	// call for a specific address might have.
	bool SendTo(const char * ip, USHORT port, UINT16 count, UINT8 * data);
	bool SendTo(const char * ip, USHORT port, DWORD count, LPWSABUF buffers);

	// Send to a specific address. We will use this most of the time as we
	// are given a sockaddr_in object in our packet processing function.
	bool SendTo(const sockaddr_in & address, UINT16 count, UINT8 * data);
	bool SendTo(const sockaddr_in & address, DWORD count, LPWSABUF buffers);

	// This function will process an event sent from a client to the server. It
	// is only to be invoked from the ProcessEventWrapper function, which is turn
	// is only invoked from the APCEventProcessThread function. You may not call
	// this function yourself as it would break the thread-safe guarantee.
	void ProcessEvent(UDPOverlappedEvent * event);

	// This function will loop through dispatching APC events as they are posted
	// to this threads APC event queue. It will also keep overlapped events posted 
	// as needed.
	void APCEventProcessThread();

	// This is the worker thread for the IOCP process. It's task is to poll overlapped
	// completion events and dispatch them to the event processing thread.
	void WorkerThread();
};

//-----------------------------------------------------------------------------

namespace Log
{
	// This is our simple logging function. It is styled to work similarly to printf.
	void Log(const char * file, const char * function, int line, const char * msg, ...);
	void Setup();
	void Cleanup();
}

//-----------------------------------------------------------------------------

// __VA_ARGS__ is only supported on VS2005 and above! We use a macro to cut down
// on having to type out the file, function, and line preprocessor macros each time.
#define LOG(msg, ...) Log::Log(__FILE__, __FUNCTION__, __LINE__, msg, __VA_ARGS__);

//-----------------------------------------------------------------------------

#endif
