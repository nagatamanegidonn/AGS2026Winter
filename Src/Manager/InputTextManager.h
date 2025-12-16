#pragma once
// なくても良いけど、
// 有効なキー入力ハンドルが取得できなかったため
class InputTextArea;

class InputTextManager
{

public:

	// 明示的にインステンスを生成する
	static void CreateInstance(void);

	// 静的インスタンスの取得
	static InputTextManager& GetInstance(void);

	// 更新
	static void Update(void);

	// 指定されたキー入力ハンドルを有効にする
	static void SetActive(InputTextArea* inputTextArea);

	// 有効なキー入力ハンドルか判断する
	static bool IsActiveHandle(InputTextArea* inputTextArea);

	static void SetTextArea(bool isSet);

private:

	// 静的インスタンス
	static InputTextManager* instance_;

	// コンストラクタ
	InputTextManager(void);

	// コピーコンストラクタ
	InputTextManager(const InputTextManager& ins);

	// デストラクタ
	~InputTextManager(void);

	// 有効なキー入力ハンドルを持つInputTextArea
	InputTextArea* inputTextArea_;

};

