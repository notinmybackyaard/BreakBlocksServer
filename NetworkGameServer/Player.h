#pragma once
#pragma once
#include "Define.h"
#include "Object.h"

class Player : public Object
{
public:
	Player();
	virtual ~Player();

private:
	//string ID;

public:
	//void SetID(string id);
	virtual void SetType(ObjType tType) {};
	virtual void update(float elapsedTime) {};
};