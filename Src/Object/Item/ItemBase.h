#pragma once
#include <string>


class ItemBase
{
public:
	// コンストラクタ
	ItemBase(void);

	// デストラクタ
	~ItemBase(void);

	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

private:

	//アイテム画像
	int itemImage_;
	//アイテム名
	std::string  name_;
		 

};

