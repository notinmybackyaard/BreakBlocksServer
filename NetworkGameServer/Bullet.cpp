#include "Bullet.h"


Bullet::Bullet() {}
Bullet::~Bullet() {}

void Bullet::SetType(ObjType tType) {
	if (tType == OBJ_BULLET1) {
		Object::SetType(OBJ_BULLET1);
	}
	else if (tType == OBJ_BULLET2) {
		Object::SetType(OBJ_BULLET2);
	}
	else if (tType == OBJ_BULLET3) {
		Object::SetType(OBJ_BULLET3);
	}
}

