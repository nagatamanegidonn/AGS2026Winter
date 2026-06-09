#include <math.h>
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Utility/AsoUtility.h"
//#include "../Manager/GravityManager.h"
#include "../Object/Common/Transform.h"

#include "InputManager.h"

#include "../Application.h"

#include "Camera.h"

Camera::Camera(void)
{
	angles_ = VECTOR();
	cameraUp_ = VECTOR();
	mode_ = MODE::NONE;
	pos_ = AsoUtility::VECTOR_ZERO;
	targetPos_ = AsoUtility::VECTOR_ZERO;
	followTransform_ = nullptr;

	stepShake_ = stepMaxShake_ = 1.0f;
	shakePower_ = 0.0f;
	isShaking_ = false;
	shakeDir_ = { 0.0f,0.0f,0.0f };
}

Camera::~Camera(void)
{
}

void Camera::Init(void)
{

	ChangeMode(MODE::FIXED_POINT);

}

void Camera::Update(void)
{
	// ここで LookAtSmoothly を呼び出す

	// 例: 特定のオブジェクトを滑らかに追従する場合
	/*if (followTransform_) {
		LookAtSmoothly(followTransform_->pos, INTERPOLATION_SPEED);
	}*/
}

void Camera::SetBeforeDraw(void)
{

	// クリップ距離を設定する(SetDrawScreenでリセットされる)
	SetCameraNearFar(CAMERA_NEAR, CAMERA_FAR);

	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		SetBeforeDrawFixedPoint();
		break;
	case Camera::MODE::FOLLOW:
		SetBeforeDrawFollow();
		// ライト方向の設定
		ChangeLightTypeDir(VNorm(VSub(targetPos_, pos_)));
		break;
	case Camera::MODE::FPS:
		SyncFollowFPS();
		// ライト方向の設定
		ChangeLightTypeDir(VNorm(VSub(targetPos_, pos_)));
		break;
	}

	// シェイク効果を上乗せ
	if (isShaking_)
	{
		UpdateShake();
	}
	
	// カメラの設定(位置と注視点による制御)
	SetCameraPositionAndTargetAndUpVec(
		pos_, 
		targetPos_, 
		cameraUp_
	);

#ifdef _DEBUG
	DrawSphere3D(targetPos_, 10, 10, 0xff00ff, 0xff00ff, true);
#endif // _DEBUG


	// DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();

}

void Camera::Draw(void)
{
}

void Camera::SetFollow(const Transform* follow)
{
	followTransform_ = follow;
}
// targetLookAtPosに向かって注目する
void Camera::LookAtSmoothly(const VECTOR& targetLookAtPos, float interpolationFactor)
{
	// 目的位置への方向ベクトル（ターゲット - カメラ位置）
	VECTOR targetDir = VSub(targetLookAtPos, pos_);
	targetDir.y = 0.0f; // 高さを無視する（XZ平面のみで方向を求める）
	targetDir = VNorm(targetDir);// 正規化

	// 現在の forward ベクトル（XZ平面）
	VECTOR currentDir = VSub(targetPos_, pos_);
	currentDir.y = 0.0f;
	currentDir = VNorm(currentDir);// 正規化

	// スムーズに方向ベクトルを補間する（XZ方向のみ）
	VECTOR smoothDir = VAdd(VScale(currentDir, 1.0f - interpolationFactor),
		VScale(targetDir, interpolationFactor));
	smoothDir = VNorm(smoothDir);// 正規化

	// 注視点を補間した方向に更新（高さはカメラと同じにする）
	targetPos_ = VAdd(pos_, smoothDir);

	// カメラのY軸回転のみ更新
	float targetYaw = atan2f(smoothDir.x, smoothDir.z);

	// スムーズに角度を補間（-π～π 対応の LerpAngle 推奨）
	angles_.y = AsoUtility::LerpAngle(angles_.y, targetYaw, interpolationFactor);

	// X軸回転（仰俯角）はそのまま維持
	// angles_.x は一切変更しない

	Quaternion gRot = Quaternion();
	rotOutX_ = gRot.Mult(Quaternion::AngleAxis(angles_.y, AsoUtility::AXIS_Y));
	rot_ = rotOutX_.Mult(Quaternion::AngleAxis(angles_.x, AsoUtility::AXIS_X)); // X軸角度はそのまま

	// カメラのアップ方向も更新
	cameraUp_ = gRot.GetUp();
}

#pragma region カメラ情報取得

VECTOR Camera::GetPos(void) const
{
	return pos_;
}

VECTOR Camera::GetAngles(void) const
{
	return angles_;
}

VECTOR Camera::GetTargetPos(void) const
{
	return targetPos_;
}

Quaternion Camera::GetQuaRot(void) const
{
	return rot_;
}

Quaternion Camera::GetQuaRotOutX(void) const
{
	return rotOutX_;
}

VECTOR Camera::GetForward(void) const
{
	return VNorm(VSub(targetPos_, pos_));
}

#pragma endregion

void Camera::ChangeMode(MODE mode)
{

	// カメラの初期設定
	//SetDefault();

	// カメラモードの変更
	mode_ = mode;

	// 変更時の初期化処理
	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		// カメラの初期設定
		SetDefault();
		break;
	case Camera::MODE::FPS:
		break;
	case Camera::MODE::FOLLOW:
		break;
	case Camera::MODE::SELF_SHOTD:
		break;
	}

}

void Camera::StartShake(float _time, float _power)
{
	if (isShaking_) return; // 二重発動防止（必要に応じて外してもOK）
	isShaking_ = true;
	shakePower_ = _power;

	shakeDir_ = { 1.0f,0.0f,0.0f };

	stepShake_ = stepMaxShake_ = _time;
	defaultPos_ = pos_;
}

void Camera::SetDefault(void)
{

	// カメラの初期設定
	pos_ = DEFAULT_CAMERA_POS;

	// 注視点
	targetPos_ = AsoUtility::VECTOR_ZERO;

	// カメラの上方向
	cameraUp_ = AsoUtility::DIR_U;

	angles_.x = AsoUtility::Deg2RadF(30.0f);
	angles_.y = 0.0f;
	angles_.z = 0.0f;

	rot_ = Quaternion();

}

void Camera::SyncFollow(void)
{

	//auto& gIns = GravityManager::GetInstance();

	// 同期先の位置
	VECTOR pos = followTransform_->pos;

	// 重力の方向制御に従う
	//Quaternion gRot = gIns.GetTransform().quaRot;
	Quaternion gRot = Quaternion();

	// 正面から設定されたY軸分、回転させる
	rotOutX_ = gRot.Mult(Quaternion::AngleAxis(angles_.y, AsoUtility::AXIS_Y));

	// 正面から設定されたX軸分、回転させる
	rot_ = rotOutX_.Mult(Quaternion::AngleAxis(angles_.x, AsoUtility::AXIS_X));

	VECTOR localPos;

	// 注視点(通常重力でいうところのY値を追従対象と同じにする)
	localPos = rotOutX_.PosAxis(LOCAL_F2T_POS);
	targetPos_ = VAdd(pos, localPos);

	// カメラ位置
	localPos = rot_.PosAxis(LOCAL_F2C_POS);
	pos_ = VAdd(pos, localPos);

	// カメラの上方向
	cameraUp_ = gRot.GetUp();

}
// カメラ操作
void Camera::ProcessRot(void)
{
	//演習① カメラの要件に沿って、カメラ操作を実装してください
	//	・ Y軸に、360度回転すること
	//	・ X軸に、上は40度、下は15度回転すること
	//	・ カメラ操作は矢印キーを用いること

	auto& ins = InputManager::GetInstance();


	// 回転軸と量決め
	//const float ROT_POW = 1.0f;
	const float ROT_POW = AsoUtility::Deg2RadF(1.0f);
	VECTOR axisDeg = AsoUtility::VECTOR_ZERO;


	if (angles_.x <= LIMIT_X_UP_RAD)
	{
		if (ins.IsNew(KEY_INPUT_DOWN)) { axisDeg.x = ROT_POW; }
	}

	if (angles_.x >= LIMIT_X_DW_RAD)
	{
		if (ins.IsNew(KEY_INPUT_UP)) { axisDeg.x = -ROT_POW; }
	}


	if (ins.IsNew(KEY_INPUT_RIGHT)) { axisDeg.y = ROT_POW; }
	if (ins.IsNew(KEY_INPUT_LEFT)) { axisDeg.y = -ROT_POW; }


	if (!AsoUtility::EqualsVZero(axisDeg))
	{
		// 今回回転させたい回転量をクォータニオンで作る
		Quaternion rotPow = Quaternion();

		/*rotPow = rotPow.Mult(
			Quaternion::AngleAxis(
				AsoUtility::Deg2RadF(axisDeg.z), AsoUtility::AXIS_Z
			));*/
		rotPow = rotPow.Mult(
			Quaternion::AngleAxis(
				AsoUtility::Deg2RadF(axisDeg.x), AsoUtility::AXIS_X
			));
		rotPow = rotPow.Mult(
			Quaternion::AngleAxis(
				AsoUtility::Deg2RadF(axisDeg.y), AsoUtility::AXIS_Y
			));

		// 回転量を加える(合成)
		angles_ = VAdd(angles_, axisDeg);

	}

}
// カメラ操作(FPS用)
void Camera::ProcessPlayRot(const bool up, const bool down, const bool right, const bool left)
{

	// 回転軸と量決め
	//const float ROT_POW = 1.0f;
	const float ROT_POW = AsoUtility::Deg2RadF(1.0f);
	VECTOR axisDeg = AsoUtility::VECTOR_ZERO;


	if (angles_.x <= FPS_LIMIT_X_UP_RAD)
	{
		if (down) { axisDeg.x = ROT_POW; }
	}

	if (angles_.x >= FPS_LIMIT_X_DW_RAD)
	{
		if (up) { axisDeg.x = -ROT_POW; }
	}


	if (right) { axisDeg.y = ROT_POW; }
	if (left) { axisDeg.y = -ROT_POW; }


	if (!AsoUtility::EqualsVZero(axisDeg))
	{
		// 今回回転させたい回転量をクォータニオンで作る
		Quaternion rotPow = Quaternion();

		/*rotPow = rotPow.Mult(
			Quaternion::AngleAxis(
				AsoUtility::Deg2RadF(axisDeg.z), AsoUtility::AXIS_Z
			));*/
		rotPow = rotPow.Mult(
			Quaternion::AngleAxis(
				AsoUtility::Deg2RadF(axisDeg.x), AsoUtility::AXIS_X
			));
		rotPow = rotPow.Mult(
			Quaternion::AngleAxis(
				AsoUtility::Deg2RadF(axisDeg.y), AsoUtility::AXIS_Y
			));

		// 回転諒を加える(合成)
		angles_ = VAdd(angles_, axisDeg);

	}
}
// カメラ操作(マウス)
void Camera::ProcessRotMause(float& x_m, float& y_m, const float fov_per)
{
	
	int x_t, y_t;
	GetMousePoint(&x_t, &y_t);
	x_m += float(std::clamp(x_t - Application::SCREEN_SIZE_X / 2, -120, 120)) * fov_per / GetFPS();
	y_m += float(std::clamp(y_t - Application::SCREEN_SIZE_Y / 2, -120, 120)) * fov_per / GetFPS();
	SetMousePoint(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2);

	// マウスを表示状態にする
	SetMouseDispFlag(FALSE);

	if (angles_.x <= FPS_LIMIT_X_UP_RAD)
	{
		angles_.x = FPS_LIMIT_X_UP_RAD;
	}
	if (angles_.x >= FPS_LIMIT_X_DW_RAD)
	{
		angles_.x = FPS_LIMIT_X_DW_RAD;
	}

}



void Camera::SetBeforeDrawFixedPoint(void)
{
	// 何もしない
}
// 追従モードのカメラ同期
void Camera::SetBeforeDrawFollow(void)
{

	// カメラ操作
	//ProcessRot();

	// 追従対象との相対位置を同期
	SyncFollow();

}
// FPSモードのカメラ同期
void Camera::SyncFollowFPS(void)
{
	ProcessRotMause(angles_.y, angles_.x, 0.2f);;

	

	const auto& ins = InputManager::GetInstance();
	Vector2 moPos = ins.GetMousePos();

	//ProcessRotMause(moPos.x, moPos.y, 0.02f);

	//auto& gIns = GravityManager::GetInstance();

	// 同期先の位置
	VECTOR pos = followTransform_->pos;

	// 重力の方向制御に従う
	//Quaternion gRot = gIns.GetTransform().quaRot;
	Quaternion gRot = Quaternion();


	// 正面から設定されたY軸分、回転させる
	rotOutX_ = gRot.Mult(Quaternion::AngleAxis(angles_.y, AsoUtility::AXIS_Y));

	// 正面から設定されたX軸分、回転させる
	rot_ = rotOutX_.Mult(Quaternion::AngleAxis(angles_.x, AsoUtility::AXIS_X));

	VECTOR localPos;

	// 注視点(通常重力でいうところのY値を追従対象と同じにする)
	//localPos = rotOutX_.PosAxis(LOCAL_F2T_POS);
	localPos = rot_.PosAxis(FPS_LOCAL_F2T_POS);
	targetPos_ = VAdd(pos, localPos);

	// カメラ位置
	localPos = rotOutX_.PosAxis(FPS_LOCAL_F2C_POS);
	pos_ = VAdd(pos, localPos);



	// カメラの上方向
	cameraUp_ = gRot.GetUp();
}
void Camera::SetBeforeDrawSelfShot(void)
{
}
// カメラシェイク
void Camera::UpdateShake(void)
{
	if (!isShaking_) return;

	// シェイク強度・減衰パラメータ
	//float shakePower_ = 2.0f;      // 初期の揺れの大きさ
	//float shakeDuration_ = 1.0f;     // 揺れる時間


	stepShake_ -= 0.016f; // Δt

	if (stepShake_ <= 0.0f)
	{
		pos_ = defaultPos_;
		isShaking_ = false;
		return;
	}

	// 減衰係数（0～1）// 経過時間/総時間
	float t = stepShake_ / stepMaxShake_;
	float currentPower = shakePower_ * t * t;

	float dx = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
	float dy = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
	float dz = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;

	//  -1～1 のランダム方向
	VECTOR randomDir = VNorm({ dx, dy, dz });
	pos_ = VAdd(defaultPos_, VScale(randomDir, currentPower));
	return;


	if (stepShake_ > 0.0f)
	{
		stepShake_ -= 0.016f; // Δt（固定フレーム時間）

		// -1～1 のランダム方向
		float dx = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
		float dy = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
		float dz = 0.0f;

		VECTOR randomDir = VNorm({ dx, dy, dz });

		// カメラ位置更新
		pos_ = VAdd(defaultPos_, VScale(randomDir, currentPower));
	}
	else
	{
		pos_ = defaultPos_; // 元の位置へ戻す
		if (followTransform_ != nullptr) {
			ChangeMode(MODE::FOLLOW);
		}
		else {
			ChangeMode(MODE::FIXED_POINT);
		}
		isShaking_ = false;
	}

	return;

	// 一定時間カメラを揺らす
	// stepShake_ -= SceneManager::GetInstance().GetDeltaTime();
	stepShake_ -= 0.01f;

	// カメラシェイク終了
	if (stepShake_ < 0.0f)
	{
		pos_ = defaultPos_;
		if (followTransform_ != nullptr)
		{
			ChangeMode(MODE::FOLLOW);
		}
		else 
		{
			ChangeMode(MODE::FIXED_POINT);
		}
		isShaking_ = true;
		return;
	}

	// -1.0f～1.0f
	float f = sinf(stepShake_ * SPEED_SHAKE);

	// -1000.0f～1000.0f
	f *= 1000.0f;

	// -1000 or 1000
	int d = static_cast<int>(f);

	// 0 or 1//揺れ方向
	int shake = d % 2;

	// 0 or 2
	shake *= 2;

	// -1 or 1
	shake -= 1;

	// 移動量
	VECTOR velocity = VScale(shakeDir_, (float)(shake)*WIDTH_SHAKE);

	// 移動先座標
	pos_ = VAdd(defaultPos_, velocity);
}


