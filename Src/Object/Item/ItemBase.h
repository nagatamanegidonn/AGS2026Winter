#pragma once
#include <string>


class ItemBase
{
public:
	// コンストラクタ
	ItemBase(int itemId);

	// デストラクタ
	~ItemBase(void);

	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

	const void Count(int num) { count_ += num; }
	const int GetCount(void) const { return count_; }

private:

	//アイテム画像
	int itemImage_;
	//アイテム名
	std::wstring  name_;
		 
	//持っている数
	int count_;
};

