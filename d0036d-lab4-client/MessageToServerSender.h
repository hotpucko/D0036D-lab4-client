#pragma once
#include "structs.h"
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>
#include <mutex>
#include <vector>

class MessageToServerSender
{
public:
	MessageToServerSender();
	MessageToServerSender(SOCKET connectsocket, Client* localClient, std::mutex *_mutex, std::vector<guitoservermsg>* _buffer);
	~MessageToServerSender();
	void InputListener();

private:
	void sendJoin();
	void sendNewPosition(Coordinate pos);
	void sendMove(std::string dir);
	void SendIllegalMove();
	void sendLeave();
	void sendTextMessage();
	void join();


	SOCKET ConnectSocket;
	int iResult;

	Client* client;

	std::vector<guitoservermsg>* buffer;
	std::mutex* cl_server_mutex;
};