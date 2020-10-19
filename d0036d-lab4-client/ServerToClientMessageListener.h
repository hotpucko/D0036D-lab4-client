#pragma once
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "structs.h"
#include <vector>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>


class ServerToClientMessageListener
{
public:
	~ServerToClientMessageListener();

	ServerToClientMessageListener(Client* inClient, std::vector<Client*> _clients, std::mutex *_mutex, std::vector<cltoguimsg>* buffer);

	void Listener();

	int Result() const { return iResult; }
	void Result(int val) { iResult = val; }

	SOCKET ConnectSocket;
private:
	int iResult;
	Client* localClient;
	std::vector<Client*> clients;


	std::vector<cltoguimsg>* buffer;
	std::mutex* cl_gui_mutex;
};
