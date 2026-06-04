#pragma once
#include "../Lib/nlohmann/json.hpp"
#include "SceneBase.h"

// 長いのでnamespaceの省略
using json = nlohmann::json;

class PixelMaterial;
class PixelRenderer;

class ConnectScene : 
	public SceneBase
{

public:

	// ボタンサイズ
	const int WIDTH = 200;
	const int HEIGHT = 30;
	// 画面中心X座標
	const int HX = Application::SCREEN_SIZE_X / 2;
	// ボタン位置
	const int B1_Y = Application::SCREEN_SIZE_Y - 100;
	const Vector2 B1_S_POS = Vector2(HX - WIDTH / 2, B1_Y - HEIGHT / 2);
	const Vector2 B1_E_POS = Vector2(HX + WIDTH / 2, B1_Y + HEIGHT / 2);

	// コンストラクタ
	ConnectScene(void);

	// デストラクタ
	~ConnectScene(void);

	// 初期化処理
	void Init(void) override;

	// 更新処理
	void Update(void) override;

	// 描画処理
	void Draw(void) override;

	// 解放処理
	void Release(void) override;

private:

	// プレイヤー
	std::vector<std::unique_ptr<ViewPlayer>> players_;
	// プレイヤー数
	int playerNum_;

	// タイトル画像
	int imgTitle_;

	// スタート画像
	int imgStart_;

	//　背景、カーソル画像
	int backImg_;	// カーソル画像
	std::unique_ptr<PixelMaterial> Material_;// ピクセルマテリアル
	std::unique_ptr<PixelRenderer> Renderer_;// ピクセルレンダラー

	// プレイヤーの追加
	void AddPlayer(const VECTOR pos, const VECTOR rot);

};
