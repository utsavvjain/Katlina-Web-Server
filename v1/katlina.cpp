#include<stdio.h>
#include<windows.h>
#include<string.h>
typedef struct Request
{
char *method;
char *resource;
char isClientSideTechnologyResource;
char *mimeType;
}REQUEST;
bool serveResource(REQUEST *request,const char *resource,int clientSocketDescriptor)
{
char header[1024];
char resourceData[1024];
long contentLength;
FILE *f=fopen(resource,"rb");
if(!f) return false; // not exists
fseek(f,0,SEEK_END);
contentLength=ftell(f);
fseek(f,0,SEEK_SET);
strcpy(header,"HTTP/1.1 200 OK\nConnection: Keep-Alive\nKeep-Alive: timeout=5, max=1000\n");
char contentType[1024];
if(!request->mimeType) strcat(header,"Content-Type : text/html\n");
else
{
sprintf(contentType,"Content-Type : %s\n",request->mimeType);
strcat(header,contentType);
}
strcat(header,"Content-Length : ");
char responseLength[11];
sprintf(responseLength,"%d\n\n",contentLength);
strcat(header,responseLength);
send(clientSocketDescriptor,header,strlen(header),0);
int chunkSize=1024;
int byteSent=0;
while(byteSent<contentLength)
{
if((contentLength-byteSent)<chunkSize) chunkSize=contentLength-byteSent;
fread(resourceData,1,chunkSize,f);
send(clientSocketDescriptor,resourceData,chunkSize,0);
byteSent+=chunkSize;
}
fclose(f);
printf("Serving resource /%s\n",(request->resource)?request->resource:"");
return true;
}
void sendError(int clientSocketDescriptor,int errorCode,REQUEST *request)
{
char header[1024];
char responseLength[11];
strcpy(header,"HTTP/1.1 200 OK\nContent-Type : text/html\nContent-Length : ");
char response[1024];
if(errorCode==404)
{
sprintf(response,"<!DOCTYPE HTML><html lang='en'><head><title>Katalina error</title><body><span style='color : red'><b>Error : 404</b><br> Resource <b>/%s</b> not found.</span><body></html>",(request->resource)?request->resource:"");
sprintf(responseLength,"%d\n\n",strlen(response));
strcat(header,responseLength);
send(clientSocketDescriptor,header,strlen(header),0);
send(clientSocketDescriptor,response,strlen(response),0);
}
}
char *getMIMEType(char *resource)
{
char *mimeType=NULL;
int len=strlen(resource);
if(len<4) return mimeType;
int dotIndex;
for(dotIndex=len-1;dotIndex>0 && resource[dotIndex]!='.';dotIndex--);
if(dotIndex==0) return mimeType;
if(stricmp(resource+dotIndex+1,"css")==0)
{
mimeType=(char *)malloc(sizeof(char)*9);
strcpy(mimeType,"text/css");
}
else if(stricmp(resource+dotIndex+1,"js")==0)
{
mimeType=(char *)malloc(sizeof(char)*8);
strcpy(mimeType,"text/js");
}
else if(stricmp(resource+dotIndex+1,"html")==0)
{
mimeType=(char *)malloc(sizeof(char)*10);
strcpy(mimeType,"text/html");
}
else if(stricmp(resource+dotIndex+1,"jpg")==0 || stricmp(resource+dotIndex+1,"jpeg")==0)
{
mimeType=(char *)malloc(sizeof(char)*11);
strcpy(mimeType,"image/jpeg");
}
else if(stricmp(resource+dotIndex+1,"ico")==0)
{
mimeType=(char *)malloc(sizeof(char)*25);
strcpy(mimeType,"image/vnd.microsoft.icon");
}

else if(stricmp(resource+dotIndex+1,"gif")==0)
{
mimeType=(char *)malloc(sizeof(char)*10);
strcpy(mimeType,"image/gif");
}
return mimeType;
}
char isClientSideResourceTechnology(char *resource)
{
return 'Y'; //this will have to be changed later
}
REQUEST *parseRequest(char *bytes)
{
int i,j;
char method[11];
char resource[1001];
for(i=0;bytes[i]!=' ';i++) method[i]=bytes[i];
method[i]='\0';
i+=2; // skipped resource name "/"
for(j=0;bytes[i]!=' ';i++,j++) resource[j]=bytes[i];
resource[j]='\0';
REQUEST *request=(REQUEST *)malloc(sizeof(REQUEST));
request->method=(char *)malloc(sizeof(char)*strlen(method)+1);
strcpy(request->method,method);
if(resource[0]=='\0')
{
request->resource=NULL;
request->isClientSideTechnologyResource='Y';
request->mimeType=NULL;
}
else
{
request->resource=(char *)malloc(sizeof(char)*strlen(resource)+1);
strcpy(request->resource,resource);
request->isClientSideTechnologyResource=isClientSideResourceTechnology(resource);
request->mimeType=getMIMEType(resource);
}
return request;
}
int main()
{
int i;
char requestBuffer[8192]; 
int bytesExtracted,len,successCode;
WORD ver;
ver=MAKEWORD(1,1);
WSAData wsaData;
WSAStartup(ver,&wsaData);
int clientSocketDescriptor,serverSocketDescriptor;
struct sockaddr_in serverSocketInformation,clientSocketInformation;
serverSocketDescriptor=socket(AF_INET,SOCK_STREAM,0);
if(serverSocketDescriptor<0)
{
printf("Unable to create socket\n");
return 0;
}
serverSocketInformation.sin_family=AF_INET;
serverSocketInformation.sin_port=htons(8081);
serverSocketInformation.sin_addr.s_addr=htonl(INADDR_ANY);
successCode=bind(serverSocketDescriptor,(struct sockaddr *)&serverSocketInformation,sizeof(serverSocketInformation));
if(successCode<0)
{
printf("Unable to bind socket to port 8081");
WSACleanup();
return 0;
}
listen(serverSocketDescriptor,10);
printf("Katlina is ready to listen HTTP requests on port 8081\n");
len=sizeof(serverSocketInformation);
while(1)
{
clientSocketDescriptor=accept(serverSocketDescriptor,(struct sockaddr *)&clientSocketInformation,&len);
if(clientSocketDescriptor<0)
{
printf("Unable to accept client connection\n");
closesocket(serverSocketDescriptor);
WSACleanup();
return 0;
}
bytesExtracted=recv(clientSocketDescriptor,requestBuffer,sizeof(requestBuffer),0);
if(bytesExtracted<0) //error
{
//what to do yet to be decide
}
else if(bytesExtracted==0) 
{
//what to do yet to be decide
}
else
{
requestBuffer[bytesExtracted]='\0';
REQUEST *request=parseRequest(requestBuffer);
printf("Request arrived : %s %s %s\n",request->method,request->resource,request->mimeType);
if(request->isClientSideTechnologyResource=='Y')
{
if(!request->resource)
{
if(!serveResource(request,"index.html",clientSocketDescriptor)) 
{
if(!serveResource(request,"index.htm",clientSocketDescriptor)) sendError(clientSocketDescriptor,404,request);
}
}
else
{
if(!serveResource(request,request->resource,clientSocketDescriptor)) sendError(clientSocketDescriptor,404,request);
}
}
else 
{
//what to do is yet to be decided
}
}
closesocket(clientSocketDescriptor);
}
closesocket(serverSocketDescriptor);
return 0;
}