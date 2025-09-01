#include <DxLib.h>
#include "../Common/InputTextArea.h"
#include "InputTextManager.h"

InputTextManager* InputTextManager::instance_ = nullptr;

void InputTextManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new InputTextManager();
	}
}

InputTextManager& InputTextManager::GetInstance(void)
{
	return *instance_;
}

void InputTextManager::Update(void)
{

	if (instance_->inputTextArea_ == nullptr)
	{
		return;
	}

	// 入力が終了しているか判断
	// ０　：キー入力はまだ完了もキャンセルもされていない
	// １　：キー入力は正常に終了した
	// ２　：キー入力はキャンセルされた
	int handle = instance_->inputTextArea_->GetKeyHandle();
	int ret = CheckKeyInput(handle);
	if (ret != 0)
	{

		// 入力された文字列をセット
		instance_->inputTextArea_->SetKeyInputStringBuffer();
		
		// キー入力が終了しているはず
		instance_->inputTextArea_ = nullptr;

	}

}

void InputTextManager::SetActive(InputTextArea* inputTextArea)
{
	// 作成したキー入力ハンドルをアクティブにする
	SetActiveKeyInput(inputTextArea->GetKeyHandle());
	instance_->inputTextArea_ = inputTextArea;
}

bool InputTextManager::IsActiveHandle(InputTextArea* inputTextArea)
{
	if (instance_->inputTextArea_ == nullptr)
	{
		return false;
	}
	return instance_->inputTextArea_ == inputTextArea;
}

void InputTextManager::SetTextArea(bool isSet)
{
	if (instance_->inputTextArea_ == nullptr)
	{
		return;
	}


	// 入力された文字列をセット
	instance_->inputTextArea_->SetKeyInputStringBuffer();

	// キー入力が終了しているはず
	instance_->inputTextArea_ = nullptr;


}

InputTextManager::InputTextManager(void)
{
	inputTextArea_ = nullptr;
}

InputTextManager::InputTextManager(const InputTextManager& ins)
{
}

InputTextManager::~InputTextManager(void)
{
}
