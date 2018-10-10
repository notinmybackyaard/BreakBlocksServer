#pragma once
#include "Define.h"
#include "Object.h"

class Block : public Object {
public:
	Block();
	virtual ~Block();

private:

public:
	virtual void SetType(ObjType tType);
	virtual void update(float elapsedTime) {};
};