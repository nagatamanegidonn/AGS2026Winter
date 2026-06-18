#pragma once
#include <vector>
#include <map>
#include <memory>
#include <list>

class ItemBase;

class ItemPoach
{

public:

	// コンストラクタ
	ItemPoach(void);

	// デストラクタ
	~ItemPoach(void);

	// 描画処理
	void Draw(int i);

	// アイテムの追加
	void AddItem(std::shared_ptr<ItemBase> _item);

	// アイテムの変更
	void UseSelectedItem(void);

	// 選択中のアイテムIDの設定、取得
	void NextItem(void);
	int GetSelectId(void) const { return selectIndex_; }

	// ポーチに選択中のアイテムがあるかどうか
	bool HasSelectedItem(void) const;

	// 選択中のアイテムが引数の名前かどうか
	bool IsSelectedItemName(const std::wstring& _name) const;
	bool IsSelectedItem(const ItemBase& _item) const;

private:

	// 選択中のアイテムID
	int selectIndex_;

	// アイテムフレーム画像
	int itemFrameImage_;

	// アイテムリスト
	std::list<std::shared_ptr<ItemBase>> ItemList_;
};

