/* network test program : test_minimsg.c

*/

#include "minithread.h"
#include "minimsg.h"
#include "synch.h"
#include "network.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

extern struct miniport;

test_get_next_bound_port(){
	int i=0;
	
	printf("\ntest_get_next_bound_port():");
	printf("\nFirst Bound Port:%d ",get_next_bound_port());
	for(i=1;i<32767;i++)get_next_bound_port();
	printf("\nLast Bound Port:%d ",get_next_bound_port());
	printf("\nNext Bound Port:%d\n",get_next_bound_port());
}

test_minimsg_initialize(){
	printf("\ntest_minimsg_initialize():");
	minimsg_initialize();
}

test_miniport_create_bound(){   // creates unbound as a part of this.
	network_address_t my_address;
	miniport_t unbound_port, bound_port;
	printf("\ntest_miniport_create_bound():");

	network_get_my_address(&my_address);
	printf("\nPrinting my address:");
	network_printaddr(my_address);

	printf("\nCreating an unbound port for listening...");
	unbound_port = miniport_create_unbound(4000);
	if(unbound_port == NULL) {printf("\nError while creating unbound port"); return;}
	printf("Port Number:%d",unbound_port->port_number);
	//dispay_port_info(unbound_port);

	printf("\nCreating an unbound port for sending....");
	bound_port = miniport_create_bound(my_address,get_port_number(unbound_port));
	if(bound_port == NULL) {printf("\nError while creating bound port"); return;}
	dispay_port_info(bound_port);
}

void test_minimsg_send(){

	int i=0;
	unsigned short us;
	char text[] = "Hello, world!\n";
	int textlen=14;
	miniport_t listen_port;  //unbound port
	miniport_t send_port;	//bounded port
	network_address_t my_address;
  
	/*us = 65535;
	while(1){
		printf("%d\n",us++);
	if(us == i) break;
	}*/


	printf("\ntest_minimsg_send():");
	network_get_my_address(&my_address);
	listen_port = miniport_create_unbound(10);
	send_port = miniport_create_bound(my_address, 10);

	dispay_port_info(listen_port);
	dispay_port_info(send_port);
	minimsg_send(listen_port, send_port, text, textlen);
}

main(int argc, char** argv) {
test_get_next_bound_port();
test_minimsg_initialize();
test_miniport_create_bound();  //testing port creation.

test_minimsg_send();


fflush(stdin);
getchar();
}
