#include "Object.h"

Object::Object() {
	xy = Vec2(0, 0);
	Vxy = Vec2(0, 0);
	size = 0;
	color = Vec4(0.f, 0.f, 0.f, 1.f);
	life = 1;				// »ý¸í
}

Object::~Object() {

}


void Object::SetPosition(Vec2 xy1) {
	xy.x = xy1.x;
	xy.y = xy1.y;
}

void Object::SetVector(Vec2 Vxy1)
{
	Vxy.x = Vxy1.x;
	Vxy.y = Vxy1.y;
}

void Object::SetInfo(int life1, float size1, Vec4 color1) {
	life = life1;
	size = size1;
	color = color1;
}

void Object::SetType(ObjType tType) {
	type = tType;
}

Vec2 Object::GetPosition() {
	return xy;
}

Vec2 Object::GetVector() {
	return Vxy;
}

void Object::SetDead() {
	life = 0;
}

void Object::SetLife(int life1) {
	life = life1;
}

bool Object::IsDead() {
	if (life < 1)
		return TRUE;
	else
		return FALSE;
}

ObjType Object::GetType() {
	return type;
}