
#include <math.h>
//#include <pthread.h>

#include "beacon.h"
#include "message.h"

int index_1;

pthread_mutex_t index_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t index_cv=PTHREAD_COND_INITIALIZER;

void *send_thread(void *arg)
{
	index_1=-1;
	struct passing_info *conn_info=((struct passing_info *) arg);
	int csock=conn_info->csock;
	cout<<"\nEntering send_thread";
	
	pthread_mutex_lock(&thread_port_mutex);
	for(int i=0;i<acc_conn_cnt;i++)
	{
		if(thread_port[i].valid==1)
		{
			if(/*!strcmp(thread_port[i].hname,conn_info->hname) &&*/ thread_port[i].port==conn_info->port)
			{
				thread_port[i].send_thread_id=pthread_self();
				index_1=i;
				break;
			}
		}
	}			
	
	if(index_1==-1)
	{
		cout<<"\nNo entry in thread_port found for send thread. Error . . .";
	}
	else
	{
		cout<<"\nsend thread index found in thread port :: "<<index_1;
	}
	pthread_mutex_unlock(&thread_port_mutex);
	pthread_cond_signal(&index_cv);
	
	pthread_mutex_lock(&thread_port_mutex);
	cout<<"\nthread_port["<<index_1<<"].conn_end_flag="<<thread_port[index_1].conn_end_flag;
	while(thread_port[index_1].conn_end_flag==0)
	{			
		while((thread_port[index_1].msg_Q)->IsEmpty())
		{
			pthread_cond_wait(&send_thread_cv,&thread_port_mutex);				//on timeout in select of recv_thread send_thread_cv broadcasted by recv_thread
			
			cout<<"\nComing out of wait in send_thread . . .";
			
			if(thread_port[index_1].conn_end_flag==1)
			{
				thread_port[index_1].valid=0;
				delete thread_port[index_1].msg_Q;
				
				pthread_mutex_unlock(&thread_port_mutex);
				cout<<"\nsend_thread exiting. conn_end_flag set . . .";
				pthread_exit(NULL);
			}
			//temp - for exiting set a flag
		}
		
		struct list_element_struct *elem=(struct list_element_struct *)(thread_port[index_1].msg_Q)->Remove();
		
		pthread_mutex_unlock(&thread_port_mutex);
		
		cout<<"\nsend thread :: ";
		cout<<"\nmsg :: "<<elem->msg;
		cout<<"\nmsg sz :: "<<elem->msg_sz;
		
		if(send(csock,elem->msg,elem->msg_sz,0)!=elem->msg_sz)
		{
			perror("\nsend() while trying to send hello message in send_thread failed :: ");
			
			pthread_mutex_lock(&thread_port_mutex);
			thread_port[index_1].valid=0;
			delete thread_port[index_1].msg_Q;
			
			pthread_mutex_unlock(&thread_port_mutex);
			cout<<"\nsend_thread exiting. send() failed . . .";
			pthread_exit(NULL);
		}
		
		

		char *temp_uoid=new char[UOIDLEN];
		
		memcpy(temp_uoid,&elem->msg[1],UOIDLEN);
		
		pthread_mutex_lock(&btree_mutex);
		unsigned int temp=RSHash(temp_uoid,UOIDLEN);
		if(btree.get(temp)==NULL)
		{
			pthread_mutex_unlock(&btree_mutex);
			struct node_message_tree n_msg_tr;
			struct node_message_tree *node=(struct node_message_tree *)calloc(1,sizeof(n_msg_tr));
			
			//char hname[100];
			//gethostname(hname,100);
			
			node->uoid=temp_uoid;
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

							
//			msg_timer[loc_index].value=inist->msglifetime;
//			msg_timer[loc_index].valid=1;
			pthread_mutex_unlock(&msg_timer_mutex);
				
			node->timerindex=loc_index;       //to keep track of the index into the timer table..
*/			
			pthread_mutex_lock(&btree_mutex);
			btree.insert((const unsigned int)temp,node);
			pthread_mutex_unlock(&btree_mutex);
		}
		else
		{
			pthread_mutex_unlock(&btree_mutex);
		}
		pthread_mutex_lock(&thread_port_mutex);
	}	
	
	cout<<"\nsend_thread exiting . . .";
	cout<<"\nsend_thread exiting . . .";
	
	//conn_end_flag=1
	//cleanup
	thread_port[index_1].valid=0;
	delete thread_port[index_1].msg_Q;
	
	pthread_mutex_unlock(&thread_port_mutex);
	pthread_exit(NULL);
}

void *recv_thread(void *arg)
{		
//	index_1=-1;						//Error
	struct passing_info *conn_info=((struct passing_info *) arg);
	int csock=conn_info->csock;
	fd_set loc_set;
	int sel_ret;
	char *hdr_buf=(char *)calloc(COMMON_HEADER_SIZE,1);
	int index1=0;
	int recvmsglen;
	struct timeval keep_alive_timeout;
	FD_ZERO(&loc_set);
	FD_SET(csock,&loc_set);
	cout<<"\nEntering recv() thread()";
	keep_alive_timeout.tv_sec=inist->keepalivetimeout;
	keep_alive_timeout.tv_usec=0;
	
	pthread_mutex_lock(&index_mutex);
	
	while(index_1==-1)	
	pthread_cond_wait(&index_cv,&index_mutex);
	
	pthread_mutex_unlock(&index_mutex);
	
	while(1)
	{
	cout<<"\nSelect timeout value::  "<<keep_alive_timeout.tv_sec;
		while(index1<COMMON_HEADER_SIZE)
		{
			if((sel_ret=select(csock+1,&loc_set,NULL,NULL,&keep_alive_timeout))>0)
			{
				if((recvmsglen=recv(csock,&hdr_buf[index1],1,0))!=1)
				{
					perror("\nRecv thread:: recv() on receiving header from connecting node failed :: ");
					pthread_mutex_lock(&thread_port_mutex);
					thread_port[index_1].conn_end_flag=1;
					pthread_mutex_unlock(&thread_port_mutex);
					pthread_cond_broadcast(&send_thread_cv);
					cout<<"\nrecv_thread exiting.recv() loop failed . . .";
					pthread_exit(NULL);
				}
				index1++;
			}
			else   //after timeout
			{
				pthread_mutex_lock(&thread_port_mutex);
				thread_port[index_1].conn_end_flag=1;
				pthread_mutex_unlock(&thread_port_mutex);
				pthread_cond_broadcast(&send_thread_cv);
				cout<<"\nrecv_thread exiting.Timeout in recv() loop . . .";
				pthread_exit(NULL);
			}
		}
		index1=0;			
		cout<<"\nrecv thread :: received Message header . . .";
					
		char *recv_uoid=(char*)calloc(UOIDLEN,1);
		strncpy(recv_uoid,&hdr_buf[1],UOIDLEN);			//not sure
		
		unsigned char ttl_ret_char;
		memset(&ttl_ret_char,0,1);
		memcpy(&ttl_ret_char,&hdr_buf[21],1);
		unsigned short recv_ttl=(unsigned short)ttl_ret_char;
					
		int datalen;
		memset(&datalen,0,sizeof(int));
		memcpy(&datalen,&hdr_buf[23],4);
		
		unsigned short int ttl=recv_ttl;
		//-----------------------------------------------------------------------
					
		char *data=(char *)calloc(datalen,1);
		if((sel_ret=select(csock+1,&loc_set,NULL,NULL,&keep_alive_timeout))>0)
		{
			if((recvmsglen=recv(csock,data,datalen,0))!=datalen)
			{
				perror("\nRecv thread:: recv() on receiving DATA from connecting node failed :: ");
				pthread_mutex_lock(&thread_port_mutex);
				thread_port[index_1].conn_end_flag=1;
				pthread_mutex_unlock(&thread_port_mutex);
				pthread_cond_broadcast(&send_thread_cv);
				cout<<"\nrecv_thread exiting.recv() failed() . . .";
				pthread_exit(NULL);
			}
		}
		else
		{
			pthread_mutex_lock(&thread_port_mutex);
			thread_port[index_1].conn_end_flag=1;
			pthread_mutex_unlock(&thread_port_mutex);
			pthread_cond_broadcast(&send_thread_cv);
			cout<<"\nrecv_thread exiting.Timeout . . .";
			pthread_exit(NULL);
		}
	
		
		cout<<"\nrecv thread :: received Message data . . .";
		
		pthread_mutex_lock(&btree_mutex);
		unsigned int temp=RSHash(recv_uoid,UOIDLEN);
		if(btree.get(temp)==NULL)
		{
			
			struct node_message_tree n_msg_tre;
			struct node_message_tree *node=(struct node_message_tree *)calloc(1,sizeof(n_msg_tre));
			
			node->uoid=recv_uoid;
			node->hname=conn_info->hname;
			node->port=conn_info->port;
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
				
			node->timerindex=loc_index;       //to keep track of the index into the timer table..
	*/		
			btree.insert((const unsigned int)temp,node);
			pthread_mutex_unlock(&btree_mutex);
			
			switch(hdr_buf[0])
			{
				case 0xfc:  
						{
							//join request msg
							
							/**********************************Log file *******************************************/
							
							unsigned short p=0;
							char *ho=(char *)calloc((datalen-6)+1,1);
							unsigned int loctn=0;
							memset(ho,0,(datalen-6)+1);
							memcpy(&loctn,&data[0],4);
							memcpy(&p,&data[4],2);
							strncpy(ho,&data[6],datalen-6);
							
							p=ntohs(p);
							loctn=ntohl(loctn);
				
							int n_id=get_node_id_from_port(p);
							
							struct timeval t_vl;
							gettimeofday(&t_vl,NULL);
							
							char *log_file_buf=new char[100];
							sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %hd %s\n",'r',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id,"JNRQ",COMMON_HEADER_SIZE+datalen,recv_ttl,recv_uoid[16],recv_uoid[17],recv_uoid[18],recv_uoid[19],p,ho);
							
							fwrite(log_file_buf,1,strlen(log_file_buf),log_file_fp);
							fflush(log_file_fp);
				
							/**********************************Log file *******************************************/
				
							
							//Create a Join response to be inserted in the connection s message queue
							cout<<"\nrecv thread :: received Join request . . .";
							char *uoid_buf=new char[20];
							GetUOID(node_inst_id,(char *)"msg",uoid_buf,UOIDLEN);
							
							unsigned int req_loc;
							memcpy(&req_loc,&data[0],4);
							
							//char hname[100];
							//gethostname(hname,100);
							
							int hnamelen=strlen(hostname);
							int datalen=hnamelen+UOIDLEN+sizeof(unsigned short int)+sizeof(int);
							cout<<"\nDatalength sent in beacon_client() :: "<<datalen;
							char *commhdr=GetCommonHeader(0xfb,uoid_buf,(unsigned char)1,datalen);
				
							cout<<"\nrecv thread :: Creating Join Response . . .";
							int join_res_msg_buf_sz=(COMMON_HEADER_SIZE+datalen);
							char *join_res_msg_buf=new char[join_res_msg_buf_sz];
							int join_port=myport;
							unsigned int dist_node=abs((int)(req_loc - inist->location));
							GetJoinResponseMsg(join_res_msg_buf,commhdr,recv_uoid,dist_node,join_port,hostname,join_res_msg_buf_sz);
							
							cout<<"\nrecv thread :: ";
							cout<<"\njoin_res_msg_buf :: "<<join_res_msg_buf;
							cout<<"\njoin_res_msg_buf_sz :: "<<join_res_msg_buf_sz;
							
							struct list_element_struct li_el_s;
							struct list_element_struct *element=(struct list_element_struct*)calloc(1,sizeof(li_el_s));
							element->msg=join_res_msg_buf;
							element->msg_sz=join_res_msg_buf_sz;
							element->f_s=0;
							
							pthread_mutex_lock(&thread_port_mutex);
							(thread_port[index_1].msg_Q)->Append((void *)element);
							pthread_mutex_unlock(&thread_port_mutex);
							
							pthread_cond_broadcast(&send_thread_cv);
							
							//Forwarding Join Requests
							if(recv_ttl>1)
							{
								cout<<"\nrecv thread :: Forwarding join requests . . .";
								unsigned char send_ttl;
								if((recv_ttl-1)>inist->ttl)
									send_ttl=(unsigned char)inist->ttl;
								else
									send_ttl= (unsigned char)(recv_ttl-1);
									
								memcpy(&hdr_buf[21],&send_ttl,sizeof(unsigned char));
									
								hdr_buf=(char*)realloc(hdr_buf,COMMON_HEADER_SIZE+datalen);
								memcpy(&hdr_buf[COMMON_HEADER_SIZE],data,datalen);
							//	free(data);
								struct list_element_struct l_e_s;
								struct list_element_struct *element=(struct list_element_struct*)calloc(1,sizeof(l_e_s));
								element->msg=hdr_buf;
								element->msg_sz=COMMON_HEADER_SIZE+datalen;
								element->f_s=1;
								
								pthread_mutex_lock(&thread_port_mutex);
								for(int i=0;i<acc_conn_cnt;i++)
								{
									if(thread_port[i].send_thread_id != thread_port[index_1].send_thread_id && thread_port[i].send_thread_id!=0)
									{
										cout<<"\nrecv thread :: Forwarding join request on connection with thread id "<<thread_port[i].send_thread_id<<" . . .";
										(thread_port[i].msg_Q)->Append((void*)element);
									}
								}
								pthread_mutex_unlock(&thread_port_mutex);
								
								pthread_cond_broadcast(&send_thread_cv);
							}
							else
							{
								cout<<"\nrecv thread :: received message ttl equal to 1. Dropping message . . .";
							}
						}
						break;
				case 0xfb:	
						{
							//Join response
							//extract uoid stored as data
							//retreive node from binary tree according to extracted uoid
							//create a send message (changes in message - ttl??) - determine on which connection s msg Queue to put the  message - extract index by hostname and port from list_thread_id array
							//hostname and port obtained from node of tree (uoid as data - lookup)
							//Broadcast sending threads
	/*******************************						
				NB1    ->      B1     ->   B2     ->   NB2	 				NB1 ->	B1 -> NB2 -> NB3 
				send(tree)   recv(tree)  recv(tree)   recv(tree)
				NB1				NB1		    B1		    B2
	*******************************/
							char *uoid_stored=new char[UOIDLEN];
							memcpy(uoid_stored,&data[0],UOIDLEN);
							
							/**********************************Log file *******************************************/
			
							unsigned short int loc_p=0;
							char *loc_ho=(char *)calloc((datalen-26)+1,1);
							unsigned int loc_dist=0;
							memset(loc_ho,0,(datalen-26)+1);
							//memcpy(loc_ho,&data[26],(datalen-26));
							strncpy(loc_ho,&data[26],(datalen-26));
							memcpy(&loc_p,&data[24],2);
							memcpy(&loc_dist,&data[20],4);
							
							int n_id=get_node_id_from_port(loc_p);
							
							struct timeval t_vl;
							gettimeofday(&t_vl,NULL);
							
							char *log_file_buf=new char[100];
							sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %02x %02x %02x %02x %d %hd %s\n",'r',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id,"JNRS",COMMON_HEADER_SIZE+datalen,recv_ttl,recv_uoid[16],recv_uoid[17],recv_uoid[18],recv_uoid[19],uoid_stored[16],uoid_stored[17],uoid_stored[18],uoid_stored[19],loc_dist,loc_p,loc_ho);
							
							fwrite(log_file_buf,1,strlen(log_file_buf),log_file_fp);
							fflush(log_file_fp);
				
							/**********************************Log file *******************************************/
							
							unsigned int temp_key=RSHash(uoid_stored,UOIDLEN);
							pthread_mutex_lock(&btree_mutex);
							struct node_message_tree *node_ex=btree.get(temp_key); 
							pthread_mutex_unlock(&btree_mutex);
							
							if(node_ex==NULL)
							{
								cout<<"\nno match found in btree (stored uoid in join response)";
								break;
							}
							
							unsigned char send_ttl;
							send_ttl= (unsigned char)1;				//ttl 1 for response message
									
							memcpy(&hdr_buf[21],&send_ttl,sizeof(unsigned char));
									
							hdr_buf=(char*)realloc(hdr_buf,COMMON_HEADER_SIZE+datalen);
							memcpy(&hdr_buf[COMMON_HEADER_SIZE],data,datalen);
							//free(data);
							struct list_element_struct l_el_s;
							struct list_element_struct *element=(struct list_element_struct*)calloc(1,sizeof(l_el_s));
							element->msg=hdr_buf;
							element->msg_sz=COMMON_HEADER_SIZE+datalen;
							element->f_s=1;
							
							//char *hname=new char[100];
							//gethostname(hname,100);
							
							if(/*!strcmp(node_ex->hname,hostname) && */ node_ex->port==myport) //not sure if strcmp works
							{
								//Originating node - Doubt????? Originating node will receive all responses in startup. This may be helpful in restart case???? 
								cout<<"\nShould have received these responses in nonbeacon_startup or if beacon, cannot be the originator. . .";
							}
							else
							{
							//	NB -> B 
								//Check in temporary connection Queue
								int loc_flag=0;
								pthread_mutex_lock(&thread_port_temp_mutex);
								for(int i=0;i<acc_conn_temp;i++)
								{
									if(thread_port_temp[i].valid==1)
									{
										if(/*!strcmp(thread_port_temp[i].hname,node_ex->hname) && */ thread_port_temp[i].port==node_ex->port)
										{
											cout<<"\nAppending Join response to temporary connection message queue . . .";
											(thread_port_temp[i].msg_Q)->Append((void *)element);
											loc_flag=1;
											pthread_cond_broadcast(&send_temp_cv);
											break;
										}
									}
								}
								pthread_mutex_unlock(&thread_port_temp_mutex);
								
								//if not found
								
								//check in permanent connection Queue
								if(loc_flag==0)
								{
									pthread_mutex_lock(&thread_port_mutex);
									for(int i=0;i<acc_conn_cnt;i++)
									{
										//pthread_mutex_lock(&list_thread_id[i].list_mutex);
										if(/*!strcmp(thread_port[i].hname,node_ex->hname) && */thread_port[i].port==node_ex->port)
										{
											cout<<"\nAppending Join response to permanent connection message queue . . .";
											cout<<"\nInserting in Msg Queue of thread with id :: "<<thread_port[i].send_thread_id;
											(thread_port[i].msg_Q)->Append((void *)element);
										}
										//pthread_mutex_unlock(&list_thread_id[i].list_mutex);
									}
									pthread_mutex_unlock(&thread_port_mutex);
									
									pthread_cond_broadcast(&send_thread_cv);
								}
							}
						}
						break; 
				case 0xac:
						{
						//creatig status response and sending to particular node
						
							char *uoid_buf=new char[20];
							GetUOID(node_inst_id,(char *)"msg",uoid_buf,UOIDLEN);
							
							unsigned short int host_info_len=sizeof(myport)+strlen(hostname);
							int len_old=(COMMON_HEADER_SIZE+UOIDLEN+sizeof(host_info_len)+sizeof(myport)+strlen(hostname));
							int len_new=0;
							
							unsigned short int host_info_len1=htons(host_info_len);
							unsigned short int myport1=htons(myport);
							cout<<"\nhost_info_len1 :: "<<host_info_len1;
							cout<<"\nmyport1 :: "<<myport1;
							len_new=len_old;
							
							
							
							char* status_response=(char*)calloc(len_old,1);
							//memcpy(&status_response[0],commhdr,COMMON_HEADER_SIZE);
							memcpy(&status_response[COMMON_HEADER_SIZE],recv_uoid,UOIDLEN);
							memcpy(&status_response[COMMON_HEADER_SIZE+UOIDLEN],&host_info_len1,sizeof(host_info_len1));
							memcpy(&status_response[COMMON_HEADER_SIZE+UOIDLEN+sizeof(host_info_len)],&myport1,sizeof(myport));
							memcpy(&status_response[COMMON_HEADER_SIZE+UOIDLEN+sizeof(host_info_len)+sizeof(myport)],hostname,strlen(hostname));
														
							pthread_mutex_lock(&thread_port_mutex);
							for(int i=0;i<acc_conn_cnt;i++)
							{
								if(thread_port[i].valid==1)
								{
									int loc_len=strlen(thread_port[i].hname)+sizeof(thread_port[i].port);
									len_new=len_new+loc_len+sizeof(loc_len);
																		
									status_response=(char*)realloc(status_response,len_new*sizeof(char));
									
									
									int loc_len1=htonl(loc_len);
									unsigned short int t_port=htons(thread_port[i].port);
									
									memcpy(&status_response[len_old],&loc_len1,sizeof(loc_len));
									memcpy(&status_response[len_old+4],&t_port,sizeof(thread_port[i].port));
									memcpy(&status_response[len_old+6],thread_port[i].hname,strlen(thread_port[i].hname));
									
									len_old=len_new;
								}
							}
							
							
							pthread_mutex_unlock(&thread_port_mutex);
							
							char* commhdr=GetCommonHeader(0xab,uoid_buf,(unsigned char)inist->ttl,(len_new-COMMON_HEADER_SIZE));
							memcpy(&status_response[0],commhdr,COMMON_HEADER_SIZE);
							
							struct list_element_struct* elem=(struct list_element_struct*)calloc(1,sizeof(struct list_element_struct));
							elem->msg=status_response;
							elem->msg_sz=len_new;
							elem->f_s=0;
							
							pthread_mutex_lock(&thread_port_mutex);
							thread_port[index_1].msg_Q->Append((void*)elem);
							pthread_mutex_unlock(&thread_port_mutex);
							pthread_cond_broadcast(&send_thread_cv);
						   //forwarding status req message
						   
						   char* send_buf=(char*)calloc((COMMON_HEADER_SIZE+datalen),sizeof(char));
						   unsigned char temp_ttl;
						   
						   if(ttl>1)
						   {
								if((ttl-1)>inist->ttl)
								temp_ttl=(unsigned char)inist->ttl;
								else
								temp_ttl=(unsigned char)(ttl-1);
							
								memcpy(&hdr_buf[21],&temp_ttl,1);
								
								memcpy(send_buf,hdr_buf,COMMON_HEADER_SIZE);
								memcpy(&send_buf[COMMON_HEADER_SIZE],data,datalen);
								
								struct list_element_struct* elem=(struct list_element_struct*)calloc(1,sizeof(struct list_element_struct));
								elem->msg=send_buf;
								elem->msg_sz=COMMON_HEADER_SIZE+datalen;
								elem->f_s=1;
								
								pthread_mutex_lock(&thread_port_mutex);
								for(int i=0;i<acc_conn_cnt;i++)
								{
									if(thread_port[i].valid==1 && thread_port[i].port!=thread_port[index_1].port)
									{
										thread_port[index_1].msg_Q->Append((void*)elem);
									}
								}
								pthread_mutex_unlock(&thread_port_mutex);
								pthread_cond_broadcast(&send_thread_cv);
							}
							else
							break;
							
							
						}
						break;
				case 0xab:
				             //status response
						{
							
							char *uoid_stored=new char[UOIDLEN];
							memcpy(uoid_stored,&data[0],UOIDLEN);
							
							/**********************************Log file *******************************************/
			/*
							unsigned short int loc_p=0;
							char *loc_ho=(char *)calloc((datalen-26)+1,1);
							unsigned int loc_dist=0;
							memset(loc_ho,0,(datalen-26)+1);
							//memcpy(loc_ho,&data[26],(datalen-26));
							strncpy(loc_ho,&data[26],(datalen-26));
							memcpy(&loc_p,&data[24],2);
							memcpy(&loc_dist,&data[20],4);
							
							int n_id=get_node_id_from_port(loc_p);
							
							struct timeval t_vl;
							gettimeofday(&t_vl,NULL);
							
							char *log_file_buf=new char[100];
							sprintf(log_file_buf,"%c %10ld.%03d %d %s %d %hd %02x %02x %02x %02x %02x %02x %02x %02x %d %hd %s\n",'r',t_vl.tv_sec,(t_vl.tv_usec/1000),n_id,"JNRS",COMMON_HEADER_SIZE+datalen,recv_ttl,recv_uoid[16],recv_uoid[17],recv_uoid[18],recv_uoid[19],uoid_stored[16],uoid_stored[17],uoid_stored[18],uoid_stored[19],loc_dist,loc_p,loc_ho);
							
							fwrite(log_file_buf,1,strlen(log_file_buf),log_file_fp);
							fflush(log_file_fp);
				*/
							/**********************************Log file *******************************************/
							
							unsigned int temp_key=RSHash(uoid_stored,UOIDLEN);
							pthread_mutex_lock(&btree_mutex);
							struct node_message_tree *node_ex=btree.get(temp_key); 
							pthread_mutex_unlock(&btree_mutex);
							
							if(node_ex==NULL)
							{
								cout<<"\nno match found in btree (stored uoid in  status response)";
								break;
							}
							
							if(node_ex->port==myport)
							{
								unsigned short host_port=0;
								char* host_hname=NULL;
								unsigned short host_info_len=0;
								int datafield=0;
								
								memcpy(&host_info_len,&data[20],2);
								memcpy(&host_port,&data[22],2);
								host_hname=(char*)calloc((host_info_len-2)+1,sizeof(char));
								memcpy(host_hname,&data[24],host_info_len-2);
								
								cout<<"\nhost_info_len :: "<<host_info_len;
								cout<<"\nhost_port :: "<<host_port;
								
								host_info_len=ntohs(host_info_len);
								host_port=ntohs(host_port);
								
								cout<<"\nhost_info_len :: "<<host_info_len;
								cout<<"\nhost_port :: "<<host_port;
								
								cout<<"\nWriting link to nam file . . . ";
								pthread_mutex_lock(&nam_node_mutex);
									char* t_b=(char*) calloc(100,1);
										memset(t_b,'\0',100);
										sprintf(t_b,"l -t * -s %hd -d %hd -c blue\n",host_port,myport);
										fwrite(t_b,1,strlen(t_b),nam_file_fp);
										fflush(nam_file_fp);
										pthread_mutex_unlock(&nam_node_mutex);	
								List *temp_list=new List();
								int fl1=0;
								unsigned short int* l1;
								pthread_mutex_lock(&nam_node_mutex);
									while((l1=(unsigned short int*)nam_node_list->Remove())!=NULL)
									{		
										cout<<"\nRemove from list :: "<<*l1;
										if(*l1==host_port)
										{
											cout<<"\n*l = "<<*l1<<"=="<<"host_port = "<<host_port;
											fl1=1;
										}
										else
										temp_list->SortedInsert((void*)l1,(int)*l1);
									}	
									nam_node_list=temp_list;
									temp_list=NULL;
									if(fl1==0)
									{
										
										nam_node_list->SortedInsert((void *)&host_port,(int)host_port);
										char* temp_buf=(char*) calloc(100,1);
										memset(temp_buf,'\0',100);
										cout<<"\nWriting node to nam file . . . ";
										sprintf(temp_buf,"n -t * -s %hd -c red -i black\n",host_port);
										fwrite(temp_buf,1,strlen(temp_buf),nam_file_fp);
										fflush(nam_file_fp);
									}
									pthread_mutex_unlock(&nam_node_mutex);			
								int temp_len=0;
								temp_len=21+host_info_len+1;
								
								
								while(temp_len!=datalen)
								{
									temp_list=new List();
									unsigned short po=0;
									int length=0;
									
									memcpy(&length,&data[temp_len],sizeof(int));
									memcpy(&po,&data[temp_len+4],sizeof(unsigned short int));
									
									length=ntohl(length);
									po=ntohs(po);
									
									cout<<"\nWriting link to nam file . . . ";
									pthread_mutex_lock(&nam_node_mutex);
									char* t_buf=(char*) calloc(100,1);
										memset(t_buf,'\0',100);
										sprintf(t_buf,"l -t * -s %hd -d %hd -c blue\n",po,host_port);
										fwrite(t_buf,1,strlen(t_buf),nam_file_fp);
										fflush(nam_file_fp);
										pthread_mutex_unlock(&nam_node_mutex);	
										
									unsigned short int* l;
									int fl=0;
									pthread_mutex_lock(&nam_node_mutex);
									while((l=(unsigned short int*)nam_node_list->Remove())!=NULL)
									{		
										cout<<"\nRemove from list :: "<<*l;
										if(*l==po)
										{
											cout<<"\n*l = "<<*l<<"=="<<"po = "<<po;
											fl=1;
										}
										else
										temp_list->SortedInsert((void*)l,(int)*l);
									}	
									nam_node_list=temp_list;
									temp_list=NULL;
									if(fl==0)
									{
										
										nam_node_list->SortedInsert((void *)&po,(int)po);
										char* temp_buf=(char*) calloc(100,1);
										memset(temp_buf,'\0',100);
										cout<<"\nWriting node to nam file . . . ";
										sprintf(temp_buf,"n -t * -s %hd -c red -i black\n",po);
										fwrite(temp_buf,1,strlen(temp_buf),nam_file_fp);
										fflush(nam_file_fp);
									}
									pthread_mutex_unlock(&nam_node_mutex);									
										
									temp_len=length+temp_len+sizeof(length);
									cout<<"\ntemp_len ::"<<temp_len<<" datalen+ common header ::"<<(datalen+COMMON_HEADER_SIZE);
								}
								
							}
							else
							{
								
								if(ttl>1)
								{
									unsigned char temp_ttl;
									if((ttl-1)>inist->ttl)
									temp_ttl=(unsigned char)inist->ttl;
									else
									temp_ttl=(unsigned char)(ttl-1);
								
									memcpy(&hdr_buf[21],&temp_ttl,1);
									char* send_buf=(char*)calloc((COMMON_HEADER_SIZE+datalen),1);
									
									memcpy(send_buf,hdr_buf,COMMON_HEADER_SIZE);
									memcpy(&send_buf[COMMON_HEADER_SIZE],data,datalen);
									
									struct list_element_struct* elem=(struct list_element_struct*)calloc(1,sizeof(struct list_element_struct));
									elem->msg=send_buf;
									elem->msg_sz=COMMON_HEADER_SIZE+datalen;
									elem->f_s=1;
									
									pthread_mutex_lock(&thread_port_mutex);
									for(int i=0;i<acc_conn_cnt;i++)
									{
										if(thread_port[i].valid==1 && thread_port[i].port==node_ex->port)
										{
											thread_port[i].msg_Q->Append((void*)elem);
										}
									}
									pthread_mutex_unlock(&thread_port_mutex);
									pthread_cond_broadcast(&send_thread_cv);
								}
								else
								break;
							}
						}	
			}			
		}
		else
		{
			pthread_mutex_unlock(&btree_mutex);
		}
	}
	
	cout<<"\nrecv_thread exiting . . .";
	cout<<"\nrecv_thread exiting . . .";
	
}


