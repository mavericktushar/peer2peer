#include <iostream>
#include <stdio.h>
#include "beacon.h"
#include "message.h"
using namespace std;

void *beacon_cmdline(void *arg)
{
//	struct node_inst *n_ist=(struct node_inst *)arg;

//	signal(SIGTERM,catch_shutdown);
	
	string cmd;
	cout<<"\n";
	while(1)
	{
		cout<<"servant:"<<myport<<"> ";
		getline(cin,cmd);
		
		
    	char *cmd_c = new char [cmd.size()];
		strncpy (cmd_c, cmd.c_str(),cmd.size());
		
		char *tok=NULL;
		tok=strtok(cmd_c," ");
		while(tok!=NULL)
		{
		cout<<"\nuyvguftiftiyftydfty";
			if(!strcmp(tok,"status"))
			{
			
				tok=strtok(NULL," ");
				if(tok!=NULL)
				{
				if(!strcmp(tok,"neighbors"))
				{
					tok=strtok(NULL," ");
					if(tok!=NULL)
					{
						int ttl=atoi(tok);
						tok=strtok(NULL," ");
						if(tok!=NULL)
						{
							char *ext_file=(char *)calloc(strlen(tok),1);
							strncpy(ext_file,tok,strlen(tok));
							tok=strtok(NULL," ");
							/***************************Creating status neighbor message**************************/
							
							//startup nam file
							pthread_mutex_lock(&nam_node_mutex);
							if((nam_file_fp=fopen(ext_file,"w+"))==NULL)
							{
								cout<<"\n\n\n\nfopen() failed for status . . .\n\n\n\n";
							}
							else
							{
								cout<<"\n\n\n\n##########fopen() success for status . . .#########\n\n\n\n";
							}
							nam_node_list=new List();
							pthread_mutex_unlock(&nam_node_mutex);
							
							char *temp_buf=new char[100];
							sprintf(temp_buf,"V -t * -v 1.0a5\n");
							fwrite(temp_buf,1,strlen(temp_buf),nam_file_fp);
							fflush(nam_file_fp);
							
							char *uoid_buf=new char[20];
							GetUOID(node_inst_id,(char *)"msg",uoid_buf,UOIDLEN);
							
							int datalen=1;
							
							cout<<"\nDatalength sent in beacon_client() :: "<<datalen;
							cout<<"\nttl :: "<<(unsigned char)ttl;
							char *commhdr=GetCommonHeader(0xac,uoid_buf,(unsigned char)ttl,datalen);
							unsigned char y=0x01;
							char *hdr_buf=new char[(COMMON_HEADER_SIZE+1)];
							memcpy(hdr_buf,commhdr,COMMON_HEADER_SIZE);
							memcpy(&hdr_buf[27],&y,1);
							
							struct list_element_struct *element=(struct list_element_struct *)calloc(1,sizeof(struct list_element_struct));
							element->msg=hdr_buf;
							element->msg_sz=(COMMON_HEADER_SIZE+1);
							element->f_s=0;  //send
							
							
							//Sending status message to all neighbors
							pthread_mutex_lock(&thread_port_mutex);
							for(int i=0;i<acc_conn_cnt;i++)
							{
								if(thread_port[i].valid)
								{
									(thread_port[i].msg_Q)->Append((void *)element);
								}
							}
							pthread_mutex_unlock(&thread_port_mutex);
							pthread_cond_broadcast(&send_thread_cv);
							
							
							//Appending itself to the nam_node_list
							//Make entry in nam file
							pthread_mutex_lock(&nam_node_mutex);
							nam_node_list->SortedInsert((void *)&myport,(int)myport);
							
							memset(temp_buf,'\0',100);
							sprintf(temp_buf,"n -t * -s %hd -c red -i black\n",myport);
							fwrite(temp_buf,1,strlen(temp_buf),nam_file_fp);
							fflush(nam_file_fp);
							pthread_mutex_unlock(&nam_node_mutex);
							
							/***********************************status neighbor***********************************/
							
						}
						else
						{
							cout<<"\nInvalid command11";
							tok=NULL;
						}
					}
					else
					{
						cout<<"\nInvalid command111";
						tok=NULL;
					}
				}
				else
				{
					cout<<"\nInvalid command1";
					tok=NULL;
				}
				}
			}
			else if(!strcmp(tok,"shutdown"))
			{
				//shutdown
				kill(0,SIGTERM);
				tok=NULL;
			}
			else
			{
				cout<<"\nInvalid command2";
				tok=NULL;
			}
		}

	}
}

