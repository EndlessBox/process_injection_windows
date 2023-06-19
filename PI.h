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
	struct s_pinfo	*next;
}				t_pinfo;

typedef struct	s_list_infos {
	t_pinfo		*head;
	t_pinfo		*tail;
}				t_list_infos;

#endif