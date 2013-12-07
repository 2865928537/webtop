#ifndef _PROXY_H
#define _PROXY_H

#include <WinSock2.h>
#include "windows.h"
#include <iostream>
#include <malloc.h>
#include <process.h>
#include "../transparent_wnd.h"
#pragma comment(lib,"ws2_32.lib")
using namespace std;

#define MAXBUFFERSIZE      20*1024      //��������С
#define HTTP  "http://"
#define HTTPS  "CONNECT "

#define DEBUG 1

//�궨��
#define BLOCK 1024
#define MAXNUM 8
#define MAX_LINK_COUNT 100

class MelodyProxy{
public:
	//Proxy �˿�
	UINT pport;// = 8888;
	bool agentRequest;
	bool replaceResponse;
	TransparentWnd* winHandler;
	static int linkCount;
	MelodyProxy(){
		pport=8888;
		agentRequest=false;
		replaceResponse=false;
	}
	MelodyProxy(TransparentWnd* _winHandler, UINT _pport=8888, bool _replaceResponse=false){
		pport=_pport;
		replaceResponse=_replaceResponse;
		winHandler=_winHandler;
	}

	struct ProxySockets
	{
		SOCKET  ProxyToUserSocket;	//���ػ�����PROXY �������socket
		SOCKET  ProxyToServSocket;	//PROXY �������Զ��������socket
		BOOL    IsProxyToUserClosed; // ���ػ�����PROXY �����״̬
		BOOL    IsProxyToServClosed; // PROXY �������Զ������״̬
	};

	static struct ProxyParam
	{
		char Address[256];		// Զ��������ַ
		HANDLE IsConnectedOK;	// PROXY �������Զ������������״̬
		HANDLE IsExit;	// PROXY �������Զ������������״̬
		HANDLE ReplaceRequestOK;	// PROXY �������Զ������������״̬
		HANDLE ReplaceResponseOK;	// PROXY �������Զ������������״̬
		bool CancelReplaceResponse;
		ProxySockets * pPair;	// ά��һ��SOCKET��ָ��
		void * pProxy;	// ά��һ��SOCKET��ָ��
		int Port;			// ��������Զ�������Ķ˿�
		char Request[MAXBUFFERSIZE];
		char* Response;
		bool isHttps;
		int retryCount;
	}; //�ṹ����PROXY SERVER��Զ����������Ϣ����

	SOCKET listentsocket; //����������SOCKET

	int StartProxyServer() //��������
	{
		WSADATA wsaData;
		sockaddr_in local;
		//	SOCKET listentsocket;

		//Log("�������������");
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			//Log("\nError in Startup session.\n");
			WSACleanup();
			return -1;
		}

		local.sin_family = AF_INET;
		local.sin_addr.s_addr = INADDR_ANY;
		local.sin_port = htons(pport);
		//
		listentsocket = socket(AF_INET,SOCK_STREAM,0);
		//Log("���������׽���");

		if(listentsocket == INVALID_SOCKET)
		{
			//Log("\nError in New a Socket.");
			WSACleanup();
			return -2;
		}

		//Log("�������׽���");
		if(::bind(listentsocket,(sockaddr *)&local,sizeof(local))!=0)
		{
			//Log("\n Error in Binding socket.");
			WSACleanup();
			return -3;	
		}

		//Log("��������");
		if(::listen(listentsocket,100)!=0)
		{
			//Log("\n Error in Listen.");
			WSACleanup(); 
			return -4;
		}
		//	::listentsocket=listentsocket; 
		//InitializeMemPool();
		//Log("���������߳�");
		HANDLE threadhandle = (HANDLE)_beginthreadex(NULL, 0, UserToProxy, (LPVOID)this, NULL, NULL);
		CloseHandle(threadhandle);
		return 1;
	}

	int CloseServer() //�رշ���
	{
		closesocket(listentsocket);
		//	WSACleanup();
		return 1;
	}

	~MelodyProxy(){
		closesocket(listentsocket);
	}
	//�������յ����ַ����õ�Զ��������ַ
	static int ResolveInformation( char * str, char *address, int * port)
	{
		char buf[MAXBUFFERSIZE], command[512], proto[128], *p=NULL;

		//CString strLog;

		sscanf(str,"%s%s%s",command,buf,proto);
		p=strstr(buf,HTTP);

		//HTTP
		if(p)
		{
			UINT i;
			p+=strlen(HTTP);
			int j=0;
			for(i=0; i<strlen(p); i++)
			{
				if(*(p+i)==':') j=i;
				if( *(p+i)=='/') break;
			}

			*(p+i)=0;
			strcpy(address,p);
			if(j!=0){
				*port=atoi(p+j);
			}
			else{
				*port=80;	//ȱʡ�� http �˿�
			}
			p = strstr(str,HTTP);

			//ȥ��Զ��������: 
			//GET http://www.njust.edu.cn/ HTTP1.1  == > GET / HTTP1.1
			/*for(UINT j=0;j< i+strlen(HTTP);j++)
			{
				*(p+j)=' ';
			}*/

			return 1;
		}
		strcpy(buf,str);
		p=strstr(buf,HTTPS);
		if(p)
		{
			UINT i;
			p+=strlen(HTTPS);
			int j=0;
			for(i=0; i<strlen(p); i++)
			{
				if(*(p+i)==':') j=i;
				if( *(p+i)==' ') break;
			}
			*(p+i)=0;
			strcpy(address,p);
			if(j!=0){
				*port=atoi(p+j+1);
				*(address+j)=0;
			}
			else{
				*port=443;	//ȱʡ�� http �˿�
			}
			p = strstr(str,address);

			return 1;
		}

		return 0; 
	}

	static int ConnectServer(){
	}

	// ȡ�����ص����ݣ�����Զ������
	static unsigned int WINAPI UserToProxy(void *pParam)
	{
		sockaddr_in from;
		MelodyProxy* proxy=(MelodyProxy *)pParam;
		int  fromlen = sizeof(from);
		bool isAccept=false;
		char Buffer[MAXBUFFERSIZE];
		int  Len;
		int count=0;

		SOCKET ProxyToUserSocket;
		ProxySockets SPair;
		ProxyParam   ProxyP;

		HANDLE pChildThread;
		int retval;

		//Log("���������û�����");
		ProxyToUserSocket = accept(proxy->listentsocket,(struct sockaddr*)&from,&fromlen);
		//Log("��������");
		HANDLE threadhandle = (HANDLE)_beginthreadex(NULL, 0, UserToProxy, (LPVOID)pParam, NULL, NULL);
		CloseHandle(threadhandle);
		if( ProxyToUserSocket==INVALID_SOCKET)
		{ 
			return -5;
		}

		//���ͻ��ĵ�һ������
		SPair.IsProxyToUserClosed = FALSE;
		SPair.IsProxyToServClosed = TRUE ;
		SPair.ProxyToUserSocket = ProxyToUserSocket;

		//Log("�����û�����");
		retval = recv(SPair.ProxyToUserSocket,Buffer,sizeof(Buffer),0);

		if(retval==SOCKET_ERROR)
		{ 
			//Log("\nError Recv"); 
			if(SPair.IsProxyToUserClosed == FALSE)
			{
				closesocket(SPair.ProxyToUserSocket);
				SPair.IsProxyToUserClosed = TRUE;
				return 0;
			}
		}

		if(retval==0)
		{
			//Log("Client Close connection\n");
			if(SPair.IsProxyToUserClosed==FALSE)
			{	
				closesocket(SPair.ProxyToUserSocket);
				SPair.IsProxyToUserClosed=TRUE;
				return 0;
			}
		}
		Len = retval;

		//
		SPair.IsProxyToUserClosed = FALSE;
		SPair.IsProxyToServClosed = TRUE;
		SPair.ProxyToUserSocket = ProxyToUserSocket;
		ProxyP.pPair=&SPair;
		ProxyP.IsConnectedOK = CreateEvent(NULL,FALSE,FALSE,NULL);
		ProxyP.ReplaceResponseOK = CreateEvent(NULL,TRUE,FALSE,NULL);
		ProxyP.pProxy=(void *)proxy;
		ProxyP.retryCount=0;
		ZeroMemory(ProxyP.Request,MAXBUFFERSIZE);
		memcpy(ProxyP.Request,Buffer, Len);
		struct hostent *hp;
		hp = gethostbyname("proxy.tencent.com");

		if(MAXBUFFERSIZE>Len){
			Buffer[Len]=0;
		}
		if(proxy->agentRequest){
			//proxy->winHandler->agentRequest(Buffer);
		}
		//Log("�����û�����");
		ResolveInformation( Buffer,ProxyP.Address,&ProxyP.Port);
		if(ProxyP.Port==443){
			ProxyP.isHttps=true;
		}
		else{
			ProxyP.isHttps=false;
		}
		char buf[1000]={0};
		strcpy(buf,ProxyP.Address);
		if(hp!=NULL){
			strcpy(ProxyP.Address,"proxy.tencent.com");
			ProxyP.Port=8080;
		}

		pChildThread = (HANDLE)_beginthreadex(NULL, 0, ProxyToServer, (LPVOID)&ProxyP, 0, NULL);

		//Log("�ȴ�����Ŀ���ַ�¼�");
		::WaitForSingleObject(ProxyP.IsConnectedOK,60000);
		::CloseHandle(ProxyP.IsConnectedOK);
		bool first=true;

		while(SPair.IsProxyToServClosed == FALSE && SPair.IsProxyToUserClosed == FALSE)
		{	
			//Log("��Ŀ���ַ��������");
			if(ProxyP.Port!=443||first==false){
				retval = send(SPair.ProxyToServSocket,Buffer,Len,0);

				if(retval==SOCKET_ERROR)
				{ 
					if(SPair.IsProxyToServClosed == FALSE)
					{
						SPair.IsProxyToServClosed = TRUE;
						closesocket(SPair.ProxyToServSocket);
					}
					break;
				}
			}
			first=false;

			//Log("���û���ַ��������");
			retval = recv(SPair.ProxyToUserSocket,Buffer,sizeof(Buffer),0);

			if(retval==SOCKET_ERROR)
			{
				//Log("\nError Recv"); 
				if(SPair.IsProxyToUserClosed==FALSE)
				{
					SPair.IsProxyToUserClosed=TRUE;
					closesocket(SPair.ProxyToUserSocket);
				}
				continue;
			}

			if(retval==0)
			{
				//Log("Client Close connection\n");
				if(SPair.IsProxyToUserClosed==FALSE)
				{
					closesocket(SPair.ProxyToUserSocket);
					SPair.IsProxyToUserClosed=TRUE;
				}
				break;
			}
			Len=retval;
			ZeroMemory(ProxyP.Request,MAXBUFFERSIZE);
			memcpy(ProxyP.Request,Buffer, Len);
		}	//End While

		if(SPair.IsProxyToServClosed == FALSE)
		{
			closesocket(SPair.ProxyToServSocket);
			SPair.IsProxyToServClosed=TRUE;
		}
		if(SPair.IsProxyToUserClosed == FALSE)
		{
			closesocket(SPair.ProxyToUserSocket);
			SPair.IsProxyToUserClosed=TRUE;
		}
		proxy->winHandler->eraseParam((long)&ProxyP);
		::WaitForSingleObject(pChildThread,6000);
		CloseHandle(pChildThread);
		return 0;
	}

	static int readChunk(char* str, int len, char** pBuffer, MelodyProxy* pProxy){
		if(len==0){
			return 0;
		}
		char* buffer=*pBuffer;
		char temp1[10]={"0x"};
		int res=len;//����ʣ���ֽ���
		char* seek=strstr(str,"\r\n");//����16�����ַ��������CRLF
		if(!seek){
			MessageBoxA(NULL,"eror","error",0);
			return 0;
		}
		int l1=seek-str;//16�����ַ�������
		strncpy(temp1+2,str,l1);//����16�����ַ���
		temp1[l1+2]=0;//��0
		char* strEnd;
		//pProxy->winHandler->agentResponse(str);
		int i = (int)strtol(temp1, &strEnd, 16);//ʮ������
		res-=l1+2;
		if(i==0){
			return -1;
		}
		if(res<=0){
			return i;
		}
		//����һ��chunk��
		if(res<i){
			//�������ݵ�content��
			memcpy(buffer,seek+2,res);//���Ƶ���������
			*pBuffer=buffer+res;
			return i-res;//����ʣ���chunk���ݵĳ���
		}
		else{
			memcpy(buffer,seek+2,i);//���Ƶ���������
			//��һ��chunk��ָ�룬ʣ���ַ������ȣ����¼��㻺������ʼλ��
			*pBuffer=buffer+i;
			res-=i+2;
			if(res>0){
				return readChunk(seek+2+i+2,res,pBuffer,pProxy);
			}
			else{
				return 0;
			}
		}
	}
	static char *replace(char *st, char *orig, char *repl, char* buffer, int l) {
		ZeroMemory(buffer,l);
		char *ch;
		if (!(ch = strstr(st, orig)))
			return st;
		strncpy(buffer, st, ch-st);  
		buffer[ch-st] = 0;
		sprintf(buffer+(ch-st), "%s%s", repl, ch+strlen(orig));
		return buffer;
	}
	static unsigned int WINAPI ProxyToServer(LPVOID pParam);
	static int Response(char* header, char* content, int count, MelodyProxy* pProxy, ProxyParam* pPar);
};
#endif
