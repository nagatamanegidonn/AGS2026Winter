#pragma once
#include <string>
#include "Vector2.h"
#include "../Manager/InputTextManager.h"

// マウスで矩形を左クリックしたら入力を受け付けるクラス
// 汎用性なし。仮作成。
class InputTextArea
{

public:

	const int COLOR_BACK_DEFAULT = 0x000000;
	const int COLOR_BACK_ACTIVE = 0x2f4f4f;
	const int COLOR_CHAR = 0xffffff;
	const int COLOR_FRAME = 0xffffff;

	InputTextArea(Vector2 pos, Vector2 size, int length);
	~InputTextArea(void);

	// 更新ステップ
	void Update(void);

	// 描画
	void Draw(void);

	// 解放処理
	void Release(void);

	// キー入力ハンドルを取得する
	int GetKeyHandle(void);

	// テキストをセット
	std::wstring GetText(void);
	void SetText(std::wstring text);

	// 入力内容をセット
	void SetKeyInputStringBuffer(void);

	//入力中かどうか
	const bool IsActive(void);

	const void TextActive(void);
	const void SetText(bool value);

	//マウスが判定に入っているか
	const bool IsCleckText(void);

private:

	InputTextManager& manager_;

	// キー入力ハンドル
	int keyInputHandle_;

	int mouseInputOld_;
	int mouseInputNew_;

	// 矩形の左上座標
	Vector2 pos_;

	// 矩形の大きさ
	Vector2 size_;

	// 文字数
	int length_;

	// 入力バッファ
	wchar_t  buffer_[256];

	// 入力テキスト
	std::wstring text_;

	// 描画
	void DrawDefault(void);
	void DrawActive(void);

};
