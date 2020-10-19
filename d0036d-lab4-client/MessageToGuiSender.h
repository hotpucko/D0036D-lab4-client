#pragma once
#include "structs.h"
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <mutex>

class MessageToGuiSender
{
public:
	MessageToGuiSender();
	MessageToGuiSender(SOCKET connectsocket, Client* localClient, std::mutex *_mutex, std::vector<cltoguimsg>* _buffer);
	~MessageToGuiSender();

	void InputListener();

private:
	void sendNewPosition(int id, Coordinate pos, Coordinate dir);
	void sendMove(int id, Coordinate dir);
	void sendLeave(int id);
	void sendJoin(int id);
	void sendTextMessage();

	SOCKET ConnectSocket;
	int iResult;

	Client* client;

	std::vector<cltoguimsg> *buffer;
	std::mutex *cl_gui_mutex;
};

