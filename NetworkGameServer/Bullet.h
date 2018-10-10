#pragma once
#include "Define.h"
#include "Object.h"

class Bullet : public Object {
public:
	Bullet();
	virtual ~Bullet();

private:


public:
	virtual void SetType(ObjType tType);
	virtual void update(float elapsedTime) {};
};