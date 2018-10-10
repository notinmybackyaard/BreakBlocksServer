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

// ���� <-> Ŭ�� ��Ŷ������ ����

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

class CFromServerData // �÷��̾� �� ������ �������� �����ϰ� Ŭ���̾�Ʈ�� �����ִ� Ŭ����
{
public:
	ServerPacketInfo			PlayerInfo[2];
};

class ClientPacketInfo {
public:
	Vec2		Pos;		// ����
	ObjType     Type;
	int			life;
};

class CFromClientData // �� Ŭ���̾�Ʈ���� �浹üũ �� ����ȭ�� ���Ͽ� ����� �� �Ѿ��� ���� ������ �Ѱ�  �� �� ���Ǵ� ����ü.
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


