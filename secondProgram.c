#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <libgen.h>
#include <getopt.h>
#include <string.h>

#include <sched.h>
#include <limits.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

int main()
{
	int choice;
	char candumpCommand[30] = "candump -l vcan";
	char buffer[10];

	do{
	printf("Select the interface on which the data dump will be made!\n");
	printf("Type in 9 to stop! Close the dump when the generation of data finished! ( CTRL + C )\n");
	scanf("%d", &choice);

	if ( choice != 9 )
	{
		sprintf(buffer, "%d", choice-1);
		strcat(candumpCommand, buffer);
		system(candumpCommand);
	}else
		printf("Exiting...\n");

	}while(choice != 9);


}


