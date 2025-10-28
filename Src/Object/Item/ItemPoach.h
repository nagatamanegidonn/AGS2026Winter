#pragma once
#include <vector>
#include <map>
#include <memory>
#include <list>

class ItemBase;

class ItemPoach
{
public:

	ItemPoach(void);
	~ItemPoach(void);

	void Draw(int i);

	//アイテムの管理メソッド
	void AddItem(int _itemId);
	void PlayItem(int _itemId);

	const std::list<int>& GetItemIds(void) const { return ItemIds_; }

	//選択中のアイテムIDの設定、取得
	void CountSelectId(void);
	int GetSelectId(void) const { return slectId_; }

private:

	/// <summary>
	/// 選択中のアイテムID
	int slectId_;

	/// <summary>
	/// int はポーチ番号
	/// </summary>
	std::map<int, std::shared_ptr<ItemBase>> Items_;
	std::list<int> ItemIds_;

};

