#include "ItemBase.h"
#include "ItemPoach.h"

ItemPoach::ItemPoach()
{
}

ItemPoach::~ItemPoach()
{
}

void ItemPoach::AddItem(int itemId)
{
	
	auto it = Items_.find(itemId);
	if (it != Items_.end()) {
		// ë∂ç›Ç∑ÇÈ
		Items_[itemId]->Count(1);
	}
	else {
		// ë∂ç›ÇµÇ»Ç¢
		auto item = std::make_shared<ItemBase>();
		Items_.emplace(itemId, std::move(item));
	}
}

void ItemPoach::PlayItem(int itemId)
{
	auto it = Items_.find(itemId);
	if (it != Items_.end()) {
		// ë∂ç›Ç∑ÇÈ
		Items_[itemId]->Count(1);
		if (Items_[itemId]->GetCount() <= 0)
		{
			Items_[itemId].reset();
		}
	}
}
