// Laboration 4 Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <iostream>
#include "Structs.h"
#include "ServerToClientMessageListener.h"
#include "MessageToServerSender.h"
#include <vector>
#include "GuiToClientMessageListener.h"
#include "MessageToGuiSender.h"
#include <mutex>

int main()
{
	Client* localClient = new Client;
	unsigned int* seq_nr = 0;
	int* client_id = 0;
	std::vector<Client*> clients;

	std::mutex _cl_gui_mutex;
	std::mutex _cl_server_mutex;
	std::mutex* cl_gui_mutex = &_cl_gui_mutex;
	std::mutex* cl_server_mutex = &_cl_server_mutex;

	std::vector<cltoguimsg> _cltoguibuffer;
	std::vector<guitoservermsg> _guitoserverbuffer;
	std::vector<cltoguimsg>* cltoguibuffer = &_cltoguibuffer;
	std::vector<guitoservermsg>* guitoserverbuffer = &_guitoserverbuffer;


	ServerToClientMessageListener* serverToClientNetHandler = new ServerToClientMessageListener(localClient, clients, cl_gui_mutex, cltoguibuffer);
	GuiToClientMessageListener* guiToClientNetHandler = new GuiToClientMessageListener(cl_server_mutex, guitoserverbuffer);

	std::thread t1(&ServerToClientMessageListener::Listener, serverToClientNetHandler);
	t1.detach();
	std::thread t2(&MessageToServerSender::InputListener, MessageToServerSender(serverToClientNetHandler->ConnectSocket, localClient, cl_server_mutex, guitoserverbuffer));
	t2.detach();
	std::thread t3(&GuiToClientMessageListener::Listener, guiToClientNetHandler);
	t3.detach();
	std::thread t4(&MessageToGuiSender::InputListener, MessageToGuiSender(guiToClientNetHandler->ConnectSocket, localClient, cl_gui_mutex, cltoguibuffer));
	while (true)
	{
		//börja debugga vad som sker med olika meddelanden -> server
	}
}
