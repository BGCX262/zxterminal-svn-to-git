#include "stdafx.h"

#define _WIN32_WINNT 0x0400 // for QueueUserAPC
#include "HighPerformanceUDPServerData.h"
#include <boost/pool/singleton_pool.hpp>

//-----------------------------------------------------------------------------

// These are our two singleton pool objects that track memory for us. For
// this code, I choose to use them as pools are extremely more efficient
// for server processing rather than constant allocations and deallocations
// on the heap. There are some drawbacks to using these boost pools though,
// but they are acceptable for now. By default, the pools start out with
// 32 objects and grow by a factor of 2 each time more memory is needed.
// We can optimize this by simply allocation a whole bunch of objects
// at startup and freeing them, keeping a high reserve of objects 
// available, but for this simple example, we do not need to.

typedef boost::singleton_pool<UDPOverlappedEvent, sizeof(UDPOverlappedEvent)> Pool_UDPOverlappedEvent;
typedef boost::singleton_pool<APCEvent, sizeof(APCEvent)> Pool_APCEvent;

//-----------------------------------------------------------------------------

// This is our asynchronous message handling function
VOID CALLBACK ProcessEventWrapper(ULONG_PTR param);

// This function runs the worker thread function
DWORD WINAPI WorkerThreadWrapper(LPVOID lpParam);

// This is the message dispatching function for our APC function.
DWORD WINAPI APCEventProcessThreadWrapper(LPVOID lpParam);

// Default function place holder
void WINAPI DefaultOnClientToServer(HighPerformanceUDPServer * server, sockaddr_in & address, UINT16 count, UINT8 * data);

//-----------------------------------------------------------------------------

HighPerformanceUDPServerData::HighPerformanceUDPServerData(HighPerformanceUDPServer * parent_)
{
	port = 0;
	completionPort = INVALID_HANDLE_VALUE;
	numberOfConcurrentThreads = 0;
	numberOfWorkerThreads = 0;
	socket_ = INVALID_SOCKET;
	hRefillEvent = NULL;
	maxPendingRecvs = 0;
	refillPendingRecvs = 0;
	curPendingRecvs = 0;
	hEventProcessingThread = NULL;
	workerThreadHandles = NULL;
	memset(&internetAddr, 0, sizeof(internetAddr));
	OnClientToServer = DefaultOnClientToServer;
	parent = parent_;
}

//-----------------------------------------------------------------------------

HighPerformanceUDPServerData::~HighPerformanceUDPServerData()
{
}

//-----------------------------------------------------------------------------

bool HighPerformanceUDPServerData::Create(UINT16 port_, UINT32 numberOfConcurrentThreads_, UINT32 numberOfWorkerThreads_, LONG maxPendingRecvs_, LONG refillPendingRecvs_)
{
	// Get the system information
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	port = port_;
	numberOfConcurrentThreads = numberOfConcurrentThreads_;
	numberOfWorkerThreads = numberOfWorkerThreads_;
	maxPendingRecvs = maxPendingRecvs_;
	refillPendingRecvs = refillPendingRecvs_;

	// Try to create an I/O completion port
	completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, numberOfConcurrentThreads);
	if(completionPort == NULL)
	{
		LOG("Could not create the IOCP. GetLastError returned [%i]", GetLastError());
		Destroy();
		return false;
	}

	// Calculate how many worker threads we should create to process IOCP events
	if(numberOfWorkerThreads == 0)
	{
		if(numberOfConcurrentThreads == 0)
		{
			// It's important to understand that the dwNumberOfProcessors is not always the 
			// 'best' method of getting the processors on a system since it will return
			// active cores rather than physical processors. Even so though, we will base
			// it on total cores.
			numberOfWorkerThreads = SystemInfo.dwNumberOfProcessors * 2;
		}
		else
		{
			numberOfWorkerThreads = numberOfConcurrentThreads * 2;
		}
	}

	// Create an array of thread information objects. Since we only will be creating
	// a handful of these only once, there is no need to make a pool of data; using
	// new is ok.
	workerThreadHandles = new WorkerThreadData[numberOfWorkerThreads];
	memset(workerThreadHandles, 0, sizeof(WorkerThreadData) * numberOfWorkerThreads);

	// This event will signal when we need to refill the posted receive queue.
	hRefillEvent = CreateEvent(0, TRUE, FALSE, 0);
	if(hRefillEvent == 0)
	{
		LOG("Could not create the Refill event. GetLastError returned [%i]", GetLastError());
		Destroy();
		return false;
	}

	// Setup our overlapped UDP socket
	socket_ = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(socket_ == INVALID_SOCKET)
	{
		LOG("Could not create the main socket. WSAGetLastError returned [%i]", WSAGetLastError());
		Destroy();
		return false;
	}

	// Allow LAN based broadcasts since this option is disabled by default.
	int value = 1;
	setsockopt(socket_, SOL_SOCKET, SO_BROADCAST, (char *)&value, sizeof(int));

	// Bind the socket to the requested port
	internetAddr.sin_family = AF_INET;
	internetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	internetAddr.sin_port = htons(port);
	int bindResult = bind(socket_, reinterpret_cast<PSOCKADDR>(&internetAddr), sizeof(internetAddr));
	if(bindResult == SOCKET_ERROR)
	{
		LOG("Could not bind the main socket. WSAGetLastError returned [%i]", WSAGetLastError());
		Destroy();
		return false;
	}

	// Connect the listener socket to IOCP
	if(CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket_), completionPort, 0, numberOfConcurrentThreads) == 0)
	{
		LOG("Could not assign the socket to the IOCP handle. GetLastError returned [%i]", GetLastError());
		Destroy();
		return false;
	}

	// Set automatic detection of pending overlapped receives. To do this we check the size of
	// a pointer. If the pointer is 8 bytes, we are in 64-bit Windows and can have 32,767
	// pending receives. Otherwise, we are in 32-bit Windows and we can only have 2500. This
	// is a bit of a hack for now, but it should work reliably.
	if(maxPendingRecvs == 0) maxPendingRecvs = (sizeof(UINT8 *) == 8 ? 32767 : 2500);
	if(refillPendingRecvs == 0) refillPendingRecvs = static_cast<LONG>(.5 * maxPendingRecvs);

	// This is our main event processing thread that we use to handle APC events
	// that hold network data.
	hEventProcessingThread = CreateThread(0, 0, APCEventProcessThreadWrapper, this, 0, 0);
	if(hEventProcessingThread == 0)
	{
		LOG("Could not create the event processing thread. GetLastError returned [%i]", GetLastError());
		Destroy();
		return false;
	}

	// Create all of our worker threads
	for(UINT32 x = 0; x < numberOfWorkerThreads; ++x)
	{
		// We only create in the suspended state so our workerThreadHandles object
		// is updated before the thread runs.
		workerThreadHandles[x].hThread = CreateThread(0, 0, WorkerThreadWrapper, this, CREATE_SUSPENDED, &workerThreadHandles[x].dwThreadId);
		if(workerThreadHandles[x].hThread != NULL)
		{
			ResumeThread(workerThreadHandles[x].hThread);
		}
		else
		{
			LOG("A worker thread was not able to be created. GetLastError returned [%i]", GetLastError());
		}
	}

	// Success
	return true;
}

//-----------------------------------------------------------------------------

void HighPerformanceUDPServerData::Destroy()
{
	// Clean up the worker threads waiting on the IOCP
	if(completionPort != INVALID_HANDLE_VALUE)
	{
		// Destroy all of our worker threads by posting an IOCP event that signals the threads
		// to exit. There is no other way around having to do this that is more efficient!
		for(UINT32 x = 0; x < numberOfWorkerThreads; ++x)
		{
			PostQueuedCompletionStatus(completionPort, 0, static_cast<ULONG_PTR>(-1), 0);
			Sleep(0); // Try to give up our time slice so the other threads can exit
		}
	}

	// If we have worker threads allocated
	if(workerThreadHandles)
	{
		// Wait for all worker threads to close
		for(UINT32 x = 0; x < numberOfWorkerThreads; x++)
		{
			if(workerThreadHandles[x].hThread != NULL)
			{
				// Arbitrarily wait a second, it should never take more than a couple ms if
				// that, but if it does timeout, something is wrong. We can terminate the
				// thread forcefully safely.
				DWORD dwResult = WaitForSingleObject(workerThreadHandles[x].hThread, 1000);
				if(dwResult == WAIT_TIMEOUT)
				{
					LOG("A worker thread did not properly terminate in 1 second.");
					TerminateThread(workerThreadHandles[x].hThread, 0);
				}
			}
		}
		delete [] workerThreadHandles;
		workerThreadHandles = 0;
	}

	// If we have a socket to cleanup, do so. We close this here to force our UDP
	// processing thread to generate an error when it uses the socket again.
	if(socket_!= INVALID_SOCKET)
	{
		closesocket(socket_);
		socket_ = INVALID_SOCKET;
	}

	if(completionPort != INVALID_HANDLE_VALUE)
	{
		CloseHandle(completionPort);
		completionPort = INVALID_HANDLE_VALUE;
	}

	// We clean up this last so we can give our socket time to close
	if(hEventProcessingThread != NULL)
	{
		// Set the refill event to trigger the event processing thread out of
		// it's waiting loop. We resort to the socket closing logic and
		// error generation to avoid checking a flag each loop to exit.
		SetEvent(hRefillEvent);
		if(WaitForSingleObject(hEventProcessingThread, 5000) == WAIT_TIMEOUT)
		{
			LOG("The packe processing thread did not properly terminate in 5 seconds.");
			TerminateThread(hEventProcessingThread, 0);
		}
		hEventProcessingThread = NULL;
		CloseHandle(hRefillEvent);
		hRefillEvent = NULL;
	}
}

//-----------------------------------------------------------------------------

bool HighPerformanceUDPServerData::Send(UDPOverlappedEvent * event)
{
	// This is not used in our function, but we will have lower layer memory exceptions raised
	// if we don't pass in an address!
	DWORD dwSent = 0;

	// Store the buffers Winsock should use for handling the send
	WSABUF sendBufferDescriptor;
	sendBufferDescriptor.buf = reinterpret_cast<char *>(event->dataBufferData);
	sendBufferDescriptor.len = event->dataBufferSize;

	// Send the packet! If all goes well, we will get an IOCP notification when it's sent
	int result = WSASendTo(socket_, &sendBufferDescriptor, 1, &dwSent, 0, reinterpret_cast<sockaddr *>(&event->address), event->addressLength, &event->overlapped, NULL);
	if(result == SOCKET_ERROR) // We don't expect this, but...
	{
		result = WSAGetLastError();
		if(result != WSA_IO_PENDING) // This means everything went well
		{
			LOG("WSASendTo failed. WSAGetLastError returned [%i]", WSAGetLastError());

			// Since we could not send the data, we need to cleanup this
			// event so it's not dead weight in our memory pool.
			Pool_UDPOverlappedEvent::free(event);
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------

// Returns a hostent object with the information about a host
hostent * HighPerformanceUDPServerData::GetHost(const char * host)
{
	if(inet_addr(host) == INADDR_NONE)
	{
		return gethostbyname(host);
	}
	else
	{
		unsigned long addr = 0;
		addr = inet_addr(host);
		return gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
	}
}

//-----------------------------------------------------------------------------

bool HighPerformanceUDPServerData::SendTo(const char * ip, USHORT port, UINT16 count, UINT8 * data)
{
	// Make sure we do not overflow the buffer
	if(count > HPS_OVERLAPPED_BUFFER_SIZE)
	{
		return false;
	}

	// Host information
	hostent * host = 0;

	// Simple cache system
	std::map<std::string, hostent *>::iterator itr = addressCache.find(ip);
	if(itr != addressCache.end())
	{
		host = itr->second;
	}
	else
	{
		host = GetHost(ip);
		addressCache[ip] = host;
	}

	// Make sure we have a valid host to sent to
	if(host == 0)
	{
		return false;
	}

	// Create our event object
	UDPOverlappedEvent * event = reinterpret_cast<UDPOverlappedEvent *>(Pool_UDPOverlappedEvent::malloc());

	// This is mandatory! We can cause crashes if we 
	// forget this due to how this object is used internally.
	memset(&event->overlapped, 0, sizeof(OVERLAPPED));

	event->addressLength = sizeof(sockaddr_in);
	event->address.sin_family = AF_INET;
	event->address.sin_port = htons(port);
	event->address.sin_addr.s_addr = *(reinterpret_cast<unsigned long *>(host->h_addr));

	event->dataBufferSize = count;
	memcpy(event->dataBufferData, data, count);

	event->operation = HPS_OPERATION_WRITE;

	// Finally we can dispatch it
	return Send(event);
}

//-----------------------------------------------------------------------------

bool HighPerformanceUDPServerData::SendTo(const sockaddr_in & address, UINT16 count, UINT8 * data)
{
	// Make sure we do not overflow the buffer
	if(count > HPS_OVERLAPPED_BUFFER_SIZE) return false;

	// Create our event object
	UDPOverlappedEvent * event = reinterpret_cast<UDPOverlappedEvent *>(Pool_UDPOverlappedEvent::malloc());

	// This is mandatory! We can cause crashes if we 
	// forget this due to how this object is used internally.
	memset(&event->overlapped, 0, sizeof(OVERLAPPED));

	event->addressLength = sizeof(sockaddr_in);
	memcpy(&event->address, &address, event->addressLength);

	event->dataBufferSize = count;
	memcpy(event->dataBufferData, data, count);

	event->operation = HPS_OPERATION_WRITE;

	// Finally we can dispatch it
	return Send(event);
}

//-----------------------------------------------------------------------------

// Send multiple buffers at a time
bool HighPerformanceUDPServerData::Send(UDPOverlappedEvent * event, DWORD count, LPWSABUF buffers)
{
	// This is not used in our function, but we will have lower layer memory exceptions raised
	// if we don't pass in an address!
	DWORD dwSent = 0;

	// Send the packet! If all goes well, we will get an IOCP notification when it's sent
	int result = WSASendTo(socket_, buffers, count, &dwSent, 0, reinterpret_cast<sockaddr *>(&event->address), event->addressLength, &event->overlapped, NULL);
	if(result == SOCKET_ERROR) // We don't expect this, but...
	{
		result = WSAGetLastError();
		if(result != WSA_IO_PENDING) // This means everything went well
		{
			LOG("WSASendTo failed. WSAGetLastError returned [%i]", WSAGetLastError());

			// Since we could not send the data, we need to cleanup this
			// event so it's not dead weight in our memory pool.
			Pool_UDPOverlappedEvent::free(event);
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------

bool HighPerformanceUDPServerData::SendTo(const sockaddr_in & address, DWORD count, LPWSABUF buffers)
{
	// Make sure we do not overflow the buffer
	if(count > HPS_OVERLAPPED_BUFFER_SIZE) return false;

	// Create our event object
	UDPOverlappedEvent * event = reinterpret_cast<UDPOverlappedEvent *>(Pool_UDPOverlappedEvent::malloc());

	// This is mandatory! We can cause crashes if we 
	// forget this due to how this object is used internally.
	memset(&event->overlapped, 0, sizeof(OVERLAPPED));
	event->addressLength = sizeof(sockaddr_in);
	memcpy(&event->address, &address, event->addressLength);
	event->operation = HPS_OPERATION_WRITE;

	// Finally we can dispatch it
	return Send(event, count, buffers);
}

//-----------------------------------------------------------------------------

bool HighPerformanceUDPServerData::SendTo(const char * ip, USHORT port, DWORD count, LPWSABUF buffers)
{
	// Make sure we do not overflow the buffer
	if(count > HPS_OVERLAPPED_BUFFER_SIZE)
	{
		return false;
	}

	// Host information
	hostent * host = 0;

	// Simple cache system
	std::map<std::string, hostent *>::iterator itr = addressCache.find(ip);
	if(itr != addressCache.end())
	{
		host = itr->second;
	}
	else
	{
		host = GetHost(ip);
		if(host)
		{
			addressCache[ip] = host;
		}
	}

	// Make sure we have a valid host to send to
	if(host == 0)
	{
		return false;
	}

	// Create our event object
	UDPOverlappedEvent * event = reinterpret_cast<UDPOverlappedEvent *>(Pool_UDPOverlappedEvent::malloc());

	// This is mandatory! We can cause crashes if we 
	// forget this due to how this object is used internally.
	memset(&event->overlapped, 0, sizeof(OVERLAPPED));

	event->addressLength = sizeof(sockaddr_in);
	event->address.sin_family = AF_INET;
	event->address.sin_port = htons(port);
	event->address.sin_addr.s_addr = *(reinterpret_cast<unsigned long *>(host->h_addr));
	event->operation = HPS_OPERATION_WRITE;

	// Finally we can dispatch it
	return Send(event, count, buffers);
}

//-----------------------------------------------------------------------------

// This function will process an event sent from a client to the server. It
// is only to be invoked from the ProcessEventWrapper function, which is turn
// is only invoked from the APCEventProcessThread function. You may not call
// this function yourself as it would break the thread-safe guarantee.
void HighPerformanceUDPServerData::ProcessEvent(UDPOverlappedEvent * event)
{
	// Dispatch the event to the external function.
	OnClientToServer(parent, event->address, event->dataBufferSize, event->dataBufferData);

	// Free memory as we no longer need it and it 
	// will not be used anywhere else in the program.
	// if the user still needs it, it will be up to them
	// to copy it and maintain it as they see fit.
	Pool_UDPOverlappedEvent::free(event);
}

//-----------------------------------------------------------------------------

// This function will loop through dispatching APC events as they are posted
// to this threads APC event queue. It will also keep overlapped events posted 
// as needed.
void HighPerformanceUDPServerData::APCEventProcessThread()
{
	// Start out by signaling that we need to fill the pending overlapped receive buffer
	SetEvent(hRefillEvent);

	// Loop forever until we need to exit the thread. When we do need to exit, a socket
	// error will force a return from the function.
	for(;;)
	{
		// Wait until we need to refill the pending receive queue or we have
		// and APC to handle.
		if(WaitForSingleObjectEx(hRefillEvent, INFINITE, TRUE) == WAIT_OBJECT_0)
		{
			// Store how many new overlapped reads we need to post. Note we always want to try
			// and post at least one event so we can detect when we need to exit. This is a more
			//efficient way of detecting exit rather than having to always check another event.
			LONG dwEnd = max(1, maxPendingRecvs - curPendingRecvs);
			for(LONG x = 0; x < dwEnd; ++x)
			{
				UDPOverlappedEvent * event = reinterpret_cast<UDPOverlappedEvent *>(Pool_UDPOverlappedEvent::malloc());

				// This is mandatory! We can cause crashes if we 
				// forget this due to how this object is used internally.
				memset(&event->overlapped, 0, sizeof(OVERLAPPED));

				event->addressLength = sizeof(sockaddr_in);

				event->address.sin_family = AF_INET;
				event->address.sin_port = htons(port);
				event->address.sin_addr.s_addr = INADDR_ANY;

				event->operation = HPS_OPERATION_READ;

				// If WSARecvFrom is completed in an overlapped manner, it is the Winsock service provider's 
				// responsibility to capture the WSABUF structures before returning from this call. This enables 
				// applications to build stack-based WSABUF arrays pointed to by the lpBuffers parameter.
				// http://msdn.microsoft.com/en-us/library/ms741686(VS.85).aspx
				WSABUF recvBufferDescriptor;
				recvBufferDescriptor.buf = reinterpret_cast<char *>(event->dataBufferData);
				recvBufferDescriptor.len = HPS_OVERLAPPED_BUFFER_SIZE;

				// Post our overlapped receive event on the socket. We expect to get an error in this situation.
				DWORD numberOfBytes = 0;
				DWORD recvFlags = 0;
				INT result = WSARecvFrom(socket_, &recvBufferDescriptor, 1, &numberOfBytes, &recvFlags, reinterpret_cast<sockaddr *>(&event->address), &event->addressLength, &event->overlapped, NULL);
				if(result == SOCKET_ERROR)
				{
					int error = WSAGetLastError();

					// We expect to get an ERROR_IO_PENDING result as we are posting overlapped events
					if(error != ERROR_IO_PENDING)
					{
						// If we get here, then we are trying to post more events than 
						// we can currently handle, so we just break out of the loop
						if(error == WSAENOBUFS)
						{
							break;
						}
						// Time to exit since the socket was closed. This should only happen
						// if Destroy is called.
						else if(error == WSAENOTSOCK)
						{
							return;
						}
						// Otherwise, we have some other error to handle
						else
						{
							LOG("WSARecvFrom failed. WSAGetLastError returned [%i]", error);
						}
					}
					// As expected
					else
					{
						// Add a pending overlapped events just posted
						InterlockedIncrement(&curPendingRecvs);
					}
				}
				else
				{
					// If we get here, we did not have enough pending receives posted on the socket.
					// This is ok because a worker thread will still process the data, it's just we
					// hit a burst operation (can be simulated by holding the console thread) and
					// we churned through the data. It is very important we do not free the 'event' 
					// pointer here! It *will* be handled by a WorkerThread. There is nothing we 
					// need to do here really.
				}
			}

			// We now need to check to make sure our pending receive count is greater
			// than our warning pending receive count. The reason we have to do this is
			// in case we consume events too fast in a burst of traffic or the thread
			// is blocked and we are not in a state of being able to satisfy more events.
			// if we do not check this here, we could essentially kill the server by not
			// having any overlapped io events to be retrieved by the worker threads!
			if(curPendingRecvs > refillPendingRecvs && curPendingRecvs > 0)
			{
				// Allow the event to be signaled now that we have more reads
				// posted. If this is not executed, then we will want to execute our
				// previous loop until this actually happens.
				ResetEvent(hRefillEvent);
			}
		}
		else
		{
			// Otherwise we received an APC and it is going to be handled by our event 
			// processing function automatically for us. We do not need to call reset
			// event here since the event was not signaled; the thread was alerted.
		}
	}
}

//-----------------------------------------------------------------------------

// This is the worker thread for the IOCP process. It's task is to poll overlapped
// completion events and dispatch them to the event processing thread.
void HighPerformanceUDPServerData::WorkerThread()
{
	BOOL result = 0;
	DWORD numberOfBytes = 0;
	ULONG_PTR key = 0;
	OVERLAPPED * lpOverlapped = 0;
	UDPOverlappedEvent * event = 0;
	for(;;)
	{
		// Wait for our IOCP notifications :) With this setup, we block indefinitely, so when we want
		// the thread to exit, we must use the PostQueuedCompletionStatus function with a special key
		// value of -1 to signal the thread should exit.
		result = GetQueuedCompletionStatus(completionPort, &numberOfBytes, &key, &lpOverlapped, INFINITE);

		// We reserve a special event of having a passed key of -1 to signal an exit. In this case
		// we just want to break from our infinite loop. Since we are using pools to manage memory,
		// we do not have to worry about anything else here.
		if(key == -1)
		{
			break;
		}

		// Obtain the UDPOverlappedEvent pointer based on the address we received from
		// GetQueuedCompletionStatus in the overlapped field. The simple theory is that
		// when we pass the overlapped field from a UDPOverlappedEvent object into 
		// WSARecvFrom, we get that pointer here, so we can calculate where the overlapped
		// field is inside the structure and move the pointer backwards to the start of the
		// object. This is the safe and preferred way of doing this task!
		event = CONTAINING_RECORD(lpOverlapped, UDPOverlappedEvent, overlapped);

		// If the GetQueuedCompletionStatus call was successful, we can pass our data to the
		// thread for processing network data.
		if(result == TRUE)
		{
			// No key means no context operation data, so WSARecvFrom/WSASendTo generated
			// this event for us.
			if(key == 0)
			{
				// WSARecvFrom event
				if(event->operation == HPS_OPERATION_READ)
				{
					// If we now have less pending receives than our minimal quota, set the event to signal
					// a refill is needed on the overlapped receives.
					if(InterlockedDecrement(&curPendingRecvs) < refillPendingRecvs)
					{
						SetEvent(hRefillEvent);
					}

					// Store how many bytes we retrieved for future use. If we don't, we won't know
					// how many bytes the data is.
					event->dataBufferSize = static_cast<UINT16>(numberOfBytes);

					// Generate the structure to pass to our event processing function
					APCEvent * apcData = reinterpret_cast<APCEvent *>(Pool_APCEvent::malloc());
					apcData->event = event;
					apcData->server = this;

					// Queue the APC event to our event processing thread. This call is thread safe!
					if(QueueUserAPC(ProcessEventWrapper, hEventProcessingThread, reinterpret_cast<ULONG_PTR>(apcData)) == 0)
					{
						LOG("QueueUserAPC failed. The event [%i bytes] will be discarded. GetLastError returned [%i]", event->dataBufferSize, GetLastError());

						// Free this data since we cannot process it in the appropriate thread.
						Pool_UDPOverlappedEvent::free(event);
						Pool_APCEvent::free(apcData);
					}
				}

				// WSASendTo event
				else if(event->operation == HPS_OPERATION_WRITE)
				{
					// We don't have anything in particular to do here framework wise,
					// but you can add additional logic similar to the read operation
					// if you wish to do post processing logic on all data sent for
					// logging or whatever you think of.

					// Free this data since Winsock has processed it already.
					Pool_UDPOverlappedEvent::free(event);
				}

				// Uh-oh! Bad data, perhaps the user did not initialize everything
				else
				{
					LOG("Invalid event operation specified [%.2X].", event->operation);
					Pool_UDPOverlappedEvent::free(event);
				}
			}
			else
			{
				// We did something via PostQueuedCompletionStatus. We don't
				// have any specific code in this example, but if we do pass
				// something via the key parameter, we have to consider if it
				// needs to be cleaned up or not. We assume in this case we
				// will pass an event we need to free, that might not always
				// be the case though!

				Pool_UDPOverlappedEvent::free(event);
			}
		}
		else
		{
			LOG("GetQueuedCompletionStatus failed. GetLastError returned [%i]", GetLastError());

			// Free this connection data since we cannot process it in the appropriate thread.
			Pool_UDPOverlappedEvent::free(event);
		}
	}
}

//-----------------------------------------------------------------------------

// This function runs the worker thread function. We use this approach to
// keep the server self contained.
DWORD WINAPI WorkerThreadWrapper(LPVOID lpParam)
{
	HighPerformanceUDPServerData * serverData = 
		reinterpret_cast<HighPerformanceUDPServerData *>(lpParam);
	serverData->WorkerThread();
	return 0;
}

//-----------------------------------------------------------------------------

// This is the message dispatching function for our APC function. It works by
// running a thread in the wait state. As soon as an APC message is queued,
// the thread processes the event and then waits for the next event to be 
// posted.
DWORD WINAPI APCEventProcessThreadWrapper(LPVOID lpParam)
{
	HighPerformanceUDPServerData * serverData = 
		reinterpret_cast<HighPerformanceUDPServerData *>(lpParam);
	serverData->APCEventProcessThread();
	return 0;
}

//-----------------------------------------------------------------------------

// This is our asynchronous message handling function invoked to handle all 
// posted APC events. We simply pass our connection data pointer from the 
// WorkerThread and free it when we are done using it. This function will 
// only be executed in one thread context.
VOID CALLBACK ProcessEventWrapper(ULONG_PTR param)
{
	APCEvent * acpData = reinterpret_cast<APCEvent *>(param);
	acpData->server->ProcessEvent(acpData->event);
	Pool_APCEvent::free(acpData);
}

//-----------------------------------------------------------------------------

// Implement a simple logging mechanism. I still prefer the printf way of 
// building custom strings rather than the << method, so that is why I have
// here. Feel free to use your own system though!
namespace Log
{
	// We need to synchronize access to the logging function, so it should only
	// be used to report unexpected errors and not log general information.
	CRITICAL_SECTION logCS;

	// This is our simple logging function. It is styled to work similarly to printf.
	void Log(const char * file, const char * function, int line, const char * msg, ...)
	{
		EnterCriticalSection(&logCS);
		static char buffer[8192];
		va_list args;
		va_start(args, msg);
		vsnprintf_s(buffer, 8192, 8191, msg, args);
		va_end(args);
		printf("File: %s\nFunction: %s\nLine: %i\nError: %s\n", file, function, line, buffer);
		LeaveCriticalSection(&logCS);
	}

	// We need to call these in our main function before we use the Log system and
	// before the program exits.
	void Setup()
	{
		InitializeCriticalSection(&logCS);
	}
	void Cleanup()
	{
		DeleteCriticalSection(&logCS);
	}
}

//-----------------------------------------------------------------------------
