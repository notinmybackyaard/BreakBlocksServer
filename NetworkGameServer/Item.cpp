#include "Item.h"


Item::Item() {}

Item::~Item() {}

void Item::SetType(ObjType tType) {
	if (tType == OBJ_ITEM1) {
		Object::SetType(OBJ_ITEM1);
	}
	else if (tType == OBJ_ITEM2) {
		Object::SetType(OBJ_ITEM2);
	}
}
