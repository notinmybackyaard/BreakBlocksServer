#pragma once
#include "Define.h"
#include "Object.h"

class Item : public Object
{
public:
	Item();
	virtual ~Item();

private:

public:
	virtual void SetType(ObjType tType);
	virtual void update(float elapsedTime) {};
};