#include "../Manager/ResourceManager.h"
#include "../../Utility/Utility.h"
#include "../../Application.h"
#include "../Common/AnimationController.h"

#include "ViewPlayer.h"

namespace
{
	// ボーン・フレーム名
	const std::wstring FRAME_SPINE2 = L"mixamorig:Spine2";
	const std::wstring FRAME_SPINE = L"mixamorig:Spine";
	// アニメーション・ファイル名
	const std::wstring DIR_PLAYER2 = L"Player2/";
	const std::wstring FILE_PLAYER2_MDL = L"Player2/Player2.mv1";
	const std::wstring FILE_ANIM_IDLE = L"Idle.mv1";
	const std::wstring FILE_WEAPON_SWORD = L"Sword/Sword.mv1";
	const std::wstring FILE_WEAPON_GREAT = L"Sword/Sword Two-Hander Base.mv1";
	const std::wstring FILE_WEAPON_BOW = L"Weapon/Bow/Bow.mv1";
	// プレイヤー初期トランスフォーム
	const VECTOR PLAYER_INIT_POS = { 0.0f, -50.0f, -250.0f };
	const VECTOR PLAYER_INIT_ROT_LOCAL = { 10.0f, 0.0f, 0.0f };
	// 武器共通の初期トランスフォーム
	const VECTOR WEAPON_INIT_POS = { 0.0f, -30.0f, 0.0f };
	const VECTOR WEAPON_INIT_ROT_LOCAL = { 160.0f, 180.0f, 0.0f };
	// 武器ごとの個別スケール
	constexpr float SCALE_SWORD = 1.0f;
	constexpr float SCALE_GREAT = 2.0f;
	constexpr float SCALE_BOW = 1.0f;
	// マトリクス抽出用
	constexpr float INVERSE_SCALE_BASE = 1.0f;
	// 両手剣（GreatSword）の調整値
	constexpr VECTOR GREAT_ROT = { 0.0f, 0.0f, 180.0f };
	constexpr float GREAT_POS_OFFSET_DOWN = 50.0f;
	constexpr float GREAT_POS_OFFSET_FORWARD = 15.0f;
	// 片手剣（Sword）の調整値
	constexpr VECTOR SWORD_ROT = { 0.0f, 0.0f, -120.0f };
	constexpr float SWORD_POS_OFFSET_FORWARD = 15.0f;
	constexpr float SWORD_POS_OFFSET_RIGHT = -30.0f;
	// 弓（Bow）の調整値
	constexpr VECTOR BOW_ROT = { 0.0f, 0.0f, 30.0f };
	constexpr float BOW_POS_OFFSET_FORWARD = 15.0f;
	// アニメーション用定数
	constexpr int ANIM_INDEX_IDLE = 0;
	constexpr float ANIM_SPEED_IDLE = 20.0f;
	// 武器ID
	constexpr int WEAPON_ID_NONE = -1;
	constexpr int SWORD_ID = 0;
	constexpr int GREAT_SWORD_ID = 1;
	constexpr int BOW_ID = 2;
}

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
	transform_.modelId = MV1LoadModel((PATH_MDL + FILE_PLAYER2_MDL).c_str());
	transform_.scl = Utility::VECTOR_ONE;
	// 初期座標
	transform_.pos = PLAYER_INIT_POS;
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion::Euler({
		Utility::Deg2RadF(PLAYER_INIT_ROT_LOCAL.x),
		Utility::Deg2RadF(PLAYER_INIT_ROT_LOCAL.y),
		Utility::Deg2RadF(PLAYER_INIT_ROT_LOCAL.z)
		});
	transform_.Update();

	// 武器の設定
	transWeapon_.scl = VScale(Utility::VECTOR_ONE, 1.0f);
	// 初期座標
	transWeapon_.pos = WEAPON_INIT_POS;
	transWeapon_.quaRot = Quaternion();
	transWeapon_.quaRotLocal = Quaternion::Euler({
			Utility::Deg2RadF(WEAPON_INIT_ROT_LOCAL.x),
			Utility::Deg2RadF(WEAPON_INIT_ROT_LOCAL.y),
			Utility::Deg2RadF(WEAPON_INIT_ROT_LOCAL.z)
		});
	transWeapon_.Update();
	updateWeapon_ = std::bind(&ViewPlayer::SyncWeaponSowrd, this);
	updateWeapon_();

	weponId_ = WEAPON_ID_NONE;
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

void ViewPlayer::SetChar(int charId)
{
	static std::wstring PATH_MDL = Application::PATH_MODEL;

	MV1DeleteModel(transform_.modelId);

	switch (charId)
	{
	case 0:// ナイト
	{
		transform_.modelId = MV1LoadModel((PATH_MDL + FILE_PLAYER2_MDL).c_str());

		std::wstring path = Application::PATH_MODEL + DIR_PLAYER2;
		animationController_.reset();
		animationController_ = std::make_unique<AnimationController>(transform_.modelId);
		animationController_->Add(ANIM_INDEX_IDLE, path + FILE_ANIM_IDLE, ANIM_SPEED_IDLE);

		animationController_->Play(ANIM_INDEX_IDLE);
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

void ViewPlayer::SetWeapon(int weponId)
{
	if (weponId_ == weponId) { return; }

	static std::wstring PATH_MDL = Application::PATH_MODEL;

	MV1DeleteModel(transWeapon_.modelId);

	weponId_ = weponId;

	switch (weponId)
	{
	case SWORD_ID:// 片手剣
		transWeapon_.modelId = MV1LoadModel((PATH_MDL + FILE_WEAPON_SWORD).c_str());

		transWeapon_.scl = VScale(Utility::VECTOR_ONE, SCALE_SWORD);
		// 初期座標
		transWeapon_.pos = WEAPON_INIT_POS;
		transWeapon_.quaRot = Quaternion();
		transWeapon_.quaRotLocal = Quaternion::Euler({
			Utility::Deg2RadF(WEAPON_INIT_ROT_LOCAL.x),
			Utility::Deg2RadF(WEAPON_INIT_ROT_LOCAL.y),
			Utility::Deg2RadF(WEAPON_INIT_ROT_LOCAL.z)
			});
		transWeapon_.Update();

		updateWeapon_ = std::bind(&ViewPlayer::SyncWeaponSowrd, this);
		break;
	case GREAT_SWORD_ID:// 両手剣
		transWeapon_.modelId = MV1LoadModel((PATH_MDL + FILE_WEAPON_GREAT).c_str());

		transWeapon_.scl = VScale(Utility::VECTOR_ONE, SCALE_GREAT);
		// 初期座標
		transWeapon_.pos = WEAPON_INIT_POS;
		transWeapon_.quaRot = Quaternion();
		transWeapon_.quaRotLocal = Quaternion::Euler({
			Utility::Deg2RadF(WEAPON_INIT_ROT_LOCAL.x),
			Utility::Deg2RadF(WEAPON_INIT_ROT_LOCAL.y),
			Utility::Deg2RadF(WEAPON_INIT_ROT_LOCAL.z)
			});
		transWeapon_.Update();

		updateWeapon_ = std::bind(&ViewPlayer::SyncWeaponGreatSowrd, this);
		break;
	case BOW_ID:// 弓
		transWeapon_.modelId = MV1LoadModel((PATH_MDL + FILE_WEAPON_BOW).c_str());

		transWeapon_.scl = VScale(Utility::VECTOR_ONE, SCALE_BOW);
		// 初期座標
		transWeapon_.pos = WEAPON_INIT_POS;
		transWeapon_.quaRot = Quaternion();
		transWeapon_.quaRotLocal = Quaternion::Euler({
			Utility::Deg2RadF(WEAPON_INIT_ROT_LOCAL.x),
			Utility::Deg2RadF(WEAPON_INIT_ROT_LOCAL.y),
			Utility::Deg2RadF(WEAPON_INIT_ROT_LOCAL.z)
			});
		transWeapon_.Update();

		updateWeapon_ = std::bind(&ViewPlayer::SyncWeaponBow, this);
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
	auto frmNo = MV1SearchFrame(transform_.modelId, FRAME_SPINE2.c_str());
	const auto& posHand = MV1GetFramePosition(transform_.modelId, frmNo);

	MATRIX handRot = MV1GetFrameLocalWorldMatrix(transform_.modelId, frmNo);
	Quaternion handWorldRot = Quaternion::GetRotation(handRot);

	auto scl = transform_.scl;
	scl.x = INVERSE_SCALE_BASE / scl.x;
	scl.y = INVERSE_SCALE_BASE / scl.y;
	scl.z = INVERSE_SCALE_BASE / scl.z;

	auto mixMat = MMult(MGetRotElem(handRot), MGetScale(scl));

	mixMat = MMult(mixMat, MGetRotAxis(VNorm(VTransformSR(Utility::DIR_R, handRot)), Utility::Deg2RadF(GREAT_ROT.x)));
	mixMat = MMult(mixMat, MGetRotAxis(VNorm(VTransformSR(Utility::DIR_U, handRot)), Utility::Deg2RadF(GREAT_ROT.y)));
	mixMat = MMult(mixMat, MGetRotAxis(VNorm(VTransformSR(Utility::DIR_F, handRot)), Utility::Deg2RadF(GREAT_ROT.z)));

	transWeapon_.matRot = mixMat;

	transWeapon_.pos = posHand;
	transWeapon_.pos = VAdd(transWeapon_.pos, VScale(transWeapon_.quaRot.GetDown(), GREAT_POS_OFFSET_DOWN));
	transWeapon_.pos = VAdd(transWeapon_.pos, VScale(transWeapon_.quaRot.GetForward(), GREAT_POS_OFFSET_FORWARD));

	transWeapon_.Update(true);
}

void ViewPlayer::SyncWeaponSowrd()
{
	auto frmNo = MV1SearchFrame(transform_.modelId, FRAME_SPINE.c_str());
	const auto& posHand = MV1GetFramePosition(transform_.modelId, frmNo);

	MATRIX handRot = MV1GetFrameLocalWorldMatrix(transform_.modelId, frmNo);
	Quaternion handWorldRot = Quaternion::GetRotation(handRot);

	auto scl = transform_.scl;
	scl.x = INVERSE_SCALE_BASE / scl.x;
	scl.y = INVERSE_SCALE_BASE / scl.y;
	scl.z = INVERSE_SCALE_BASE / scl.z;

	auto mixMat = MMult(MGetRotElem(handRot), MGetScale(scl));

	mixMat = MMult(mixMat, MGetRotAxis(VNorm(VTransformSR(Utility::DIR_R, handRot)), Utility::Deg2RadF(SWORD_ROT.x)));
	mixMat = MMult(mixMat, MGetRotAxis(VNorm(VTransformSR(Utility::DIR_U, handRot)), Utility::Deg2RadF(SWORD_ROT.y)));
	mixMat = MMult(mixMat, MGetRotAxis(VNorm(VTransformSR(Utility::DIR_F, handRot)), Utility::Deg2RadF(SWORD_ROT.z)));

	transWeapon_.matRot = mixMat;

	transWeapon_.pos = posHand;
	transWeapon_.pos = VAdd(transWeapon_.pos, VScale(handWorldRot.GetForward(), SWORD_POS_OFFSET_FORWARD));
	transWeapon_.pos = VAdd(transWeapon_.pos, VScale(handWorldRot.GetRight(), SWORD_POS_OFFSET_RIGHT));

	transWeapon_.Update(true);
}

void ViewPlayer::SyncWeaponBow()
{
	auto frmNo = MV1SearchFrame(transform_.modelId, FRAME_SPINE.c_str());
	const auto& posHand = MV1GetFramePosition(transform_.modelId, frmNo);

	MATRIX handRot = MV1GetFrameLocalWorldMatrix(transform_.modelId, frmNo);
	Quaternion handWorldRot = Quaternion::GetRotation(handRot);

	auto scl = transform_.scl;
	scl.x = INVERSE_SCALE_BASE / scl.x;
	scl.y = INVERSE_SCALE_BASE / scl.y;
	scl.z = INVERSE_SCALE_BASE / scl.z;

	auto mixMat = MMult(MGetRotElem(handRot), MGetScale(scl));

	mixMat = MMult(mixMat, MGetRotAxis(VNorm(VTransformSR(Utility::DIR_R, handRot)), Utility::Deg2RadF(BOW_ROT.x)));
	mixMat = MMult(mixMat, MGetRotAxis(VNorm(VTransformSR(Utility::DIR_U, handRot)), Utility::Deg2RadF(BOW_ROT.y)));
	mixMat = MMult(mixMat, MGetRotAxis(VNorm(VTransformSR(Utility::DIR_F, handRot)), Utility::Deg2RadF(BOW_ROT.z)));

	transWeapon_.matRot = mixMat;

	transWeapon_.pos = posHand;
	transWeapon_.pos = VAdd(transWeapon_.pos, VScale(handWorldRot.GetForward(), BOW_POS_OFFSET_FORWARD));

	transWeapon_.Update(true);
}