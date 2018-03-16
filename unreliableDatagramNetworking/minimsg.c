/*
 *	Implementation of minimsgs and miniports.
 */
#include "minimsg.h"
#include "queue.h"
#include "network.h"
#include "miniheader.h"
#include "synch.h"

#define TESTING 0

#define MINIMSG_BOUND_START 32767   // 32768-1 
#define MINIMSG_MAX_PORTS 65535
miniport_t all_ports[MINIMSG_MAX_PORTS];
semaphore_t sema_minimsg;
unsigned int bound_port = MINIMSG_BOUND_START;
void network_handler(network_interrupt_arg_t *nit) ;

struct miniport
{
	char port_type; //'b' for bound, 'u' for unbound
	int port_number;
	union {
			struct {
			queue_t incoming_data;
			semaphore_t datagrams_ready;
			} unbound;

			struct {
			network_address_t remote_address;
			int remote_unbound_port;
			} bound;
		} u;
};

unsigned int get_next_bound_port() {
	int i = bound_port;
	semaphore_P(sema_minimsg);
	do {
		bound_port = (bound_port + 1) > 65535 ? 32768 : (bound_port + 1);
		if( NULL == all_ports[bound_port] )
			break;
	} while( i != bound_port );
	semaphore_V(sema_minimsg);
	if( i == bound_port )
		return -1;
	else
		return bound_port;
}

int get_port_number(miniport_t port){
	if(NULL != port)
		return port->port_number;
	return -1;
}

void dispay_port_info(miniport_t port){
	printf("\n\nPort contents:");
	printf("\nPort-number:%d", port->port_number);
	printf("\tPort-type:%c", port->port_type);
	if(port->port_type == 'u'){
		printf("\nPort-Queue-Pointer-loc:%d", port->u.unbound.incoming_data);
		printf("\tPort-datagram-Pointer-loc:%d", port->u.unbound.datagrams_ready);
	}
	else{ //bounded with the remote address and port
		printf("\nRemote-Address:");
		network_printaddr(port->u.bound.remote_address);
		printf("\tRemote-Receiving-Port(unbound):%d", port->u.bound.remote_unbound_port);
	}
}

/* performs any required initialization of the minimsg layer.
 */
void minimsg_initialize()
{
	int i;
	if(TESTING)
		printf("Initializing minimsg\n");
	if(0 > network_initialize(network_handler)) {
		printf("Unable to initialize network. Exiting...");
		exit(-1);
	}
	for(i = 0; i < MINIMSG_MAX_PORTS; i++)
		all_ports[i] = NULL;

//	for(i = 0; i < MINIMSG_MAX_LOCAL_PORTS; i++)
//		local_ports[i] = NULL;
	
	sema_minimsg = semaphore_create();
	semaphore_initialize(sema_minimsg, 1);
	bound_port = MINIMSG_BOUND_START;
}

/* Creates an unbound port for listening. Multiple requests to create the same
 * unbound port should return the same miniport reference. It is the responsibility
 * of the programmer to make sure he does not destroy unbound miniports while they
 * are still in use by other threads -- this would result in undefined behavior.
 * Unbound ports must range from 0 to 32767. If the programmer specifies a port number
 * outside this range, it is considered an error.
 */
miniport_t miniport_create_unbound(int port_number)
{
	if( port_number < 0 || port_number > 32767 )
		return NULL;

	if( all_ports[port_number] != NULL )
		return all_ports[port_number];
	else
	{
		miniport_t new_port = (miniport_t) malloc(sizeof(struct miniport));
		if( NULL == new_port )
		{
			printf("Unable to allocate memory for Unbound port\n");
			return NULL;
		}
		new_port->port_number = port_number;
		new_port->port_type = 'u';
		new_port->u.unbound.incoming_data = queue_new();
		new_port->u.unbound.datagrams_ready = semaphore_create();
		semaphore_initialize(new_port->u.unbound.datagrams_ready, 0);
		all_ports[port_number] = new_port;
		return new_port;
	}
}

/* Creates a bound port for use in sending packets. The two parameters, addr and
 * remote_unbound_port_number together specify the remote's listening endpoint.
 * This function should assign bound port numbers incrementally between the range
 * 32768 to 65535. Port numbers should not be reused even if they have been destroyed,
 * unless an overflow occurs (ie. going over the 65535 limit) in which case you should
 * wrap around to 32768 again, incrementally assigning port numbers that are not
 * currently in use.
 */
miniport_t miniport_create_bound(network_address_t addr, int remote_unbound_port_number)
{
	int i;
	miniport_t new_port = (miniport_t) malloc(sizeof(struct miniport));
	if( NULL == new_port )
	{
		printf("Unable to allocate memory for Bound port\n");
		return NULL;
	}
	i = get_next_bound_port();
	if( i < 0 ) {
		printf("No free bound ports\n");
		miniport_destroy(new_port);
		return NULL;
	}
	new_port->port_number = i;
	new_port->port_type = 'b';

	//Copy passed address to new port structure
	network_address_copy(addr, new_port->u.bound.remote_address);

	new_port->u.bound.remote_unbound_port = remote_unbound_port_number;
	all_ports[i] = new_port;
	return new_port;
}

/* Destroys a miniport and frees up its resources. If the miniport was in use at
 * the time it was destroyed, subsequent behavior is undefined.
 */
void miniport_destroy(miniport_t miniport)
{
	interrupt_level_t old_level = set_interrupt_level(DISABLED);
	
	if( NULL != miniport ) 
	{
		semaphore_P(sema_minimsg);
		all_ports[miniport->port_number] = NULL;
		if (miniport->port_type == 'u') {
			queue_free(miniport->u.unbound.incoming_data);
			semaphore_destroy(miniport->u.unbound.datagrams_ready);
		}
		free(miniport);
		semaphore_V(sema_minimsg);
	}
	set_interrupt_level(old_level);
}

/* Sends a message through a locally bound port (the bound port already has an associated
 * receiver address so it is sufficient to just supply the bound port number). In order
 * for the remote system to correctly create a bound port for replies back to the sending
 * system, it needs to know the sender's listening port (specified by local_unbound_port).
 * The msg parameter is a pointer to a data payload that the user wishes to send and does not
 * include a network header; your implementation of minimsg_send must construct the header
 * before calling network_send_pkt(). The return value of this function is the number of
 * data payload bytes sent not inclusive of the header.
 */
int minimsg_send(miniport_t local_unbound_port, miniport_t local_bound_port, minimsg_t msg, int len)
{
	char headr[sizeof(struct mini_header)];
	struct mini_header* packet_header;
	network_address_t source_address;
	int data_size;
	short header_length;

	if (local_unbound_port == NULL) {
		printf("Send error: Local port does not exist\n");
		return -1;
	} else if (local_bound_port == NULL) {
		printf("Send error: Remote port does not exist\n");
		return -1;
	} else if (msg == NULL) {
		printf("Send error: Message does not exist\n");
		return -1;
	} else if (len > MINIMSG_MAX_MSG_SIZE) {
		printf("Send error: Message size too large. Truncating\n");
		len = MINIMSG_MAX_MSG_SIZE;
		//return -1;
	} /*else if (local_bound_port->u.bound.remote_address == NULL ) {
		printf("Send error: Invalid remote address\n");
		return -1;
	} else if (local_bound_port->u.bound.remote_unbound_port == NULL ) {
		printf("Send error: Invalid remote port\n");
		return -1;
	}*/
	
	packet_header = (mini_header_t) malloc(sizeof(struct mini_header));
	if( NULL == packet_header ) {
		printf("Unable to allocate memory for sending packet header\n");
		return -1;
	}
	packet_header->protocol = PROTOCOL_MINIDATAGRAM;
	
	network_get_my_address(&source_address);  // Array variable, can be called without &
	if(TESTING){
			printf("\nPrinting source_address:");
			network_printaddr(source_address);
	}
	header_length = (short)sizeof(struct mini_header);
	
	pack_address(packet_header->source_address, source_address);
	pack_address(packet_header->destination_address, local_bound_port->u.bound.remote_address);
	pack_unsigned_short(packet_header->destination_port, local_bound_port->u.bound.remote_unbound_port);
	pack_unsigned_short(packet_header->source_port, local_unbound_port->port_number);
	
	if(TESTING){
			printf("\n\nPrinting HEADER INFO:");
			unpack_address(packet_header->source_address, &source_address);
			printf("\nPrinting unpacked source_address:");
			network_printaddr(source_address);

			printf("\nPrinting unpacked source_port:%d",unpack_unsigned_short(packet_header->source_port));

			unpack_address(packet_header->destination_address, &source_address);
			printf("\nPrinting unpacked destination_address:");
			network_printaddr(source_address);

			printf("\nPrinting unpacked destination_port:%d",unpack_unsigned_short(packet_header->destination_port));

			printf("\n\nPrinting Receiver Address bound to sending port:");
			network_printaddr(local_bound_port->u.bound.remote_address);
	}

	semaphore_P(sema_minimsg);
	data_size = network_send_pkt(local_bound_port->u.bound.remote_address, header_length, 
		                          (char *) packet_header, len, (char *) msg);
	semaphore_V(sema_minimsg);
	free(packet_header);
	return data_size - header_length;
}

/* Receives a message through a locally unbound port. Threads that call this function are
 * blocked until a message arrives. Upon arrival of each message, the function must create
 * a new bound port that targets the sender's address and listening port, so that use of
 * this created bound port results in replying directly back to the sender. It is the
 * responsibility of this function to strip off and parse the header before returning the
 * data payload and data length via the respective msg and len parameter. The return value
 * of this function is the number of data payload bytes received not inclusive of the header.
 */
int minimsg_receive(miniport_t local_unbound_port, miniport_t* new_local_bound_port, minimsg_t msg, int *len)
{
		void *message_buffer;
	char char_header[sizeof(struct mini_header)], temp_buffer[2];
	network_interrupt_arg_t *packet;
	minimsg_t message;
	mini_header_t packet_header;
	network_address_t addr;
	int i, remote_port, msg_len;
	interrupt_level_t intr_level;

	//message_buffer = malloc(sizeof(char) * MAX_NETWORK_PKT_SIZE);

	if( NULL == local_unbound_port ) {
		printf("Null local unbound port\n");
		return -1;
	}

	semaphore_P(local_unbound_port->u.unbound.datagrams_ready);
	
	semaphore_P(sema_minimsg);
	intr_level = set_interrupt_level(DISABLED);
	if( -1 == queue_dequeue(local_unbound_port->u.unbound.incoming_data, &message_buffer))
	{
		printf("Message dequeue error\n");
		semaphore_V(sema_minimsg);
		set_interrupt_level(intr_level);
		return -1;
	}
	semaphore_V(sema_minimsg);
	set_interrupt_level(intr_level);
	
	packet = (network_interrupt_arg_t *)message_buffer;
	packet_header = (mini_header_t)(packet->buffer);
	message = (minimsg_t)(packet->buffer + sizeof(struct mini_header));
	msg_len = packet->size - sizeof(struct mini_header);
	
	if( msg_len < *len )
		memcpy((void *) msg, (void *)message, msg_len);
	else
		memcpy((void *) msg, (void *) message, *len);
	
	unpack_address(packet_header->source_address, addr);
	*new_local_bound_port = miniport_create_bound(addr, (int) unpack_unsigned_short(packet_header->source_port));

	free(packet);
	packet = NULL;

	return msg_len;
}

void network_handler(network_interrupt_arg_t *nit) {
	network_address_t source_address, destination_address;
	unsigned short destination_port;
	char buffer[MAX_NETWORK_PKT_SIZE];
	mini_header_t packet_header;
	miniport_t dest_miniport;
	int i;

	if(TESTING){
		printf("\n\n:I am in network_handler():");
	}

	if( NULL == nit )
		return;

	//network_address_copy(nit->addr, source_address);

	packet_header = (mini_header_t)(nit->buffer);
	destination_port = unpack_unsigned_short(packet_header->destination_port);

	dest_miniport = all_ports[destination_port];
	
	if( dest_miniport != NULL )
	{
		if( dest_miniport->port_type == 'u' )
		{
			queue_append(dest_miniport->u.unbound.incoming_data, nit);
			semaphore_V(dest_miniport->u.unbound.datagrams_ready);
		}
		else
			printf("Error! Trying to write to a bounded port.\n");
	}
	else
		printf("Error! Trying to write to a non-existing port.\n");
	if(TESTING)
		printf("Packet arrived\n");

}