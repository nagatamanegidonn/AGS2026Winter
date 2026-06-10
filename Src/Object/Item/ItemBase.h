#pragma once
#include <string>

class ItemBase
{

public:

	// コンストラクタ
	ItemBase(std::wstring _name);

	// デストラクタ
	~ItemBase(void);

	// 初期化処理
	virtual void Init(int _id);

	// 更新処理
	virtual void Update(void);

	// 描画処理
	virtual void Draw(void);

	// 解放処理
	virtual void Release(void);

	// 格納IDの取得
	const int GetId(void) const { return selectId_; }
	// アイテム名の取得
	const std::wstring& GetName(void) const { return name_; }

	// アイテム数の取得、変更
	void Count(int num) { count_ += num; }
	int GetCount(void) const { return count_; }

	// 同じ派生クラスかどうかを判定するメソッド
	virtual bool IsSameItem(const ItemBase& _item) const;
	virtual bool IsSameName(const ItemBase& other) const {
		return name_ == other.name_; // 同じアイテムかどうか
	}

private:

	// 格納ID
	int selectId_;

	// アイテム画像
	int itemImage_;

	// アイテム名
	std::wstring  name_;
		 
	// 持っている数
	int count_;
};

