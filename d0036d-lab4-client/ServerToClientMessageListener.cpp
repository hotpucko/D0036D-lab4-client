#include "ServerToClientMessageListener.h"


#pragma comment(lib,"ws2_32.lib") //Winsock Library

constexpr auto DEFAULT_PORT = "49152";
constexpr auto DEFAULT_BUFLEN = 1024;

ServerToClientMessageListener::ServerToClientMessageListener(Client* inClient, std::vector<Client*> _clients, std::mutex *_mutex, std::vector<cltoguimsg>* _buffer) : localClient(inClient)
{
	this->buffer = _buffer;
	this->cl_gui_mutex = _mutex;
	clients = _clients;

	WSAData wsaData;
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return;
	}

	struct addrinfo* result = NULL, * ptr = NULL, infoa;
	ZeroMemory(&infoa, sizeof(infoa));
	infoa.ai_family = AF_UNSPEC;
	infoa.ai_socktype = SOCK_STREAM;
	infoa.ai_protocol = IPPROTO_TCP;

	std::string sargv = "130.240.40.6";
	const char* argv = sargv.c_str();

	iResult = getaddrinfo(argv, DEFAULT_PORT, &infoa, &result);
	if (iResult != 0)
	{
		printf("Getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return;
	}

	ConnectSocket = INVALID_SOCKET;

	ptr = result;

	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);	

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}


	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server!");
		WSACleanup();
		return;
	}
}

void ServerToClientMessageListener::Listener()
{
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];

	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			printf("Bytes received: %d\n", iResult);
			MsgHead* msghead;
			msghead = (MsgHead*)recvbuf;
			std::cout << "id " << msghead->id << " length " << msghead->length << " seq " << msghead->seq_no << std::endl;
			//continue;
			this->localClient->seq_nr = msghead->seq_no;

			if (msghead->type == Join)
			{
				std::cout << "Join message\n";
				
				JoinMsg* msgType;
				msgType = (JoinMsg*)recvbuf;

				if (this->localClient->client_id == -1)
				{
					std::cout << "Joined Server.\n";
					this->localClient->client_id = msghead->id;

					//
					std::lock_guard<std::mutex> lock(*cl_gui_mutex);
					cltoguimsg msg;
					msg.id = this->localClient->client_id;
					msg.type = Join;
					buffer->push_back(msg);
				}
			}
			else if (msghead->type == Change)
			{
				ChangeMsg* msgType;
				msgType = (ChangeMsg*)recvbuf;
				std::cout << "Received change message " << ChangeTypeToString(msgType->type) << std::endl;
				if (msgType->type == NewPlayer)
				{
					NewPlayerMsg* newPlayer;
					newPlayer = (NewPlayerMsg*)recvbuf;

					if (msghead->id != localClient->client_id)
					{
						bool flag = true;
						for (int i = 0; i < clients.size(); i++)
						{
							if (clients[i]->client_id == msghead->id)
							{
								flag = false;
							}
						}
						if (flag)
						{
							std::cout << "Opponent Joined. id" << msghead->id << std::endl;
							Client* c = new Client;
							c->client_id = msghead->id;
							clients.push_back(c);

							//
							std::lock_guard<std::mutex> lock(*cl_gui_mutex);
							cltoguimsg msg;
							msg.id = c->client_id;
							msg.type = Join;
							buffer->push_back(msg);
							continue;
						}
					}
				}
				else if (msgType->type == NewPlayerPosition)
				{
					NewPlayerPositionMsg* newPlayerPositionMsg;
					newPlayerPositionMsg = (NewPlayerPositionMsg*)recvbuf;
					std::cout << "x:" << newPlayerPositionMsg->pos.x << " y: " << newPlayerPositionMsg->pos.y << std::endl;
					std::cout << "long id boi: " << newPlayerPositionMsg->msg.head.id << std::endl;
					if (newPlayerPositionMsg->msg.head.id == localClient->client_id)
					{
						localClient->pos.x = newPlayerPositionMsg->pos.x;
						localClient->pos.y = newPlayerPositionMsg->pos.y;
						std::cout << "set localclient position" << std::endl;

						//
						std::lock_guard<std::mutex> lock(*cl_gui_mutex);
						cltoguimsg msg;
						msg.id = this->localClient->client_id;
						msg.type = Change;
						msg.pos = localClient->pos;
						msg.dir = localClient->dir;
						buffer->push_back(msg);
						continue;
					}
					for (int i = 0; i < clients.size(); i++)
					{
						if (newPlayerPositionMsg->msg.head.id == clients[i]->client_id)
						{
							std::cout << "set opponent id: " << clients[i]->client_id <<" position to x:" << newPlayerPositionMsg->pos.x << " y: " << newPlayerPositionMsg->pos.y << std::endl;
							clients[i]->pos = newPlayerPositionMsg->pos;
							clients[i]->dir = newPlayerPositionMsg->dir;

							//
							std::lock_guard<std::mutex> lock(*cl_gui_mutex);
							cltoguimsg msg;
							msg.id = clients[i]->client_id;
							msg.type = Change;
							msg.pos = clients[i]->pos;
							msg.dir = clients[i]->dir;
							buffer->push_back(msg);
						}
					}
				}
			}
			else if (msghead->type == Event)
			{
				std::cout << "Event message\n";
			}
			else if (msghead->type == Leave)
			{
				std::cout << "Leave message" << std::endl;
				std::cout << "Player ID:" << msghead->id << " has left the game" << std::endl;

				for (int i = 0; i < clients.size(); i++)
				{
					if (clients[i]->client_id == msghead->id)
					{
						std::cout << "Removed client: " << clients[i]->client_id << " from the client." << std::endl;

						//
						std::lock_guard<std::mutex> lock(*cl_gui_mutex);
						cltoguimsg msg;
						msg.id = clients[i]->client_id;
						msg.type = Leave;
						buffer->push_back(msg);
						//

						delete clients[i];
						clients[i] = NULL;
						clients.erase(clients.begin() + i);
						break;
					}
				}
			}
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());
	} while (iResult > 0);
}

ServerToClientMessageListener::~ServerToClientMessageListener()
{
	for (int i = clients.size() - 1; i >= 0; i--)
	{
		delete clients[i];
		clients[i] = NULL;
		clients.erase(clients.end());
	}
}
