#include "message.h"

char * GetCommonHeader(unsigned char msgType,char *UOID,unsigned char TTL, int datalen)
{	
	//int temp_datalen;
	//temp_datalen=htonl(datalen);
	char *buf;
	buf=(char *)malloc(COMMON_HEADER_SIZE*1);		//Allocate memory for the header
	
	memset(buf,0,COMMON_HEADER_SIZE);							//clean the header
	memcpy(&buf[0],&msgType,sizeof(char));      //copy 1 byte message type to header memory
	printf("\nmsgType :: d=%d c=%c 0x=%0x",msgType,msgType,msgType);
	printf("\nbuf[0] :: d=%d c=%c 0x=%0x",buf[0],buf[0],buf[0]);

	memcpy(&buf[1],UOID,UOIDLEN);		    //copy 20 bytes of UOID in header memory

	memcpy(&buf[21],&TTL,sizeof(unsigned char));			//copy 1 byte TTL into header memory
	printf("\nTTL :: %d",TTL);
	printf("\nbuf[21] :: %d",buf[21]);

	memcpy(&buf[23],&datalen,sizeof(datalen));  //copy 4 byte datalen into header memory

	return buf;
}

 char *GetHelloMsg(char *hello_msg_buf,char *common_header,unsigned short int host_port,char *host_name,int hello_msg_buf_sz)
{
	unsigned short int temp_port;
	temp_port=htons(host_port);
	memset(hello_msg_buf,0,hello_msg_buf_sz);
	memcpy(&hello_msg_buf[0],common_header,COMMON_HEADER_SIZE);
	memcpy(&hello_msg_buf[COMMON_HEADER_SIZE],&temp_port,2);
	memcpy(&hello_msg_buf[COMMON_HEADER_SIZE+2],host_name,strlen(host_name));
	return hello_msg_buf;
}

char *GetJoinRequestMsg(char *join_msg_buf,char *common_header,unsigned int host_location,unsigned short int join_port,char *host_name,int join_msg_buf_sz)
{
	unsigned short int temp_port;
	temp_port=htons(join_port);
	unsigned int temp_location;
	temp_location=htonl(host_location);
	memset(join_msg_buf,0,join_msg_buf_sz);
	memcpy(&join_msg_buf[0],common_header,COMMON_HEADER_SIZE);
	memcpy(&join_msg_buf[COMMON_HEADER_SIZE],&temp_location,4);
	memcpy(&join_msg_buf[COMMON_HEADER_SIZE+4],&temp_port,2);
	memcpy(&join_msg_buf[COMMON_HEADER_SIZE+6],host_name,strlen(host_name));
	return join_msg_buf;
}

char *GetJoinResponseMsg(char *join_msg_buf,char *common_header,char *uoid_buf,unsigned int dist,unsigned short int join_port,char *host_name,int join_msg_buf_sz)
{
	unsigned short int temp_port;
	temp_port=htons(join_port);
	unsigned int temp_dist;
	temp_dist=htonl(dist);
	memset(join_msg_buf,0,join_msg_buf_sz);
	memcpy(&join_msg_buf[0],common_header,COMMON_HEADER_SIZE);
	memcpy(&join_msg_buf[COMMON_HEADER_SIZE],uoid_buf,UOIDLEN);
	memcpy(&join_msg_buf[COMMON_HEADER_SIZE+UOIDLEN],&temp_dist,4);
	memcpy(&join_msg_buf[COMMON_HEADER_SIZE+UOIDLEN+4],&join_port,2);
	memcpy(&join_msg_buf[COMMON_HEADER_SIZE+UOIDLEN+6],host_name,strlen(host_name));	
	return join_msg_buf;
}
 
#ifndef min
  #define min(A,B) (((A)>(B)) ? (B) : (A))
  #endif /* ~min */

   char *GetUOID(
          char *node_inst_id,
          char *obj_type,
          char *uoid_buf,
          int uoid_buf_sz)
{
	static unsigned long seq_no=(unsigned long)1;
	char sha1_buf[SHA_DIGEST_LENGTH], str_buf[104];
	sprintf(str_buf, "%s_%s_%1ld",node_inst_id, obj_type, (long)seq_no++);
	
	SHA1((unsigned char*)str_buf, strlen(str_buf),(unsigned char*)sha1_buf);
	memset(uoid_buf, 0, uoid_buf_sz);
	memcpy(uoid_buf, sha1_buf,min(uoid_buf_sz,sizeof(sha1_buf)));
	return uoid_buf;
}
  
/*
int main()
{
  char UOID[20];

  char *message=(char *)malloc(20*sizeof(char));
  printf("\nsizeof(message) :: %d",strlen(message));
  sprintf(message,"%s","nunki.usc.edu_12345_");
   printf("\nsizeof(message) :: %d",strlen(message));
  char * type=(char *)malloc(3);
  snprintf(type,sizeof(type),"msg");
 
  GetUOID(message,type,UOID,sizeof(UOID));
 printf("\nstrlen(UOID) :: %d",strlen(UOID));
 char *bufff=(char *)malloc(27*sizeof(char));
 bufff=getCommonHeader(0xFF,UOID,3,232);
char UOID1[20];
 char *UOID1_ret=(char *)malloc(20);
 UOID1_ret=GetUOID(message,type,UOID1,sizeof(UOID1));
printf("\nstrlen(UOID1) :: %d",strlen(UOID1_ret));
 printf("\ncmoparision: %d\n",strcmp(UOID1,UOID));
 printf("%d\n%d\n",UOID1[0],UOID[21]);

  char *buff=(char *)malloc(27*sizeof(char));
  buff=getCommonHeader(0xFC,UOID1,1,20);
  printf("aututututut\n");
  printf("asfasffa\n%s\n",buff);
}
*/
