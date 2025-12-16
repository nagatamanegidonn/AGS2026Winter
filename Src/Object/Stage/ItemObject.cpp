
#include "../Common/Transform.h"
#include "../Player/Player.h"

#include "../Utility/AsoUtility.h"

#include "ItemObject.h"

ItemObject::ItemObject(Player& player, const Transform& transform, STATE state, int itemId)
	: Object(player, transform, state)
{
	itemId_ = itemId;
}

ItemObject::~ItemObject(void)
{
}

void ItemObject::Init(void)
{
	transform_.Update();

	ChangeState(state_);
}

void ItemObject::Update(void)
{
	// ƒvƒŒƒCƒ„[‚Ì‰º‚É‚¢‚é‚©
	if (player_.CollisionUnderSphere(transform_.collider->pos_, transform_.collider->radius_))
	{
		player_.SetItemId(itemId_);
	}

}

void ItemObject::Draw(void)
{
	MV1DrawModel(transform_.modelId);

#ifdef _DEBUG
	DrawSphere3D(transform_.pos, transform_.collider->radius_, 10, 0xffffff, 0xffffff, false);
#endif // _DEBUG

}
