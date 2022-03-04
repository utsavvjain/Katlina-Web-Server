#include<iostream>
#include<map>
#include<katutil>
#include <sstream>
using namespace std;
using namespace kat_util;
string URLEncoder::encode(string rawString)
{
return "Cool";
}
string URLEncoder::decode(string encodedString)
{
if(encodedString.length()==0) return "";
StringUtils::replaceAll(encodedString,"%20"," ");
StringUtils::replaceAll(encodedString,"+"," ");
StringUtils::replaceAll(encodedString,"%21","!");
StringUtils::replaceAll(encodedString,"%22","\"");
StringUtils::replaceAll(encodedString,"%23","#");
StringUtils::replaceAll(encodedString,"%24","$");
StringUtils::replaceAll(encodedString,"%26","%");
StringUtils::replaceAll(encodedString,"%27","`");
StringUtils::replaceAll(encodedString,"%28","(");
StringUtils::replaceAll(encodedString,"%29",")");
StringUtils::replaceAll(encodedString,"%2A","*");
StringUtils::replaceAll(encodedString,"%2B","+");
StringUtils::replaceAll(encodedString,"%2C",",");
StringUtils::replaceAll(encodedString,"%2D","-");
StringUtils::replaceAll(encodedString,"%2E",".");
StringUtils::replaceAll(encodedString,"%2F","/");
StringUtils::replaceAll(encodedString,"%30","0");
StringUtils::replaceAll(encodedString,"%31","1");
StringUtils::replaceAll(encodedString,"%32","2");
StringUtils::replaceAll(encodedString,"%33","3");
StringUtils::replaceAll(encodedString,"%34","4");
StringUtils::replaceAll(encodedString,"%35","5");
StringUtils::replaceAll(encodedString,"%36","6");
StringUtils::replaceAll(encodedString,"%37","7");
StringUtils::replaceAll(encodedString,"%38","8");
StringUtils::replaceAll(encodedString,"%39","9");
StringUtils::replaceAll(encodedString,"%3A",":");
StringUtils::replaceAll(encodedString,"%3B",";");
StringUtils::replaceAll(encodedString,"%3C","<");
StringUtils::replaceAll(encodedString,"%3D","=");
StringUtils::replaceAll(encodedString,"%3E",">");
StringUtils::replaceAll(encodedString,"%3F","?");
StringUtils::replaceAll(encodedString,"%40","@");
StringUtils::replaceAll(encodedString,"%41","A");
StringUtils::replaceAll(encodedString,"%42","B");
StringUtils::replaceAll(encodedString,"%43","C");
StringUtils::replaceAll(encodedString,"%44","D");
StringUtils::replaceAll(encodedString,"%45","E");
StringUtils::replaceAll(encodedString,"%46","F");
StringUtils::replaceAll(encodedString,"%47","G");
StringUtils::replaceAll(encodedString,"%48","H");
StringUtils::replaceAll(encodedString,"%49","I");
StringUtils::replaceAll(encodedString,"%4A","J");
StringUtils::replaceAll(encodedString,"%4B","K");
StringUtils::replaceAll(encodedString,"%4C","L");
StringUtils::replaceAll(encodedString,"%4D","M");
StringUtils::replaceAll(encodedString,"%4E","N");
StringUtils::replaceAll(encodedString,"%4F","O");
StringUtils::replaceAll(encodedString,"%50","P");
StringUtils::replaceAll(encodedString,"%51","Q");
StringUtils::replaceAll(encodedString,"%52","R");
StringUtils::replaceAll(encodedString,"%53","S");
StringUtils::replaceAll(encodedString,"%54","T");
StringUtils::replaceAll(encodedString,"%55","U");
StringUtils::replaceAll(encodedString,"%56","V");
StringUtils::replaceAll(encodedString,"%57","W");
StringUtils::replaceAll(encodedString,"%58","X");
StringUtils::replaceAll(encodedString,"%59","Y");
StringUtils::replaceAll(encodedString,"%5A","Z");
StringUtils::replaceAll(encodedString,"%5B","[");
StringUtils::replaceAll(encodedString,"%5C","\\");
StringUtils::replaceAll(encodedString,"%5D","]");
StringUtils::replaceAll(encodedString,"%5E","^");
StringUtils::replaceAll(encodedString,"%5F","_");
StringUtils::replaceAll(encodedString,"%60","`");
StringUtils::replaceAll(encodedString,"%61","a");
StringUtils::replaceAll(encodedString,"%62","b");
StringUtils::replaceAll(encodedString,"%63","c");
StringUtils::replaceAll(encodedString,"%64","d");
StringUtils::replaceAll(encodedString,"%65","e");
StringUtils::replaceAll(encodedString,"%66","f");
StringUtils::replaceAll(encodedString,"%67","g");
StringUtils::replaceAll(encodedString,"%68","h");
StringUtils::replaceAll(encodedString,"%69","i");
StringUtils::replaceAll(encodedString,"%6A","j");
StringUtils::replaceAll(encodedString,"%6B","k");
StringUtils::replaceAll(encodedString,"%6C","l");
StringUtils::replaceAll(encodedString,"%6D","m");
StringUtils::replaceAll(encodedString,"%6E","n");
StringUtils::replaceAll(encodedString,"%6F","o");
StringUtils::replaceAll(encodedString,"%70","p");
StringUtils::replaceAll(encodedString,"%71","q");
StringUtils::replaceAll(encodedString,"%72","r");
StringUtils::replaceAll(encodedString,"%73","s");
StringUtils::replaceAll(encodedString,"%74","t");
StringUtils::replaceAll(encodedString,"%75","u");
StringUtils::replaceAll(encodedString,"%76","v");
StringUtils::replaceAll(encodedString,"%77","w");
StringUtils::replaceAll(encodedString,"%78","x");
StringUtils::replaceAll(encodedString,"%79","y");
StringUtils::replaceAll(encodedString,"%7A","z");
StringUtils::replaceAll(encodedString,"%7B","{");
StringUtils::replaceAll(encodedString,"%7C","|");
StringUtils::replaceAll(encodedString,"%7D","}");
StringUtils::replaceAll(encodedString,"%7E","~");
StringUtils::replaceAll(encodedString,"%7F"," ");
// TODO :: more to be addded
return encodedString; 
}
void StringUtils::replaceAll(string &orignalString,string toReplace,string replaceWith)
{
ostringstream oss;
size_t pos=0;
size_t prevPos=pos;
while(1)
{
prevPos=pos;
pos=orignalString.find(toReplace,pos);
if(pos==string::npos) break; // npos datatype is size_t, and means end of string
oss<<orignalString.substr(prevPos,pos-prevPos);
oss<<replaceWith;
pos+=toReplace.size();
}
oss<<orignalString.substr(prevPos); //TODO :: WORKOUT THIS LINE
orignalString=oss.str();
}
bool StringUtils::endsWith(const string& a, const string& b) 
{
if (b.size() > a.size()) return false;
return std::equal(a.begin() + a.size() - b.size(), a.end(), b.begin());
}
string KSTUUID::generateUUID()
{
UUID uuid;
UuidCreate(&uuid);
char *str;
UuidToStringA(&uuid, (RPC_CSTR*)&str);
string uuidStr(str);
RpcStringFreeA((RPC_CSTR*)&str);
return uuidStr;
}
template<typename FunctionAddress,typename ...T1>
ThreadManager<FunctionAddress,T1...>::~ThreadManager()
{
if(this->t) 
{
this->t->join();
delete this->t;
}
}
template<typename FunctionAddress,typename ...T1>
void ThreadManager<FunctionAddress,T1...>::start()
{
threadStarter(index_sequence_for<T1...>());
}
template<typename FunctionAddress,typename ...T1>
string ThreadManager<FunctionAddress,T1...>::getId()
{
return this->id;
}
template<typename FunctionAddress,typename ...T1>
template<size_t ...sequence>
void ThreadManager<FunctionAddress,T1...>::threadStarter(index_sequence<sequence ...>)
{
this->t=new thread(p2f,get<sequence>(this->args)...,id);
}
template class ThreadManager<void (*)(int,string),int>;