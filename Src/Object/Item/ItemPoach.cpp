#include "ItemBase.h"
#include "ItemPoach.h"
#include "../../Application.h"
#include <DxLib.h>
#include <typeindex>
#include <algorithm> // 追加

ItemPoach::ItemPoach()
	:
	selectIndex_(0),
	ItemList_()
{
}

ItemPoach::~ItemPoach()
{
}

void ItemPoach::Draw(int i)
{
	if (ItemList_.empty()) return;

	// 現在の選択アイテムを取得
	auto it = ItemList_.begin();
	std::advance(it, selectIndex_);

	if (it != ItemList_.end()) {
		(*it)->Draw();
		DrawFormatString(Application::SCREEN_SIZE_X - 200, Application::SCREEN_SIZE_Y - 80,
			0x0000ff, L"選択中のID=%d", selectIndex_);
	}

#ifdef _DEBUG
	int py = 400;
	for (auto& item : ItemList_)
	{
		DrawFormatString(800, py, 0x000000, L"アイテム名=%s", item->GetName().c_str());
		DrawFormatString(810, py + 16, 0x000000, L"アイテム数=%d", item->GetCount());
		py += 32;
	}
#endif // _DEBUG

}

// ここもintではなくItemBaseを引数にする
void ItemPoach::AddItem(std::shared_ptr<ItemBase> _item)
{
	if (!_item) return;

	// 同名アイテムを検索
	auto it = std::find_if(ItemList_.begin(), ItemList_.end(),
		[&](const std::shared_ptr<ItemBase>& i) {
			return i->IsSameName(*_item);
		});

	if (it != ItemList_.end()) {
		// 既にある → 内容量追加
		(*it)->Count(1);
	}
	else {
		// 新規追加
		_item->Init(static_cast<int>(ItemList_.size()));
		ItemList_.push_back(_item);
	}
}

// 選択中のアイテムを使用するメソッド
void ItemPoach::UseSelectedItem(void)
{
	// アイテムが無ければ終了
	if (ItemList_.empty()) return;
	// 現在の選択アイテムを取得
	auto it = ItemList_.begin();
	std::advance(it, selectIndex_);

	if (it == ItemList_.end()) return;

	auto& item = *it;
	item->Count(-1);

	// アイテム数が0以下なら削除処理
	if (item->GetCount() <= 0) {
		// 削除
		it = ItemList_.erase(it);

		// アイテムのId調整
		int id = 0;
		for (auto& itm : ItemList_) {
			itm->Init(id++);
		}

		// 要素が削除されてリストが空になった場合
		if (ItemList_.empty()) {
			selectIndex_ = 0;
			return;
		}

		// 次の要素が存在しない場合は先頭へ戻る
		if (it == ItemList_.end()) {
			selectIndex_ = 0;
		}
		else if (selectIndex_ >= static_cast<int>(ItemList_.size())) {
			selectIndex_ = static_cast<int>(ItemList_.size()) - 1;
		}
	}
}

//選択中のアイテムIDを次に進めるメソッド
void ItemPoach::NextItem(void)
{
	if (ItemList_.empty()) return;
	selectIndex_ = (selectIndex_ + 1) % ItemList_.size();
}

//ポーチに選択中のアイテムがあるかどうか
bool ItemPoach::HasSelectedItem(void) const
{
	// アイテムが無ければ終了
	if (ItemList_.empty()) return false;

	// 現在の選択アイテムを取得
	auto it = ItemList_.begin();
	// イテレータをselectIndex_分進める
	std::advance(it, selectIndex_);
	return (it != ItemList_.end());
	/*for (const auto& item : ItemList_)
	{
		if (item->GetId() == selectIndex_)
		{
			return true;
		}
	}
	return false;*/
}

//選択中のアイテムが引数の名前かどうか
bool ItemPoach::IsSelectedItemName(const std::wstring& _name) const
{
	// アイテムが無ければ終了
	if (ItemList_.empty()) return false;

	// 現在の選択アイテムを取得
	auto it = ItemList_.begin();
	// イテレータをselectIndex_分進める
	std::advance(it, selectIndex_);
	return ((*it)->GetName() == _name);
	/*for (const auto& item : ItemList_)
	{
		if (item->GetId() == selectIndex_ && item->GetName() == name)
		{
			return true;
		}
	}
	return false;*/
}

bool ItemPoach::IsSelectedItem(ItemBase _item) const
{
	// アイテムが無ければ終了
	if (ItemList_.empty()) return false;
	// 現在の選択アイテムを取得
	auto it = ItemList_.begin();
	// イテレータをselectIndex_分進める
	std::advance(it, selectIndex_);
	// 名前と同じかどうかを判定
	return ((*it)->IsSameItem(_item));

}
