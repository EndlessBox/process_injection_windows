#include "PI.h"

void	printListInfos(t_list_infos *list) {
	t_pinfo	*head_tmp = list->head;
	while (head_tmp) {
		printProcessInfos(*head_tmp);
		printf("--------------------------------------\n");
		head_tmp = head_tmp->next;
	}
}

void	printProcessInfos(t_pinfo process) {
	printf("\tProcess id: %d\n\tProcess Handle: 0x%p\n\tTime64: %lld\n", process.pid, process.HProcess, process.time64);
}