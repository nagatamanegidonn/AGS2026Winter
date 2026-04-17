#include <DxLib.h>

#include "../Manager/InputManager.h"
#include "InputController.h"

#define PAD_INPUT_RSTICK_UP      (0x10000000)
#define PAD_INPUT_RSTICK_DOWN    (0x20000000)
#define PAD_INPUT_RSTICK_LEFT    (0x40000000)
#define PAD_INPUT_RSTICK_RIGHT   (0x80000000)

InputController::InputController(int padType)
    :
    padType_(padType),
	rightX(0),
	rightY(0),
	currentInput_(),
	lastInput_(),
    inputTable_(
    {
        {KEY::OK,
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_RETURN},
                {PeripheralType::KEYBOARD,KEY_INPUT_SPACE},
                {PeripheralType::GAME_PAD,PAD_INPUT_B},     //Bボタン
                {PeripheralType::GAME_PAD_INS,(int)InputManager::JOYPAD_BTN::RIGHT},     //Bボタン
                {PeripheralType::MOUSE,MOUSE_INPUT_LEFT},   //
            }
        },
        {KEY::OK_SECOND,
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_RETURN},
                {PeripheralType::KEYBOARD,KEY_INPUT_SPACE},
                {PeripheralType::GAME_PAD,PAD_INPUT_B},     //Bボタン
            }
        },
        {KEY::START,
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_P},
                {PeripheralType::GAME_PAD,PAD_INPUT_R},     //STARTボタン
            }
        },

        //左十字移動
        {KEY::FORWARD,
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_W},
                {PeripheralType::GAME_PAD,PAD_INPUT_UP},   
            }
        },
        {KEY::BACK,
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_S},
                {PeripheralType::GAME_PAD,PAD_INPUT_DOWN},  
            }
        },
        {KEY::RIGHT,
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_D},
                {PeripheralType::GAME_PAD,PAD_INPUT_RIGHT},    
            }
        },
        {KEY::LEFT,
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_A},
                {PeripheralType::GAME_PAD,PAD_INPUT_LEFT},    
            }
        },
        //右十字移動
        {KEY::R_FORWARD,
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_UP},
                {PeripheralType::GAME_PAD_STICK,PAD_INPUT_RSTICK_UP},
            }
        },
        {KEY::R_BACK,
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_DOWN},
                {PeripheralType::GAME_PAD_STICK,PAD_INPUT_RSTICK_DOWN},
            }
        },
        {KEY::R_RIGHT,
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_RIGHT},
                {PeripheralType::GAME_PAD_STICK,PAD_INPUT_RSTICK_RIGHT},
            }
        },
        {KEY::R_LEFT,
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_LEFT},
                {PeripheralType::GAME_PAD_STICK,PAD_INPUT_RSTICK_LEFT},
            }
        },

        {KEY::DRAW,//抜刀コマンド
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_RETURN},
                {PeripheralType::KEYBOARD,KEY_INPUT_F},
                {PeripheralType::GAME_PAD_INS,(int)InputManager::JOYPAD_BTN::TOP},
                {PeripheralType::MOUSE,MOUSE_INPUT_LEFT},
            }
        },
        {KEY::CLOSE,//納刀コマンド
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_F},
                {PeripheralType::GAME_PAD_INS,(int)InputManager::JOYPAD_BTN::LEFT}, 
                {PeripheralType::MOUSE,MOUSE_INPUT_RIGHT},
            }
        },
        {KEY::ATTRCK,//攻撃コマンド
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_RETURN},
                {PeripheralType::GAME_PAD,PAD_INPUT_X},
                {PeripheralType::MOUSE,MOUSE_INPUT_LEFT},
            }
        },
        {KEY::DASH,//ダッシュコマンド
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_SPACE},
                {PeripheralType::KEYBOARD,KEY_INPUT_LSHIFT},
                {PeripheralType::GAME_PAD_INS,(int)InputManager::JOYPAD_BTN::R_BTN},
            }
        },
        {KEY::ROLL,//回避コマンド
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_SPACE},
                {PeripheralType::GAME_PAD_INS,(int)InputManager::JOYPAD_BTN::DOWN}, //Aボタン
                {PeripheralType::GAME_PAD,PAD_INPUT_A},                             //Aボタン
            }
        },
        {KEY::AIM,//注目コマンド
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_Q},
                {PeripheralType::GAME_PAD_INS,(int)InputManager::JOYPAD_BTN::L_TRIGGER}, //Aボタン
                //{PeripheralType::GAME_PAD,PAD_INPUT_A},                             //Aボタン
            }
        },
        {KEY::ITEM_SELECT,//採取コマンド
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_B},
                {PeripheralType::GAME_PAD_INS,(int)InputManager::JOYPAD_BTN::L_BTN}, //Aボタン
            }
        },
        {KEY::GET,//採取コマンド
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_G},
                {PeripheralType::GAME_PAD_INS,(int)InputManager::JOYPAD_BTN::RIGHT}, //Aボタン
                {PeripheralType::MOUSE,MOUSE_INPUT_RIGHT},
            }
        },
        {KEY::USE,//使用コマンド
            {
                {PeripheralType::KEYBOARD,KEY_INPUT_V},
                {PeripheralType::GAME_PAD_INS,(int)InputManager::JOYPAD_BTN::LEFT}, //Aボタン
            }
        },
    })
{
    //デッドゾーンの設定
    SetJoypadDeadZone(padType_, 0.35);

    for (const auto& keyValue : inputTable_)
    {
        currentInput_[keyValue.first] = false;
    }
    lastInput_ = currentInput_;
}

InputController::~InputController(void)
{
}

void InputController::Update(void)
{

    lastInput_ = currentInput_;

    /*char keyState[256] = {};
    GetHitKeyStateAll(keyState);*/

    InputManager& ins = InputManager::GetInstance();

    //パッドの状態取得
    int padState = GetJoypadInputState(padType_);
    InputManager::JOYPAD_NO jno = static_cast<InputManager::JOYPAD_NO>(padType_);

    // ( 使用非推奨 )
    // Xboxコントローラー（パッド1）の右スティックの入力状態を取得
    // 右スティックのX/Yを取得
    GetJoypadAnalogInputRight(&rightX, &rightY, padType_);

    //マウスの状態取得
    int mouseState = GetMouseInput();

    const int STICK_THRESHOLD = 10000;

    //if (rightY < -STICK_THRESHOLD) padState |= input.code;
   /* if (rightY > STICK_THRESHOLD)  padState |= PAD_INPUT_RSTICK_DOWN;
    if (rightX < -STICK_THRESHOLD) padState |= PAD_INPUT_RSTICK_LEFT;
    if (rightX > STICK_THRESHOLD)  padState |= PAD_INPUT_RSTICK_RIGHT;*/

    //複数のinputTable_を確認
    for (const auto& keyValue : inputTable_)
    {
        //inputTable_のstd::vector<InputState>の数ぶん回す
        for (auto& input : keyValue.second)
        {
            bool pressed = false;
            if (input.type == PeripheralType::KEYBOARD)
            {
                //pressed = keyState[input.code];
                pressed = ins.IsNew(input.code);
            }
            else if (input.type == PeripheralType::GAME_PAD)
            {
                if (padType_ > 0) { pressed = padState & input.code; }
            }
            else if (input.type == PeripheralType::GAME_PAD_INS)
            {
                if (padType_ > 0) { pressed = ins.IsPadBtnNew(jno, input.code); }
            }
            else if (input.type == PeripheralType::GAME_PAD_STICK)
            {
                if (padType_ > 0) { pressed = IsStick(input.code); }
            }
            else if (input.type == PeripheralType::MOUSE)
            {
                pressed = mouseState & input.code;
            }

            currentInput_[keyValue.first] = pressed;

            //true確認できたらbreak
            if (pressed)
            {
                break;
            }

        }
    }
}

void InputController::DebugDraw(void)
{
#ifdef _DEBUG
    
    InputManager& ins = InputManager::GetInstance();
    InputManager::JOYPAD_NO jno = static_cast<InputManager::JOYPAD_NO>(padType_);

    // 左スティックの横軸
    auto leftStickX = ins.GetJPadInputState(jno).AKeyLX;
    // 左スティックの縦軸
    auto leftStickY = ins.GetJPadInputState(jno).AKeyLY;

    DrawFormatString(300, 100, GetColor(255, 255, 255), L"アナログスティック右 x:%d  y:%d", rightX, rightY);
    DrawFormatString(300, 126, GetColor(255, 255, 255), L"アナログスティック左 x:%d  y:%d", leftStickX, leftStickY);
    //DrawFormatString(300, 116, 0xFFFFFF, "flag: %s", inputController_->IsNew(InputController::KEY::ATTRCK) ? "true" : "false");

#endif
}

bool InputController::IsTriggered(const KEY key)const 
{
	return currentInput_.at(key) && !lastInput_.at(key);
}

bool InputController::IsNew(const KEY key) const
{
    return currentInput_.at(key);
}

float InputController::GetLStickX() const
{
    InputManager& ins = InputManager::GetInstance();
    InputManager::JOYPAD_NO jno = static_cast<InputManager::JOYPAD_NO>(padType_);

    // 左スティックの横軸
    auto leftStickX = ins.GetJPadInputState(jno).AKeyLX;
   
    return static_cast<float>(leftStickX);
}

float InputController::GetLStickY() const
{
    InputManager& ins = InputManager::GetInstance();
    InputManager::JOYPAD_NO jno = static_cast<InputManager::JOYPAD_NO>(padType_);

    // 左スティックの縦軸
    auto leftStickY = ins.GetJPadInputState(jno).AKeyLY;

    return static_cast<float>(leftStickY);
}

bool InputController::IsPeripheralTriggered(const PeripheralType type) const
{
    InputManager& ins = InputManager::GetInstance();
    int padState = GetJoypadInputState(padType_);
    InputManager::JOYPAD_NO jno = static_cast<InputManager::JOYPAD_NO>(padType_);
    int mouseState = GetMouseInput();

    switch (type)
    {
    case PeripheralType::KEYBOARD:
        for (int code = 0; code < 256; ++code)
        {
            if (ins.IsNew(code)) return true;
        }
        break;

    case PeripheralType::GAME_PAD:
        if (padType_ > 0 && padState != 0) return true;
        break;

    case PeripheralType::GAME_PAD_INS:
        if (padType_ > 0)
        {
            for (int btn = 0; btn < static_cast<int>(InputManager::JOYPAD_BTN::MAX); ++btn)
            {
                if (ins.IsPadBtnNew(jno, btn)) return true;
            }
        }
        break;

    case PeripheralType::GAME_PAD_STICK:
        if (padType_ > 0)
        {
            if (IsStick(PAD_INPUT_RSTICK_UP) ||
                IsStick(PAD_INPUT_RSTICK_DOWN) ||
                IsStick(PAD_INPUT_RSTICK_LEFT) ||
                IsStick(PAD_INPUT_RSTICK_RIGHT))
            {
                return true;
            }
        }
        break;

    case PeripheralType::MOUSE:
        if (mouseState != 0) return true;
        break;
    }

    return false;
}

bool InputController::IsStick(uint32_t value)const
{
    if (value == PAD_INPUT_RSTICK_UP)
    {
        if (rightY < -500)return true;
    }
    else if (value == PAD_INPUT_RSTICK_DOWN)
    {
        if (rightY > 500)return true;
    }
    else if (value == PAD_INPUT_RSTICK_LEFT)
    {
        if (rightX < -500)return true;
    }
    else if (value == PAD_INPUT_RSTICK_RIGHT)
    {
        if (rightX > 500)return true;
    }

    return false;
}
