#pragma once
#include "SceneBase.h"

class PixelMaterial;
class PixelRenderer;

class ConnectScene : 
	public SceneBase
{

public:

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

	// 背景、カーソル画像
	int backImg_;

	// 背景画像
	std::unique_ptr<PixelMaterial> backGroundMaterial_;// ピクセルマテリアル
	std::unique_ptr<PixelRenderer> backGroundRenderer_;// ピクセルレンダラー

	// プレイヤーの追加
	void AddPlayer(const VECTOR pos, const VECTOR rot);

	// ゲームスタート
	void PlayStart(void);

};