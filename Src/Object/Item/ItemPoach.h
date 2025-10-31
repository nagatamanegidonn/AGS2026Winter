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
	void AddItem(std::shared_ptr<ItemBase> _item);
	void UseSelectedItem(void);

	//選択中のアイテムIDの設定、取得
	void NextItem(void);
	int GetSelectId(void) const { return selectIndex_; }

	//ポーチに選択中のアイテムがあるかどうか
	bool HasSelectedItem(void) const;

	//選択中のアイテムが引数の名前かどうか
	bool IsSelectedItemName(const std::wstring& _name) const;
	bool IsSelectedItem(ItemBase _item) const;

private:

	/// <summary>
	/// 選択中のアイテムID
	int selectIndex_;

	std::list<std::shared_ptr<ItemBase>> ItemList_;

};

