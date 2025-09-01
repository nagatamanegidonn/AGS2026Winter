#include <DxLib.h>
#include "../Manager/SceneManager.h"
#include "../Application.h"
#include "Fader.h"

Fader::NET_STATE Fader::GetState(void) const
{
	return state_;
}

bool Fader::IsEnd(void) const
{
	return isEnd_;
}

void Fader::SetFade(NET_STATE state)
{
	state_ = state;
	if (state_ != NET_STATE::NONE)
	{
		isPreEnd_ = false;
		isEnd_ = false;
		
	}
}

void Fader::Init(void)
{
	state_ = NET_STATE::FADE_STOP;
	alpha_ = 0;
	isPreEnd_ = true;
	isEnd_ = true;

}

void Fader::Update(void)
{

	if (isEnd_)
	{
		return;
	}

	switch (state_)
	{
	case NET_STATE::NONE:
		return;

	case NET_STATE::FADE_OUT:
		alpha_ += SPEED_ALPHA;
		if (alpha_ > 255)
		{
			// フェード終了
			alpha_ = 255;
			if (!isPreEnd_)
			{
				// 1フレーム後(Draw後)に終了とする
				isEnd_ = true;
			}
			isPreEnd_ = true;
		}

		break;
	case NET_STATE::FADE_IN:
		alpha_ -= SPEED_ALPHA;
		if (alpha_ < 0)
		{
			// フェード終了
			alpha_ = 0;
			if (isPreEnd_)
			{
				// 1フレーム後(Draw後)に終了とする
				isEnd_ = true;
			}
			isPreEnd_ = true;
		}
		break;

	case NET_STATE::FADE_STOP:
		
		break;

	default:
		return;
	}

}

void Fader::Draw(void)
{

	switch (state_)
	{
	case NET_STATE::NONE:
		return;
	case NET_STATE::FADE_OUT:
	case NET_STATE::FADE_IN:
	case NET_STATE::FADE_STOP:

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)alpha_);
		DrawBox(
			0, 0,
			Application::SCREEN_SIZE_X,
			Application::SCREEN_SIZE_Y,
			0x000000, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		break;
	}

}
