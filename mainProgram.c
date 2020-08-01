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

/* Programul functioneaza in 2 moduri
   1. Alegerea unei configuratii prestabilite
   2. Crearea propriei configuratii
   Pentru alegerea modului de lucru se
   decomenteaza blocul de cod respectiv. */

int typeList[3][3]={0, 0, 0, 0, 0, 0, 0, 0, 0}; // Reprezentare matriceala a interfetelor si a starii acestora, matrice alocata static for testing purposes :)
int activeList[3]={0, 0, 0}; // starea de "up" sau "down" a interfetelor ( configuratie de 3 interfete pentru motive de testare )

void print_frames(int number_of_interfaces,int bitrate)
{
	system("canplayer -I \"$(ls -t | head -n1)\" -v"); // print al ultimelor frame-uri generate

}

void change_filters(int number_of_interfaces, int bitrate)  // schimbarea rutelor de comunicare intre interfete
{
	int chosen_interface_send, chosen_interface_receive;
	char buffer[10], buffer2[30], buffer3[10];
	char command[150] = "sudo ip link set vcan";
	char routingCall[100] = "sudo cangw -A -s vcan";

	printf("\nChoose the interface that you want to send data\n");
	scanf("%d", &chosen_interface_send);
	printf("Choose the interface that you want to receive data\n");
	scanf("%d", &chosen_interface_receive);

	if ((chosen_interface_send != chosen_interface_receive) &&
 		(chosen_interface_send <= number_of_interfaces && chosen_interface_send > 0) &&
		(chosen_interface_receive <= number_of_interfaces && chosen_interface_receive > 0))
	{
		  printf("Enabling chosen interfaces....\n");

		  activeList[chosen_interface_send-1] = 1;
		  activeList[chosen_interface_receive-1] = 1;

		  sprintf(buffer, "%d", chosen_interface_send-1);
		  strcat(command, buffer);
		  strcat(command, " up type can bitrate ");
		  sprintf(buffer2, "%d", bitrate);
		  strcat(command, buffer2);
		  system(command); // Activarea interfetei care genereaza date.
		  printf("\n%s\n", command);

		  strcpy(command, "sudo ip link set vcan");
		  sprintf(buffer, "%d", chosen_interface_receive-1);
		  strcat(command, buffer);
		  strcat(command, " up type can bitrate ");
		  sprintf(buffer2, "%d", bitrate);
		  strcat(command, buffer2);
		  system(command); // Activarea interfetei care primeste date.
		  printf("\n%s\n", command);
		  printf("Establishing routing rules...\n");


		  sprintf(buffer3, "%d", chosen_interface_send-1);
		  strcat(routingCall, buffer3);
		  strcat(routingCall, " -d vcan");
		  sprintf(buffer3, "%d", chosen_interface_receive-1);
		  strcat(routingCall, buffer3);
		  strcat(routingCall, " -e");
		  system(routingCall); // setarea rutei de comunicare
		  printf("\n%s\n", routingCall);

		  typeList[chosen_interface_send-1][chosen_interface_receive-1] = 1; // setarea directiei de comunicare a interfetei ( rand - interfata, coloana - interfata cu care comunica )

	}else
	printf("Choose a valid interface!\n");

}

void control_interface(int chosen_interface, int bitrate) // Functie folosita pentru oprirea sau pornirea interfetei alese de utilizator.
{
	char buffer[25], buffer2[50];
	char command[150] = "sudo ip link set vcan";
	int choice;

	printf("Do you want to shutdown or to activate the interface?\n");
	printf("1.Shutdown. 2.Activate.\n");
	scanf("%d", &choice);

	if(choice == 1){
		  activeList[chosen_interface-1] = 0;
		  sprintf(buffer, "%d", choice);
		  strcat(command, buffer);
		  strcat(command, " down");
		  system(command);
	}else{
		  activeList[chosen_interface-1] = 1;
		  sprintf(buffer, "%d", choice);
		  strcat(command, buffer);
		  strcat(command, " up type can bitrate ");
		  sprintf(buffer2, "%d", bitrate);
		  strcat(command, buffer2);
		  system(command);
		  printf("%s", command);
	}
}

void generate_data(int number_of_interfaces, int chosen_interface) // Generare de date de catre interfata aleasa de utilizator
{								   // Atentie insa este necesara rularea celuilalt program inainte de generarea de date!
	char buffer[25];
	char command[150] = "cangen vcan";

	if((chosen_interface > number_of_interfaces) || chosen_interface  <= 0){
	    printf("That interface doesn't exist!");
	    exit(1);
	  }

	  sprintf(buffer, "%d", chosen_interface-1);
	  strcat(command, buffer);
	  strcat(command, " -m -v");
	  system(command);
}

int main()
{
	int configuration_number, tipul_interfetei, bitrate,number_of_interfaces, workingState;

	///////// HERE ARE CONFIGURATIONS ALREADY MADE FOR TESTING PURPOSES //////////

 	/*do{

	printf("Choose one configuration: \n 1.VCAN config (only working for some reason)\n 2.CAN config \n 3.CAN FD config \n 4.CAN FD+CAN config\n");
	scanf("%d", &configuration_number);

	if(configuration_number == 1)
	{
		number_of_interfaces=3;
		typeList[0][0]=3;
		typeList[1][1]=3;
		typeList[2][2]=3;

		typeList[0][1]=1;
		typeList[0][2]=0;

		typeList[1][0]=0;
		typeList[1][2]=1;

		typeList[2][0]=1;
		typeList[2][1]=1;

		bitrate=25000;
		activeList[0]=1;
		activeList[1]=1;
		activeList[2]=1;
	//pe diagonala principala e trecut tipul de can 1.CAN; 2.CAN FD; 3.VCAN;
	//0 inseamna ca nu trimite si 1 inseamna ca trimite la interfata cu indicele corespunzator

	}else if(configuration_number == 2)
	{
		number_of_interfaces=3;
		//typeList={1, 1, 0, 0, 1, 1, 1, 1, 1};
		bitrate=50000;
		//activeList[3]={1, 1, 1};
	}else if(configuration_number == 3)
	{
		number_of_interfaces=3;
		//typeList={2, 1, 0, 0, 2, 1, 1, 1, 2};
		bitrate=4000000;
		//activeList[3]={1, 1, 1};
	}else if(configuration_number == 4){
		number_of_interfaces=3;
		//typeList={1, 1, 0, 0, 2, 1, 1, 1, 1};
		bitrate=25000;
		//activeList[3]={1, 1, 1};
	}
	}while(configuration_number >  3 && configuration_number < 1);
	*/
	/////////////////// MANUALLY SETUP THE CONFIGURATION //////////////////////

	printf("Introdu numarul de interfete dorit.\n");
	scanf("%d", &number_of_interfaces);

	int** typeList = (int**)malloc(number_of_interfaces * sizeof(int*));
	for(int i = 0; i < number_of_interfaces; i++ ) {
  	typeList[i] = (int*)malloc(number_of_interfaces * sizeof(int));
	}
	if(number_of_interfaces >= 2)
	{

		for ( int i = 0; i < number_of_interfaces; i++ )
		{
			do{

			printf("Alege tipul de interfata pentru interfata %d : \n 1.CAN \n 2.CAN FD \n 3.VCAN \n", i+1);
			scanf("%d", &tipul_interfetei);
			typeList[i][i] = tipul_interfetei;

			}while(tipul_interfetei > 3 && tipul_interfetei < 0);
		}

		printf("Introdu bitrate-ul.\n");
		scanf("%d", &bitrate);
		for ( int i = 0; i < number_of_interfaces; i++ )
		{
			do{

			for(int j =0;j<number_of_interfaces;j++)
			{

				if(i!=j)
				{
					printf("Alege regilile de rutare pentru interfata %d raportat la interfata %d: \n 0.Nu trimite \n 1.Trimite \n", i+1,j+1);
					scanf("%d", &tipul_interfetei);
					typeList[i][j] = tipul_interfetei;
				}
			}
			}while(tipul_interfetei > 3 && tipul_interfetei < 0);
		}
	}

	int* activeList = malloc(number_of_interfaces * sizeof(int));

	for (int i = 0; i < number_of_interfaces; i++)
	{
		printf("Seteaza starea de lucru a interfetei %d. (0. Oprit, 1. Pornit) \n", i+1);
		scanf("%d", &workingState);
		activeList[i] = workingState;
	}

	////////////// APPLYING USER'S SETUP //////////////

	for (int i = 0; i < number_of_interfaces; i++)
	{
		char canSetupCall[150] = "sudo ip link";
		char secondCanSetupCall[150] = "sudo ip link set up vcan";
		char buffer[25];

		if(typeList[i][i] == 1) // Interfata == CAN
		{
			strcat(canSetupCall, " set can");
			sprintf(buffer, "%d", i);
			strcat(canSetupCall, buffer);
			strcat(canSetupCall, " up type can bitrate ");
			sprintf(buffer, "%d", bitrate);
			strcat(canSetupCall, buffer);
			system(canSetupCall);
			for (int j = 0; j < number_of_interfaces; j++)
			{
				char routingCall[100] = "sudo cangw -A -s vcan";
				if(i != j && typeList[i][j] != 0)
				{
					// ...
				}
			}
		}else if(typeList[i][i] == 2) // Interfata == CAN FD
		{
			strcat(canSetupCall, " set can");
			sprintf(buffer, "%d", i);
			strcat(canSetupCall, buffer);
			strcat(canSetupCall, " up type can bitrate ");
			sprintf(buffer, "%d", bitrate);
			strcat(canSetupCall, buffer);
			strcat(canSetupCall, " dbitrate 4000000 fd on");

			system(canSetupCall);
			for (int j = 0; j < number_of_interfaces; j++)
			{
				char routingCall[100] = "sudo cangw -A -s vcan";
				if(i != j && typeList[i][j] != 0)
				{
					// ...
				}
			}
		}else if(typeList[i][i] == 3) // Interfata == VCAN
		{
			strcat(canSetupCall, " add vcan");
			sprintf(buffer, "%d", i);
			strcat(canSetupCall, buffer);
			strcat(canSetupCall, " type vcan");
			sprintf(buffer, "%d", i);
			strcat(secondCanSetupCall, buffer);
			system(canSetupCall);
			printf("%s\n",canSetupCall);
			printf("%s\n",secondCanSetupCall);
			system(secondCanSetupCall);
		}
	}

	for (int i = 0; i < number_of_interfaces; i++)
	{
		char buffer[25];

		for (int j = 0; j < number_of_interfaces; j++)
		{
			char routingCall[100] = "sudo cangw -A -s vcan";
			if(i != j && typeList[i][j] != 0)
			{
				sprintf(buffer, "%d", i);
				strcat(routingCall, buffer);
				strcat(routingCall, " -d vcan");
				sprintf(buffer, "%d", j);
				strcat(routingCall, buffer);
				strcat(routingCall, " -e");
				system(routingCall);
				printf("%s\n", routingCall);

			}
		}
	}

	/////////////////////////////// INITIAL SETUP DONE ///////////////////////////////

	printf("\nBitrate = %d\n", bitrate);

	for(int i=0;i<number_of_interfaces;i++)
	{
		printf("Interfata nr %d - ", i+1);
		for(int j=0;j<number_of_interfaces;j++)
			printf("%d ",typeList[i][j]);
		printf("\n");
	}

	printf("\nActive interfaces : 1 - %d | 2 - %d | 3 - %d . (1-active, 0-inactive)\n", activeList[0], activeList[1], activeList[2]);

	/////////////////////////////// USER INTERFACE ///////////////////////////////////

	int choice, chosen_interface;

	  do{
	    printf("----------USER INTERFACE----------\n");
	    printf("========================================\n");
	    printf("1. Print can frame. (NOTE: Before print please make sure to use the second program.)\n");
	    printf("2. Modify routing rules\n");
	    printf("3. Generate data for interface x. ( CTRL + C to stop generating data )\n");
	    printf("4. Control interfaces.\n");
	    printf("5. Display interface info.\n");
	    printf("6. Exit\n");

	    scanf("%d", &choice);
	    switch(choice){

	      case 1: print_frames(number_of_interfaces, bitrate);
	      break;

	      case 2: change_filters(number_of_interfaces, bitrate);
		break;

	      case 3:
		printf("What interface do you want to use?");
		scanf("%d", &chosen_interface);
		generate_data(number_of_interfaces, chosen_interface);
		break;

	      case 4:
		printf("What interface do you want to use?");
		scanf("%d", &chosen_interface);
		control_interface(chosen_interface, bitrate);
		break;

	      case 5:
		printf("\nBitrate = %d\n", bitrate);

		for(int i=0;i<number_of_interfaces;i++)
		{
			printf("Interfata nr %d - ", i+1);
			for(int j=0;j<number_of_interfaces;j++)
				printf("%d ",typeList[i][j]);
			printf("\n");
		}

			printf("\nActive interfaces : 1 - %d | 2 - %d | 3 - %d . (1-active, 0-inactive)\n", activeList[0], activeList[1], activeList[2]);

		break;
	      case 6: exit(1);
		break;

		default: printf("Please select a valid option from the menu.");
	      }
	}while(choice != 6);
}

