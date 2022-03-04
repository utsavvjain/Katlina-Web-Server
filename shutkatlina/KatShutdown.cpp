#include<windows.h>
#include<iostream>
using namespace std;
int main()
{
WSADATA wsaData;
WORD ver;
ver=MAKEWORD(1,1);
int clientSocketDescriptor;
struct sockaddr_in clientSocketInformation;
WSAStartup(ver,&wsaData);
clientSocketDescriptor=socket(AF_INET,SOCK_STREAM,0);
clientSocketInformation.sin_family=AF_INET;
clientSocketInformation.sin_port=htons(8080);
clientSocketInformation.sin_addr.s_addr=inet_addr("127.0.0.1");
connect(clientSocketDescriptor,(struct sockaddr *)&clientSocketInformation,sizeof(clientSocketInformation));
string request="SHUTDOWN-KATLINA";
send(clientSocketDescriptor,request.c_str(),request.length(),0);
return 0;
}