
#include <DxLib.h>
#include <typeindex>
#include <algorithm>

#include "ItemBase.h"
#include "ItemPoach.h"
#include "../../Application.h"
#include "../../Manager/ResourceManager.h"

namespace
{
	// UI配置・描画用の微調整（オフセット）
	constexpr int UI_FRAME_OFFSET_X = 100;	// 画面右端からフレーム中心までの余白
	constexpr int UI_FRAME_OFFSET_Y = 100;	// 画面下端からフレーム中心までの余白
	// カラーコード (RGB)
	constexpr unsigned int COLOR_BLUE = 0x0000ff;
	constexpr unsigned int COLOR_BLACK = 0x000000;
	// アイテム増減量・初期値
	constexpr int ITEM_COUNT_INCREMENT = 1;   // アイテム入手時の加算数
	constexpr int ITEM_COUNT_DECREMENT = -1;  // アイテム使用時の減算数
	constexpr int INITIAL_INDEX_ZERO = 0;
	constexpr int ITEM_EMPTY_COUNT = 0;
	// デバッグ表示用設定
#ifdef _DEBUG
	const std::wstring DBG_FMT_SELECT_ID = L"選択中のID=%d";
	const std::wstring DBG_FMT_ITEM_NAME = L"アイテム名=%s";
	const std::wstring DBG_FMT_ITEM_CNT = L"アイテム数=%d";

	constexpr int DBG_UI_OFFSET_X = 200;	// 選択中IDのXオフセット
	constexpr int DBG_UI_OFFSET_Y = 80;	// 選択中IDのYオフセット
	constexpr int DBG_LIST_START_X = 800;	// リスト表示開始X座標
	constexpr int DBG_LIST_START_Y = 400;	// リスト表示開始Y座標
	constexpr int DBG_TEXT_LINE_H = 16;	// デバッグテキストの1行の高さ
	constexpr int DBG_ITEM_STEP_Y = 32;	// アイテムごとのY移動幅
#endif
}

ItemPoach::ItemPoach()
	:
	selectIndex_(INITIAL_INDEX_ZERO),
	ItemList_()
{
	// アイテムフレーム画像読み込み
	itemFrameImage_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::ITEM_FREAM).handleId_;
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

	// アイテムフレーム描画
	DrawRotaGraph(Application::SCREEN_SIZE_X - UI_FRAME_OFFSET_X, Application::SCREEN_SIZE_Y - UI_FRAME_OFFSET_Y, 1.0f, 0.0f, itemFrameImage_, true);

	if (it != ItemList_.end()) {
		(*it)->Draw();
#ifdef _DEBUG
		DrawFormatString(Application::SCREEN_SIZE_X - DBG_UI_OFFSET_X, Application::SCREEN_SIZE_Y - DBG_UI_OFFSET_Y,
			COLOR_BLUE, DBG_FMT_SELECT_ID.c_str(), selectIndex_);
#endif // DEBUG
	}

#ifdef _DEBUG
	int py = DBG_LIST_START_X;
	for (auto& item : ItemList_)
	{
		DrawFormatString(DBG_LIST_START_X, py, COLOR_BLACK, DBG_FMT_ITEM_NAME.c_str(), item->GetName().c_str());
		DrawFormatString(DBG_LIST_START_X + 10, py + DBG_TEXT_LINE_H, COLOR_BLACK, DBG_FMT_ITEM_CNT.c_str(), item->GetCount());
		py += DBG_ITEM_STEP_Y;
	}
#endif // _DEBUG
}

// アイテムの追加
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
		(*it)->Count(ITEM_COUNT_INCREMENT);
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
	item->Count(ITEM_COUNT_DECREMENT);

	// アイテム数が0以下なら削除処理
	if (item->GetCount() <= ITEM_EMPTY_COUNT) {
		// 削除
		it = ItemList_.erase(it);

		// アイテムのId調整
		int id = INITIAL_INDEX_ZERO;
		for (auto& itm : ItemList_) {
			itm->Init(id++);
		}

		// 要素が削除されてリストが空になった場合
		if (ItemList_.empty()) {
			selectIndex_ = INITIAL_INDEX_ZERO;
			return;
		}

		// 次の要素が存在しない場合は先頭へ戻る
		if (it == ItemList_.end()) {
			selectIndex_ = INITIAL_INDEX_ZERO;
		}
		else if (selectIndex_ >= static_cast<int>(ItemList_.size())) {
			selectIndex_ = static_cast<int>(ItemList_.size()) - ITEM_COUNT_INCREMENT;
		}
	}
}

// 選択中のアイテムIDを次に進めるメソッド
void ItemPoach::NextItem(void)
{
	if (ItemList_.empty()) return;
	selectIndex_ = (selectIndex_ + ITEM_COUNT_INCREMENT) % ItemList_.size();
}

// ポーチに選択中のアイテムがあるかどうか
bool ItemPoach::HasSelectedItem(void) const
{
	// アイテムが無ければ終了
	if (ItemList_.empty()) return false;

	// 現在の選択アイテムを取得
	auto it = ItemList_.begin();
	// イテレータをselectIndex_分進める
	std::advance(it, selectIndex_);
	return (it != ItemList_.end());
}

// 選択中のアイテムが引数の名前かどうか
bool ItemPoach::IsSelectedItemName(const std::wstring& _name) const
{
	// アイテムが無ければ終了
	if (ItemList_.empty()) return false;

	// 現在の選択アイテムを取得
	auto it = ItemList_.begin();
	// イテレータをselectIndex_分進める
	std::advance(it, selectIndex_);
	return ((*it)->GetName() == _name);
}

bool ItemPoach::IsSelectedItem(const ItemBase& _item) const
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