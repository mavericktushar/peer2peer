#include <iostream>
#include <stdio.h>

#include "iniparser.h"

using namespace std;

#define BEACONNUM 5

struct beaconinfo{
char *hostname;
unsigned short int port;
};

class inistruct
{	
	public:
	
	inistruct();
	~inistruct();
	
	void read_inifile(char *fname); 
	
	unsigned short int port;
	unsigned int location;
	char *homedir;
	char *logfilename;
	int autoshutdown;
	unsigned short int ttl;
	int msglifetime;
	int getmsglifetime;
	int initneighbors;
	int keepalivetimeout;
	int minneighbors;
	int nocheck;
	double cacheprob;
	double storeprob;
	double neighborstoreprob;
	int cachesize;
	int retry;
	struct beaconinfo *beacon[BEACONNUM];
	int beaconcount;
	
};




