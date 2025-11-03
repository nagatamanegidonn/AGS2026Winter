#pragma once
#include <DxLib.h>
#include "../Common/Quaternion.h"
class Transform;

class Camera
{

public:

	// カメラスピード(度)
	static constexpr float SPEED = 1.0f;

	// カメラクリップ：NEAR
	static constexpr float CAMERA_NEAR = 10.0f;

	// カメラクリップ：NEAR
	static constexpr float CAMERA_FAR = 30000.0f;

	// カメラの初期座標
	static constexpr VECTOR DEFAULT_CAMERA_POS = { 0.0f, 100.0f, -500.0f };

	// 追従位置からカメラ位置までの相対座標
	static constexpr VECTOR LOCAL_F2C_POS = { 0.0f, 50.0f, -400.0f };
	static constexpr VECTOR FPS_LOCAL_F2C_POS = { 0.0f, 250.0f, -300.0f };

	// 追従位置から注視点までの相対座標
	static constexpr VECTOR LOCAL_F2T_POS = { 0.0f, 0.0f, 500.0f };
	static constexpr VECTOR FPS_LOCAL_F2T_POS = { 0.0f, 100.0f, 500.0f };

	// カメラのX回転上限度角
	static constexpr float LIMIT_X_UP_RAD = 40.0f * (DX_PI_F / 180.0f);
	static constexpr float FPS_LIMIT_X_UP_RAD = -0.65f;
	static constexpr float LIMIT_X_DW_RAD = 15.0f * (DX_PI_F / 180.0f);
	static constexpr float FPS_LIMIT_X_DW_RAD = 0.25f;
	
	const float INTERPOLATION_SPEED = 0.1f; // 補間の速さ

	//カメラシェイク
	const float SPEED_SHAKE = 0.1f; // 横の幅
	const float WIDTH_SHAKE = 0.1f; // 横の幅

	// カメラモード
	enum class MODE
	{
		NONE,
		FIXED_POINT,
		FOLLOW,
		FPS,
		SELF_SHOTD,
		SHAKE,
	};

	Camera(void);
	~Camera(void);

	void Init(void);
	void Update(void);
	void SetBeforeDraw(void);
	void Draw(void);

	// カメラ位置
	VECTOR GetPos(void) const;
	// カメラの操作角度
	VECTOR GetAngles(void) const;
	// カメラの注視点
	VECTOR GetTargetPos(void) const;

	// カメラ角度
	Quaternion GetQuaRot(void) const;
	// X回転を抜いたカメラ角度
	Quaternion GetQuaRotOutX(void) const;
	// カメラの前方方向
	VECTOR GetForward(void) const;

	// カメラモードの変更
	void ChangeMode(MODE mode);

	// 追従対象の設定
	void SetFollow(const Transform* follow);
	//targetLookAtPosに向かって注目する
	void LookAtSmoothly(const VECTOR& targetLookAtPos, float interpolationFactor);

	// カメラ操作
	void ProcessPlayRot(const bool up = false, const bool down = false
		, const bool right = false, const bool left = false);

private:

	// カメラが追従対象とするTransform
	const Transform* followTransform_;

	// カメラモード
	MODE mode_;
	// カメラの位置
	VECTOR pos_;
	// カメラ角度(rad)
	VECTOR angles_;
	// X軸回転が無い角度
	Quaternion rotOutX_;
	// カメラ角度
	Quaternion rot_;
	// 注視点
	VECTOR targetPos_;
	// カメラの上方向
	VECTOR cameraUp_;

#pragma region カメラシェイク用
	//画面揺らし用
	float stepShake_;

	//画面揺れが終わったか
	bool finishShake_;
	//演出前位置
	VECTOR defaultPos_;
	//揺れ方向
	VECTOR shakeDir_;
#pragma endregion


	// カメラを初期位置に戻す
	void SetDefault(void);

	// 追従対象との位置同期を取る
	void SyncFollow(void);

	// カメラ操作
	void ProcessRot(void);
	void ProcessRotMause(float& x_m, float& y_m, const float fov_per);

	// モード別更新ステップ
	void SetBeforeDrawFixedPoint(void);
	void SetBeforeDrawFollow(void);
	void SyncFollowFPS(void);
	void SetBeforeDrawSelfShot(void);
	void SetBeforeDrawShake(void);

};

