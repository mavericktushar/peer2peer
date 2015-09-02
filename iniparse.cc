#include "iniparse.h"

inistruct :: inistruct()
{
	homedir=NULL;
	logfilename=NULL;
	
	beaconcount=0;
}

inistruct :: ~inistruct()
{
	delete homedir;
	delete logfilename;
}
				  
void inistruct :: read_inifile(char *fname) 
{
	dictionary *d;
	
	d=iniparser_load(fname);
	
	port=iniparser_getint(d,(char *)"init:port",0);
	location=iniparser_getint(d,(char *)"init:location",0);
	char *temp=iniparser_getstr(d,(char *)"init:homedir");
	homedir=new char[strlen(temp)+1];
	strcpy(homedir,temp);//,strlen(temp));     //-<change
	char *log=iniparser_getstring(d,(char *)"init:logfilename",(char *)"servant.log");
	logfilename=new char[strlen(log)+1];   //change:: instead of streln(log)+1  -> streln(log)
	memset(logfilename,0,strlen(log)+1);
	strncpy(logfilename,log,strlen(log));  //change...strncpy intead of memcpy
	autoshutdown=iniparser_getint(d,(char *)"init:autoshutdown",900);
	ttl=iniparser_getint(d,(char *)"init:ttl",30);
	msglifetime=iniparser_getint(d,(char *)"init:msglifetime",30);
	getmsglifetime=iniparser_getint(d,(char *)"init:getmsglifetime",300);
	initneighbors=iniparser_getint(d,(char *)"init:initneighbors",3);
	keepalivetimeout=iniparser_getint(d,(char *)"init:kepalivetimeout",60);
	minneighbors=iniparser_getint(d,(char *)"init:minneighbors",2);
	nocheck=iniparser_getboolean(d,(char *)"init:nocheck",0);
	cacheprob=iniparser_getdouble(d,(char *)"init:cacheprob",0.1);
	storeprob=iniparser_getdouble(d,(char *)"init:storeprob",0.1);
	neighborstoreprob=iniparser_getdouble(d,(char *)"init:neighborstoreprob",0.2);
	cachesize=iniparser_getint(d,(char *)"init:cachesize",500);
	retry=iniparser_getint(d,(char *)"beacons:retry",30);

	
	
	cout<<"\n--------------startup.ini-------------------\n";
	cout<<"\nport :: "<<port;
	cout<<"\nlocation :: "<<location;
	cout<<"\nhomedir :: "<<homedir;
	cout<<"\nlogfilename :: "<<logfilename<<"  strlen(logfilename)::"<<strlen(logfilename);
	
	FILE *fp;
	fp=fopen(fname,"r");
//	iniparser_dump(d,fp);
	
	char rdline[100];
	while(!feof(fp))
	{
		fgets(rdline,100,fp);
		cout<<rdline;
//		sleep(1);
		//if(!strncmp(rdline,"  [beacons]",11))
		char *tempc;
		while(strcmp("beacons",(tempc=strtok(rdline," 	[]"))))
		{
			cout<<"\ntempc :: "<<tempc;
			cout<<"\nIn while . . .";
			fgets(rdline,100,fp);
//			sleep(1);
		}
		cout<<"\ntempc :: "<<tempc<<"\n";
		while(!feof(fp))
		{
			memset(&rdline,'\0',100);
			fgets(rdline,100,fp);
			cout<<"\nrdline :: "<<rdline;
			char *s=strtok(rdline," 	=\n");
			if(s!=NULL)
			{
				if(!strcmp("Retry",s))
				continue;
				else
				{
					beacon[beaconcount]=new beaconinfo;
					char *h=strtok(s,":");
					cout<<"\nhostname :: "<<h<<" strlen(hostname in iniparse::"<<strlen(h);
					beacon[beaconcount]->hostname=(char *)calloc(strlen(h),1);
					strncpy(beacon[beaconcount]->hostname,h,strlen(h));
					cout<<"\nbeacon[beaconcount]->hostname :: "<<beacon[beaconcount]->hostname<<"strlen(hostname in iniparse::"<<strlen(beacon[beaconcount]->hostname);
					char *str=strtok(NULL,"");
					cout<<"\nport :: "<<str;
					beacon[beaconcount]->port=atoi(str);
					beaconcount++;
					h=NULL;
				}
			}
		}
		
	}
	fclose(fp);
	
//	remove("tempfl");
	
//	return inist;
}
