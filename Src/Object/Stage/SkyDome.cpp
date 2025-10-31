#include "../../Manager/ResourceManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Utility/AsoUtility.h"
#include "../Common/Transform.h"
#include "SkyDome.h"

SkyDome::SkyDome(const Transform& syncTransform) : syncTransform_(syncTransform)
{

	state_ = STATE::NONE;
	
}

SkyDome::~SkyDome(void)
{
}

void SkyDome::Init(void)
{

	// モデル制御の基本情報
	transform_.SetModel(
		ResourceManager::GetInstance().LoadModelDuplicate(
			ResourceManager::SRC::SKY_DOME));
	transform_.scl = SCALES;
	transform_.pos = AsoUtility::VECTOR_ZERO;
	transform_.quaRot = Quaternion::Euler(
		0.0f, 
		AsoUtility::Deg2RadF(180.0f),
		0.0f
	);
	transform_.quaRotLocal = Quaternion();
	transform_.Update();

	// Zバッファ無効(突き抜け対策)
	MV1SetUseZBuffer(transform_.modelId, false);
	MV1SetWriteZBuffer(transform_.modelId, false);

	// 状態遷移
	auto sceneId = SceneManager::GetInstance().GetSceneID();
	if(sceneId == SceneManager::SCENE_ID::TITLE)
	{
		ChangeState(STATE::STAY);
	}
	else
	{
		ChangeState(STATE::FOLLOW);
	}

}

void SkyDome::Update(void)
{

	// 更新ステップ
	switch (state_)
	{
	case SkyDome::STATE::NONE:
		UpdateNone();
		break;
	case SkyDome::STATE::STAY:
		UpdateStay();
		break;
	case SkyDome::STATE::FOLLOW:
		UpdateFollow();
		break;
	}

}

void SkyDome::Draw(void)
{
	MV1DrawModel(transform_.modelId);
}

void SkyDome::ChangeState(STATE state)
{

	// 状態変更
	state_ = state;

	// 各状態遷移の初期処理
	switch (state_)
	{
	case SkyDome::STATE::NONE:
		ChangeStateNone();
		break;
	case SkyDome::STATE::STAY:
		ChangeStateStay();
		break;
	case SkyDome::STATE::FOLLOW:
		ChangeStateFollow();
		break;
	}

}

void SkyDome::ChangeStateNone(void)
{
}

void SkyDome::ChangeStateStay(void)
{
}

void SkyDome::ChangeStateFollow(void)
{
	transform_.pos = syncTransform_.pos;
	transform_.Update();
}

void SkyDome::UpdateNone(void)
{
}

void SkyDome::UpdateStay(void)
{
}

void SkyDome::UpdateFollow(void)
{
	transform_.pos = syncTransform_.pos;
	transform_.Update();
}
