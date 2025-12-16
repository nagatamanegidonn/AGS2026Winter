
#include "../Manager/SceneManager.h"
#include "../Utility/AsoUtility.h"

#include "HitDamage.h"

namespace
{
	constexpr int WEAK_DAMAGE = 20;
}

HitDamage::HitDamage(int& model, std::string boneName, int damage) :parModel_(model)
{
	boneName_ = boneName;

	state_ = STATE::PLAY;
	uiPos_ = AsoUtility::VECTOR_ZERO;
	uiRate_ = 1.0f;
	uiDame_ = damage;

	randPosX_ = (rand() % RAND_RATE) - (RAND_RATE / 2);
	randPosY_ = (rand() % RAND_RATE) - (RAND_RATE / 2);
}

HitDamage::~HitDamage(void)
{
}

void HitDamage::Init(int damage)
{
	state_ = STATE::PLAY;
	uiRate_ = 1.0f;
	uiDame_ = damage;
}

void HitDamage::Update(void)
{
	uiPos_ = AsoUtility::MV1GetFreamPos(parModel_, L"Chest_M");

	if (uiRate_ > 0.0f)
	{
		uiRate_ -= SceneManager::GetInstance().GetDeltaTime();
		if (uiRate_ <= 0.0f)
		{
			uiRate_ = 0.0f;
			state_ = STATE::END;
		}
	}
}

void HitDamage::Draw(void)
{
	// ダメージの表記
	if (state_ == STATE::PLAY)
	{
		VECTOR twoDPos = ConvWorldPosToScreenPos(uiPos_);
		if (twoDPos.z >= 0.0f && twoDPos.z <= 1.0f)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255 * uiRate_);
			std::wstring msg = std::to_wstring(uiDame_);
			SetFontSize(28);
			int len = (int)wcslen(msg.c_str());
			int width = GetDrawStringWidth(msg.c_str(), len);

			// ダメージの大きさで色を変える
			if (uiDame_ > WEAK_DAMAGE) {
				DrawFormatString(twoDPos.x + randPosX_, twoDPos.y + randPosY_, 0xffd700, msg.c_str());
			}
			else
			{
				DrawFormatString(twoDPos.x + randPosX_, twoDPos.y + randPosY_, 0xffffff, msg.c_str());
			}

			SetFontSize(16);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}
}
