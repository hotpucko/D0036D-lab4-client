#include "GuiToClientMessageListener.h"
#include <sstream>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

constexpr auto DEFAULT_RECEIVE_PORT = 49152;
constexpr auto DEFAULT_SEND_PORT = 49153;
constexpr auto DEFAULT_BUFLEN = 1024;

GuiToClientMessageListener::~GuiToClientMessageListener()
{
}

static struct sockaddr_in server, si_other;
static int slen, recv_len;
static char buf[DEFAULT_BUFLEN];
static WSADATA wsa;
static SOCKET _socket;


#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)

BOOL bNewBehavior = FALSE;
DWORD dwBytesReturned = 0;

GuiToClientMessageListener::GuiToClientMessageListener(std::mutex *_mutex, std::vector<guitoservermsg>* _buffer)
{
	this->cl_server_mutex = _mutex;
	this->buffer = _buffer;

	slen = sizeof(si_other);

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((this->ConnectSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
#pragma warning(suppress : 4996)
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(DEFAULT_RECEIVE_PORT);
#pragma warning(suppress : 4996)
	si_other.sin_addr.s_addr = inet_addr("127.0.0.1");
	si_other.sin_port = htons(DEFAULT_SEND_PORT);

	//Bind
	if (bind(this->ConnectSocket, (struct sockaddr*) & server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");
	_socket = ConnectSocket;

	WSAIoctl(_socket, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
	//this->Listener();
}

void GuiToClientMessageListener::Listener()
{
	//int recvbuflen = DEFAULT_BUFLEN;
	//char recvbuf[DEFAULT_BUFLEN];

	int recvlen;


	while (true) {
		printf("Waiting for data...");
		fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', DEFAULT_BUFLEN);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(this->ConnectSocket, buf, DEFAULT_BUFLEN, 0, (struct sockaddr*) &si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//print details of the client/peer and the data received
#pragma warning(suppress : 4996)
		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("Data: %s\n", buf);


		std::string s = convertToString(buf, recv_len);
		std::vector<std::string> indata = split(s, ":");

		if (indata[0] == "join")
		{
			const std::lock_guard<std::mutex> lock(*cl_server_mutex);
			guitoservermsg msg;
			msg.type = Join;
			buffer->push_back(msg);
			std::cout << "Received Join from GUI" << std::endl;
		}
		else if (indata[0] == "leave")
		{
			const std::lock_guard<std::mutex> lock(*cl_server_mutex);
			guitoservermsg msg;
			msg.type = Leave;
			buffer->push_back(msg);
			std::cout << "Received Leave from GUI" << std::endl;
		}
		else if (indata[0] == "move")
		{
			const std::lock_guard<std::mutex> lock(*cl_server_mutex);
			guitoservermsg msg;
			msg.type = Change;
			msg.client.pos.x = stoi(indata[1]);
			msg.client.pos.y = stoi(indata[2]);
			buffer->push_back(msg);
			std::cout << "received Move from GUI with x: " << indata[1] << " & y: " << indata[2] << std::endl;
		}
	}


	closesocket(this->ConnectSocket);
	WSACleanup();
}

void GuiToClientMessageListener::SendMessage(std::string data)
{
	if (_socket != NULL)
	{
		const char* sendbuf = data.c_str();
		std::cout << "wow wend data: " + data;
#pragma warning(suppress : 4996)
		si_other.sin_addr.s_addr = inet_addr("127.0.0.1");
		si_other.sin_port = htons(DEFAULT_SEND_PORT);
		if (sendto(_socket, sendbuf, strlen(sendbuf), 0, (struct sockaddr*) & si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
	}	
}

std::string GuiToClientMessageListener::convertToString(char* a, int size)
{
	int i;
	std::string s = "";
	for (i = 0; i < size; i++) {
		if (a[i] == '\n')
			return s;
		s = s + a[i];
	}
	return s;
}

std::vector<std::string> GuiToClientMessageListener::split(std::string s, std::string delimiter)
{
	size_t pos = 0;
	std::string token;
	std::vector<std::string> returnValue;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		returnValue.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	if (returnValue.size() == 0)
		returnValue.push_back(s);
	return returnValue;
}
