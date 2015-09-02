all:sv_node

sv_node:beacon_hello.o message.o beacon_connect.o beacon_listen.o beacon.o beacon_cmd.o send_recv.o timer.o list.o dictionary.o iniparser.o strlib.o
	g++ -g -Wno-deprecated -Wall -lpthread -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib -lcrypto -lsocket -lnsl -lresolv beacon_hello.o message.o beacon_connect.o beacon_listen.o beacon.o beacon_cmd.o send_recv.o timer.o list.o dictionary.o iniparser.o strlib.o -o sv_node

beacon_hello.o:beacon_hello.cc
	g++ -Wall -c beacon_hello.cc

message.o:message.cc
	g++ -Wall -c message.cc

beacon_connect.o:beacon_connect.cc
	g++ -Wall -c beacon_connect.cc

beacon_listen.o:beacon_listen.cc
	g++ -Wall -c beacon_listen.cc

beacon.o:beacon.cc
	g++ -Wall -c beacon.cc

beacon_cmd.o:beacon_cmd.cc
	g++ -Wall -c beacon_cmd.cc

send_recv.o:send_recv.cc
	g++ -Wall -c send_recv.cc

timer.o:timer.cc
	g++ -Wall -c timer.cc

list.o:list.cc
	g++ -Wall -c list.cc

dictionary.o:dictionary.cc
	g++ -Wall -c dictionary.cc

iniparser.o:iniparser.cc
	g++ -Wall -c iniparser.cc

strlib.o:strlib.cc
	g++ -Wall -c strlib.cc

clean:
	rm -f *.o *.coff *~ core sv_node
