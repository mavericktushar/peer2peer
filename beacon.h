#define UOIDLEN 20
#define MAXCONN 20
#define RECVBUFSIZE 65530
#define COMMON_HEADER_SIZE 27
#define MAX_MSGTIMERS 35000
#define MAX_NODEIDS 20
//#define MAX_CONNS 1000

#include<sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "iniparse.h"
#include "bstree.h"
#include "list.h"

using namespace std;

extern unsigned short int myport;
extern char hostname[100];
extern int acc_conn_cnt;
extern int acc_conn_temp;
extern inistruct *inist;
extern FILE *log_file_fp;
extern List *nam_node_list;
extern FILE *nam_file_fp;
//extern char *nam_file_name;

extern pthread_t NB_temp_conn_id;
extern int NB_temp_conn_jointimeout;

// Structure to map thread ids with ports used for simulaneous connection detection and termination
//in case of beacon node-->it contains the port no and hostname of opposite connected beacon node and its flag status..

// Structure containing mapping of index to a sending thread, for lookup in message Queue
//Contains mapping of those threads to their message Queues which are considered to be fully connected.
//(Meaning) Fully Connected - A connection is fully connected if Hello messages have been exchanged. (Excluding connections with Non beacon nodes which are waiting for join responses) 

struct thread_port_map
{
	int valid;
	int accept_flag;
	int conn_flag;
	int conn_end_flag;
	pthread_t send_thread_id;
	List *msg_Q;
	//pthread_mutex_t list_mutex;
	unsigned short int port;
	char *hname;
//	int keep_alive;
	int jointimeout;
};

extern struct thread_port_map *thread_port;
extern struct thread_port_map *thread_port_temp;

extern struct id_port_map_struct *id_port_map;
extern int own_id;
extern int id_cnt;

struct node_message_tree		//Structure for a node of the tree containing messages
{	
	char *uoid;					//Entry in the tree - While sending store the node information as follows (uoid - no conflict, hostname - host name of sender(own), port - port of sender(own))  
	char *hname;				//					- While receiving store the node information as follows (uoid - no conflict, hostname - host of sender, port - port of sender)			
	unsigned short int port;
	int msg_time;				//index into timer
};

// Structure for making a new entry to be inserted in message Queue

struct list_element_struct
{
	char *msg;
	int msg_sz;
	int f_s; //0- send 1- forwarded
};

//Structure to keep track of msglifetime timeout of messages

struct msg_timer_struct
{
	short int valid;
	int value;
};

//Structure to pass host name and port number of the other end of the connection (also csock is passed)

struct passing_info 
{
	int csock;
	char *hname;
	unsigned short int port;
};

struct id_port_map_struct
{
	int id;
	unsigned short int port;
};

//extern struct msg_timer_struct msg_timer[MAX_MSGTIMERS];		//may be reason for std::bad_alloc
extern unsigned int msg_index;			//arrived msg count

extern char *node_inst_id;

int get_index_from_port(unsigned short int,char *);
char * get_hname_from_port(unsigned short int);
int get_node_id_from_port(unsigned short int);
void init_network();
void init_logfile();
unsigned int RSHash(char *, unsigned int);
extern void *send_thread(void *);
extern void *recv_thread(void *);
void *timer_thread(void *);

void catch_signal(int);
void catch_SIGUSR1(int);

extern pthread_mutex_t thread_port_mutex;
extern pthread_mutex_t thread_port_temp_mutex;
extern pthread_mutex_t send_thread_ids_mutex;
extern pthread_mutex_t msg_timer_mutex;
extern pthread_mutex_t shutdown_mutex;				//Also used for soft restart
extern pthread_mutex_t btree_mutex;
extern pthread_mutex_t node_id_mutex;
extern pthread_mutex_t nam_node_mutex;

extern pthread_cond_t send_thread_cv;
extern pthread_cond_t send_temp_cv;

extern int SIGUSR1_flag;
extern int beacon_flag;
extern int shutdown_flag;
extern int alarm_flag;
extern int soft_restart_flag;

extern int isjoined;		//for non beacon node

extern bsTree <unsigned int,struct node_message_tree> btree;



