#define UOIDLEN 20

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include<sys/socket.h>
#include <sys/types.h>
#include <openssl/sha.h>

#define COMMON_HEADER_SIZE 27

using namespace std;

char * GetCommonHeader(unsigned char ,char *,unsigned char , int );

char *GetHelloMsg(char *,char *,unsigned short int ,char *,int );

char *GetUOID(char *,char *,char *,int);

char *GetJoinRequestMsg(char *,char *,unsigned int ,unsigned short int ,char *,int );

char *GetJoinResponseMsg(char *,char *,char *,unsigned int ,unsigned short int ,char *,int );
