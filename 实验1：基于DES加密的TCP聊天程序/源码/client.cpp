// 聊天程序客户端
#include<iostream>
#include<Winsock2.h>//socket头文件
#include<cstring>
#include"des.h"

using namespace std;


//载入系统提供的socket动态链接库
#pragma warning(disable: 4996)
#pragma comment(lib,"ws2_32.lib")   //socket库

const int BUFFER_SIZE = 1024;//缓冲区大小

DWORD WINAPI recvMsgThread(LPVOID IpParameter);

int main() {
	//1、初始化socket库
	WSADATA wsaData;//获取版本信息，说明要使用的版本
	WSAStartup(MAKEWORD(2, 2), &wsaData);//MAKEWORD(主版本号, 副版本号)

	//2、创建socket
	SOCKET cliSock = socket(AF_INET, SOCK_STREAM, 0);//面向网路的流式套接字,第三个参数代表自动选择协议

	//3、打包地址
	//客户端
	SOCKADDR_IN cliAddr = { 0 };
	cliAddr.sin_family = AF_INET;
	cliAddr.sin_addr.s_addr = inet_addr("127.0.0.1");//IP地址
	cliAddr.sin_port = htons(8888);//端口号
	//服务端
	SOCKADDR_IN servAddr = { 0 };
	servAddr.sin_family = AF_INET;//和服务器的socket一样，sin_family表示协议簇，一般用AF_INET表示TCP/IP协议。
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//服务端地址设置为本地回环地址
	servAddr.sin_port = htons(9999);//host to net short 端口号设置为12345

	bind(cliSock, (SOCKADDR*)&cliAddr, sizeof(cliAddr));

	if (connect(cliSock, (SOCKADDR*)&servAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		cout << "链接出现错误，错误代码" << WSAGetLastError() << endl;
	}

	//创建接受消息线程
	CloseHandle(CreateThread(NULL, 0, recvMsgThread, (LPVOID)&cliSock, 0, 0));
	//主线程用于输入要发送的消息
	while (1)
	{
		char buf[BUFFER_SIZE] = { 0 };
		cin.getline(buf, sizeof(buf));
		if (strcmp(buf, "quit") == 0)//若输入“quit”，则退出聊天室
		{
			break;
		}
		send(cliSock, buf, sizeof(buf), 0);
	}
	closesocket(cliSock);
	WSACleanup();
	return 0;
}

DWORD WINAPI recvMsgThread(LPVOID IpParameter)//接收消息的线程
{
	SOCKET cliSock = *(SOCKET*)IpParameter;//获取客户端的SOCKET参数

	while (1)
	{
		char buffer[BUFFER_SIZE] = { 0 };//字符缓冲区，用于接收和发送消息
		int nrecv = recv(cliSock, buffer, sizeof(buffer), 0);//nrecv是接收到的字节数
		if (nrecv > 0)//如果接收到的字符数大于0
		{
			des_decry(buffer);
			cout << MingWen << endl;
		}
		else if (nrecv < 0)//如果接收到的字符数小于0就说明断开连接
		{
			cout << "与服务器断开连接" << endl;
			break;
		}
	}
	return 0;
}