#define DIRECTINPUT_VERSION 0x0800
#include<dinput.h>
//キー最大数
#define KEY_MAX 256

class Window;
class Input{
public: 
	//コンストラクタ
	Input();
	//デストラクタ
	~Input();
	//キー入力
	bool CheckKey(UINT index);
	//トリガーの入力
	bool TriggerKey(UINT index);
private:
	//インプットの生成
	HRESULT CreateInput(void);
	//キーデバイスの生成
	HRESULT CreateKey(void);
	//キーフォーマットのセット
	HRESULT SetKeyFormat(void);
	//キーの協調レベルのセット
	HRESULT SetKeyCooperative(HWND hwnd);

	//ウィンドウ
	Window *win;
	//参照結果
	HRESULT result;
	//インプット
	IDirectInput8*input;
	//インプットデバイス
	IDirectInputDevice*key;
	//キー情報
	BYTE keys[KEY_MAX];
	//前のキー情報
	BYTE olds[KEY_MAX];
};