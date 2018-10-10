#pragma once
#include "Define.h"
class Object
{
public:
	Object();
	virtual ~Object();

public:
	Vec2 xy;
	Vec2 Vxy;
	float size;
	Vec4 color;
	int life;				// »ý¸í
	ObjType type;

public:
	void SetPosition(Vec2 xy1);
	void SetVector(Vec2 Vxy1);
	void SetInfo(int life, float size, Vec4 color);
	void SetDead();
	void SetLife(int life);

public:
	Vec2 GetPosition();
	Vec2 GetVector();
	ObjInfo GetObjInfo();
	bool IsDead();
	ObjType GetType();

	virtual void SetType(ObjType tType);
	virtual void update(float elapsedTime) = 0;
};