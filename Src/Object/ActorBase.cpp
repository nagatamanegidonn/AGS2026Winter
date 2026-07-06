#include "ActorBase.h"

ActorBase::ActorBase(void)
{
}

ActorBase::~ActorBase(void)
{
}

void ActorBase::DrawShadow(void)
{
	MV1DrawModel(transform_.modelId);
}

const Transform& ActorBase::GetTransform(void) const
{
	return transform_;
}
