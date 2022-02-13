#include<iostream>
#include<windows.h>
#include<map>
#include<kst2c>
#include<katutil>
#include<fstream>
#include<fileapi.h>
#include <dirent.h>
#include<list>
using namespace std;
using namespace kat_kst2c;
using namespace kat_util;
//Helper functions
string HEADER_FILE_NAME="kst.h";
FILETIME getFileTime(string fileName)
{
HANDLE hFile;
hFile=CreateFile(fileName.c_str(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
FILETIME ftWrite,ftCreate,ftAccess;
if(hFile==INVALID_HANDLE_VALUE) {
CloseHandle(hFile);
return ftWrite;
}
GetFileTime(hFile,&ftCreate,&ftAccess,&ftWrite);
CloseHandle(hFile);
return ftWrite;
}
int compareFileTime(string rawFileName,string generatedFileName)
{
FILETIME rawFile=getFileTime(rawFileName);
FILETIME genFile=getFileTime(generatedFileName);
return CompareFileTime(&rawFile,&genFile);
}
void KST2C::generate()
{
list<pair<string,string>> hFn; // ds to populate header file
struct dirent *d;
DIR *dir;
dir = opendir(".");
string fnDec;
string fnName;
for(d=readdir(dir); d!=NULL; d=readdir(dir))
{
string fileName(d->d_name);
if(!StringUtils::endsWith(fileName,".kst")) continue;
fstream file(fileName,ios::in);
if(file.fail()) continue;
string generatedFileName=fileName+".cpp";
fnName=fileName.substr(0,fileName.length()-4)+"_KST";
hFn.push_back({fnName,fileName});
//scope of generatedFile of type fstream starts here
{
fstream generatedFile(generatedFileName,ios::in);
if(generatedFile.fail()==false && compareFileTime(fileName,generatedFileName)==-1) 
{
generatedFile.close();
file.close();
continue;
}
generatedFile.close();
}
//scope of generatedFile of type fstream ends here
ofstream generatedFile(generatedFileName,ios::out);
//populate header files
list<string> headerList;
generatedFile<<"#include<katweb>"<<endl;
generatedFile<<"using namespace kat_web;"<<endl;
//populate function 
fnDec="void get"+fnName+"(Request &request,Response &response)";
generatedFile<<fnDec<<endl;
generatedFile<<"{"<<endl;
size_t propStartIndex,propEndIndex;
string propertyName;
int chunkSize=1024;
char *chunk;
int fileSize;

// calc file size start
file.seekg(0,ios::end);
fileSize=file.tellg();
file.seekg(0,ios::beg);
//file size calc ends

int totalRead=0;
while(totalRead<fileSize)
{
if(fileSize-totalRead<1024) chunkSize=fileSize-totalRead;
chunk=new char[chunkSize]();
file.read(chunk,chunkSize);
totalRead+=chunkSize;
string line(chunk);
delete []chunk;
StringUtils::replaceAll(line,"\"","\\\"");
while(1)
{
propStartIndex=line.find("${");
if(propStartIndex==string::npos) break;
if(propStartIndex!=string::npos)
{
propEndIndex=line.find("}",propStartIndex+1);
if(propEndIndex==string::npos) break;
if(propEndIndex!=string::npos)
{
propertyName=line.substr(propStartIndex+2,propEndIndex-propStartIndex-2);
line.replace(propStartIndex,propEndIndex-propStartIndex+1,")\"+request.getValue(\""+propertyName+"\")+R\"(");
}
}
}
generatedFile<<"response.write(R\"("<<line<<")\");"<<endl;
}
generatedFile<<"}"<<endl;
file.close();
generatedFile.close();
}
ofstream headerFile(HEADER_FILE_NAME,ios::out);
headerFile<<"#ifndef __KST"<<endl;
headerFile<<"#define __KST 5465"<<endl;
headerFile<<"#include<katweb>"<<endl;
headerFile<<"using namespace kat_web;"<<endl;
for(auto fnPair : hFn)
{
fnDec="void get"+fnPair.first+"(Request &request,Response &response);";
headerFile<<fnDec<<endl;
}
headerFile<<"void registerKSTs(Katlina *server)"<<endl;
headerFile<<"{"<<endl;
for(auto fnPair : hFn) headerFile<<"server->onRequest("<<"\"/"<<fnPair.second<<"\""<<","<<"get"<<fnPair.first<<");"<<endl;
headerFile<<"}"<<endl;
headerFile<<"#endif"<<endl;
headerFile.close();

closedir(dir);
}