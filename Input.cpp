#include "Input.h"
#include"Window.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

//����}�N��
#define Release(X){if((X)!=nullptr)(X)->Release();(X)=nullptr;}

//�R���X�g���N�^
Input::Input()
	: result(S_OK), input(nullptr), key(nullptr)
{
	memset(&keys, 0, sizeof(keys));
	memset(&olds, 0, sizeof(olds));

	CreateInput();
	CreateKey();
	SetKeyFormat();
}

//�f�X�g���N�^
Input::~Input()
{
	Release(key);
	Release(input);
	delete win;
}

//�C���v�b�g����
bool Input::CheckKey(UINT index)
{
	//�`�F�b�N�t���O
	bool flag = false;

	//�L�[�����擾
	key->GetDeviceState(sizeof(keys), &keys);
	if (keys[index] & 0x80) {
		flag = true;
	}
	olds[index] = keys[index];

	return flag;
}

//�L�[�f�o�C�X�̐���
bool Input::TriggerKey(UINT index)
{
	//�`�F�b�N�t���O
	bool flag = false;

	//�L�[�����擾
	key->GetDeviceState(sizeof(keys), &keys);
	if ((keys[index] & 0x80) && !(olds[index] & 0x80)) {
		flag = true;
	}
	olds[index] = keys[index];

	return flag;
}

//�L�[�t�H�[�}�b�g�̃Z�b�g
HRESULT Input::SetKeyFormat(void)
{
	result = key->SetDataFormat(&c_dfDIKeyboard);

	return result;
}

HRESULT Input::SetKeyCooperative(HWND hwnd)
{
	//�r�����䃌�x���̃Z�b�g
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


