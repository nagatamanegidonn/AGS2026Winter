
#include <DxLib.h>
#include "../Manager/ResourceManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Common/Transform.h"
#include "../Player/Player.h"

#include "../Utility/AsoUtility.h"

#include "Object.h"

Object::Object(
	Player& player, const Transform& transform, STATE state) : player_(player)
{

	transform_ = transform;

	state_ = state;

	// 状態管理
	stateChanges_.emplace(STATE::PLAY, std::bind(&Object::ChangeStatePlay, this));
	stateChanges_.emplace(STATE::ROLL, std::bind(&Object::ChangeStateRoll, this));

}

Object::~Object(void)
{
}

void Object::Init(void)
{

	transform_.Update();

	ChangeState(state_);

}

void Object::Update(void)
{
	VECTOR light = GetLightDirection();

	// 更新ステップ
	stateUpdate_();

}

void Object::Draw(void)
{
	//renderer_->Draw();
	MV1DrawModel(transform_.modelId);
}

void Object::ChangeState(STATE state)
{

	// 状態変更
	state_ = state;

	// 各状態遷移の初期処理
	stateChanges_[state_]();

}

void Object::ChangeStateNone(void)
{
}
void Object::ChangeStatePlay(void)
{
	stateUpdate_ = std::bind(&Object::UpdatePlay, this);
}
void Object::ChangeStateRoll(void)
{
	stateUpdate_ = std::bind(&Object::UpdateRoll, this);
}


void Object::UpdateNone(void)
{
}
void Object::UpdatePlay(void)
{
	
}
void Object::UpdateRoll(void)
{
	transform_.quaRot = transform_.quaRot.Mult(
		Quaternion::Euler(0.0f, AsoUtility::Deg2RadF(-1.0f), 0.0f));
	transform_.Update();
}
