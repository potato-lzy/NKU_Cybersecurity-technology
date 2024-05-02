// �������ͻ���
#include<iostream>
#include<Winsock2.h>//socketͷ�ļ�
#include<cstring>
#include"des.h"

using namespace std;


//����ϵͳ�ṩ��socket��̬���ӿ�
#pragma warning(disable: 4996)
#pragma comment(lib,"ws2_32.lib")   //socket��

const int BUFFER_SIZE = 1024;//��������С

DWORD WINAPI recvMsgThread(LPVOID IpParameter);

int main() {
	//1����ʼ��socket��
	WSADATA wsaData;//��ȡ�汾��Ϣ��˵��Ҫʹ�õİ汾
	WSAStartup(MAKEWORD(2, 2), &wsaData);//MAKEWORD(���汾��, ���汾��)

	//2������socket
	SOCKET cliSock = socket(AF_INET, SOCK_STREAM, 0);//������·����ʽ�׽���,���������������Զ�ѡ��Э��

	//3�������ַ
	//�ͻ���
	SOCKADDR_IN cliAddr = { 0 };
	cliAddr.sin_family = AF_INET;
	cliAddr.sin_addr.s_addr = inet_addr("127.0.0.1");//IP��ַ
	cliAddr.sin_port = htons(8888);//�˿ں�
	//�����
	SOCKADDR_IN servAddr = { 0 };
	servAddr.sin_family = AF_INET;//�ͷ�������socketһ����sin_family��ʾЭ��أ�һ����AF_INET��ʾTCP/IPЭ�顣
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//����˵�ַ����Ϊ���ػػ���ַ
	servAddr.sin_port = htons(9999);//host to net short �˿ں�����Ϊ12345

	bind(cliSock, (SOCKADDR*)&cliAddr, sizeof(cliAddr));

	if (connect(cliSock, (SOCKADDR*)&servAddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		cout << "���ӳ��ִ��󣬴������" << WSAGetLastError() << endl;
	}

	//����������Ϣ�߳�
	CloseHandle(CreateThread(NULL, 0, recvMsgThread, (LPVOID)&cliSock, 0, 0));
	//���߳���������Ҫ���͵���Ϣ
	while (1)
	{
		char buf[BUFFER_SIZE] = { 0 };
		cin.getline(buf, sizeof(buf));
		if (strcmp(buf, "quit") == 0)//�����롰quit�������˳�������
		{
			break;
		}
		send(cliSock, buf, sizeof(buf), 0);
	}
	closesocket(cliSock);
	WSACleanup();
	return 0;
}

DWORD WINAPI recvMsgThread(LPVOID IpParameter)//������Ϣ���߳�
{
	SOCKET cliSock = *(SOCKET*)IpParameter;//��ȡ�ͻ��˵�SOCKET����

	while (1)
	{
		char buffer[BUFFER_SIZE] = { 0 };//�ַ������������ڽ��պͷ�����Ϣ
		int nrecv = recv(cliSock, buffer, sizeof(buffer), 0);//nrecv�ǽ��յ����ֽ���
		if (nrecv > 0)//������յ����ַ�������0
		{
			des_decry(buffer);
			cout << MingWen << endl;
		}
		else if (nrecv < 0)//������յ����ַ���С��0��˵���Ͽ�����
		{
			cout << "��������Ͽ�����" << endl;
			break;
		}
	}
	return 0;
}