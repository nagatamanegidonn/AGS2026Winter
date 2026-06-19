
#include "../Manager/SceneManager.h"
#include "../Utility/Utility.h"

#include "HitDamage.h"

namespace
{
	constexpr int WEAK_DAMAGE = 20;
	constexpr int FONT_SIZE_BEGIN = 28;
	constexpr int FONT_SIZE_END = 16;

	constexpr float BLEND_ALPHA_MAX = 255.0f;
}

HitDamage::HitDamage(int& model, std::wstring boneName, int damage)
	:
	parModel_(model),
	boneName_(boneName),
	state_(STATE::PLAY),
	uiPos_(Utility::VECTOR_ZERO),
	uiRate_(1.0f),
	uiDame_(damage),
	randPosX_((rand() % RAND_RATE) - (RAND_RATE / 2)),
	randPosY_((rand() % RAND_RATE) - (RAND_RATE / 2))
{
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
	// ダメージ表記の位置を更新
	uiPos_ = Utility::MV1GetFreamPos(parModel_, boneName_);

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

void HitDamage::Draw(void) const
{
	// ダメージの表記
	if (state_ == STATE::PLAY)
	{
		VECTOR twoDPos = ConvWorldPosToScreenPos(uiPos_);
		if (twoDPos.z >= 0.0f && twoDPos.z <= 1.0f)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(BLEND_ALPHA_MAX * uiRate_));
			std::wstring msg = std::to_wstring(uiDame_);
			SetFontSize(FONT_SIZE_BEGIN);

			int len = static_cast<int>(wcslen(msg.c_str()));
			int width = GetDrawStringWidth(msg.c_str(), len);

			// ダメージの大きさで色を変える
			if (uiDame_ > WEAK_DAMAGE) {
				DrawFormatString(static_cast<int>(twoDPos.x) + randPosX_, static_cast<int>(twoDPos.y) + randPosY_, 0xffd700, msg.c_str());
			}
			else
			{
				DrawFormatString(static_cast<int>(twoDPos.x) + randPosX_, static_cast<int>(twoDPos.y) + randPosY_, 0xffffff, msg.c_str());
			}

			SetFontSize(FONT_SIZE_END);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}
}
