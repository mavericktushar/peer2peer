#include "beacon.h"
#include "message.h"

struct dist_info_struct
{
	char *hname;
	unsigned short int port;
	unsigned int dist;
	int hnamelen;
};

struct init_file_write_struct
{
	char *hname;
	unsigned short int port;
};

FILE *init_neighbors_fp;

unsigned short int port_to_connect;

//void *beacon_client_recv(void *arg)

void *neighbor_connect(void *arg)
{
	int	clientsock;
	struct sockaddr_in serv_addr_info;
	struct hostent* he;
	int index;
	struct dist_info_struct *read_data=(struct dist_info_struct *)arg;
	
	if((clientsock = socket(AF_INET, SOCK_STREAM, 0))==-1)
	{
		perror("\nsocket() failed in beacon_client :: ");
		exit(1);
	}
	cout<<"\nIn neighbor_connect . . .";
	cout<<"\nread_data->hname :: "<<read_data->hname;
	cout<<"\nread_data->hname :: "<<read_data->hname;
	cout<<"\nread_data->port :: "<<read_data->port;
	cout<<"\nread_data->port :: "<<read_data->port;
	cout<<"\nHname length :: "<<read_data->hnamelen;
	cout<<"\nHname length :: "<<read_data->hnamelen;
//	char *temp_hname=new char[read_data->hnamelen];
//	strncpy(temp_hname,read_data->hname,read_data->hnamelen);
	if((he=gethostbyname(read_data->hname))==NULL)
		{
			herror("beacon_client gethostbyname() failed : ");
			exit(1);
		}

		memset(&serv_addr_info,0,sizeof(serv_addr_info));
		serv_addr_info.sin_family = AF_INET;
		serv_addr_info.sin_port = htons(read_data->port);
		serv_addr_info.sin_addr = *((struct in_addr*)he->h_addr);  //i think here we need to get ip addr of host name of beacon node.
		
		pthread_mutex_lock(&thread_port_mutex);
		if((index=get_index_from_port(read_data->port,read_data->hname))==-1)
				{
						//-1 index, means first time connection between these two nodes (connect thread has not yet sent connect to the other node)
						//realloc thread_port as entry not found;
						//Increment count - number of connections accepted till date(connected or disconnected)
						//Valid bit set  
						//accept_flag set						
						acc_conn_cnt++;
						index=acc_conn_cnt-1;
						
						cout<<"\nindex not found in thread_port, new index :: "<<index;
						
						struct thread_port_map td_p_mp;
						thread_port=(struct thread_port_map *)realloc(thread_port,(acc_conn_cnt*sizeof(td_p_mp)));
						thread_port[index].hname=read_data->hname;        //new char[read_data->hnamelen];
						thread_port[index].msg_Q=new List();
						thread_port[index].valid=1;
						thread_port[index].conn_flag=0;
						thread_port[index].accept_flag=0;
						thread_port[index].conn_end_flag=0;
						thread_port[index].port=read_data->port;
						//strncpy(thread_port[index].hname,read_data->hname,read_data->hnamelen);
						thread_port[index].send_thread_id=0;
						//thread_port[index].keep_alive=inist->keepalivetimeout;
						//pthread_mutex_init(&thread_port[index].list_mutex,NULL);
						
				}
				else
				{
						//search thread_port 
						
						//if valid bit not set but port entry found - means Old connection which was disconnected (connect thread has not yet sent connect to the other node)
						//set valid  bit and set accept_flag
						if(thread_port[index].valid!=1)
						{
							cout<<"\nindex found in thread_port(valid!=1) :: "<<index;
							thread_port[index].valid=1;
							//thread_port[index].keep_alive=inist->keepalivetimeout;
							thread_port[index].send_thread_id=0;
							thread_port[index].msg_Q=new List();
							thread_port[index].conn_flag=0;
							thread_port[index].accept_flag=0;
							thread_port[index].conn_end_flag=0;
							
						}
						
						//If valid bit set, port entry found - means connect thread has already initiated connection - conn_flag will be set
						//so, check simultaneous connection
						else
						{
							if(thread_port[index].accept_flag==1)
							{
								//compare
								cout<<"\nindex found in thread_port (valid==1) :: "<<index;
								cout<<"\naccept flag set . . .exiting ";
								pthread_mutex_unlock(&thread_port_mutex);
								pthread_exit(NULL);
							}
						}
						
				}
				pthread_mutex_unlock(&thread_port_mutex);

		pthread_mutex_lock(&thread_port_mutex);
		if(thread_port[index].accept_flag==0)
		{
			while(connect(clientsock, (sockaddr *) &serv_addr_info, sizeof(serv_addr_info))<0)
			{
				perror("\nconnection failed in beacon_client()");
			
				pthread_mutex_unlock(&thread_port_mutex);
				sleep(inist->retry);
				//check flag to see if accept has created a connection 
				pthread_mutex_lock(&thread_port_mutex);
				
				if(thread_port[index].accept_flag==1)
				{
					pthread_mutex_unlock(&thread_port_mutex);
					cout<<"\nnot trying to connect. Exiting. accept flag set . . .";
					pthread_exit(NULL);
				}
			
			}
		}
		else
		{
			cout<<"\nbefore trying to connect. Exiting. accept flag set . . .";
			pthread_mutex_unlock(&thread_port_mutex);
			pthread_exit(NULL);
			//terminate
		}
	
		cout<<"\nconnection flag set . . .";
		thread_port[index].conn_flag=1;
		
		pthread_mutex_unlock(&thread_port_mutex);
		
		
		char uoid_buf[20];
		GetUOID(node_inst_id,(char *)"msg",uoid_buf,UOIDLEN);
		
		//char hname[100];
		//gethostname(hname,100);
		
		int hnamelen=strlen(hostname);
		int datalen=hnamelen+sizeof(unsigned short int);
		cout<<"\nDatalength sent in beacon_client() :: "<<datalen;
		char *commhdr=GetCommonHeader(0xfa,uoid_buf,(unsigned char)1,datalen);
	
		int hello_msg_buf_sz=(COMMON_HEADER_SIZE+datalen);
		char *hello_msg_buf=(char *)calloc(hello_msg_buf_sz,1);
		GetHelloMsg(hello_msg_buf,commhdr,myport,hostname,hello_msg_buf_sz);
		
		if(send(clientsock,hello_msg_buf,hello_msg_buf_sz,0)!=hello_msg_buf_sz)
		{
			perror("\nsend() while trying to send hello message failed (in neighbor_connect()):: ");
		}
		
		/**********************************Log file *******************************************/
	
		int n_id_s=get_node_id_from_port(read_data->port);
				
		struct timeval t_vl;
		gettimeofday(&t_vl,NULL);
				
		char *log_file_buf=new char[100];
		sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %hd %s\n",'s',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id_s,"HLLO",hello_msg_buf_sz,1,uoid_buf[16],uoid_buf[17],uoid_buf[18],uoid_buf[19],myport,hostname);
				
		fwrite(log_file_buf,1,strlen(log_file_buf),log_file_fp);
		fflush(log_file_fp);
	
		/**********************************Log file *******************************************/
		
		char *hdr_buf=(char *)calloc(COMMON_HEADER_SIZE,1);
		int index1=0;
		int recvmsglen;
		while(index1<COMMON_HEADER_SIZE)
		{
			if((recvmsglen=recv(clientsock,&hdr_buf[index1],1,0))!=1)
			{
				perror("\nrecv() on receiving header from node sending hello reply failed :: ");
			}
			index1++;
		}
		int datalen_recv;
		memcpy(&datalen_recv,&hdr_buf[23],4);
		
		char *data=(char *)calloc(datalen_recv,1);
		if((recvmsglen=recv(clientsock,data,datalen_recv,0))!=datalen_recv)
		{
			perror("\nrecv() on receiving DATA from node sending hello reply failed :: ");
		}
		unsigned short p=0;
		char *ho=(char *)calloc(datalen_recv-2,1);
		memcpy(&p,&data[0],2);
		memcpy(ho,&data[2],(datalen_recv-2));
		
		p=ntohs(p);
		
		cout<<"\nnonbeacon_client() . . .";
		cout<<"\nData length received :: "<<datalen_recv;
		cout<<"\nreceived port ::"<<p;
		cout<<"\nreceived host name ::"<<ho;
		
		/**********************************Log file *******************************************/
	
		int n_id=get_node_id_from_port(p);
				
	//	struct timeval t_vl;
		gettimeofday(&t_vl,NULL);
				
	//	char *log_file_buf=new char[100];
		memset(log_file_buf,'\0',100);
		sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %hd %s\n",'r',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id,"HLLO",COMMON_HEADER_SIZE+datalen_recv,(unsigned short int)hdr_buf[21],hdr_buf[17],hdr_buf[18],hdr_buf[19],hdr_buf[20],p,ho);
				
		fwrite(log_file_buf,1,strlen(log_file_buf),log_file_fp);
	
		/**********************************Log file *******************************************/
		
		//Consider point of single connection establishment
		
		/*****************************
		//Fork send receive threads
		*****************************/
		pthread_t send_t,recv_t;
		
		struct passing_info p_i_temp;
		struct passing_info *conn_info=(struct passing_info *)calloc(1,sizeof(p_i_temp));
		conn_info->csock=clientsock;
		conn_info->hname=read_data->hname;
		conn_info->port=read_data->port;
		
		if(pthread_create(&send_t,NULL,send_thread,(void *)conn_info))
		{
			perror("pthread_create() for send_thread failed :: ");
			exit(1);
		}
		
		if(pthread_create(&recv_t,NULL,recv_thread,(void *)conn_info))
		{
			perror("pthread_create() for recv_thread failed :: ");
			exit(1);
		}
		
		void *send_thread_st;
		pthread_join(send_t,&send_thread_st);
		
		void *recv_thread_st;
		pthread_join(recv_t,&recv_thread_st);
		
		//Threads terminated - Time to close socket and exit
		
		close(clientsock);
		pthread_exit(NULL);
			
}
		
		
void beacon_startup(struct passing_info info)
{
	char *hdr_buf=(char *)calloc(COMMON_HEADER_SIZE,1);
	int index1=0;
	int recvmsglen;
	int clientsock=info.csock;
	while(index1<COMMON_HEADER_SIZE)
	{
		if((recvmsglen=recv(clientsock,&hdr_buf[index1],1,0))!=1)
		{
			perror("\nrecv() on receiving header from connecting node failed :: ");
		}
		index1++;
	}
	int datalen_recv;
	memcpy(&datalen_recv,&hdr_buf[23],4);
	datalen_recv=ntohl(datalen_recv);
	
	char *data=(char *)calloc(datalen_recv,1);
	if((recvmsglen=recv(clientsock,data,datalen_recv,0))!=datalen_recv)
	{
		perror("\nrecv() on receiving DATA from connecting node failed :: ");
	}
	
	unsigned short p=0;
	char *ho=(char *)calloc(datalen_recv-2,1);
	memcpy(&p,&data[0],2);
	memcpy(ho,&data[2],(datalen_recv-2));
	p=ntohs(p);
	
	cout<<"\nbeacon_client() . . .";
	cout<<"\nData length received :: "<<datalen_recv;
	cout<<"\nreceived port ::"<<p;
	cout<<"\nreceived host name ::"<<ho;
	
	/**********************************Log file *******************************************/
	
	int n_id=get_node_id_from_port(p);
				
	struct timeval t_vl;
	gettimeofday(&t_vl,NULL);
				
	char *log_file_buf=new char[100];
	sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %hd %s\n",'r',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id,"HLLO",COMMON_HEADER_SIZE+datalen_recv,(unsigned short int)hdr_buf[21],hdr_buf[17],hdr_buf[18],hdr_buf[19],hdr_buf[20],p,ho);
				
	fwrite(log_file_buf,1,strlen(log_file_buf),log_file_fp);
	
	/**********************************Log file *******************************************/
	
	/*****************************
	//Fork send receive threads
	*****************************/
	
	pthread_t send_t,recv_t;
	
	struct passing_info p_inf_temp;
	struct passing_info *conn_info=(struct passing_info *)calloc(1,sizeof(p_inf_temp));
	conn_info->csock=clientsock;
	conn_info->hname=info.hname;
	conn_info->port=info.port;
		
	if(pthread_create(&send_t,NULL,send_thread,(void *)conn_info))
	{
		perror("pthread_create() for send_thread failed :: ");
		exit(1);
	}
		
	if(pthread_create(&recv_t,NULL,recv_thread,(void *)conn_info))
	{
		perror("pthread_create() for recv_thread failed :: ");
		exit(1);	
	}
	
	void *send_thread_st;
	pthread_join(send_t,&send_thread_st);
	
	void *recv_thread_st;
	pthread_join(recv_t,&recv_thread_st);
	
	//Threads terminated - Time to close socket and exit
	
	close(clientsock);
	pthread_exit(NULL);
	
	//point of single connection establishment
	/*************************
	pthread_mutex_lock(&send_thread_ids_mutex);
	for(int i=0;i<MAX_CONNS;i++)
	{
		if(list_thread_id[i].send_thread_id==0)
		{
			list_thread_id[i].send_thread_id=listen_send;
		}
		else if(i==(MAX_CONNS-1))
		{	
			cout<<"\nMaximum connection limit reached. Error . . .";
			exit(1);
		}
	}				
	pthread_mutex_unlock(&send_thread_ids_mutex);
	****************************************/

}

void nonbeacon_startup(int arg)
{
	int csock=arg;
	int sv_datalen;
	int num_res=0;
	
	cout<<"\nIn nonbeacon_startup . . .";
	
	NB_temp_conn_id = pthread_self();
	NB_temp_conn_jointimeout=inist->msglifetime;
	
	List *dist_list=new List();
	if(beacon_flag==0 && isjoined==0)
	{
		while(SIGUSR1_flag==0)
		{
				char *hdr_buf=(char *)calloc(COMMON_HEADER_SIZE,1);
				int index1=0;
				int recvmsglen;
				while(index1<COMMON_HEADER_SIZE)
				{
					if((recvmsglen=recv(csock,&hdr_buf[index1],1,0))!=1)
					{
						perror("\nrecv() on receiving header from connecting node failed :: ");
						if(/*errno==EINTR && */SIGUSR1_flag==1)
						{
							cout<<"\nIn nonbeacon_startup. errno=EINTR recv() loop . . .";
							break;
						}
					}
					index1++;
				}
				if(SIGUSR1_flag==1)
				break;
				
				cout<<"\nIn nonbeacon_startup. Message header received . . .";
				cout<<"\nIn nonbeacon_startup. Message header received . . .";
				
				char *recv_uoid=(char*)calloc(UOIDLEN,1);
				strncpy(recv_uoid,&hdr_buf[1],UOIDLEN);
				
				unsigned char ttl_ret_char;
				memset(&ttl_ret_char,0,1);
				memcpy(&ttl_ret_char,&hdr_buf[21],1);
				unsigned short recv_ttl=(unsigned short)ttl_ret_char;
				
				int datalen;
				memset(&datalen,0,sizeof(int));
				memcpy(&datalen,&hdr_buf[23],4);
				datalen=ntohl(datalen);
				sv_datalen=datalen;
				//-----------------------------------------------------------------------
				
				char *data=(char *)calloc(datalen,1);
				if((recvmsglen=recv(csock,data,datalen,0))!=datalen)
				{
					perror("\nrecv() on receiving DATA from connecting node failed :: ");
					if(/*errno==EINTR && */SIGUSR1_flag==1)
						{
							cout<<"\nIn nonbeacon_startup. errno=EINTR recv() . . .";
							break;
						}
				}
				
				num_res++;
				cout<<"\nIn nonbeacon_startup. Response Message "<<num_res<<" received . . .";
				
				char *stored_uoid=new char[UOIDLEN];
				unsigned short p=0;
				cout<<"\ndatalen :: "<<datalen;
				cout<<"\nsv_datalen :: "<<sv_datalen;
				int hosz=datalen-(UOIDLEN+6);
				cout<<"\nhosz :: "<<hosz;
				cout<<"\nhosz :: "<<hosz;
				char *ho=(char *)calloc(hosz+1,1);
				unsigned int calc_dist=0;
				memcpy(stored_uoid,&data[0],UOIDLEN);
				memcpy(&calc_dist,&data[UOIDLEN],4);
				memcpy(&p,&data[UOIDLEN+4],2);
			//	hosz=sv_datalen-(UOIDLEN+6);
				memset(ho,'\0',hosz+1);
			//	memcpy(ho,&data[UOIDLEN+6],hosz);
				strncpy(ho,&data[UOIDLEN+6],hosz);
				p=ntohs(p);
				calc_dist=ntohl(calc_dist);
				
				cout<<"Response "<<num_res<<" :: ";
				cout<<"\nhost name :: "<<ho;
				cout<<"\ncalc_dist :: "<<calc_dist;
				cout<<"\nport :: "<<p;
				
				/**********************************Log file *******************************************/
	
				int n_id=get_node_id_from_port(p);
				
				struct timeval t_vl;
				gettimeofday(&t_vl,NULL);
				
				char *log_file_buf=new char[100];
				sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %02x %02x %02x %02x %d %hd %s\n",'r',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id,"JNRS",COMMON_HEADER_SIZE+datalen,recv_ttl,recv_uoid[16],recv_uoid[17],recv_uoid[18],recv_uoid[19],stored_uoid[16],stored_uoid[17],stored_uoid[18],stored_uoid[19],calc_dist,p,ho);
				
				fwrite(log_file_buf,1,strlen(log_file_buf),log_file_fp);
				fflush(log_file_fp);
	
				/**********************************Log file *******************************************/
				
				unsigned int temp=RSHash(stored_uoid,UOIDLEN);
				
				//Populating structure to contain distances received in response messages from other nodes 
				struct dist_info_struct d_i_t;
				struct dist_info_struct *dist_info=(struct dist_info_struct *)calloc(1,sizeof(d_i_t));
				dist_info->hname=ho;
				dist_info->port=p;
				dist_info->dist=calc_dist;
				dist_info->hnamelen=(datalen-(UOIDLEN+6));
				
				dist_list->SortedInsert((void *)dist_info,dist_info->dist);
				
				node_message_tree *ret_node;
				pthread_mutex_lock(&btree_mutex);
				if((ret_node=btree.get(temp))==NULL)// && SIGUSR1_flag!=1)  //If statement not required as not yet joined the network - may be removed
				{
					pthread_mutex_unlock(&btree_mutex);
					struct node_message_tree n_m_tr;
					struct node_message_tree *node=(struct node_message_tree *)calloc(1,sizeof(n_m_tr));
					node->uoid=recv_uoid;
					node->hname=ho;
					node->port=p;
/*					
					int loc_index;
					
					pthread_mutex_lock(&msg_timer_mutex);
//std::bad_alloc
					{		//just to limit scope of i
						int i;
						for(i=0;i<msg_index;i++)
						{
							if(msg_timer[i].valid==0)
							{
								loc_index=i;
								break;
							}
						}
						if(i==msg_index)
						{
							loc_index=msg_index;
							msg_index++;
						}
					}
					
					msg_timer[loc_index].value=inist->msglifetime;
					msg_timer[loc_index].valid=1;

					pthread_mutex_unlock(&msg_timer_mutex);
					
					node->timerindex=loc_index;
*/
					pthread_mutex_lock(&btree_mutex);
					btree.insert((const unsigned int)temp,node);
					pthread_mutex_unlock(&btree_mutex);
				}
				else
				{
				//	if(strcmp(ret_node->hname)
					cout<<"\nNot yet joined the network. Cannot receive join respones to be forwarded or anything else . . .";
					pthread_mutex_unlock(&btree_mutex);
				}
				
		}
		
		cout<<"\nResponses "<<num_res<<" received by nonbeacon. Finding its neighbor list . . .";
		cout<<"\nResponses "<<num_res<<" received by nonbeacon. Finding its neighbor list . . .";
		
		SIGUSR1_flag=0;
		//Calculate distance and find neighbors to send request
		//Populate the thread_port structure with port numbers of the selected neighbors
		//to be fiiled in with thread IDs later to break simultaneous connection ties
				
		struct dist_info_struct **neighbors=(struct dist_info_struct **)calloc(inist->initneighbors,sizeof(struct dist_info_struct *));
		for(int i=0;i<inist->initneighbors;i++)
		{
			neighbors[i]=(struct dist_info_struct *)calloc(1,sizeof(struct dist_info_struct));
		}
				
		char *loc_home_dir=(char *)calloc(strlen(inist->homedir)+19+1,1);
		memset(loc_home_dir,0,strlen(loc_home_dir));
		strcpy(loc_home_dir,inist->homedir);
		strcat(loc_home_dir,"init_neighbor_list");
				
		init_neighbors_fp=fopen(loc_home_dir,"w+");
				
				
		for(int i=0;i<inist->initneighbors;i++)
		{
			if(dist_list->IsEmpty())
			{
				cout<<"\nMinimum neighbors not found. deleting file . . .";
				cout<<"\nMinimum neighbors not found. deleting file . . .";
				pthread_mutex_lock(&shutdown_mutex);
				soft_restart_flag=1;
				pthread_mutex_unlock(&shutdown_mutex);
				
				if( remove(loc_home_dir) != 0 )
				perror( "Error deleting file" );
				
				close(csock);							//cleanup
				pthread_exit(NULL);
				
				//Not enough neighbors
			}
			else
			{
				neighbors[i]=(struct dist_info_struct *)dist_list->Remove();
						
				struct init_file_write_struct temp_info;
				temp_info.hname=neighbors[i]->hname;
				temp_info.port=neighbors[i]->port;
				
				cout<<"\nWriting to file :: ";
				cout<<"\ntemp_info.hname :: "<<temp_info.hname;
				cout<<"\ntemp_info.port :: "<<temp_info.port;
			/*	struct init_file_write_struct i_f_w_s;
				fwrite(&temp_info,1,sizeof(i_f_w_s),init_neighbors_fp);*/
				char file_entry[50];
				memset(file_entry,'\0',50);
				sprintf(file_entry,"%s %hd\n",temp_info.hname,temp_info.port);
				fwrite(file_entry,1,strlen(file_entry),init_neighbors_fp);
				//fwrite(&temp_info.port,1,2,init_neighbors_fp);
				fflush(init_neighbors_fp);
			}
		}
		
		cout<<"\nclosing socket as all responses received . . .";
/*		struct init_file_write_struct temp_read[inist->initneighbors+1];
		for(int i=0;i<inist->initneighbors;i++)
		{
			temp_read[i].hname=new char[100];
		}
		cout<<"\nReading file . . .";
*/
		int y=0;
/*		while(fread(&temp_read[y],sizeof(struct init_file_write_struct),1,init_neighbors_fp)!=EOF)
		{
			cout<<"\nhost name :: "<<temp_read[y].hname;
			cout<<"\nport :: "<<temp_read[y].port;
			y++;
		}
*/
//		close(csock);
		
//		struct init_file_write_struct temp_read[inist->initneighbors+1];
//		for(int i=0;i<inist->initneighbors;i++)
//		{
//			temp_read[i].hname=new char[100];
//		}
		int x=0;
//		while(fread(&temp_read[x],sizeof(temp_read),1,init_neighbors_fp)!=EOF)
		while(x<inist->initneighbors)
		{
			pthread_t neighbor_connect_thread;
			//thread_port_NB[x].port=temp_read.port;
			//thread_port_NB[x].hname=temp_read.hname;
//			if(pthread_create(&neighbor_connect_thread,NULL,neighbor_connect,(void*)&temp_read[x]))
			cout<<"\nConnecting to neighbor "<<x<<" on port neighbors[x]->port . . .";
			if(pthread_create(&neighbor_connect_thread,NULL,neighbor_connect,(void*)neighbors[x]))
			{
				perror("pthread_create() for recv_thread failed :: ");
				exit(1);
			}
			x++;
		}
		//delete temp_read.hname;
		
		isjoined=1;		//indicate node joined the network
		
	}

	//After getting all responses from nodes and selecting future neighbors
	
}

unsigned short int random_beacon(void)
{
	srand((unsigned int)(time(0)));		
	return((rand()%(inist->beaconcount)));
}	
	
void *beacon_client(void *arg)
{		
	int	clientsock;
	struct sockaddr_in serv_addr_info;
	struct hostent* he;
	
	if((clientsock = socket(AF_INET, SOCK_STREAM, 0))==-1)
	{
		perror("\nsocket() failed in beacon_client :: ");
		exit(1);
	}
	
	if(beacon_flag==1)
	{
		port_to_connect=*((unsigned short int*)arg);
		cout<<"\nport_to_connect::"<<port_to_connect;
		int index;
		
		char *hname_t=get_hname_from_port(port_to_connect);
		char *hname=(char *)calloc(strlen(hname_t)+1,1);
		strcpy(hname,hname_t);     //,strlen(hname_t));
		cout<<"\nhostname:: "<<hname<<"\nstrlen(hostname) ::"<<strlen(hname);
		
		pthread_mutex_lock(&thread_port_mutex);
		if((index=get_index_from_port(port_to_connect,hname))==-1)
		{
			//-1 index, means first time connection between these two nodes (connect thread has not yet sent connect to the other node)
			//realloc thread_port as entry not found;
			//Increment count - number of connections accepted till date(connected or disconnected)
			//Valid bit set  
			//accept_flag set	
			
			acc_conn_cnt++;
			index=acc_conn_cnt-1;
			
			struct thread_port_map td_p_mp; 
			thread_port=(struct thread_port_map *)realloc(thread_port,(acc_conn_cnt*sizeof(td_p_mp)));
			thread_port[index].hname=hname;        //new char[strlen(hname)];
			thread_port[index].msg_Q=new List();
			thread_port[index].valid=1;
			thread_port[index].accept_flag=0;
			thread_port[index].conn_flag=0;
			thread_port[index].conn_end_flag=0;
			thread_port[index].port=port_to_connect;
			//strncpy(thread_port[index].hname,hname,strlen(hname));
			thread_port[index].send_thread_id=0;
			//pthread_mutex_init(&thread_port[index].list_mutex,NULL);
						
		}
		else
		{
			//search thread_port 
						
			//if valid bit not set but port entry found - means Old connection which was disconnected (connect thread has not yet sent connect to the other node)
			//set valid  bit and set accept_flag
			if(thread_port[index].valid!=1)
			{
				thread_port[index].valid=1;
				thread_port[index].conn_end_flag=0;
				thread_port[index].msg_Q=new List();
				//thread_port[index].conn_flag=1;
				thread_port[index].send_thread_id=0;
			}
						
			//If valid bit set, port entry found - means connect thread has already initiated connection - conn_flag will be set
			//so, check simultaneous connection
			else
			{
				if(thread_port[index].accept_flag==1)
				{
					pthread_mutex_unlock(&thread_port_mutex);
					pthread_exit(NULL);
				}
			}
						
		}
		pthread_mutex_unlock(&thread_port_mutex);
		//thread_port[index].thread_id_conn=pthread_self();
		
		
		if((he=gethostbyname(hname))==NULL)
		{
			herror("beacon_client gethostbyname() failed : ");
			exit(1);
		}
	
		memset(&serv_addr_info,0,sizeof(serv_addr_info));
		serv_addr_info.sin_family = AF_INET;
		serv_addr_info.sin_port = htons(port_to_connect);
		serv_addr_info.sin_addr = *((struct in_addr*)he->h_addr);  //i think here we need to get ip addr of host name of beacon node.

		pthread_mutex_lock(&thread_port_mutex);
		if(thread_port[index].accept_flag==0)
		{
			while(connect(clientsock, (sockaddr *) &serv_addr_info, sizeof(serv_addr_info))<0)
			{
				perror("\nconnection failed in beacon_client()");
			
				pthread_mutex_unlock(&thread_port_mutex);
				sleep(inist->retry);
				//check flag to see if accept has created a connection 
				pthread_mutex_lock(&thread_port_mutex);
				
				if(thread_port[index].accept_flag==1)
				{
					pthread_mutex_unlock(&thread_port_mutex);
					cout<<"\nnot trying to connect. Exiting. accept flag set . . .";
					pthread_exit(NULL);
				}
			
			}
		}
		else
		{
			cout<<"\nbefore trying to connect. Exiting. accept flag set . . .";
			pthread_mutex_unlock(&thread_port_mutex);
			pthread_exit(NULL);
			//terminate
		}
	
		cout<<"\nconnection flag set . . .";
		thread_port[index].conn_flag=1;
		
		pthread_mutex_unlock(&thread_port_mutex);
	}
	else
	{
		cout<<"Value of arg = NULL. Non Beacon node";
			
		do
		{
			cout<<"\nTrying to connect to beacon";
			int ind=random_beacon();
			port_to_connect=inist->beacon[ind]->port;
				
			if((he=gethostbyname(inist->beacon[ind]->hostname))==NULL)
			{
				herror("beacon_client gethostbyname() failed : ");
				exit(1);
			}
		
			memset(&serv_addr_info,0,sizeof(serv_addr_info));
			serv_addr_info.sin_family = AF_INET;
			serv_addr_info.sin_port = htons(port_to_connect);
			serv_addr_info.sin_addr = *((struct in_addr*)he->h_addr);  //i think here we need to get ip addr of host name of beacon node.
		}
		while(connect(clientsock, (sockaddr *) &serv_addr_info, sizeof(serv_addr_info))<0);
		
	}
	
		
/*	if(thread_port[index].accept_flag==1)
	{
		//compare
	}
*/	
	
	//cout<<"\nCONNECTION SUCCESSFULL";
	
	//pthread_t recv_thread;
	//if(pthread_create(recv_thread,NULL,beacon_client_recv,(void *)&clientsock))
	//{
	//	perror("pthread_create() for recv_thread failed :: ");
	//	exit(1);
	//}
	
	
	
	
	
	char uoid_buf[20];
	GetUOID(node_inst_id,(char *)"msg",uoid_buf,UOIDLEN);

	unsigned int temp=RSHash(uoid_buf,UOIDLEN);
	
	if(beacon_flag==1)
	{
		//char hname[100];
		//gethostname(hname,100);
		int hnamelen=strlen(hostname);
		int datalen=hnamelen+sizeof(unsigned short int);
		cout<<"\nDatalength sent in beacon_client() :: "<<datalen;
		char *commhdr=GetCommonHeader(0xfa,uoid_buf,(unsigned char)1,datalen);
	
		int hello_msg_buf_sz=(COMMON_HEADER_SIZE+datalen);
		char *hello_msg_buf=(char *)calloc(hello_msg_buf_sz,1);
		GetHelloMsg(hello_msg_buf,commhdr,myport,hostname,hello_msg_buf_sz);
		//free(commhdr);
/*****************************************************		Hello msg not to be added to tree containing sent/received msg                    
		struct node_message_tree *node=(struct node_message_tree *)calloc(1,sizeof(struct node_message_tree));
		node->uoid=uoid_buf;
		node->hname=hostname;
		node->port=myport;
		int loc_index;
					
		pthread_mutex_lock(&msg_timer_mutex);
		for(int i=0;i<msg_index;i++)
		{
			if(msg_timer[i].valid==0)
			{
				loc_index=i;
				break;
			}
		}
		if(i==msg_index)
		{
			loc_index=msg_index;
			msg_index++;
		}
					
		msg_timer[loc_index].value=inist->msglifetime;
		msg_timer[loc_index].valid=1;
		pthread_mutex_unlock(&msg_timer_mutex);
		
		node->timerindex=loc_index;       //to keep track of the index into the timer table..
		
		btree.insert((const unsigned int)temp,node);
*************************************************************************************************************************************/
		
		if(send(clientsock,hello_msg_buf,hello_msg_buf_sz,0)!=hello_msg_buf_sz)
		{
			perror("\nsend() while trying to send hello message failed :: ");
		}
		
		/**********************************Log file *******************************************/
	
		int n_id=get_node_id_from_port(port_to_connect);
				
		struct timeval t_vl;
		gettimeofday(&t_vl,NULL);
				
		char *log_file_buf=new char[100];
		sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %hd %s\n",'s',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id,"HLLO",hello_msg_buf_sz,1,uoid_buf[16],uoid_buf[17],uoid_buf[18],uoid_buf[19],myport,hostname);
				
		fwrite(log_file_buf,1,strlen(log_file_buf),log_file_fp);
		fflush(log_file_fp);
	
		/**********************************Log file *******************************************/
		
		cout<<"\nHello msg is sent";
		
		char *hname_temp=get_hname_from_port(port_to_connect);
		char* hname=(char*)calloc(strlen(hname_temp)+1,1);
		strcpy(hname,hname_temp);      //,strlen(hname_temp));
		
		struct passing_info bcn_info;
		bcn_info.csock=clientsock;
		bcn_info.hname=hname;
		bcn_info.port=port_to_connect;
		beacon_startup(bcn_info);
	}
	else
	{
		//char hname[100];
		//gethostname(hname,100);
		cout<<"\nSending Join Request msg . . .";
		int hnamelen=strlen(hostname);
		int datalen=sizeof(inist->location)+sizeof(unsigned short int)+hnamelen;
		cout<<"\nDatalength sent in beacon_client() :: "<<datalen;
		char *commhdr=GetCommonHeader(0xfc,uoid_buf,(unsigned char)inist->ttl,datalen);          //join message
			
		int join_msg_buf_sz=(COMMON_HEADER_SIZE+datalen);
		char *join_msg_buf=(char *)calloc(join_msg_buf_sz,1);
		GetJoinRequestMsg(join_msg_buf,commhdr,inist->location,myport,hostname,join_msg_buf_sz);
	//	free(commhdr);
		struct node_message_tree n_m_tree;
		struct node_message_tree *node=(struct node_message_tree *)calloc(1,sizeof(n_m_tree));
		node->uoid=uoid_buf;
		node->hname=hostname;
		node->port=myport;
/*
		int loc_index;
					
		pthread_mutex_lock(&msg_timer_mutex);
//std::bad_alloc		
		{		//just to limit scope of i
			int i;
			for(i=0;i<msg_index;i++)
			{
				if(msg_timer[i].valid==0)
				{
					loc_index=i;
					break;
				}
			}
			if(i==msg_index)
			{
				loc_index=msg_index;
				msg_index++;
			}
		}
					
		msg_timer[loc_index].value=inist->msglifetime;
		msg_timer[loc_index].valid=1;

		pthread_mutex_unlock(&msg_timer_mutex);
		
		node->timerindex=loc_index;
*/		
		pthread_mutex_lock(&btree_mutex);
		btree.insert((const unsigned int)temp,node);
		pthread_mutex_unlock(&btree_mutex);
		
		if(send(clientsock,join_msg_buf,join_msg_buf_sz,0)!=join_msg_buf_sz)
		{
			perror("\nsend() while trying to send join message failed :: ");
		}
		
		/**********************************Log file *******************************************/
		
			int n_id=get_node_id_from_port(port_to_connect);
					
			struct timeval t_vl;
			gettimeofday(&t_vl,NULL);
					
			char *log_file_buf=new char[100];
			sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %hd %s\n",'s',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id,"JNRQ",join_msg_buf_sz,1,uoid_buf[16],uoid_buf[17],uoid_buf[18],uoid_buf[19],myport,hostname);
					
			fwrite(log_file_buf,1,strlen(log_file_buf),log_file_fp);
			fflush(log_file_fp);
		
			/**********************************Log file *******************************************/
				
		cout<<"\njoin req msg sent";
		nonbeacon_startup(clientsock);					
	}
	
	
	
	// pthread - recv()
/*										SHOULD BE ONLY IN recv THREAD OF THIS CONNECTION	
	char *hdr_buf=(char *)calloc(COMMON_HEADER_SIZE,sizeof(char));
	int index1=0;
	int recvmsglen;
	while(index1<COMMON_HEADER_SIZE)
	{
		if((recvmsglen=recv(clientsock,&hdr_buf[index1],1,0))!=1)
		{
			perror("\nrecv() on receiving header from connecting node failed :: ");
		}
		index1++;
	}
	int datalen_recv;
	memcpy(&datalen_recv,&hdr_buf[23],4);
	
	char *data=(char *)calloc(datalen_recv,sizeof(char));
	if((recvmsglen=recv(clientsock,data,datalen_recv,0))!=datalen_recv)
	{
		perror("\nrecv() on receiving DATA from connecting node failed :: ");
	}
	unsigned short p=0;
	char *ho=(char *)calloc(datalen_recv-2,sizeof(char));
	memcpy(&p,&data[0],2);
	memcpy(ho,&data[2],(datalen_recv-2));
	
	cout<<"\nbeacon_client() . . .";
	cout<<"\nData length received :: "<<datalen_recv;
	cout<<"\nreceived port ::"<<p;
	cout<<"\nreceived host name ::"<<ho;
	
	//point of single connection establishment
	
	pthread_mutex_lock(&send_thread_ids_mutex);
	for(int i=0;i<MAX_CONNS;i++)
	{
		if(list_thread_id[i].send_thread_id==0)
		{
			list_thread_id[i].send_thread_id=listen_send;
		}
		else if(i==(MAX_CONNS-1))
		{	
			cout<<"\nMaximum connection limit reached. Error . . .";
			exit(1);
		}
	}				
	pthread_mutex_unlock(&send_thread_ids_mutex);
	
*/
}
