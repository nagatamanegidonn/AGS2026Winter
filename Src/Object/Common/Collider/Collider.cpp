#include "Collider.h"



Collider::Collider(TYPE type, int modelId, VECTOR pos, float rad)
	:
	type_(type),
	modelId_(modelId),
	pos_(pos),
	radius_(rad)
{
}

Collider::~Collider(void)
{
}
