#include "PI.h"

int		main(int argc, char** argv) {


	t_list_infos *currentProcessesList;
	while (!(currentProcessesList = getProcessesList()));

	// printListInfos(currentProcessesList);


	char *payload = "\x69\x69";
	size_t payloadSize = sizeof(payload);


	HANDLE HProcess = NULL;
	int pid = currentProcessesList->choosenProcess->pid;

	// debug to see what process are we going to mess with !
	printProcessInfos(*(currentProcessesList->choosenProcess));
	printf("Press anything to complete !\n");
	scanf("XDXDXD");
	// end of debug	

	if (allocateVirtualMemory(payloadSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE, currentProcessesList->choosenProcess))
		return EXIT_FAILURE;

	printf("HProcess : 0x%p\nVMPointer : 0x%p\n", currentProcessesList->choosenProcess->HProcess, currentProcessesList->choosenProcess->VMPointer);

	if (populateProcessMemory(payload, payloadSize, currentProcessesList->choosenProcess))
		return EXIT_FAILURE;
	
	printf("payloadSize: %zu, dataWritten: %zu\n", payloadSize, currentProcessesList->choosenProcess->dataWritten);


	if (summonExecuterThread(NULL, 0, NULL, 0, currentProcessesList->choosenProcess))
		return EXIT_FAILURE;

	printf("Thread was summoned correctly with id : %d\n", currentProcessesList->choosenProcess->threadId);

	cleanUp(currentProcessesList, payload, payloadSize);

	return EXIT_SUCCESS; 
}