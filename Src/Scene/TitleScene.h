#pragma once
#include <map>
#include <functional>
#include <memory>
#include <string>

#include "../Application.h"
#include "../Common/Vector2.h"
#include "SceneBase.h"

class InputController;
class InputTextArea;
class ViewPlayer;
class PixelMaterial;
class PixelRenderer;

class TitleScene : 
	public SceneBase
{

public:

	// メニュー選択肢
	enum class MENU
	{
		NET_SELECT,		// 通信状態の設定
		IP_SET,			// IPアドレス設定
		WEPON_SELECT,	// 武器選択
		GAME_START,		// ゲームスタート
		MAX,			
	};

	// 武器選択肢
	enum class WEPON_ID
	{
		BLONZ_SOERD,	// 片手剣
		BASTER_SOERD,	// 大剣
		ARROW,			// 弓
		MAX,
	};

	// ボタン位置情報
	struct PosTri
	{
		std::wstring Name = L"";	// 表示テキスト
		int WIDTH = 0;				// 横の長さ
		int HEIGHT = 0;				// 縦の長さ
		Vector2 CenterPos;			// 生成位置
		Vector2 StartPos;			// 左上
		Vector2 EndPos;				// 右下
	};

	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void);

	// 初期化処理
	void Init(void) override;

	// 更新処理
	void Update(void) override;

	// 描画処理
	void Draw(void) override;

	// 解放処理
	void Release(void) override;

private:

	// 設定
	std::unique_ptr<InputController> inputController_;

	// IPアドレス
	std::unique_ptr<InputTextArea> inputTextArea_;

	// プレイヤー
	std::unique_ptr<ViewPlayer> viewPlayer_;

	// 状態管理(更新ステップ)
	std::function<void(void)> typeUpdate_;	// 更新処理
	void UpdateMouse(void);		// マウス更新処理
	void UpdatePad(void);		// パッド更新処理

	// パッド操作
	std::function<void(void)> padUpdate_;	// 更新処理
	void PNormalUpdate(void);	// 通常処理
	void PWeaponUpdate(void);	// 武器変更処理
	void PIpUpdate(void);		// IPアドレス変更処理
	// マウス操作
	std::function<void(void)> mouseUpdate_;	// 更新処理
	void MouseUpdate(void);		// 通常処理
	void MWeaponUpdate(void);	// 武器変更処理

	// 前回のマウス情報
	Vector2 agoMousePos_;	// 前フレームのマウス位置

	// このシーンで変更するステータス
	bool isPad_;		// パッド操作中はの判定
	int selectId_;		// 選択中の項目ID
	int weponId_;		// 選択中の武器ID 
	bool isWpSelect_;	// 武器を選択中か
	std::map<int, std::unique_ptr<PosTri>> weponsPos_;	// 武器一覧用の文字位置リスト

	// 背景、カーソル画像
	int titleImg_;	// タイトルロゴ
	int backImg_;	// 背景
	int cursorImg_;	// カーソル

	// タイトルロゴ表示中か
	bool isTitle_;	

	// 背景画像
	std::unique_ptr<PixelMaterial> backGroundMaterial_;
	std::unique_ptr<PixelRenderer> backGroundRenderer_;

	// カーソル画像
	std::unique_ptr<PixelMaterial> cursorMaterial_;
	std::unique_ptr<PixelRenderer> cursorRenderer_;
	
	// タイトルロゴ
	std::unique_ptr<PixelMaterial> titleMaterial_;
	std::unique_ptr<PixelRenderer> titleRenderer_;

	// ボタン位置追加
	void AddPosTri(std::wstring name, int weponId, const Vector2 size, const Vector2 cPos);
};

