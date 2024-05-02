#include<iostream>
#include<Winsock2.h>//socket头文件
#include<cstring>
#include"des.h"

using namespace std;

//载入系统提供的socket动态链接库
#pragma warning(disable: 4996)
#pragma comment(lib,"ws2_32.lib")   //socket库

//==============================全局变量区===================================
const int BUFFER_SIZE = 1024;//缓冲区大小
int RECV_TIMEOUT = 10;//接收消息超时
int SEND_TIMEOUT = 10;//发送消息超时
const int WAIT_TIME = 10;//每个客户端等待事件的时间，单位毫秒
const int MAX_LINK_NUM = 10;//服务端最大链接数
SOCKET cliSock[MAX_LINK_NUM];//客户端套接字 0号为服务端
SOCKADDR_IN cliAddr[MAX_LINK_NUM];//客户端地址
WSAEVENT cliEvent[MAX_LINK_NUM];//客户端事件 0号为服务端,它用于让程序的一部分等待来自另一部分的信号。例如，当数据从套接字变为可用时，winsock 库会将事件设置为信号状态
int total = 0;//当前已经链接的客服端服务数


//==============================函数声明===================================
DWORD WINAPI servEventThread(LPVOID IpParameter);//服务器端处理线程


int main()
{
	//1、初始化socket库
	WSADATA wsaData;//获取版本信息，说明要使用的版本
	WSAStartup(MAKEWORD(2, 2), &wsaData);//MAKEWORD(主版本号, 副版本号)
	
	//2、创建socket
	SOCKET servSock = socket(AF_INET, SOCK_STREAM, 0);//面向网路的流式套接字

	//3、将服务器地址打包在一个结构体里面
	SOCKADDR_IN servAddr; //sockaddr_in 是internet环境下套接字的地址形式
	servAddr.sin_family = AF_INET;//和服务器的socket一样，sin_family表示协议簇，一般用AF_INET表示TCP/IP协议。
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//服务端地址设置为本地回环地址
	servAddr.sin_port = htons(9999);//host to net short 端口号设置为12345


	//4、绑定服务端的socket和打包好的地址
	bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr));

	//4.5给服务端sokect绑定一个事件对象，用来接收客户端链接的事件
	WSAEVENT servEvent = WSACreateEvent();//创建一个人工重设为传信的事件对象
	WSAEventSelect(servSock, servEvent, FD_ALL_EVENTS);//绑定事件对象，并且监听所有事件

	cliSock[0] = servSock;
	cliEvent[0] = servEvent;

	//5、开启监听
	listen(servSock, 10);//监听队列长度为10

	//6、创建接受链接的线程
	//不需要句柄所以直接关闭
	CloseHandle(CreateThread(NULL, 0, servEventThread, (LPVOID)&servSock, 0, 0));

	cout << "聊天室服务器已开启" << endl;
	connect;
		//int addrLen = sizeof(SOCKADDR);//用于接收客户端的地址包结构体长度
		//SOCKET cliSOCK = accept(servSock, (SOCKADDR*)&servAddr,&addrLen);
		//if (cliSOCK != INVALID_SOCKET) 
		//{
		//	cout << "链接成功" << endl;
		//}
		//while (1)
		//{
		//	char buf[100] = { 0 };//测试缓冲区
		//	int nrecv = recv(cliSOCK, buf, sizeof(buf), 0);
		//	if (nrecv > 0)//如果接收到客户端的信息就输出到屏幕
		//	{
		//		cout << buf << endl;
		//	}
		//}
		//需要让主线程一直运行下去
		//发送消息给全部客户端
		while (1)
		{

			char contentBuf[BUFFER_SIZE] = { 0 };
			char sendBuf[BUFFER_SIZE] = { 0 };
			cin.getline(contentBuf, sizeof(contentBuf));
			sprintf(sendBuf, "[柳哥bot:]%s", contentBuf);
			for (int j = 1; j <= total; j++)
			{
				send(cliSock[j], sendBuf, sizeof(sendBuf), 0);
			}

		}
	//1-关闭socket库的收尾工作
	WSACleanup();
	return 0;
}

DWORD WINAPI servEventThread(LPVOID IpParameter) //服务器端线程
{
	//该线程负责处理服务端和各个客户端发生的事件
	//将传入的参数初始化
	SOCKET servSock = *(SOCKET*)IpParameter;//LPVOID为空指针类型，需要先转成SOCKET类型再引用，即可使用传入的SOCKET
	while (1) //不停执行
	{
		for (int i = 0; i < total + 1; i++)//i代表现在正在监听事件的终端
		{
			//若有一个客户端链接，total==1，循环两次，包含客户端和服务端
			//对每一个终端（客户端和服务端），查看是否发生事件，等待WAIT_TIME毫秒
			int index = WSAWaitForMultipleEvents(1, &cliEvent[i], false, WAIT_TIME, 0);

			index -= WSA_WAIT_EVENT_0;//此时index为发生事件的终端下标

			if (index == WSA_WAIT_TIMEOUT || index == WSA_WAIT_FAILED)
			{
				continue;//如果出错或者超时，即跳过此终端
			}

			else if (index == 0)
			{
				WSANETWORKEVENTS networkEvents;
				WSAEnumNetworkEvents(cliSock[i], cliEvent[i], &networkEvents);//查看是什么事件

				//事件选择
				if (networkEvents.lNetworkEvents & FD_ACCEPT)//若产生accept事件（此处与位掩码相与）
				{
					
					if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
					{
						cout << "连接时产生错误，错误代码" << networkEvents.iErrorCode[FD_ACCEPT_BIT] << endl;
						continue;

					}
					//接受链接
					if (total + 1 < MAX_LINK_NUM)//若增加一个客户端仍然小于最大连接数，则接受该链接
					{
						//total为已连接客户端数量
						int nextIndex = total + 1;//分配给新客户端的下标
						int addrLen = sizeof(SOCKADDR);
						SOCKET newSock = accept(servSock, (SOCKADDR*)&cliAddr[nextIndex], &addrLen);
						if (newSock != INVALID_SOCKET)
						{
							//设置发送和接收时限
							/*setsockopt(newSock, SOL_SOCKET, SO_SNDTIMEO, (const char*) & SEND_TIMEOUT, sizeof(SEND_TIMEOUT));
							setsockopt(newSock, SOL_SOCKET, SO_SNDTIMEO, (const char*) &RECV_TIMEOUT, sizeof(RECV_TIMEOUT));*/
							//给新客户端分配socket
							cliSock[nextIndex] = newSock;
							//新客户端的地址已经存在cliAddr[nextIndex]中了
							//为新客户端绑定事件对象,同时设置监听，close，read，write
							WSAEVENT newEvent = WSACreateEvent();
							WSAEventSelect(cliSock[nextIndex], newEvent, FD_CLOSE | FD_READ | FD_WRITE);
							cliEvent[nextIndex] = newEvent;
							total++;//客户端连接数增加
							cout << "#" << nextIndex << "游客（IP：" << inet_ntoa(cliAddr[nextIndex].sin_addr) << ")进入了聊天室，当前连接数：" << total << endl;

							//给所有客户端发送欢迎消息
							char buf[BUFFER_SIZE] = "[柳哥bot]欢迎游客（IP：";
							strcat(buf, inet_ntoa(cliAddr[nextIndex].sin_addr));
							strcat(buf, ")进入聊天室");
							for (int j = i; j <= total; j++)
							{

								send(cliSock[j], buf, sizeof(buf), 0);

							}
						}
					}

				}
				else if (networkEvents.lNetworkEvents & FD_CLOSE)//客户端被关闭，即断开连接
				{

					//i表示已关闭的客户端下标
					total--;
					cout << "#" << i << "游客（IP：" << inet_ntoa(cliAddr[i].sin_addr) << ")退出了聊天室,当前连接数：" << total << endl;
					//释放这个客户端的资源
					closesocket(cliSock[i]);
					WSACloseEvent(cliEvent[i]);

					//数组调整,用顺序表删除元素
					for (int j = i; j < total; j++)
					{
						cliSock[j] = cliSock[j + 1];
						cliEvent[j] = cliEvent[j + 1];
						cliAddr[j] = cliAddr[j + 1];
					}
					//给所有客户端发送退出聊天室的消息
					char buf[BUFFER_SIZE] = "[柳哥bot]（IP：";
					strcat(buf, inet_ntoa(cliAddr[i].sin_addr));
					strcat(buf, ")退出聊天室");
					for (int j = 1; j <= total; j++)
					{
						send(cliSock[j], buf, sizeof(buf), 0);

					}


				}
				else if (networkEvents.lNetworkEvents & FD_READ)//接收到消息
				{

					char buffer[BUFFER_SIZE] = { 0 };//字符缓冲区，用于接收和发送消息
					char buffer2[BUFFER_SIZE] = { 0 };

					for (int j = 1; j <= total; j++)
					{
						int nrecv = recv(cliSock[j], buffer, sizeof(buffer), 0);//nrecv是接收到的字节数
						if (nrecv > 0)//如果接收到的字符数大于0
						{
							sprintf(buffer2, "[#%d]%s", j, buffer);
							//在服务端显示
							cout << buffer2 << endl;
							//在其他客户端显示（广播给其他客户端）
							for (int k = 1; k <= total; k++)
							{
								send(cliSock[k], buffer2, sizeof(buffer), 0);
							}
						}

					}



				}
			}
		}


	}
	return 0;
}