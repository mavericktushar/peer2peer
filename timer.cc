#include "beacon.h"
#include "message.h"

int count_1;
bsTreeNode <unsigned int,struct node_message_tree> **remove_node;

void traversal(bsTreeNode<unsigned int,struct node_message_tree> *node) 
{
    if (node != NULL)
	{
        traversal(node->less);  // print left subtree
		
        if(node->data->msg_time > 1)
		{
			node->data->msg_time--;
		}
		else if(node->data->msg_time==1)     //dont remove this condition
		{
			count_1++;
			node->data->msg_time--;
			//free(node->data->uoid);
			remove_node=(bsTreeNode <unsigned int,struct node_message_tree>**)realloc(remove_node,count_1*(sizeof(node/* bsTreeNode <unsigned int,struct node_message_tree>* */)));
			remove_node[count_1-1]=node;
			cout <<"\nremoving node key::"<< node->key << endl; // print this node
		}
        traversal(node->more); // print right subtree
    }
}

void *timer_thread(void *arg)
{
	//Msgtimeout
	//Keep alive
	//Autoshutdown
	remove_node=NULL;
	int keep_alive_send=0;
	struct timeval init_tval,fin_tval;
	
	float time_diff=0.0;
	
	while(1/*autoshutdown flag not set*/)
	{
		count_1=0;
		if(remove_node!=NULL)
		free(remove_node);
		
		if(time_diff<1.0)
		{
			usleep(((float)(1.0-time_diff))*1000000);
		}
		
		gettimeofday(&init_tval,NULL);
		
		//Msgtimeout
		//Traverse the binary tree
		//decrement msg time values in each entry...if after decrementing, value is 0; delete that entry
		pthread_mutex_lock(&btree_mutex);
		traversal(btree.root);
		while(count_1!=0)
		{
/*			pthread_mutex_lock(&thread_port_temp_mutex);
			for(int i=0;i<acc_conn_temp;i++)
			{
				if(thread_port_temp[i].valid==1)
				{
					if(
					thread_port_temp[i].conn_end_flag=1;
					pthread_cond_broadcast(&send_temp_cv);
				}
			}
			pthread_mutex_unlock(&thread_port_temp_mutex);
*/			
			unsigned int key11=RSHash(remove_node[count_1-1]->data->uoid,UOIDLEN);
			btree.destroy(key11);
			count_1--;
		}
		pthread_mutex_unlock(&btree_mutex);
		
		//For Temporary connections
		pthread_mutex_lock(&thread_port_temp_mutex);
		for(int i=0;i<acc_conn_temp;i++)
		{
			if(thread_port_temp[i].valid==1)
			{
				if(thread_port_temp[i].jointimeout>1)
				{
					thread_port_temp[i].jointimeout--;
				}
				else if(thread_port_temp[i].jointimeout==1)     //if after broadcasting send() thread, it has not sufficient time to make valid bit 0 and by that time again timer thread come into acton then this thread will again do timeout-- and value will become -1 which we r checking in send thread.
				{
					thread_port_temp[i].jointimeout--;
					thread_port_temp[i].conn_end_flag=1;
					pthread_cond_broadcast(&send_temp_cv);
				}
			}
		}
		pthread_mutex_unlock(&thread_port_temp_mutex);
		
		if(NB_temp_conn_jointimeout>1)
		{
			NB_temp_conn_jointimeout--;
		}
		else if(NB_temp_conn_jointimeout==1)
			{
				NB_temp_conn_jointimeout=-1;
				cout<<"\nSending signal SIGUSR1 to temporary connection to beacon . . .";
				pthread_kill(NB_temp_conn_id,SIGUSR1);
			}
		
		//keep alive
		//recv thread will reset the keep alive timer vaue after recving data.......timer thread will decrement keepalivetime value...if value=(keepalive/2), create keepalive msg, and
		//add that msg to the corresponding connection msgQ..(threadportmap->index) and broadcast. 
		//if keepalivetime value is 0->
			//disconnect connection
			//close socket
			//terminate send recv threads 
			//invalidate thread port map entries
			//memory free.
		//All of above Not used, select and signalling used
		
		keep_alive_send++;
		if(keep_alive_send==(inist->keepalivetimeout/2))			
		{
			keep_alive_send=0;
			//make a keep alive message 		
			//broadcast on all connections
			char *uoid_buf=new char[20];
			GetUOID(node_inst_id,(char *)"msg",uoid_buf,UOIDLEN);
			int datalen=0;
			
			char *commhdr=GetCommonHeader(0xf8,uoid_buf,(unsigned char)1,datalen);
			
			struct list_element_struct *element=(struct list_element_struct*)calloc(1,sizeof(struct list_element_struct));
			element->msg=commhdr;
			element->msg_sz=COMMON_HEADER_SIZE;
			
			pthread_mutex_lock(&thread_port_mutex);
			for(int i=0;i<acc_conn_cnt;i++)
			{
				if(thread_port[i].valid==1)
				{
					if(thread_port[i].msg_Q!=NULL)
					{
						cout<<"\nAppending keep Alive message . . .";
						(thread_port[i].msg_Q)->Append((void *)element);
					}
					else
					{
						cout<<"\nAppending to NULL list . . .";
					}
				}
			}
			pthread_mutex_unlock(&thread_port_mutex);
			pthread_cond_broadcast(&send_thread_cv);
		}
				
		//autoshutdown
		//derement the autoshutdown value..is zero, then set autoshutdown flag...
		
		gettimeofday(&fin_tval,NULL);
		
		time_diff=((float)fin_tval.tv_sec+((float)fin_tval.tv_usec/1000000.0)) - ((float)init_tval.tv_sec+((float)init_tval.tv_usec/1000000.0));
		
	}
	
	
}
