#include<iostream>
#include<Winsock2.h>//socketͷ�ļ�
#include<cstring>
#include"des.h"

using namespace std;

//����ϵͳ�ṩ��socket��̬���ӿ�
#pragma warning(disable: 4996)
#pragma comment(lib,"ws2_32.lib")   //socket��

//==============================ȫ�ֱ�����===================================
const int BUFFER_SIZE = 1024;//��������С
int RECV_TIMEOUT = 10;//������Ϣ��ʱ
int SEND_TIMEOUT = 10;//������Ϣ��ʱ
const int WAIT_TIME = 10;//ÿ���ͻ��˵ȴ��¼���ʱ�䣬��λ����
const int MAX_LINK_NUM = 10;//��������������
SOCKET cliSock[MAX_LINK_NUM];//�ͻ����׽��� 0��Ϊ�����
SOCKADDR_IN cliAddr[MAX_LINK_NUM];//�ͻ��˵�ַ
WSAEVENT cliEvent[MAX_LINK_NUM];//�ͻ����¼� 0��Ϊ�����,�������ó����һ���ֵȴ�������һ���ֵ��źš����磬�����ݴ��׽��ֱ�Ϊ����ʱ��winsock ��Ὣ�¼�����Ϊ�ź�״̬
int total = 0;//��ǰ�Ѿ����ӵĿͷ��˷�����


//==============================��������===================================
DWORD WINAPI servEventThread(LPVOID IpParameter);//�������˴����߳�


int main()
{
	//1����ʼ��socket��
	WSADATA wsaData;//��ȡ�汾��Ϣ��˵��Ҫʹ�õİ汾
	WSAStartup(MAKEWORD(2, 2), &wsaData);//MAKEWORD(���汾��, ���汾��)
	
	//2������socket
	SOCKET servSock = socket(AF_INET, SOCK_STREAM, 0);//������·����ʽ�׽���

	//3������������ַ�����һ���ṹ������
	SOCKADDR_IN servAddr; //sockaddr_in ��internet�������׽��ֵĵ�ַ��ʽ
	servAddr.sin_family = AF_INET;//�ͷ�������socketһ����sin_family��ʾЭ��أ�һ����AF_INET��ʾTCP/IPЭ�顣
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//����˵�ַ����Ϊ���ػػ���ַ
	servAddr.sin_port = htons(9999);//host to net short �˿ں�����Ϊ12345


	//4���󶨷���˵�socket�ʹ���õĵ�ַ
	bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr));

	//4.5�������sokect��һ���¼������������տͻ������ӵ��¼�
	WSAEVENT servEvent = WSACreateEvent();//����һ���˹�����Ϊ���ŵ��¼�����
	WSAEventSelect(servSock, servEvent, FD_ALL_EVENTS);//���¼����󣬲��Ҽ��������¼�

	cliSock[0] = servSock;
	cliEvent[0] = servEvent;

	//5����������
	listen(servSock, 10);//�������г���Ϊ10

	//6�������������ӵ��߳�
	//����Ҫ�������ֱ�ӹر�
	CloseHandle(CreateThread(NULL, 0, servEventThread, (LPVOID)&servSock, 0, 0));

	cout << "�����ҷ������ѿ���" << endl;
	connect;
		//int addrLen = sizeof(SOCKADDR);//���ڽ��տͻ��˵ĵ�ַ���ṹ�峤��
		//SOCKET cliSOCK = accept(servSock, (SOCKADDR*)&servAddr,&addrLen);
		//if (cliSOCK != INVALID_SOCKET) 
		//{
		//	cout << "���ӳɹ�" << endl;
		//}
		//while (1)
		//{
		//	char buf[100] = { 0 };//���Ի�����
		//	int nrecv = recv(cliSOCK, buf, sizeof(buf), 0);
		//	if (nrecv > 0)//������յ��ͻ��˵���Ϣ���������Ļ
		//	{
		//		cout << buf << endl;
		//	}
		//}
		//��Ҫ�����߳�һֱ������ȥ
		//������Ϣ��ȫ���ͻ���
		while (1)
		{

			char contentBuf[BUFFER_SIZE] = { 0 };
			char sendBuf[BUFFER_SIZE] = { 0 };
			cin.getline(contentBuf, sizeof(contentBuf));
			sprintf(sendBuf, "[����bot:]%s", contentBuf);
			for (int j = 1; j <= total; j++)
			{
				send(cliSock[j], sendBuf, sizeof(sendBuf), 0);
			}

		}
	//1-�ر�socket�����β����
	WSACleanup();
	return 0;
}

DWORD WINAPI servEventThread(LPVOID IpParameter) //���������߳�
{
	//���̸߳��������˺͸����ͻ��˷������¼�
	//������Ĳ�����ʼ��
	SOCKET servSock = *(SOCKET*)IpParameter;//LPVOIDΪ��ָ�����ͣ���Ҫ��ת��SOCKET���������ã�����ʹ�ô����SOCKET
	while (1) //��ִͣ��
	{
		for (int i = 0; i < total + 1; i++)//i�����������ڼ����¼����ն�
		{
			//����һ���ͻ������ӣ�total==1��ѭ�����Σ������ͻ��˺ͷ����
			//��ÿһ���նˣ��ͻ��˺ͷ���ˣ����鿴�Ƿ����¼����ȴ�WAIT_TIME����
			int index = WSAWaitForMultipleEvents(1, &cliEvent[i], false, WAIT_TIME, 0);

			index -= WSA_WAIT_EVENT_0;//��ʱindexΪ�����¼����ն��±�

			if (index == WSA_WAIT_TIMEOUT || index == WSA_WAIT_FAILED)
			{
				continue;//���������߳�ʱ�����������ն�
			}

			else if (index == 0)
			{
				WSANETWORKEVENTS networkEvents;
				WSAEnumNetworkEvents(cliSock[i], cliEvent[i], &networkEvents);//�鿴��ʲô�¼�

				//�¼�ѡ��
				if (networkEvents.lNetworkEvents & FD_ACCEPT)//������accept�¼����˴���λ�������룩
				{
					
					if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
					{
						cout << "����ʱ�������󣬴������" << networkEvents.iErrorCode[FD_ACCEPT_BIT] << endl;
						continue;

					}
					//��������
					if (total + 1 < MAX_LINK_NUM)//������һ���ͻ�����ȻС�����������������ܸ�����
					{
						//totalΪ�����ӿͻ�������
						int nextIndex = total + 1;//������¿ͻ��˵��±�
						int addrLen = sizeof(SOCKADDR);
						SOCKET newSock = accept(servSock, (SOCKADDR*)&cliAddr[nextIndex], &addrLen);
						if (newSock != INVALID_SOCKET)
						{
							//���÷��ͺͽ���ʱ��
							/*setsockopt(newSock, SOL_SOCKET, SO_SNDTIMEO, (const char*) & SEND_TIMEOUT, sizeof(SEND_TIMEOUT));
							setsockopt(newSock, SOL_SOCKET, SO_SNDTIMEO, (const char*) &RECV_TIMEOUT, sizeof(RECV_TIMEOUT));*/
							//���¿ͻ��˷���socket
							cliSock[nextIndex] = newSock;
							//�¿ͻ��˵ĵ�ַ�Ѿ�����cliAddr[nextIndex]����
							//Ϊ�¿ͻ��˰��¼�����,ͬʱ���ü�����close��read��write
							WSAEVENT newEvent = WSACreateEvent();
							WSAEventSelect(cliSock[nextIndex], newEvent, FD_CLOSE | FD_READ | FD_WRITE);
							cliEvent[nextIndex] = newEvent;
							total++;//�ͻ�������������
							cout << "#" << nextIndex << "�οͣ�IP��" << inet_ntoa(cliAddr[nextIndex].sin_addr) << ")�����������ң���ǰ��������" << total << endl;

							//�����пͻ��˷��ͻ�ӭ��Ϣ
							char buf[BUFFER_SIZE] = "[����bot]��ӭ�οͣ�IP��";
							strcat(buf, inet_ntoa(cliAddr[nextIndex].sin_addr));
							strcat(buf, ")����������");
							for (int j = i; j <= total; j++)
							{

								send(cliSock[j], buf, sizeof(buf), 0);

							}
						}
					}

				}
				else if (networkEvents.lNetworkEvents & FD_CLOSE)//�ͻ��˱��رգ����Ͽ�����
				{

					//i��ʾ�ѹرյĿͻ����±�
					total--;
					cout << "#" << i << "�οͣ�IP��" << inet_ntoa(cliAddr[i].sin_addr) << ")�˳���������,��ǰ��������" << total << endl;
					//�ͷ�����ͻ��˵���Դ
					closesocket(cliSock[i]);
					WSACloseEvent(cliEvent[i]);

					//�������,��˳���ɾ��Ԫ��
					for (int j = i; j < total; j++)
					{
						cliSock[j] = cliSock[j + 1];
						cliEvent[j] = cliEvent[j + 1];
						cliAddr[j] = cliAddr[j + 1];
					}
					//�����пͻ��˷����˳������ҵ���Ϣ
					char buf[BUFFER_SIZE] = "[����bot]��IP��";
					strcat(buf, inet_ntoa(cliAddr[i].sin_addr));
					strcat(buf, ")�˳�������");
					for (int j = 1; j <= total; j++)
					{
						send(cliSock[j], buf, sizeof(buf), 0);

					}


				}
				else if (networkEvents.lNetworkEvents & FD_READ)//���յ���Ϣ
				{

					char buffer[BUFFER_SIZE] = { 0 };//�ַ������������ڽ��պͷ�����Ϣ
					char buffer2[BUFFER_SIZE] = { 0 };

					for (int j = 1; j <= total; j++)
					{
						int nrecv = recv(cliSock[j], buffer, sizeof(buffer), 0);//nrecv�ǽ��յ����ֽ���
						if (nrecv > 0)//������յ����ַ�������0
						{
							sprintf(buffer2, "[#%d]%s", j, buffer);
							//�ڷ������ʾ
							cout << buffer2 << endl;
							//�������ͻ�����ʾ���㲥�������ͻ��ˣ�
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