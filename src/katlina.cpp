#include<stdio.h>
#include<windows.h>
#include<string.h>
#include<katweb>
#include<iostream>
#include<fstream>
#include<map>
#include<condition_variable>

using namespace std;
using namespace kat_web;
using namespace kat_util;
map<string,function<void(Request &,Response &)>> Katlina::mappings;
map<string,ThreadManager<void (*)(int,string),int> *> Katlina::threads;
set<string> Katlina::discardedThreadIds;
volatile bool Katlina::isServerRunning=true;
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
strcpy(header,"HTTP/1.1 200 OK\nConnection: close\n");
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
char *getMIMEType(const char *resource)
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
//katlina server side template
if(stricmp(resource+dotIndex+1,"kst")==0)
{
mimeType=(char *)malloc(sizeof(char)*10);
strcpy(mimeType,"text/kst");
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
void parseResourceNameAndQS(const char *bytes,REQUEST *request)
{
int i,j;
char resource[1001];
char **data;
int dataCount=0;
for(j=0,i=0;bytes[i]!=' ' && bytes[i]!='?';i++,j++) resource[j]=bytes[i];
resource[j]='\0';
if(bytes[i]=='?')
{
i++;
int si=i; // 1st data index
dataCount=1;
while(bytes[i]!=' ' && bytes[i]!='\0') 
{
if(bytes[i]=='&') dataCount++;
i++;
}
data=(char **)malloc(sizeof(char *)*dataCount);
int *pc=(int *)malloc(sizeof(int )*dataCount);
i=si;
int j=0;
while(bytes[i]!=' ' && bytes[i]!='\0')
{
if(bytes[i]=='&')pc[j++]=i;
i++;
}
pc[j]=i;
i=si;
j=0;
int howManyToPick;
while(j<dataCount)
{
howManyToPick=pc[j]-i;
data[j]=(char *)malloc(sizeof(char)*(howManyToPick+1));
strncpy(data[j],bytes+i,howManyToPick);
data[j][howManyToPick]='\0';
i=pc[j]+1;
j++;
}
}//QString exists
request->dataCount=dataCount;
request->data=data;
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
if(request->mimeType==NULL) request->isClientSideTechnologyResource='N';
if(stricmp(request->mimeType,"text/kst")==0) 
{
request->isClientSideTechnologyResource='N';
request->isKST=true;
}
else request->isKST=false;
}
}
REQUEST *parseRequest(char *bytes)
{
REQUEST *request=(REQUEST *)malloc(sizeof(REQUEST));
int i,j;
char method[11];
for(i=0;bytes[i]!=' ';i++) method[i]=bytes[i];
method[i]='\0';
i+=2; // skipped resource name "/"
if(strcmp(method,"GET")==0)
{
parseResourceNameAndQS(bytes+i,request);
} // method is GET type
request->method=(char *)malloc(sizeof(char)*strlen(method)+1);
strcpy(request->method,method);
return request;
}
void Katlina::start()
{
int i;char requestBuffer[8192]; 
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
return;
}
serverSocketInformation.sin_family=AF_INET;
serverSocketInformation.sin_port=htons(this->portNumber);
serverSocketInformation.sin_addr.s_addr=htonl(INADDR_ANY);
successCode=bind(serverSocketDescriptor,(struct sockaddr *)&serverSocketInformation,sizeof(serverSocketInformation));
if(successCode<0)
{
printf("Unable to bind socket to port 8081");
WSACleanup();
return;
}
listen(serverSocketDescriptor,10);
printf("Katlina is ready to listen HTTP requests on port %d\n",this->portNumber);
this->isServerRunning=true;
len=sizeof(serverSocketInformation);
string threadId;
thread destroyerThread(threadDestroyer);
while(isServerRunning)
{
clientSocketDescriptor=accept(serverSocketDescriptor,(struct sockaddr *)&clientSocketInformation,&len);
if(!isServerRunning) break;
if(clientSocketDescriptor<0)
{
printf("Unable to accept client connection\n");
closesocket(serverSocketDescriptor);
WSACleanup();
return;
}
ThreadManager<void (*)(int,string),int> *t=new ThreadManager<void (*)(int,string),int>(this->requestHandler,clientSocketDescriptor);
threads.insert({t->getId(),t});
t->start();
}
ThreadManager<void (*)(int,string),int> *thd;
set<string> threadIds;
for(auto it=threads.begin();it!=threads.end();it++) threadIds.insert(it->first);
for(string threadId:threadIds) 
{
thd=threads[threadId];
if(thd)
{
delete thd;
threads.erase(threadId);
}
}
destroyerThread.join();
closesocket(serverSocketDescriptor);
return;
}
Katlina::Katlina(int portNumber)
{
this->portNumber=portNumber;
}
void Katlina::onRequest(string url,std::function<void(Request &,Response &)> callback)
{
mappings[url.substr(1)]=callback;
}
void Request::loadQueryString(int dataCount,char **data)
{
char key[1024],value[1024];
int i,j,kvLen;
char *kvPair;
for(i=0;i<dataCount;i++)
{
kvPair=data[i];
kvLen=strlen(kvPair);
j=0;
while(kvPair[j]!='=' && j<kvLen) j++;
if(j==kvLen) break;
strncpy(key,kvPair,j);
key[j]='\0';
j++;
strncpy(value,kvPair+j,kvLen-j);
value[kvLen-j]='\0';
queryKeyValuePairs.insert({URLEncoder::decode(key),URLEncoder::decode(value)});
}
}
string Request::get(string param)
{
if(queryKeyValuePairs.find(param)==queryKeyValuePairs.end()) return "";
return queryKeyValuePairs[param];
}
string Request::getMethodType()
{
return this->request->method;
}
string Request::getResource()
{
return this->request->resource;
}
void Request::forward(string resourceName)
{
this->forwardedToResource=resourceName;
}
void Request::_forward()
{
const char *resource=forwardedToResource.c_str();
REQUEST *request=(REQUEST *)malloc(sizeof(REQUEST));
request->isForwarded=true;
parseResourceNameAndQS(resource,request);
if(request->isClientSideTechnologyResource=='Y')
{
if(!serveResource(request,request->resource,this->clientSocketDescriptor)) sendError(clientSocketDescriptor,404,request);
}
else 
{
if(Katlina::mappings.find(request->resource)==Katlina::mappings.end()) sendError(clientSocketDescriptor,404,request);
else
{
function<void(Request &,Response &)> callback=Katlina::mappings[request->resource];
Request req(request,clientSocketDescriptor);
Response res(clientSocketDescriptor);
req.intData=this->intData;
req.strData=this->strData;
if(request->dataCount>0) req.loadQueryString(request->dataCount,request->data);
callback(req,res);
if(request->dataCount>0) 
{
for(int k=0;k<request->dataCount;k++) free(request->data[k]);
free(request->data);
}
}
}
}
int Request::getInt(string key)
{
return this->intData[key];
}
int Request::isForwarded()
{
return this->request->isForwarded;
}
void Request::setInt(string key,int value)
{
this->intData[key]=value;
}
void Request::setValue(string key,string value){
this->strData[key]=value;
}
string Request::getValue(string key)
{
return this->strData[key];
}
void Response::close()
{
closesocket(this->clientSocketDescriptor);
}

void Response::write(string data)
{
char header[1024];
if(!this->headerSent)
{
strcpy(header,"HTTP/1.1 200 OK\nConnection: close\nContent-Type : text/html\n\n");
send(this->clientSocketDescriptor,header,strlen(header),0);
this->headerSent=true;
}
const char *dataArray=data.c_str();
int contentLength=strlen(dataArray);
int chunkSize=1024;
int byteSent=0;
while(byteSent<contentLength)
{
if((contentLength-byteSent)<chunkSize) chunkSize=contentLength-byteSent;
send(clientSocketDescriptor,dataArray+byteSent,chunkSize,0);
byteSent+=chunkSize;
}
}
void Katlina::requestHandler(int clientSocketDescriptor,string threadId)
{
int i;char requestBuffer[8192]; 
int bytesExtracted,len,successCode;
bytesExtracted=recv(clientSocketDescriptor,requestBuffer,sizeof(requestBuffer),0);
if(stricmp(requestBuffer,"SHUTDOWN-KATLINA")==0)
{
cout<<"Shutting server down, Active threads : "<<threads.size()<<endl;
isServerRunning=false;
return;
}
if(bytesExtracted<0) //error
{
isServerRunning=false;
}
else if(bytesExtracted==0) 
{
isServerRunning=false;
}
else
{
requestBuffer[bytesExtracted]='\0';
REQUEST *request=parseRequest(requestBuffer);
if(request->isClientSideTechnologyResource=='Y')
{
if(!request->resource) {
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
if(mappings.find(request->resource)==mappings.end()) sendError(clientSocketDescriptor,404,request);
else
{
function<void(Request &,Response &)> callback=mappings[request->resource];
Request req(request,clientSocketDescriptor);
Response res(clientSocketDescriptor);
if(request->dataCount>0) req.loadQueryString(request->dataCount,request->data);
callback(req,res);
if(request->dataCount>0) 
{
for(int k=0;k<request->dataCount;k++) free(request->data[k]);
free(request->data);
}
if(req.forwardedToResource.length()>0) req._forward();
}
}
}
closesocket(clientSocketDescriptor);
discardedThreadIds.insert(threadId);
}
void Katlina::threadDestroyer()
{
while(isServerRunning)
{
if(discardedThreadIds.size()!=0)
{
ThreadManager<void (*)(int,string),int> *thd;
for(auto it=threads.begin();it!=threads.end();it++)
{
thd=it->second;
delete thd;
threads.erase(it->first);
}
}
}
}