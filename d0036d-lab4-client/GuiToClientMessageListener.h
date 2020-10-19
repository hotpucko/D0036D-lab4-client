#pragma once
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "structs.h"
#include <vector>
#include <iostream>
#include <mutex>


class GuiToClientMessageListener
{
public:
	~GuiToClientMessageListener();

	GuiToClientMessageListener(std::mutex *_mutex, std::vector<guitoservermsg>* _buffer);

	void Listener();

	int Result() const { return iResult; }
	void Result(int val) { iResult = val; }

	SOCKET ConnectSocket;

	static void SendMessage(std::string data);
private:
	int iResult;
	std::string convertToString(char*, int);
	std::vector<std::string> split(std::string s, std::string delimiter);

	std::vector<guitoservermsg>* buffer;
	std::mutex *cl_server_mutex;
	
};
