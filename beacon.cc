#include <iostream>
#include "beacon.h"

using namespace std;

unsigned short int myport;
char hostname[100];
int acc_conn_cnt;
int acc_conn_temp;
inistruct *inist;
FILE *log_file_fp;
List *nam_node_list;
FILE *nam_file_fp;
//char *nam_file_name;

pthread_t NB_temp_conn_id;
int NB_temp_conn_jointimeout;

int SIGUSR1_flag;
int shutdown_flag;
int alarm_flag;
int beacon_flag;
int soft_restart_flag;
struct thread_port_map *thread_port;
struct thread_port_map *thread_port_temp;

struct id_port_map_struct *id_port_map;
int own_id;
int id_cnt;

char *node_inst_id;

pthread_mutex_t thread_port_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t thread_port_temp_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t send_thread_ids_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t msg_timer_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t shutdown_mutex=PTHREAD_MUTEX_INITIALIZER;				//Also used for soft restart
pthread_mutex_t btree_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t node_id_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t nam_node_mutex=PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t send_thread_cv=PTHREAD_COND_INITIALIZER;		//condition variable on which all sending threads will be waiting until signal received and message Queue empty 
pthread_cond_t send_temp_cv=PTHREAD_COND_INITIALIZER;

//struct msg_timer_struct msg_timer[MAX_MSGTIMERS];
unsigned int msg_index;

int isjoined;

bsTree <unsigned int,struct node_message_tree> btree; 

/*
int main(void)
{
	return 0;
}
*/

int get_index_from_port(unsigned short int port,char *hname)
{
	int index11=-1;
	for(int i=0;i<acc_conn_cnt;i++)
	{
		if(port==thread_port[i].port && !strcmp(hname,thread_port[i].hname))
		{
			index11=i;
			break;
		}
	}
	
	return index11;
}


char * get_hname_from_port(unsigned short int port)
{
cout<<"\n######### in get_hname_from_port **********";
	char *temp;
	temp=NULL;
	cout<<"\nPORT::"<<port;
	for(int i=0;i<inist->beaconcount;i++)
	{
	cout<<"\ninist->beacon[i]->port::"<<inist->beacon[i]->port;
		if(port==inist->beacon[i]->port)
		{
			temp=inist->beacon[i]->hostname;
			break;
		}
	}
	
	return temp;
}


void catch_signal(int sig)
{
	shutdown_flag=1;
//	signal(SIGINT,catch_signal);
	signal(SIGTERM,catch_signal);
//	signal(SIGALRM,catch_signal);
//	signal(SIGPIPE,catch_signal);
	cout<<"\nIn signal handler catch_signal . . .";
	exit(0);
}

void catch_alarm(int sig)
{
	alarm_flag=1;
	signal(SIGALRM,catch_alarm);
}

void catch_SIGUSR1(int sig)
{
	cout<<"\nSIGUSR1 received . . .";
	cout<<"\nSIGUSR1 received . . .";
	signal(SIGUSR1,catch_SIGUSR1);
	SIGUSR1_flag=1;
}

void init_network()
{	
	shutdown_flag=0;
	isjoined=0;
	//soft_restart_flag=1;
	SIGUSR1_flag=0;
	inist=new inistruct();
	//Checking if beacon node or not
	beacon_flag=0;   //initializing beacon_flag to zero
	
	acc_conn_cnt=0; //number of connections accepted (connected or disconnected) - to keep track of entries in thread_port
	acc_conn_temp=0;//temporary connection count such as nonbeacon->beacon initial join request and responses back to originating nonbeacon
	
	NB_temp_conn_id=0;
	NB_temp_conn_jointimeout=-1;
	
	nam_node_list=NULL;
	
	nam_file_fp=NULL;
//	nam_file_name=NULL;
/*	
	msg_index=0;
	for(int i=0;i<MAX_MSGTIMERS;i++)
	{
		msg_timer[i].valid=0;		//valid - (1 means being used, cannot use for new timer) (0 means not being used, can be used for a new timer) 
	}
*/
	
}

void init_logfile()
{
	int fname_len=strlen(inist->homedir)+strlen(inist->logfilename)+1;
	char *loc_home_dir=(char *)calloc(fname_len,1);
	memset(loc_home_dir,0,fname_len);
	strcpy(loc_home_dir,inist->homedir);
	strcat(loc_home_dir,inist->logfilename);

	log_file_fp=fopen(loc_home_dir,"w+");
	
	struct id_port_map_struct i_p_m_s;
	id_port_map=(struct id_port_map_struct *)calloc(MAX_NODEIDS,sizeof(i_p_m_s));
	own_id=0;
	id_cnt=1;		//As node id=0 for own 
}

unsigned int RSHash(char* str, unsigned int len)
{
   unsigned int b    = 378551;
   unsigned int a    = 63689;
   unsigned int hash = 0;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash = hash * a + (*str);
      a    = a * b;
   }

   return hash;
}

int get_node_id_from_port(unsigned short int port)
{
	pthread_mutex_lock(&node_id_mutex);
	int id=-1;
	for(int i=0;i<(id_cnt-1);i++)
	{
		if(port==id_port_map[i].port)
		{
			id=id_port_map[i].id;
		//	return id_port_map[i].id;
		}
	}
	
	if(id==-1)
	{
		id=id_cnt;
		id_port_map[id-1].id=id;
		id_port_map[id-1].port=port;
		id_cnt++;
	}
	pthread_mutex_unlock(&node_id_mutex);
	
	return id;
}
