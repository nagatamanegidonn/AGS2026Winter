#include "ActorBase.h"

ActorBase::ActorBase(void)
{
}

ActorBase::~ActorBase(void)
{
}

const Transform& ActorBase::GetTransform(void) const
{
	return transform_;
}
