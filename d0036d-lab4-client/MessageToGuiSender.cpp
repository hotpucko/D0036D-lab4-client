#include "MessageToGuiSender.h"
#include "GuiToClientMessageListener.h"

MessageToGuiSender::MessageToGuiSender()
{
}

MessageToGuiSender::MessageToGuiSender(SOCKET connectsocket, Client* localClient, std::mutex *_mutex, std::vector<cltoguimsg>* _buffer)
{
	this->ConnectSocket = connectsocket;
	this->client = localClient;
	this->buffer = _buffer;
	this->cl_gui_mutex = _mutex;

}

MessageToGuiSender::~MessageToGuiSender()
{
}

void MessageToGuiSender::sendNewPosition(int id, Coordinate pos, Coordinate dir)
{
	std::string data = "move:" + std::to_string(id) + ":" + std::to_string(pos.x) + ":" + std::to_string(pos.y) + ":";
	GuiToClientMessageListener::SendMessageW(data);
}

void MessageToGuiSender::sendMove(int id, Coordinate dir)
{
	std::cout << "Undefined function MessageToGuSender::sendMove called." << std::endl;
}

void MessageToGuiSender::sendLeave(int id)
{
	std::string data = "leave:" + std::to_string(id) + ":";
	GuiToClientMessageListener::SendMessageW(data);
}

void MessageToGuiSender::sendJoin(int id)
{
	std::string data = "join:" + std::to_string(id) + ":";
	GuiToClientMessageListener::SendMessageW(data);
}

void MessageToGuiSender::sendTextMessage()
{
}

void MessageToGuiSender::InputListener()
{
	while (true)
	{
		while (buffer->size() > 0)
		{
			const std::lock_guard<std::mutex> lock(*cl_gui_mutex); //mutex lock
			cltoguimsg msg = buffer->at(0);
			bool flag = true;

			if (msg.type == Join)
			{
				sendJoin(msg.id);
			}
			else if (msg.type == Leave)
			{
				sendLeave(msg.id);
			}
			else if (msg.type == Change)
			{
				bool flag = true;
				if (msg.dir.x != NULL)
				{
					if (msg.pos.x != NULL)
					{
						sendNewPosition(msg.id, msg.pos, msg.dir);
						flag = false;
					}
					if (flag)
					{
						sendMove(msg.id, msg.dir);
					}
				}
			}
			//delete& buffer[0];
			buffer->erase(buffer->begin());
		}
	}
}
