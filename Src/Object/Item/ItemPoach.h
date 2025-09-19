#pragma once
#include <vector>
#include <map>
#include <memory>

class ItemBase;

class ItemPoach
{
public:

	ItemPoach(void);
	~ItemPoach(void);


	//アイテムの管理メソッド
	void AddItem(void);
	void PlayItem(int itemId);

private:

	int slectCnt_;

	/// <summary>
	/// int はポーチ番号
	/// 
	/// </summary>
	std::map<int, std::shared_ptr<ItemBase>> Items_;

};

