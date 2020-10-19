#pragma once

#define MAXNAMELEN 32
enum ObjectDesc
{
	Human,
	NonHuman,
	Vehicle,
	StaticObject
};
enum ObjectForm
{
	Cube,
	Sphere,
	Pyramid,
	Cone
};
struct Coordinate
{
	int x;
	int y;
};

enum MsgType
{
	Join, // Client joining game at server
	Leave, // Client leaving game
	Change, // Information to clients
	Event, // Information from clients to server
	TextMessage // Send text messages to one or all
};
// Included first in all messages
struct MsgHead
{
	unsigned int length; // Total length for whole message
	unsigned int seq_no; // Sequence number
	unsigned int id; // Client ID or 0;
	MsgType type; // Type of message
};

struct JoinMsg
{
	MsgHead head;
	ObjectDesc desc;
	ObjectForm form;
	char name[MAXNAMELEN]; // nullterminated!, or empty
};

struct LeaveMsg
{
	MsgHead head;
};

enum ChangeType
{
	NewPlayer,
	PlayerLeave,
	NewPlayerPosition
};

inline const char* ChangeTypeToString(ChangeType v)
{
	switch (v)
	{
	case NewPlayer:   return "NewPlayer";
	case PlayerLeave:   return "PlayerLeave";
	case NewPlayerPosition: return "NewPlayerPosition";
	default:      return "[Unknown changeType]";
	}
}

// Included first in all Change messages
struct ChangeMsg
{
	MsgHead head;
	ChangeType type;
};
struct NewPlayerMsg
{
	ChangeMsg msg; //Change message header with new client id
	ObjectDesc desc;
	ObjectForm form;
	char name[MAXNAMELEN]; // nullterminated!, or empty
};
struct PlayerLeaveMsg
{
	ChangeMsg msg; //Change message header with new client id
};
struct NewPlayerPositionMsg
{
	ChangeMsg msg; //Change message header
	Coordinate pos; //New object position
	Coordinate dir; //New object direction
};

enum EventType
{
	Move
};
// Included first in all Event messages
struct EventMsg
{
	MsgHead head;
	EventType type;
};
struct MoveEvent
{
	EventMsg event;
	Coordinate pos; //New object position
	Coordinate dir; //New object direction
};

struct TextMessageMsg // Optional at client side!!!
{
	MsgHead head;
	char text[1]; // NULL-terminerad array av chars.
};

struct Client //Client info
{
	unsigned int seq_nr = -1;
	int client_id = -1;
	Coordinate pos;
	Coordinate dir;
};

struct cltoguimsg
{
	int id;
	Coordinate pos;
	Coordinate dir;
	MsgType type;
};

struct guitoservermsg
{
	Client client;
	MsgType type;
};