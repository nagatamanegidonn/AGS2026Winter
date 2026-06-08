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

Fader::Fader(int color)
{
	color_ = color;
	Init();
}

void Fader::Init(float _speed)
{
	state_ = NET_STATE::FADE_STOP;
	alpha_ = 0;
	speed_ = _speed;
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

	case NET_STATE::FADE_OUT:// 徐々に暗転
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
	case NET_STATE::FADE_IN:// 徐々に明転
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

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(alpha_));
		DrawBox(
			0, 0,
			Application::SCREEN_SIZE_X,
			Application::SCREEN_SIZE_Y,
			color_, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		break;
	}
}
