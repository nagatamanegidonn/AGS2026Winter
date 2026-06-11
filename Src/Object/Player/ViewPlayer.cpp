#include "../Manager/ResourceManager.h"
#include "../../Utility/AsoUtility.h"
#include "../../Application.h"
#include "../Common/AnimationController.h"

#include "ViewPlayer.h"

ViewPlayer::ViewPlayer()
	:
	animationController_(nullptr),
	weponId_(-1),
	updateWeapon_(nullptr)
{
}

ViewPlayer::~ViewPlayer()
{
	MV1DeleteModel(transform_.modelId);
	MV1DeleteModel(transWeapon_.modelId);
}

void ViewPlayer::Init(void)
{

	static std::wstring PATH_MDL = Application::PATH_MODEL;

	// モデルの基本設定
	transform_.modelId = MV1LoadModel((PATH_MDL + L"Player2/Player2.mv1").c_str());
	transform_.scl = AsoUtility::VECTOR_ONE;
	// 初期座標
	transform_.pos = { 0.0f,-50.0f,-250.0f };
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal =
		Quaternion::Euler({ AsoUtility::Deg2RadF(10.0f), AsoUtility::Deg2RadF(0.0f), AsoUtility::Deg2RadF(0.0f) });
	transform_.Update();

	// 武器の設定
	transWeapon_.scl = VScale(AsoUtility::VECTOR_ONE, 1.0f);
	// 初期座標
	transWeapon_.pos = { 0.0f, -30.0f, 0.0f };
	transWeapon_.quaRot = Quaternion();
	transWeapon_.quaRotLocal =
		Quaternion::Euler({ AsoUtility::Deg2RadF(160.0f), AsoUtility::Deg2RadF(180.0f), AsoUtility::Deg2RadF(0.0f) });
	transWeapon_.Update();
	updateWeapon_ = std::bind(&ViewPlayer::SyncWeaponSowrd, this);
	updateWeapon_();

	weponId_ = -1;
}

void ViewPlayer::Update(void)
{
	transform_.Update();

	animationController_->Update();
	updateWeapon_();
}

void ViewPlayer::Draw(void)
{
	MV1DrawModel(transform_.modelId);
	MV1DrawModel(transWeapon_.modelId);
}

void ViewPlayer::Release(void)
{
}

void ViewPlayer::SetChar(const int charId)
{
	static std::wstring PATH_MDL = Application::PATH_MODEL;


	MV1DeleteModel(transform_.modelId);

	switch (charId)
	{
	case 0:// ナイト
	{
		transform_.modelId = MV1LoadModel((PATH_MDL + L"Player2/Player2.mv1").c_str());

		std::wstring path = Application::PATH_MODEL + L"Player2/";
		animationController_.reset();
		animationController_ = std::make_unique<AnimationController>(transform_.modelId);
		animationController_->Add(0, path + L"Idle.mv1", 20.0f);

		animationController_->Play(0);
	}
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		break;
	}

	transform_.Update();

}

void ViewPlayer::SetWeapon(const int weponId)
{
	if (weponId_ == weponId) { return; }

	static std::wstring PATH_MDL = Application::PATH_MODEL;

	MV1DeleteModel(transWeapon_.modelId);

	weponId_ = weponId;

	switch (weponId)
	{
	case 0:// 片手剣
		transWeapon_.modelId = MV1LoadModel((PATH_MDL + L"Sword/Sword.mv1").c_str());

		transWeapon_.scl = VScale(AsoUtility::VECTOR_ONE, 1.0f);
		// 初期座標
		transWeapon_.pos = { 0.0f, -30.0f, 0.0f };
		transWeapon_.quaRot = Quaternion();
		transWeapon_.quaRotLocal =
			Quaternion::Euler({ AsoUtility::Deg2RadF(160.0f), AsoUtility::Deg2RadF(180.0f), AsoUtility::Deg2RadF(0.0f) });
		transWeapon_.Update();

		updateWeapon_ = std::bind(&ViewPlayer::SyncWeaponSowrd, this);
		break;
	case 1:// 両手剣
		transWeapon_.modelId = MV1LoadModel((PATH_MDL + L"Sword/Sword Two-Hander Base.mv1").c_str());

		transWeapon_.scl = VScale(AsoUtility::VECTOR_ONE, 2.0f);
		// 初期座標
		transWeapon_.pos = { 0.0f, -30.0f, 0.0f };
		transWeapon_.quaRot = Quaternion();
		transWeapon_.quaRotLocal =
			Quaternion::Euler({ AsoUtility::Deg2RadF(160.0f), AsoUtility::Deg2RadF(180.0f), AsoUtility::Deg2RadF(0.0f) });
		transWeapon_.Update();

		updateWeapon_ = std::bind(&ViewPlayer::SyncWeaponGreatSowrd, this);
		break;
	case 2:// 弓
		transWeapon_.modelId = MV1LoadModel((PATH_MDL + L"Weapon/Bow/Bow.mv1").c_str());

		transWeapon_.scl = VScale(AsoUtility::VECTOR_ONE, 1.0f);
		// 初期座標
		transWeapon_.pos = { 0.0f, -30.0f, 0.0f };
		transWeapon_.quaRot = Quaternion();
		transWeapon_.quaRotLocal =
			Quaternion::Euler({ AsoUtility::Deg2RadF(160.0f), AsoUtility::Deg2RadF(180.0f), AsoUtility::Deg2RadF(0.0f) });
		transWeapon_.Update();

		updateWeapon_ = std::bind(&ViewPlayer::SyncWeaponBow, this);
		break;
	case 3:
		break;
	}
	
	updateWeapon_();
}

const void ViewPlayer::SetPos(const VECTOR pos)
{
	transform_.pos = pos;
	transform_.Update();
}

const void ViewPlayer::SetLocalQua(const VECTOR rot)
{
	transform_.quaRotLocal =
		Quaternion::Euler({rot.x, rot.y, rot.z });
	transform_.Update();
}

void ViewPlayer::SyncWeaponGreatSowrd()
{
	auto frmNo = MV1SearchFrame(transform_.modelId, L"mixamorig:Spine2");//ナイト
	// プレイヤーの手の位置
	const auto& posHand = MV1GetFramePosition(transform_.modelId, frmNo);

	// プレイヤーの手のグローバルマトリクス
	MATRIX handRot = MV1GetFrameLocalWorldMatrix(transform_.modelId, frmNo);
	// 手のワールド回転
	Quaternion handWorldRot = Quaternion::GetRotation(handRot);

	// 参照元の大きさを考慮
	auto scl = transform_.scl;
	scl.x = 1.0f / scl.x;
	scl.y = 1.0f / scl.y;
	scl.z = 1.0f / scl.z;

	// 回転だけを取り出す（大きさも出るので考慮）
	auto mixMat = MMult(MGetRotElem(handRot), MGetScale(scl));

	// 回転
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_R, handRot))//X回転
			, AsoUtility::Deg2RadF(0.0f))
	);
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_U, handRot))//Y回転
			, AsoUtility::Deg2RadF(0.0f))
	);
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_F, handRot))//Z回転
			, AsoUtility::Deg2RadF(180.0f))
	);

	// 最終回転（手 + 握り補正）
	transWeapon_.matRot = mixMat;

	// 最終位置
	transWeapon_.pos = posHand;
	transWeapon_.pos = VAdd(transWeapon_.pos, VScale(transWeapon_.quaRot.GetDown(), 50.0f));
	transWeapon_.pos = VAdd(transWeapon_.pos, VScale(transWeapon_.quaRot.GetForward(), 15.0f));

	// モデルの更新
	transWeapon_.Update(true);
}

void ViewPlayer::SyncWeaponSowrd()
{
	auto frmNo = MV1SearchFrame(transform_.modelId, L"mixamorig:Spine");//ナイト腰
	// プレイヤーの手の位置
	const auto& posHand = MV1GetFramePosition(transform_.modelId, frmNo);

	// プレイヤーの手のグローバルマトリクス
	MATRIX handRot = MV1GetFrameLocalWorldMatrix(transform_.modelId, frmNo);
	// 手のワールド回転
	Quaternion handWorldRot = Quaternion::GetRotation(handRot);

	// 参照元の大きさを考慮
	auto scl = transform_.scl;
	scl.x = 1.0f / scl.x;
	scl.y = 1.0f / scl.y;
	scl.z = 1.0f / scl.z;

	// 回転だけを取り出す（大きさも出るので考慮）
	auto mixMat = MMult(MGetRotElem(handRot), MGetScale(scl));

	// 回転
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_R, handRot))// X回転
			, AsoUtility::Deg2RadF(0.0f))
	);
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_U, handRot))// Y回転
			, AsoUtility::Deg2RadF(0.0f))
	);
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_F, handRot))// Z回転
			, AsoUtility::Deg2RadF(-120.0f))
	);

	// 最終回転（手 + 握り補正）
	transWeapon_.matRot = mixMat;

	// 最終位置
	transWeapon_.pos = posHand;
	transWeapon_.pos = VAdd(transWeapon_.pos, VScale(handWorldRot.GetForward(), 15.0f));
	transWeapon_.pos = VAdd(transWeapon_.pos, VScale(handWorldRot.GetRight(), -30.0f));

	// モデルの更新
	transWeapon_.Update(true);
}

void ViewPlayer::SyncWeaponBow()
{
	auto frmNo = MV1SearchFrame(transform_.modelId, L"mixamorig:Spine");// ナイト腰
	// プレイヤーの手の位置
	const auto& posHand = MV1GetFramePosition(transform_.modelId, frmNo);

	// プレイヤーの手のグローバルマトリクス
	MATRIX handRot = MV1GetFrameLocalWorldMatrix(transform_.modelId, frmNo);
	// 手のワールド回転
	Quaternion handWorldRot = Quaternion::GetRotation(handRot);

	// 参照元の大きさを考慮
	auto scl = transform_.scl;
	scl.x = 1.0f / scl.x;
	scl.y = 1.0f / scl.y;
	scl.z = 1.0f / scl.z;

	// 回転だけを取り出す（大きさも出るので考慮）
	auto mixMat = MMult(MGetRotElem(handRot), MGetScale(scl));

	// 回転
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_R, handRot))// X回転
			, AsoUtility::Deg2RadF(0.0f))
	);
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_U, handRot))// Y回転
			, AsoUtility::Deg2RadF(0.0f))
	);
	mixMat = MMult(mixMat,
		MGetRotAxis(
			VNorm(VTransformSR(AsoUtility::DIR_F, handRot))// Z回転
			, AsoUtility::Deg2RadF(30.0f))
	);

	// 最終回転（手 + 握り補正）
	transWeapon_.matRot = mixMat;

	// 最終位置
	transWeapon_.pos = posHand;
	transWeapon_.pos = VAdd(transWeapon_.pos, VScale(handWorldRot.GetForward(), 15.0f));

	// モデルの更新
	transWeapon_.Update(true);
}
