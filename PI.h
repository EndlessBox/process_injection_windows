#ifndef PI_H
#define PI_H

#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <unistd.h>
#include <aclapi.h>

typedef struct	s_pinfo {
	DWORD			pid;
	HANDLE			HProcess;
	FILETIME		PCreationTime;
	FILETIME		PExitTime;
	FILETIME		PKernelTime;
	FILETIME		PUserTime;
	// number of 100-nanosecond elapsed since 12:00 A.M. January 1, 1601
	long long int	time64;
	LPVOID			VMPointer;
	SIZE_T			dataWritten;
	DWORD			threadId;
	HANDLE			HThread;
	boolean			wasDelivered;
	struct s_pinfo	*next;
}				t_pinfo;

typedef struct	s_list_infos {
	t_pinfo		*head;
	t_pinfo		*tail;
	// oldest process ! 
	t_pinfo		*choosenProcess;
}				t_list_infos;

void			printListInfos(t_list_infos	*list);
void			printProcessInfos(t_pinfo process);
t_list_infos	*getProcessesList(void);
int				allocateVirtualMemory(size_t size, DWORD allocationType, DWORD protectionType ,t_pinfo *process);
int				populateProcessMemory(void *payload, size_t payloadSize, t_pinfo *process);
int				summonExecuterThread(LPSECURITY_ATTRIBUTES securityAttr, SIZE_T stackSize, void *threadParam, DWORD creationFlags, t_pinfo *process);

#endif