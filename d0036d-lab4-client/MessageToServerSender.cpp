#include "MessageToServerSender.h"
#include <iostream>
#include <string>
#include <sstream>

MessageToServerSender::MessageToServerSender(SOCKET connectsocket, Client* localClient, std::mutex *_mutex, std::vector<guitoservermsg>* _buffer) : client(localClient)
{
	this->cl_server_mutex = _mutex;
	this->buffer = _buffer;
	this->ConnectSocket = connectsocket;
	this->client = localClient;
}

MessageToServerSender::~MessageToServerSender()
{
}

void MessageToServerSender::join()
{
	this->sendJoin();

	//wait for client to get assigned an id from ClientToServerNetworkHandler
	while (client->client_id == -1)
	{
	}
	this->SendIllegalMove(); //Send an illegal move to receive our initial position, idk server kinda fucked


}

void MessageToServerSender::sendJoin()
{
	JoinMsg join;
	char sendbuf[sizeof(join)];

	//filling join msg
	join.head.type = Join;
	join.head.id = 0;
	join.head.seq_no = 0;
	join.head.length = sizeof(join);

	//join.desc = Human;
	//join.form = Cube;

	memcpy((void*)sendbuf, (void*)&join, sizeof(join));

	iResult = send(ConnectSocket, sendbuf, sizeof(sendbuf), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("Send Failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	printf("Bytes sent %ld\n", iResult);

}

void MessageToServerSender::sendNewPosition(Coordinate pos)
{
	MoveEvent move;
	char sendbuf[sizeof(move)];
	move.event.type = Move;
	move.event.head.id = client->client_id;
	std::cout << "client id: " << client->client_id << std::endl;
	move.event.head.length = sizeof(move);
	move.event.head.seq_no = client->seq_nr++;
	move.event.head.type = Event;

	move.pos = pos;

	memcpy((void*)sendbuf, (void*)&move, sizeof(move));

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, sizeof(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	printf("Bytes Sent: %ld\n", iResult);
}



void MessageToServerSender::sendMove(std::string dir)
{
	MoveEvent move;
	char sendbuf[sizeof(move)];
	move.event.type = Move;
	move.event.head.id = client->client_id;
	std::cout << "client id: " << client->client_id << std::endl;
	move.event.head.length = sizeof(move);
	move.event.head.seq_no = client->seq_nr++;
	move.event.head.type = Event;

	if (dir == "left")
	{
		Coordinate newPos;
		Coordinate newDir;
		newPos.x = client->pos.x - 1;
		newPos.y = client->pos.y;
		newDir.x = -1;
		newDir.y = 0;
		move.pos = newPos;
		move.dir = newDir;
	}
	else if (dir == "right")
	{
		Coordinate newPos;
		Coordinate newDir;
		newPos.x = client->pos.x + 1;
		newPos.y = client->pos.y;
		newDir.x = 1;
		newDir.y = 0;
		move.pos = newPos;
		move.dir = newDir;
	}

	memcpy((void*)sendbuf, (void*)&move, sizeof(move));

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, sizeof(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	printf("Bytes Sent: %ld\n", iResult);

}

void MessageToServerSender::SendIllegalMove()
{
	MoveEvent move;
	char sendbuf[sizeof(move)];
	move.event.type = Move;
	move.event.head.id = client->client_id;
	std::cout << "client id: " << client->client_id << std::endl;
	move.event.head.length = sizeof(move);
	move.event.head.seq_no = client->seq_nr++;
	move.event.head.type = Event;

	Coordinate newPos;
	Coordinate newDir;
	newPos.x = -200;//client->pos.x - 1;
	newPos.y = -200;//client->pos.y;
	newDir.x = 0;
	newDir.y = 0;
	move.pos = newPos;
	move.dir = newDir;

	memcpy((void*)sendbuf, (void*)&move, sizeof(move));

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, sizeof(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	printf("Bytes Sent: %ld\n", iResult);
}

void MessageToServerSender::sendLeave()
{
	LeaveMsg leave;

	char sendbuf[sizeof(leave)];

	//Filling leave msg
	leave.head.type = Leave;
	leave.head.id = client->client_id;
	leave.head.seq_no = client->seq_nr++;
	leave.head.length = sizeof(leave);

	memcpy((void*)sendbuf, (void*)&leave, sizeof(leave));

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, sizeof(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	printf("Bytes Sent: %ld\n", iResult);
}

void MessageToServerSender::sendTextMessage()
{
	//nej
}

void MessageToServerSender::InputListener()
{
	while (true)
	{
		while (buffer->size() > 0)
		{
			const std::lock_guard<std::mutex> lock(*cl_server_mutex); //mutex lock
			guitoservermsg msg = buffer->at(0);
			if (msg.type == Join)
			{
				std::cout << "received Join from Gui, in the buffer" << std::endl;
				join();//sendJoin();
			}
			else if (msg.type == Leave)
			{
				std::cout << "Received Leave from Gui, in the buffer" << std::endl;
				sendLeave();
			}
			else if (msg.type == Change)
			{
				std::cout << "Received Change from Gui, in the buffer, pos x: " << msg.client.pos.x << ", y: " << msg.client.pos.y << std::endl;
				Coordinate pos;
				pos.x = msg.client.pos.x;
				pos.y = msg.client.pos.y;
				sendNewPosition(pos);
			}

			buffer->erase(buffer->begin());
		}
	}
}
