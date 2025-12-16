#include "Sound.h"

Sound::Sound(void)
{
	soundType_ = TYPE::NONE;
	path_ = L"";

	handleId_ = -1;
	pos_ = { 0.0f,0.0f,0.0f };
	radius_ = 0.0f;
	maxVolume_ = 255;

	pauseTime_ = 0; // 停止時の再生位置（ミリ秒）
	isPaused_ = false;
}

Sound::Sound(TYPE type, const std::wstring& path)
{
	soundType_ = type;
	path_ = path;

	handleId_ = -1;
	pos_ = { 0.0f,0.0f,0.0f };
	radius_ = 0.0f;
	maxVolume_ = 255;

	pauseTime_ = 0; // 停止時の再生位置（ミリ秒）
	isPaused_ = false;
}

Sound::~Sound(void)
{
}

void Sound::Update(VECTOR pos)
{
	if (soundType_ != TYPE::SOUND_3D || CheckSoundMem(handleId_) != 1)
	{
		// 3D以外or再生していない場合
		return;
	}
	int i = Set3DPositionSoundMem(pos, handleId_);
	if (i == -1)
	{
		return ;
	}
	pos_ = pos;
}

void Sound::Load(void)
{
	switch (soundType_)
	{
	case Sound::TYPE::NONE:
		break;
	case Sound::TYPE::SOUND_2D:
		SetCreate3DSoundFlag(false);
		handleId_ = LoadSoundMem(path_.c_str());
		break;
	case Sound::TYPE::SOUND_3D:
		SetCreate3DSoundFlag(true);
		handleId_ = LoadSoundMem(path_.c_str());
		break;
	default:
		break;
	}
}


void Sound::Release(void)
{
	if (handleId_ == -1)
	{
		return;
	}
	if (CheckSoundMem(handleId_) == 1)
	{
		StopSoundMem(handleId_);
	}
	DeleteSoundMem(handleId_);
}

bool Sound::Play(TIMES times, bool isForce)
{
	if (soundType_ != TYPE::SOUND_2D || handleId_ == -1)
	{
		return false;	// 失敗
	}
	if (!isForce && CheckMove())
	{
		return false;
	}
	int i = PlaySoundMem(handleId_, times == TIMES::ONCE ? DX_PLAYTYPE_BACK : DX_PLAYTYPE_LOOP, true);
	//ChengeVolume(1.0f);
	return i == 0 ? true : false;
}

bool Sound::Play(VECTOR pos, float radius, TIMES times)
{
	if (soundType_ != TYPE::SOUND_3D || handleId_ == -1)
	{
		return false;	// 失敗
	}
	if (CheckMove())
	{
		return false;
	}
	int i = Set3DPositionSoundMem(pos, handleId_);
	if (i == -1)
	{
		return false;
	}
	i = Set3DRadiusSoundMem(radius, handleId_);
	if (i == -1)
	{
		return false;
	}
	i = PlaySoundMem(handleId_, times == TIMES::ONCE ? DX_PLAYTYPE_BACK : DX_PLAYTYPE_LOOP, true);
	if (i == -1)
	{
		return false;
	}
	pos_ = pos;
	radius_ = radius;
	return true;
}

void Sound::Stop(void)
{
	if (CheckSoundMem(handleId_) != 1)
	{
		return;
	}
	int i = StopSoundMem(handleId_);
	if (i == -1)
	{
		return;
	}
	pos_ = { 0.0f,0.0f,0.0f };
	radius_ = 0.0f;
}

bool Sound::CheckMove(void)
{
	if (CheckSoundMem(handleId_) != 1)
	{
		return false;
	}
	return true;
}

bool Sound::CheckLoad(void)
{
	return handleId_ != -1;
}

void Sound::ChengeVolume(float per)
{
	if (per > 1.0f)
	{
		per = 1.0f;
	}
	if (per < 0.0f)
	{
		per = 0.0f;
	}
	ChangeVolumeSoundMem(static_cast<int>(per * static_cast<float>(maxVolume_)), handleId_);
}
// 基本的にInit時のみ使用
void Sound::ChengeMaxVolume(float per)
{
	maxVolume_ = 255 * per;
	ChengeVolume(1.0f);
}

// ポーズ一時停止(未完成)
void Sound::Pause(TIMES times)
{
	if (handleId_ == -1 || isPaused_) return;

	if (CheckSoundMem(handleId_) == 1) // 再生中なら
	{
		pauseTime_ = GetSoundCurrentTime(handleId_);
		StopSoundMem(handleId_);
		isPaused_ = true;
	}
}
void Sound::Resume(TIMES times)
{
	if (handleId_ == -1 || !isPaused_) return;

	SetSoundCurrentTime(pauseTime_, handleId_);
	PlaySoundMem(handleId_, times == TIMES::ONCE ? DX_PLAYTYPE_BACK : DX_PLAYTYPE_LOOP, true); // ループで再生
	isPaused_ = false;
}