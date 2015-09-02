#include "beacon.h"
#include "message.h"

extern void *beacon_server(void *);
extern void *beacon_client(void *);
extern void *beacon_cmdline(void *);


void try_msghdr()
{
	char uoid_buf[20];
	GetUOID((char *)"nunki.usc.edu_12311_13243554679",(char *)"msg",uoid_buf,20);
	
	char *commhdr=GetCommonHeader(0xfc,uoid_buf,(unsigned char)1,15);
	cout<<"\n-----------Printing Common Header----------------------\n";
	for(int i=0;i<COMMON_HEADER_SIZE;i++)
	{
		cout<<commhdr[i];
	}
//	GetHelloMsg(char *hello_msg_buf,char *common_header,int host_port,char *host_name,int hello_msg_buf_sz)
	
	int hello_msg_buf_sz=COMMON_HEADER_SIZE+15;
	char *hello_msg_buf=new char[hello_msg_buf_sz];
	GetHelloMsg(hello_msg_buf,commhdr,12311,(char *)"nunki.usc.edu",hello_msg_buf_sz);
	
	cout<<"\n-----------Printing Hello Message Header----------------------\n";
	for(int i=0;i<(COMMON_HEADER_SIZE+15);i++)
	{
		cout<<hello_msg_buf[i];
	}
	
	cout<<"\nstrlen(commhdr) :: "<<strlen(commhdr);
	cout<<"\nstrlen(hello_msg_buf) :: "<<strlen(hello_msg_buf);
	
	for(int i=0;i<30;i++)
	{
		if(commhdr[i]=='\0' && hello_msg_buf[i]=='\0')
		{
			cout<<"\n'\0' at "<<i; 
		}
	}
	
	if(!strcmp(commhdr,hello_msg_buf))
	{
		cout<<"\nBoth headers equal . . .\n";
	}
	
	unsigned char msgtype_ret;
	memset(&msgtype_ret,0,1);
	memcpy(&msgtype_ret,hello_msg_buf,1);
	printf("\nmsgtype_ret :: %0x",msgtype_ret);
	
	char uoid_ret[20];
	memset(&uoid_ret,0,20);
	memcpy(&uoid_ret,&hello_msg_buf[1],20);
	cout<<"\nuoid_ret :: ";
	for(int i=0;i<20;i++)
	{
		printf("\n%d. %c",i,uoid_ret[i]);
		sleep(1);
	}
	
	unsigned char ttl_ret_char;
	memset(&ttl_ret_char,0,1);
	memcpy(&ttl_ret_char,&hello_msg_buf[21],1);
	unsigned short ttl_ret=(unsigned short)ttl_ret_char;
	printf("\nTTL :: %d",ttl_ret);
	
	int datalen_ret;
	memset(&datalen_ret,0,4);
	memcpy(&datalen_ret,&hello_msg_buf[23],4);
	printf("\ndatalen :: %d",datalen_ret);
	
	unsigned short int port_ret;
	memset(&port_ret,0,2);
	memcpy(&port_ret,&hello_msg_buf[27],2);
	printf("\nport_ret :: %d",port_ret);
	
	char hname_ret[14];
	memset(&hname_ret,0,14);
	memcpy(&hname_ret,&hello_msg_buf[29],13);
	printf("\nhostname_ret :: %s",hname_ret);
	
}
/*
int count1=0;
bsTreeNode <unsigned int,string>* remove_node;
void traversal(bsTreeNode<unsigned int,string>* node) 
{
    if (node != NULL)
	{
        traversal(node->less);  // print left subtree
		cout <<"\n++++++++"<< node->key << endl; // print this node
        if(count1 ==4)
		{
			remove_node=(bsTreeNode <unsigned int,string>*)realloc(remove_node,1*(sizeof(bsTreeNode <unsigned int,string>*)));
			remove_node=node;
			cout <<"\nremoving node key::"<< node->key << endl; // print this node
		}
		count1++;
        traversal(node->more); // print right subtree
    }
}

void try_btree()
{
	count1=0;
	bsTree <unsigned int,string> btree;
	
	char *uoid_buf1=new char[20];
	GetUOID((char *)"nunki.usc.edu_12311_1000",(char *)"msg",uoid_buf1,UOIDLEN);
	string s1(uoid_buf1);
	unsigned int key1=RSHash(uoid_buf1,UOIDLEN);
	cout<<"\nInserting at key :: "<<key1;
	btree.insert((const unsigned int)key1,&s1);
	
	char *uoid_buf2=new char[20];
	GetUOID((char *)"nunki.usc.edu_12312_1001",(char *)"msg",uoid_buf2,UOIDLEN);
	string s2(uoid_buf2);
	unsigned int key2=RSHash(uoid_buf2,UOIDLEN);
	cout<<"\nInserting at key :: "<<key2;
	btree.insert((const unsigned int)key2,&s2);
	
	char *uoid_buf3=new char[20];
	GetUOID((char *)"nunki.usc.edu_12313_1002",(char *)"msg",uoid_buf3,UOIDLEN);
	string s3(uoid_buf3);
	unsigned int key3=RSHash(uoid_buf3,UOIDLEN);
	cout<<"\nInserting at key :: "<<key3;
	btree.insert((const unsigned int)key3,&s3);
	
	char *uoid_buf4=new char[20];
	GetUOID((char *)"nunki.usc.edu_12314_1003",(char *)"msg",uoid_buf4,UOIDLEN);
	string s4(uoid_buf4);
	unsigned int key4=RSHash(uoid_buf4,UOIDLEN);
	cout<<"\nInserting at key :: "<<key4;
	btree.insert((const unsigned int)key4,&s4);
	
	char *uoid_buf5=new char[20];
	GetUOID((char *)"nunki.usc.edu_12315_1004",(char *)"msg",uoid_buf5,UOIDLEN);
	string s5(uoid_buf5);
	unsigned int key5=RSHash(uoid_buf5,UOIDLEN);
	cout<<"\nInserting at key :: "<<key5;
	btree.insert((const unsigned int)key5,&s5);
	
	char *uoid_buf6=new char[20];
	GetUOID((char *)"nunki.usc.edu_12316_1005",(char *)"msg",uoid_buf6,UOIDLEN);
	string s6(uoid_buf6);
	unsigned int key6=RSHash(uoid_buf6,UOIDLEN);
	cout<<"\nInserting at key :: "<<key6;
	btree.insert((const unsigned int)key6,&s6);
	
	unsigned int temp=RSHash(uoid_buf4,UOIDLEN);
	if(btree.get(temp)==NULL)
	cout<<"\nexpected entry not found\n";
	else
	cout<<"\nentry found in btree\n";
	
	traversal(btree.root);
	
	unsigned int key11=RSHash((char*)((remove_node->data)->c_str()),UOIDLEN);
	btree.destroy(key11);
	cout<<"\n\n";
	traversal(btree.root);
	
}
*/


char* create_node_inst_id(unsigned short int port)
{
	time_t t;
	time(&t);
	
	//	char *hname=new char[128];
	//	gethostname(hname,128);
	
	char *buf=new char[200];
	sprintf(buf, "%s_%d_%lu",hostname,port,t);
	
	cout<<"\nhname :: "<<hostname;
	cout<<"\nport :: "<<port;
	cout<<"\nt :: "<<t;
	cout<<"\nbuf :: "<<buf;
	//delete hname;
	
	return buf;
}

//void init_thread_port(struct thread_port_map *thread_port)   //populating port numbers of all beacon nodes in the list
//{
	
//

pthread_t listen_thread;		//Thread listening for requests
pthread_t cmdline_thread;		//Thread to handle command line
//pthread_t *conn_thread;			//Threads for requesting connections 
pthread_t conn_thread[10];
pthread_t timer_thr;			//Thread for timers
pthread_attr_t attr;


int main(int argc,char *argv[])
{
	char *fname=*(argv+1);
	if(fname==NULL)
	{
		cout<<"\nPlease specify an ini file . Error . . .";
		exit(1);
	}
	
	signal(SIGUSR1,catch_SIGUSR1);
	
//	signal(SIGINT,catch_signal);
//	signal(SIGTERM,catch_signal);
//	signal(SIGALRM,catch_alarm);
//	signal(SIGPIPE,catch_signal);

	cout<<"\nfile name :: "<<fname;
	
	init_network();
	
	inist->read_inifile(fname);
	
	init_logfile();
	
	cout<<"\n--------------startup.ini-------------------\n";
	cout<<"\nport :: "<<inist->port;
	cout<<"\nlocation :: "<<inist->location;
	cout<<"\nhomedir :: "<<inist->homedir;
	cout<<"\nlogfilename :: "<<inist->logfilename;
	
	//Initializing port for the node
	myport=inist->port;
	
	//Initialize host name for the node
	gethostname(hostname,100);
	
	//Creating node_inst_id
	node_inst_id=create_node_inst_id(inist->port);
	
//	try_msghdr();

//	try_btree();
	
//	sleep(15);
	
	
	for(int i=0;i<inist->beaconcount;i++)
	{
	if(myport==inist->beacon[i]->port && (!strcmp(hostname,inist->beacon[i]->hostname)))
		{
			beacon_flag=1;
			break;
		}
	}
	
	
	//Set attributes for threads
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	//Creating a thread for listening
	if(pthread_create(&listen_thread,&attr,beacon_server,NULL))
	{
		perror("pthread_create() for listen_thread failed :: ");
		exit(1);
	}
	
	//Creating a thread for command line handling
	if(pthread_create(&cmdline_thread,&attr,beacon_cmdline,NULL))
	{
		perror("pthread_create() for cmdline_thread failed :: ");
		exit(1);
	}

	
	//Creating threads for connection to other beacon nodes
	int conn_thread_count=0;
	if(beacon_flag==1)
	{
	//	pthread_t temp_t;
	//	conn_thread=(pthread_t *)calloc((inist->beaconcount-1),sizeof(temp_t));
		for(int i=0;i<inist->beaconcount;i++)
		{
			if(inist->port!=inist->beacon[i]->port)
			{
				cout<<"\ninist->beacon[i]->port in beacon_hello.cc ::"<<inist->beacon[i]->port;
		
				if(pthread_create(&conn_thread[conn_thread_count],&attr,beacon_client,(void*)(&(inist->beacon[i]->port))))   //parameters: connecting port number
				{
					perror("pthread_create() for conn_thread failed :: ");
					exit(1);
				}
				conn_thread_count++;
			}
		}
	}
	else
	{
	//	pthread_t temp_t;
	//	conn_thread=(pthread_t *)calloc(1,sizeof(temp_t));
		if(pthread_create(conn_thread,&attr,beacon_client,NULL))   
		{
			perror("pthread_create() for conn_thread failed :: ");
			exit(1);
		}
	}
	
	//Creating thread for timers
	if(pthread_create(&timer_thr,&attr,timer_thread,NULL))
	{
		perror("pthread_create() for timer_thr failed :: ");
		exit(1);
	}



	void *listen_thread_st;
	pthread_join(listen_thread,&listen_thread_st);
	
	void *cmdline_thread_st;
	pthread_join(cmdline_thread,&cmdline_thread_st);
	
	if(beacon_flag==1)
	{
		for(int i=0;i<conn_thread_count;i++)
		{
			void *conn_thread_st;
			pthread_join(conn_thread[i],&conn_thread_st);
		}
	}
	else
	{
		void *conn_thread_st;
		pthread_join(*conn_thread,&conn_thread_st);
	}

	//In cleanup delete all thread port map->hostname,
	//when we r taking uoid from node, allocate memory for that  uoid...bcoz i m deleting node->uoid memry at the time of deleting node entry
	//delete commnhdr after forming a any message...
	//after sending any message free that message memory
	//
	//in non beacn startup..SIGUSR1 is coming so early that it can not recv even one response msg.....(.????)
	//and hence no entry is appended in list...so when we are trying to remove without chking IsEmpty(), it is giving seg fault..i think...now its 6:28..so i m sleeping now...hehe..
/*	
	void *timer_thr_st;
	pthread_join(timer_thr,&timer_thr_st);
*/	
	return 0;
}

//g++ beacon_hello.cc iniparse.cc ./src/iniparser.cc ./src/dictionary.cc ./src/strlib.cc message.cc beacon_connect.cc beacon_listen.cc beacon.cc beacon_cmd.cc -o beacon_hello -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib -lcrypto -lsocket -lnsl -lresolv -lpthread

//./binary_hash/btr/bstree.cpp deliberately not included to prevent errors while compiling(All function definitions included in bstree.h)
//g++ beacon_hello.cc iniparse.cc ./src/iniparser.cc ./src/dictionary.cc ./src/strlib.cc message.cc beacon_connect.cc beacon_listen.cc beacon.cc beacon_cmd.cc send_recv.cc ./list/list.cc -o beacon_hello -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib -lcrypto -lsocket -lnsl -lresolv -lpthread

//g++ beacon_hello.cc iniparse.cc ./src/iniparser.cc ./src/dictionary.cc ./src/strlib.cc message.cc beacon_connect.cc beacon_listen.cc beacon.cc beacon_cmd.cc send_recv.cc ./list/list.cc timer.cc -o beacon_hello -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib -lcrypto -lsocket -lnsl -lresolv -lpthread
