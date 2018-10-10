#pragma once

class Vec2
{
public:
	float x;
	float y;

	Vec2() {}

	Vec2(float _x, float _y)
		: x(_x), y(_y) {}
};

class Vec4
{
public:
	float x;
	float y;
	float z;
	float w;

	Vec4() {}

	Vec4(float _x, float _y, float _z, float _w)
		: x(_x), y(_y), z(_z), w(_w) {}
};

class ObjInfo
{
public:
	Vec2      Pos;
	float      Size;
	Vec2      Dir;
	int		life;
	ObjType      Type;

	ObjInfo() {}

	ObjInfo(Vec2 _pos, float _size, Vec2 _dir, int _life, ObjType _objType)
		: Pos(_pos), Size(_size), Dir(_dir), life(_life), Type(_objType) { }
};

// 서버 <-> 클라간 패킷데이터 정의

class DataTypeInfo {
public:
	DataType dataType;
};

class StartPacket {
public:
	int playerNum;
};

class ServerPacketInfo {
public:
	Vec2		Pos;
	int			life;
	ObjType     Type;
};

class CFromServerData // 플레이어 별 정보를 서버에서 저장하고 클라이언트로 보내주는 클래스
{
public:
	ServerPacketInfo			PlayerInfo[2];
};

class ClientPacketInfo {
public:
	Vec2		Pos;		// 생명
	ObjType     Type;
	int			life;
};

class CFromClientData // 각 클라이언트에서 충돌체크 및 동기화를 위하여 사용자 및 총알의 정보 서버로 넘겨  줄 때 사용되는 구조체.
{
public:
	ClientPacketInfo				PlayerInfo;
};

class CBulletData {
public:
	ClientPacketInfo				BulletInfo;
};

class CBlockData {
public:
	ObjType     Type[9];
	ObjType		ItemType[9];

};

class CItemData {
public:
	int     ItemIndex;
	ObjType ItemType;
};


