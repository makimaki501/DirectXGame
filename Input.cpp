#include "Input.h"
#include"Window.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

//解放マクロ
#define Release(X){if((X)!=nullptr)(X)->Release();(X)=nullptr;}

//コンストラクタ
Input::Input()
	: result(S_OK), input(nullptr), key(nullptr)
{
	memset(&keys, 0, sizeof(keys));
	memset(&olds, 0, sizeof(olds));

	CreateInput();
	CreateKey();
	SetKeyFormat();
}

//デストラクタ
Input::~Input()
{
	Release(key);
	Release(input);
	delete win;
}

//インプット生成
bool Input::CheckKey(UINT index)
{
	//チェックフラグ
	bool flag = false;

	//キー情報を取得
	key->GetDeviceState(sizeof(keys), &keys);
	if (keys[index] & 0x80) {
		flag = true;
	}
	olds[index] = keys[index];

	return flag;
}

//キーデバイスの生成
bool Input::TriggerKey(UINT index)
{
	//チェックフラグ
	bool flag = false;

	//キー情報を取得
	key->GetDeviceState(sizeof(keys), &keys);
	if ((keys[index] & 0x80) && !(olds[index] & 0x80)) {
		flag = true;
	}
	olds[index] = keys[index];

	return flag;
}

//キーフォーマットのセット
HRESULT Input::SetKeyFormat(void)
{
	result = key->SetDataFormat(&c_dfDIKeyboard);

	return result;
}

HRESULT Input::SetKeyCooperative(HWND hwnd)
{
	//排他制御レベルのセット
	result = key->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);

	return result;
}


HRESULT Input::CreateInput(void)
{
	result = DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)(&input), nullptr);

	return result;
}

HRESULT Input::CreateKey(void)
{
	//result = input->CreateDevice(GUID_SysKeyboard, &key, NULL);

	return result;
}


