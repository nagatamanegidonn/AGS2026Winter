#pragma once
#include <map>
#include <functional>
#include<memory>  //ptrを使うために必要
#include <string>

#include "../Application.h"
#include "../Common/Vector2.h"
#include "SceneBase.h"

class InputController;

class InputTextArea;
class ViewPlayer;
class PixelMaterial;
class PixelRenderer;

class TitleScene : public SceneBase
{

public:

	enum class MENU
	{
		USER_SELECT,
		IP_SET,
		WEPON_SELECT,
		GAME_START,
		MAX,
	};
	
	enum class WEPON_ID
	{
		BLONZ_SOERD,
		BASTER_SOERD,
		ARROW,
		MAX,
	};

	struct PosTri
	{
		std::wstring  Name = L"";
		int WIDTH = 0;
		int HEIGHT = 0;

		Vector2 CenterPos;
		Vector2 StartPos;
		Vector2 EndPos;
	};

	// ボタンサイズ
	const int WIDTH = 200;
	const int HEIGHT = 30;

	int HX = Application::SCREEN_SIZE_X / 2;
	int HY = Application::SCREEN_SIZE_Y / 2;

	// ボタン位置
	const int B1_Y = Application::SCREEN_SIZE_Y - 100;
	/*const Vector2 B1_S_POS = Vector2(HX - WIDTH / 2, B1_Y - HEIGHT / 2);
	const Vector2 B1_E_POS = Vector2(HX + WIDTH / 2, B1_Y + HEIGHT / 2);*/
	const Vector2 B1_C_POS = Vector2(845, 80 );
	const Vector2 B1_S_POS = Vector2(B1_C_POS.x - WIDTH / 2, B1_C_POS.y - HEIGHT / 2);
	const Vector2 B1_E_POS = Vector2(B1_C_POS.x + WIDTH / 2, B1_C_POS.y + HEIGHT / 2);


	const Vector2 IP_C_POS = Vector2(845, 80 + 60);
	const Vector2 IP_S_POS = Vector2(IP_C_POS.x - WIDTH / 2, IP_C_POS.y - HEIGHT / 2);
	const Vector2 IP_E_POS = Vector2(IP_C_POS.x + WIDTH / 2, IP_C_POS.y + HEIGHT / 2);
	
	const Vector2 WP_C_POS = Vector2(IP_C_POS.x, IP_C_POS.y + 60);
	const Vector2 WP_S_POS = Vector2(WP_C_POS.x - WIDTH / 2, WP_C_POS.y - HEIGHT / 2);
	const Vector2 WP_E_POS = Vector2(WP_C_POS.x + WIDTH / 2, WP_C_POS.y + HEIGHT / 2);

	//startボタンPos
	const int B2_Y = B1_Y + 40;
	const Vector2 B2_C_POS = Vector2(HX, B2_Y);
	const Vector2 B2_S_POS = Vector2(B2_C_POS.x - WIDTH / 2, B2_C_POS.y - HEIGHT / 2);
	const Vector2 B2_E_POS = Vector2(B2_C_POS.x + WIDTH / 2, B2_C_POS.y + HEIGHT / 2);

	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

private:



	//設定
	std::unique_ptr<InputController> inputController_;

	// IPアドレス
	InputTextArea* inputTextArea_;

	//プレイヤー
	std::unique_ptr<ViewPlayer> player_;

	// 状態管理(更新ステップ)
	std::function<void(void)> typeUpdate_;
	void UpdateMouse(void);
	void UpdateNormal(void);

	std::function<void(void)> padUpdate_;
	void PNormalUpdate(void);
	void PWeponUpdate(void);
	void PIpUpdate(void);
	std::function<void(void)> mouseUpdate_;
	void MouseUpdate(void);
	void MWeponUpdate(void);
	

	Vector2 agoMousePos_;
	bool agoMouseTrg_;

	//ここで変更するステータス
	bool isPad_;
	bool isHost_;
	int selectId_;
	int weponId_;
	bool isWpSelect_;
	std::map<int, std::unique_ptr<PosTri>> weponsPos_;

	//背景、カーソル画像
	int titleImg_;
	int backImg_;
	int cursorImg_;

	bool isTitle_;

	std::unique_ptr<PixelMaterial> Material_;
	std::unique_ptr<PixelRenderer> Renderer_;

	std::unique_ptr<PixelMaterial> cursorMaterial_;
	std::unique_ptr<PixelRenderer> cursorRenderer_;
	
	std::unique_ptr<PixelMaterial> titleMaterial_;
	std::unique_ptr<PixelRenderer>  titleRenderer_;

	const bool IsTrggerdMleft(void)const;

	void AddPosTri(std::wstring name, int weponId, const Vector2 size
		, const Vector2 cPos);
};

