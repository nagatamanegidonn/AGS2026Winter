#pragma once
#include <map>
#include <vector>


class InputController
{
public:
	enum class KEY
	{
		OK,
		OK_SECOND,
		OK_MOUSE,
		START,

		A_KEY,
		B_KEY,
		C_KEY,
		D_KEY,

		FORWARD,
		BACK,
		RIGHT,
		LEFT,
		
		R_FORWARD,
		R_BACK,
		R_RIGHT,
		R_LEFT,
		
		DRAW,
		CLOSE,
		ATTRCK,
		DASH,
		ROLL,

		AIM,

		ITEM_SELECT,
		GET,
		USE,
	};

	enum class PeripheralType {
		KEYBOARD,		//
		GAME_PAD,		//
		GAME_PAD_INS,	//INputManager用のTyoe
		GAME_PAD_STICK,	//INputManager用のTyoe
		MOUSE,			//
	};

	InputController(int padType);
	~InputController(void);

	//更新
	void Update(void);
	void DebugDraw(void);

	bool IsTriggered(const KEY key)const;
	bool IsNew(const KEY key)const;

	float GetLStickX()const;
	float GetLStickY()const;

	bool IsPeripheralTriggered(const PeripheralType type) const;

private:
	struct InputState
	{
		PeripheralType type;//周辺機器種別
		uint32_t code;			//入力コード（汎用）
	};

	using InputTable_t = std::map<KEY, std::vector<InputState>>;
	InputTable_t inputTable_;	//機器ごとによる判定を多なうためのkey

	using InputData_t = std::map<KEY, bool>;//機器ごとによる判定
	InputData_t currentInput_;	//最新の
	InputData_t lastInput_;		//直前の

	int padType_;
	int rightX, rightY = 0;

	bool IsStick(uint32_t value)const;
};

