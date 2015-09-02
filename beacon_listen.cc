#define UOIDLEN 20

#include <iostream>
#include <stdio.h>


#include "beacon.h"
#include "message.h"

using namespace std;

unsigned short int listening_port;

void term_simult_conn(unsigned short int p,char * hname_p)
{
//	int ind_acc,ind_conn;
	
//	ind_acc=get_index_from_port(myport);
//	ind_conn=get_index_from_port(p);
	
	if(p>myport)
	{
		//terminate accepted thread of p
		cout<<"\nterminating own connecting thread p>myport . . .";
		//set flag indicating own connecting thread to terminate
		
	//	pthread_cancel(thread_port[ind_conn].thread_id_conn);//terminate        //set connection falg in thread port map to 1
	}
	else if(p<myport)
	{
		cout<<"\nterminating own accepted thread myport>p . . .";
		pthread_mutex_unlock(&thread_port_mutex);
		pthread_exit(NULL);
	}
	else
	{
//		char *hname_myport=get_hname_from_port(myport);
//		char *hname_p=get_hname_from_port(p);
		
		if(strcmp(hname_p,hostname)>0)
		{
			//terminate accepted thread of p
			cout<<"\nterminating own connecting thread hname_p>hname_myport";
			//set flag indicating own connecting thread to terminate
			
	//		pthread_cancel(thread_port[ind_conn].thread_id_conn);//terminate                //set connection falg in thread port map to 1
		}
		else if(strcmp(hname_p,hostname)<0)
		{
			cout<<"\nterminating own accepted thread hname_p<hname_myport . . .";
			pthread_mutex_unlock(&thread_port_mutex);
			pthread_exit(NULL);
		}
		else
		{
			cout<<"\nSame port numbers and hostname. Error . . .";
		}
	}
}

/*
void *listen_child_send(void *arg)
{
	if(send(csock,hello_msg_buf,hello_msg_buf_sz,0)!=hello_msg_buf_sz)
	{
		perror("\nsend() while trying to send hello message failed :: ");
	}
}
*/

void *beacon_listen_child(void *arg)
{
	cout<<"\nACCEPTED ONE CONNECTION--child thread";
	cout<<"\nACCEPTED ONE CONNECTION--child thread";
	int *temp_sock=(int*)arg;
	int csock=*temp_sock;
//	free(arg);         //memory at the time of passing clientsock in pthread  create.
	int sv_datalen;
	cout<<"\nclientsock :: "<<csock;
	cout<<"\nclientsock :: "<<csock;
	cout<<"\nclientsock :: "<<csock;
	
	char *hdr_buf=(char *)calloc(COMMON_HEADER_SIZE,1);
	int index1=0;
	int recvmsglen;
	while(index1<COMMON_HEADER_SIZE)
	{
		if((recvmsglen=recv(csock,&hdr_buf[index1],1,0))!=1)
		{
			perror("\nBeacon listen child:: recv() on receiving header from connecting node failed :: ");
		}
		index1++;
	}
	index1=0;
	cout<<"\nreceived message header in beacon_listen_child . . .";
	
	//------------recovering header parameters-------------------------------
	unsigned char recv_msg_type=(unsigned char)hdr_buf[0];
	
	char *recv_uoid=(char*)calloc(UOIDLEN,1);
	strncpy(recv_uoid,&hdr_buf[1],UOIDLEN);
	
	unsigned char ttl_ret_char;
	memset(&ttl_ret_char,0,1);
	memcpy(&ttl_ret_char,&hdr_buf[21],1);
	unsigned short recv_ttl=(unsigned short)ttl_ret_char;
	
	int datalen;
	memset(&datalen,0,4);
	memcpy(&datalen,&hdr_buf[23],4);
	cout<<"\ndatalen :: "<<datalen;
	datalen=ntohl(datalen);
	cout<<"\nntohl(datalen) :: "<<datalen;
	sv_datalen=datalen;
	cout<<"\nsv_datalen :: "<<sv_datalen;
	//-----------------------------------------------------------------------
	
	char *data=(char *)calloc(datalen,1);
	if((recvmsglen=recv(csock,data,datalen,0))!=datalen)
	{
		perror("\nBeacon listen child::recv() on receiving DATA from connecting node failed :: ");
	}
		
	cout<<"\nreceived message data in beacon_listen_child . . .";
	
	switch(recv_msg_type)
	{
		case 0xfc:							//Join Request - Means this node is beacon and Non beacon is sending Join Request(Initial startup phase)
			{	//from non beacon
				cout<<"\nreceived Join Request in beacon_listen_child . . .";
				cout<<"\nreceived Join Request in beacon_listen_child . . .";
				unsigned short p=0;
				char *ho=(char *)calloc((datalen-6)+1,1);
				unsigned int loctn=0;
				memset(ho,0,(datalen-6)+1);
				memcpy(&loctn,&data[0],4);
				memcpy(&p,&data[4],2);
				strncpy(ho,&data[6],datalen-6);
				
				p=ntohs(p);
				loctn=ntohl(loctn);
				
				/**********************************Log file *******************************************/
	
				int n_id=get_node_id_from_port(p);
				
				struct timeval t_vl;
				gettimeofday(&t_vl,NULL);
				
				char *log_file_buf=new char[100];
				sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %hd %s\n",'r',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id,"JNRQ",COMMON_HEADER_SIZE+datalen,recv_ttl,recv_uoid[16],recv_uoid[17],recv_uoid[18],recv_uoid[19],p,ho);
				
				fwrite(log_file_buf,1,strlen(log_file_buf),log_file_fp);
				fflush(log_file_fp);
	
				/**********************************Log file *******************************************/
				
				pthread_mutex_lock(&btree_mutex);
				unsigned int temp=RSHash(recv_uoid,UOIDLEN);
				if(btree.get(temp)==NULL)
				
				{
					pthread_mutex_unlock(&btree_mutex);
					struct node_message_tree n_m_temp;
					struct node_message_tree *node=(struct node_message_tree *)calloc(1,sizeof(n_m_temp));
					node->msg_time=inist->msglifetime;
					node->uoid=recv_uoid;
					node->hname=ho;
					node->port=p;
				
					pthread_mutex_lock(&btree_mutex);
					btree.insert((const unsigned int)temp,node);
					pthread_mutex_unlock(&btree_mutex);
				}
				else
				{	
					pthread_mutex_unlock(&btree_mutex);
				}
				
				int index=-1;
				pthread_mutex_lock(&thread_port_temp_mutex);
				for(int i=0;i<acc_conn_temp;i++)
				{
					if(p==thread_port_temp[i].port/* && !strcmp(ho,thread_port_temp[i].hname)*/)
					{
						index=i;
						break;
					}
				}
				
				if(index==-1)
				{
					cout<<"\nreceiving join_request : populating thread_port_temp. no previous entry found. Absolutely new connection . . ."; 
					cout<<"\nreceiving join_request : populating thread_port_temp. no previous entry found. Absolutely new connection . . ."; 
					
					
					acc_conn_temp++;
					index=acc_conn_temp-1;
					
					struct thread_port_map  t_p_map;
					thread_port_temp=(struct thread_port_map *)realloc(thread_port_temp,(acc_conn_temp*sizeof(t_p_map)));
					int memsize=datalen-6;
				//	thread_port_temp[index].hname=(char *)calloc(memsize,sizeof(char));//new char[memsize];
					thread_port_temp[index].msg_Q=new List();
					if(thread_port_temp[index].msg_Q==NULL)
					{
						cout<<"MESSAGE QUEUE NULL (beacon_listen_child) for thread_port_map, index "<<index<<". . .";
						cout<<"MESSAGE QUEUE NULL (beacon_listen_child) for thread_port_map, index "<<index<<". . .";
					}
					else
					{
						cout<<"MESSAGE QUEUE NOT NULL (beacon_listen_child) for thread_port_map, index "<<index<<". . .";
						cout<<"MESSAGE QUEUE NOT NULL (beacon_listen_child) for thread_port_map, index "<<index<<". . .";
					}
					thread_port_temp[index].valid=1;
					thread_port_temp[index].conn_end_flag=0;
				//	thread_port_temp[index].accept_flag=1;
					thread_port_temp[index].port=p;
				//	memcpy(thread_port_temp[index].hname,ho,(memsize));
				//	strncpy(thread_port_temp[index].hname,ho,memsize);
					thread_port_temp[index].hname=ho;
					thread_port_temp[index].send_thread_id=0;
					thread_port_temp[index].jointimeout=inist->msglifetime;
					//pthread_mutex_init(&thread_port[index].list_mutex,NULL);
				}
				else
				{
					if(thread_port_temp[index].valid==1)
					{
						cout<<"Error. valid bit not unset by previous connection while exiting . . .";
					}
					else
					{	
						cout<<"\nreceiving join_request : populating thread_port_temp. previous entry found. Need to make entry valid . . ."; 
						thread_port_temp[index].valid=1;
						thread_port_temp[index].conn_end_flag=0;
						thread_port_temp[index].jointimeout=inist->msglifetime;
						thread_port_temp[index].msg_Q=new List();
					}
				}
				pthread_mutex_unlock(&thread_port_temp_mutex);
				
				
				//Forwarding join request on all connections
				cout<<"\nForwarding Join request on all connections . . .";
				cout<<"\nForwarding Join request on all connections . . .";
				int send_buf_sz=COMMON_HEADER_SIZE+sv_datalen;
				cout<<"\nsv_datalen :: "<<sv_datalen;
				cout<<"\nCOMMON_HEADER_SIZE :: "<<COMMON_HEADER_SIZE;
				cout<<"\nsend_buf_sz :: "<<send_buf_sz;
				cout<<"\nsend_buf_sz :: "<<send_buf_sz;
				char *send_buf=(char *)calloc(send_buf_sz,1);
				
				memcpy(send_buf,hdr_buf,COMMON_HEADER_SIZE);
				memcpy(&send_buf[27],&data[0],datalen);    //why this was commented
				
				struct list_element_struct temp_elem;
				struct list_element_struct *element=(struct list_element_struct*)calloc(1,sizeof(temp_elem));
				element->msg=send_buf;
				element->msg_sz=COMMON_HEADER_SIZE+datalen;
				element->f_s=1;
				
				pthread_mutex_lock(&thread_port_mutex);
				for(int i=0;i<acc_conn_cnt;i++)
				{
					if(thread_port[i].valid==1)
					{
						//pthread_mutex_lock(&thread_port[i].list_mutex);
						(thread_port[i].msg_Q)->Append((void*)element);
						//pthread_mutex_unlock(&thread_port[i].list_mutex);
					}
				}
				pthread_mutex_unlock(&thread_port_mutex);
				
				pthread_cond_broadcast(&send_thread_cv);
				
				//Creating Join Response (sending its own and other responses)
				
				cout<<"\nCreating Join response . . ."; 
				
				char *uoid_buf=new char[20];
				GetUOID(node_inst_id,(char *)"msg",uoid_buf,UOIDLEN);
						
				unsigned int req_loc;
				memcpy(&req_loc,&data[0],4);
						
				int hnamelen=strlen(hostname);
				int datalen=hnamelen+UOIDLEN+sizeof(unsigned short int)+sizeof(int);
				cout<<"\nDatalength sent in beacon_client() :: "<<datalen;
				char *commhdr=GetCommonHeader(0xfb,uoid_buf,(unsigned char)1,datalen);
			
				int join_res_msg_buf_sz=(COMMON_HEADER_SIZE+datalen);
				char *join_res_msg_buf=new char[join_res_msg_buf_sz];
				int join_port=myport;
				unsigned int dist_node=abs((int)(req_loc - inist->location));
				GetJoinResponseMsg(join_res_msg_buf,commhdr,recv_uoid,dist_node,join_port,hostname,join_res_msg_buf_sz);
			//	free(commhdr);		
				struct list_element_struct temp_elem1;
				struct list_element_struct *element1=(struct list_element_struct*)calloc(1,sizeof(temp_elem1));
				element1->msg=join_res_msg_buf;
				element1->msg_sz=join_res_msg_buf_sz;
				element1->f_s=0;
				
				cout<<"\nJoin response msg sent:: ";
				cout<<"\nhost name :: "<<hostname;
				cout<<"\nport :: "<<join_port;
				cout<<"\ndistance :: "<<dist_node;
				
				pthread_mutex_lock(&thread_port_temp_mutex);
						
				//pthread_mutex_lock(&thread_port_temp[index].list_mutex);
				(thread_port_temp[index].msg_Q)->Append((void *)element1);
				//pthread_mutex_unlock(&thread_port_temp[index].list_mutex);
				
				
				int frst=0;
				while(thread_port_temp[index].conn_end_flag==0/*need timeout values*/)		//may be the reason for std::bad_alloc
				{
					cout<<"\nIn while. Sending join responses over temporary connection . . .";
					//pthread_mutex_lock(&thread_port_temp_mutex);
					
					//pthread_mutex_lock(&thread_port_temp[index].list_mutex);		
					while(thread_port_temp[index].msg_Q->IsEmpty())
					{
						//pthread_mutex_unlock(&thread_port_temp_mutex);
						
						pthread_cond_wait(&send_temp_cv,&thread_port_temp_mutex);
						if(thread_port_temp[index].conn_end_flag==1)
						{
							cout<<"\nconn_end_flag==1 index :: "<<index<<" . . .";
							cout<<"\nconn_end_flag==1 index :: "<<index<<" . . .";
							thread_port_temp[index].valid=0;
							thread_port_temp[index].send_thread_id=0;
							thread_port_temp[index].conn_end_flag=0;
						//	delete thread_port_temp[index].msg_Q;
							pthread_mutex_unlock(&thread_port_temp_mutex);
							pthread_exit(NULL);
						}
						
						//pthread_mutex_lock(&thread_port_temp_mutex);
					}
					
					struct list_element_struct *elem=(struct list_element_struct *)(thread_port_temp[index].msg_Q)->Remove();
			
					//pthread_mutex_unlock(&thread_port_temp_mutex);
			
					if(send(csock,elem->msg,elem->msg_sz,0)!=elem->msg_sz)
					{
						perror("\nsend() while trying to send hello message in send_thread failed :: ");
						perror("\nsend() while trying to send hello message in send_thread failed :: ");
						thread_port_temp[index].valid=0;
						thread_port_temp[index].send_thread_id=0;
						thread_port_temp[index].conn_end_flag=0;
					//	delete thread_port_temp[index].msg_Q;
						pthread_mutex_unlock(&thread_port_temp_mutex);
						pthread_exit(NULL);
					}
					
					/**********************************Log file *******************************************/
	
					//int n_id=get_node_id_from_port(p);
					
					//struct timeval t_vl;
					gettimeofday(&t_vl,NULL);
					
					//char *log_file_buf=new char[100];
					
					memset(log_file_buf,'\0',100);
					
					unsigned short int loc_p=0;
					char *loc_ho=(char *)calloc((elem->msg_sz-53)+1,1);
					unsigned int loc_dist=0;
					memset(loc_ho,0,(elem->msg_sz-53)+1);
				//	memcpy(loc_ho,&elem->msg[53],(elem->msg_sz-53));
					memcpy(&loc_p,&elem->msg[51],2);
					memcpy(&loc_dist,&elem->msg[47],4);
					strncpy(loc_ho,&elem->msg[53],(elem->msg_sz-53));
					
					int n_id_s=get_node_id_from_port(ntohs(loc_p));
					
					if(frst==0)
					{
						sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %02x %02x %02x %02x %d %hd %s\n",'s',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id_s,"JNRS",elem->msg_sz,elem->msg[21],elem->msg[17],elem->msg[18],elem->msg[19],elem->msg[20],elem->msg[43],elem->msg[44],elem->msg[45],elem->msg[46],loc_dist,loc_p,loc_ho);
						frst=1;
					}
					else
					{
						sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %02x %02x %02x %02x %d %hd %s\n",'f',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id_s,"JNRS",elem->msg_sz,elem->msg[21],elem->msg[17],elem->msg[18],elem->msg[19],elem->msg[20],elem->msg[43],elem->msg[44],elem->msg[45],elem->msg[46],loc_dist,loc_p,loc_ho);
					}
					
					
					fwrite(log_file_buf,1,strlen(log_file_buf),log_file_fp);
					fflush(log_file_fp);
		
					/**********************************Log file *******************************************/
				}
				
				//beacon received a join request from a non beacon. Temporary connection.
				//This can be closed close(csock);
				//invalidate thread_port_temp entry
				
				thread_port_temp[index].valid=0;
				thread_port_temp[index].send_thread_id=0;
				thread_port_temp[index].conn_end_flag=0;
			//	delete thread_port_temp[index].msg_Q;
				pthread_mutex_unlock(&thread_port_temp_mutex);
				pthread_exit(NULL);
				
				
			}
			break;
			
		case 0xfa://--HELLO MESSAGE--
			{	//from beacon to beacon or non beacon to non beacon
				cout<<"\nreceived hello message in beacon_listen_child . . .";
				cout<<"\nreceived hello message in beacon_listen_child . . .";
				unsigned short p=0;
				char *ho=(char *)calloc((datalen-2)+1,1);
				memset(ho,0,(datalen-2)+1);
				memcpy(&p,&data[0],2);
				strncpy(ho,&data[2],(datalen-2));
				
				p=ntohs(p);
				
				/**********************************Log file *******************************************/
	
				int n_id=get_node_id_from_port(p);
				
				struct timeval t_vl;
				gettimeofday(&t_vl,NULL);
				
				char *log_file_buf=new char[100];
				sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %hd %s\n",'r',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id,"HLLO",COMMON_HEADER_SIZE+datalen,recv_ttl,recv_uoid[16],recv_uoid[17],recv_uoid[18],recv_uoid[19],p,ho);
				
				fwrite(log_file_buf,1,strlen(log_file_buf),log_file_fp);
	
				/**********************************Log file *******************************************/

				int from_beacon_flag=0;
				for(int i=0;i<inist->beaconcount;i++)
				{
					if(p==inist->beacon[i]->port/* && (!strcmp(inist->beacon[i]->hostname,ho))*/)
					{
						from_beacon_flag=1;
						cout<<"\nnode is non beacon . . .";
					}
				}
				
				//Between 1)beacon<->beacon, 2)nonbeacon<->nonbeacon 3)nonbeacon->beacon(no simultaneous connection) 	 			
				
				int index; 
					
				
				pthread_mutex_lock(&thread_port_mutex);
				if((index=get_index_from_port(p,ho))==-1)
				{
						//-1 index, means first time connection between these two nodes (connect thread has not yet sent connect to the other node)
						//realloc thread_port as entry not found;
						//Increment count - number of connections accepted till date(connected or disconnected)
						//Valid bit set  
						//accept_flag set		

						cout<<"\npopulating thread_port. No previous entry found. absolutely new connection . . .";
						acc_conn_cnt++;
						cout<<"\nEntry in thread port :: "<<acc_conn_cnt-1;
						cout<<"\nEntry in thread port :: "<<acc_conn_cnt-1;
						index=acc_conn_cnt-1;
						
						cout<<"\n0. . .";
						cout<<"\n0. . .";
						
						struct thread_port_map t_p_mp;
						thread_port=(struct thread_port_map *)realloc(thread_port,(acc_conn_cnt*sizeof(t_p_mp)));
						
						cout<<"\n1. . .";
						cout<<"\n1. . .";
						thread_port[index].hname=new char[(datalen-6)+1];
						cout<<"\n2. . .";
						cout<<"\n2. . .";
						thread_port[index].msg_Q=new List();
						cout<<"\n3. . .";
						cout<<"\n3. . .";
						thread_port[index].valid=1;
						thread_port[index].conn_flag=0;
						thread_port[index].accept_flag=1;
						thread_port[index].conn_end_flag=0;
						thread_port[index].port=p;
						cout<<"\n4. . .";
						cout<<"\n4. . .";
						strcpy(thread_port[index].hname,ho);
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
							cout<<"\npopulating thread_port. entry found. Need to make entry valid . . .";
							thread_port[index].valid=1;
							thread_port[index].accept_flag=1;
							thread_port[index].conn_end_flag=0;
							thread_port[index].send_thread_id=0;
							thread_port[index].msg_Q=new List();
							//thread_port[index].keep_alive=inist->keepalivetimeout;
						}
						
						//If valid bit set, port entry found - means connect thread has already initiated connection - conn_flag will be set
						//so, check simultaneous connection
						else
						{
							thread_port[index].accept_flag=1;
							cout<<"\nAccept flag set . . .";
						
							if(thread_port[index].conn_flag==1)
							{
								//compare
								cout<<"\nsimultaneous connections. comparing . . .";
								term_simult_conn(p,ho);
							}
						}
						
				}
				pthread_mutex_unlock(&thread_port_mutex);
	
	
			
				//int temp_dlen=(int)datalen;
				//int dlen=ntohl(temp_dlen);
	//point of single connection establishment
	
				cout<<"\nbeacon_listen_child() . . .";
				cout<<"\nData length received :: "<<datalen;
				cout<<"\nreceived port ::"<<p;
				cout<<"\nreceived host name ::"<<ho;
	
				char uoid_buf[20];
				GetUOID(node_inst_id,(char *)"msg",uoid_buf,UOIDLEN);
				
				//char hname[100];
				//gethostname(hname,100);
				int hnamelen=strlen(hostname);
				int datalen_send=hnamelen+sizeof(unsigned short int);
				cout<<"\nDatalength sent in beacon_listen_child() :: "<<datalen_send;
				char *commhdr=GetCommonHeader(0xfa,uoid_buf,(unsigned char)1,datalen_send);
				
				int hello_msg_buf_sz=(COMMON_HEADER_SIZE+datalen_send);
				char *hello_msg_buf=(char *)calloc(hello_msg_buf_sz,1);
			//	char hello_msg_buf[hello_msg_buf_sz];
				GetHelloMsg(hello_msg_buf,commhdr,myport,hostname,hello_msg_buf_sz);
			//	free(commhdr);
				if(send(csock,hello_msg_buf,hello_msg_buf_sz,0)!=hello_msg_buf_sz)
				{
					perror("\nsend() while trying to send hello message failed :: ");
				}
				
				/**********************************Log file *******************************************/
	
			//	int n_id=get_node_id_from_port(p);
				
			//	struct timeval t_vl;
				gettimeofday(&t_vl,NULL);
				
			//	char *log_file_buf=new char[100];
				memset(log_file_buf,'\0',100);
				sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %hd %s\n",'s',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id,"HLLO",hello_msg_buf_sz,1,uoid_buf[16],uoid_buf[17],uoid_buf[18],uoid_buf[19],myport,hostname);
				
				fwrite(log_file_buf,1,strlen(log_file_buf),log_file_fp);
				fflush(log_file_fp);
	
				/**********************************Log file *******************************************/
				
				/*****************************
				//Fork send receive threads
				*****************************/
				pthread_t send_t,recv_t;
				
				struct passing_info temp_p_info;
				struct passing_info *conn_info=(struct passing_info *)calloc(1,sizeof(temp_p_info));
				conn_info->csock=csock;
				conn_info->hname=ho;
				conn_info->port=p;
				
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
				
				close(csock);
				pthread_exit(NULL);
				
			}
			break;
			
	}
}

void *beacon_server(void *arg)
{
	listening_port=myport;
	int servsock;
	
	socklen_t clientlen;
	struct sockaddr_in client_addr_info, serv_addr_info;

	if((servsock = socket (AF_INET, SOCK_STREAM, 0))==-1)
	{
		perror("\nsocket() failed in beacon_server :: ");
		exit(1);
	}
	
	cout<<"\nsocket creation successfull in beacon_server . . .";
	
	int opval1=1;
	int sockopt_chk;
	if((sockopt_chk=setsockopt(servsock,SOL_SOCKET,SO_REUSEADDR,&opval1,(socklen_t)sizeof(opval1)))<0)
	{
		perror("\nbeacon_server setsockopt() failed . . .\n");
		exit(1);
	}	

	memset(&serv_addr_info,0, sizeof(serv_addr_info));
	serv_addr_info.sin_family = AF_INET;
	serv_addr_info.sin_addr.s_addr = htonl (INADDR_ANY);
	serv_addr_info.sin_port = htons (listening_port);
	
	if(bind(servsock, (sockaddr *) &serv_addr_info, sizeof(serv_addr_info))==-1)
	{
		perror("\nbind() failed in beacon_server :: ");
		exit(1);
	}
	
	cout<<"\nsocket bind() successfull in beacon_server . . .";
	
	if(listen(servsock, MAXCONN)==-1)
	{
		perror("\nlisten() failed in beacon_server :: ");
		exit(1);
	}
	
	cout<<"\nsocket listen() successfull in beacon_server . . .";
	
	while(1)
	{
		int clientsock;
		clientlen = sizeof(client_addr_info);
		cout<<"\nAbout to accept . . .";
		if((clientsock = accept(servsock, (sockaddr *) &client_addr_info, &clientlen))==-1)
		{
			perror("\nAccept failed in beacon server");
			exit(1);
		}
		cout<<"\nACCEPTED ONE CONNECTION";
		//Creating child thread for incoming connection request
		pthread_t child_thread;
		cout<<"\nclientsock :: "<<clientsock;
		//	sleep(1);
	//	int temp_csock=clientsock;
		int *client_sock=(int *) calloc (1,4);
		client_sock=&clientsock;
		pthread_create(&child_thread,NULL,beacon_listen_child,(void *)client_sock);
	}	
	//close(servsock);       
}
