#include <DxLib.h>
#include "../Manager/SceneManager.h"
#include "NetManager.h"
#include "NetBase.h"

NetBase::NetBase(NetManager& manager) : manager_(manager)
{
	step_ = 0.0f;
	stepAction_ = 0.0f;
}

NetBase::~NetBase(void)
{
}
