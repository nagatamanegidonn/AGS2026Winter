#include <DxLib.h>
#include "../Manager/InputTextManager.h"
#include "InputTextArea.h"

InputTextArea::InputTextArea(const Vector2 pos, const Vector2 size, int length) 
	: manager_(InputTextManager::GetInstance()),
	pos_(pos), 
	size_(size), 
	length_(length)
{
	keyInputHandle_ = -1;
	buffer_[0] = '\0';

	// 入力タイプはオーバーライドして変えると良いです
	//keyInputHandle_ = MakeKeyInput(length_, false, true, true);
	// 入力制限をすべて無効にして自由入力に
	keyInputHandle_ = MakeKeyInput(length_, false, false, false);
	mouseInputOld_ = mouseInputNew_ = -1;
}

InputTextArea::~InputTextArea(void)
{
	// デストラクタでも安全にハンドルを削除する
	Release();
}

void InputTextArea::Update(void)
{
	// InputManagerの方が早いかもですが、
	// 無くても動くようにDxLib直接を使用
	mouseInputNew_ = GetMouseInput();

	if (mouseInputNew_ == MOUSE_INPUT_LEFT
		&& mouseInputOld_ == 0)
	{

		Vector2 mousePos;
		GetMousePoint(&mousePos.x, &mousePos.y);

		int x1 = pos_.x;
		int y1 = pos_.y;
		int x2 = pos_.x + size_.x;
		int y2 = pos_.y + size_.y;
		if (mousePos.x >= x1 && mousePos.y >= y1
			&& mousePos.x <= x2 && mousePos.y <= y2)
		{
			// 作成したキー入力ハンドルをアクティブにする
			manager_.SetActive(this);
		}
	}

	mouseInputOld_ = mouseInputNew_;
}

void InputTextArea::Draw(void)
{
	if (manager_.IsActiveHandle(this))
	{
		DrawActive();
	}
	else
	{
		DrawDefault();
	}
}

void InputTextArea::Release(void)
{
	// 二重解放を防ぐため、有効なハンドル（-1以外）のときだけ削除
	if (keyInputHandle_ != -1)
	{
		// 用済みのインプットハンドルを削除する
		DeleteKeyInput(keyInputHandle_);
		keyInputHandle_ = -1; // 削除済みフラグとして-1に戻す
	}
}

int InputTextArea::GetKeyHandle(void)
{
	return keyInputHandle_;
}

const std::wstring& InputTextArea::GetText(void)
{
	return text_;
}

void InputTextArea::SetText(const std::wstring& text)
{
	text_ = text;
}

void InputTextArea::SetKeyInputStringBuffer(void)
{
	// 必要なサイズ分のメモリをstd::wstring側に確保する（+1は終端文字用）
	std::wstring tempBuffer(length_ + 1, L'\0');

	// stringの内部バッファ（&tempBuffer[0]）に直接DxLibから書き込んでもらう
	GetKeyInputString(&tempBuffer[0], keyInputHandle_);

	// 余分な末尾のヌル文字をカットして、正規のtext_に代入
	text_ = tempBuffer.c_str();
}

const bool InputTextArea::IsActive(void)
{
	return manager_.IsActiveHandle(this);
}

const void InputTextArea::TextActive(void)
{
	// 作成したキー入力ハンドルをアクティブにする
	manager_.SetActive(this);
}

const void InputTextArea::SetText(bool value)
{
	if (IsActive())
	{
		return;
	}
	//テキスト設定
	manager_.SetTextArea(value);
}

const bool InputTextArea::IsCleckText(void)
{
	Vector2 mousePos;
	GetMousePoint(&mousePos.x, &mousePos.y);

	int x1 = pos_.x;
	int y1 = pos_.y;
	int x2 = pos_.x + size_.x;
	int y2 = pos_.y + size_.y;
	if (mousePos.x >= x1 && mousePos.y >= y1
		&& mousePos.x <= x2 && mousePos.y <= y2)
	{
		return true;
	}
	else{
		return false;
	}
	return false;
}

void InputTextArea::DrawDefault(void)
{
	// 背景
	DrawBox(pos_.x, pos_.y,
		pos_.x + size_.x, pos_.y + size_.y, COLOR_BACK_DEFAULT, true);

	// 枠
	DrawBox(pos_.x, pos_.y,
		pos_.x + size_.x, pos_.y + size_.y, COLOR_FRAME, false);

	// 文字表示
	DrawString(pos_.x + 5, pos_.y + 7, text_.c_str(), COLOR_CHAR);
}

void InputTextArea::DrawActive(void)
{
	// 背景
	DrawBox(pos_.x, pos_.y,
		pos_.x + size_.x, pos_.y + size_.y, COLOR_BACK_ACTIVE, true);

	// 枠
	DrawBox(pos_.x, pos_.y,
		pos_.x + size_.x, pos_.y + size_.y, COLOR_FRAME, false);

	// 入力文字
	DrawKeyInputString(pos_.x + 5, pos_.y + 7, keyInputHandle_);
}
