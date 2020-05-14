#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<vector>
#include<DirectXMath.h>
#include<d3dcompiler.h>
#include<dinput.h>

using namespace DirectX;

#define DIRECTINPUT_VERSION  0x0800//DirectInputのバージョン指定

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

HRESULT result;
ID3D12Device*dev = nullptr;
IDXGIFactory6*dxgiFactory = nullptr;
IDXGISwapChain4*swapchain = nullptr;
ID3D12CommandAllocator*cmdAllocator = nullptr;
ID3D12GraphicsCommandList*cmdList = nullptr;
ID3D12CommandQueue*cmdQueue = nullptr;
ID3D12DescriptorHeap*rtvHeaps = nullptr;



LRESULT WindowsProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//メッセージで分岐
	switch (msg) {
	case WM_DESTROY://ウィンドウが破棄された
		PostQuitMessage(0);//OSに対して、アプリの終了を伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);//標準の処理を行う
}

void DrawInstanced(
	UINT VertexCountPerInstance,  //頂点数
	UINT InstanceCount,           //インスタンス数(1でよい)
	UINT StartVertexLocation,     //開始頂点番号(0でよい)
	UINT StartInstanceLocation    //インスタンスごとの加算番号(0でよい)
);

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	const int window_width = 1280;//横幅
	const int window_height = 720;//縦幅

	WNDCLASSEX w{};//ウィンドウクラスの設定
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowsProc;//ウィンドウプロシージャを設定
	w.lpszClassName = L"DirectXGame";//ウィンドウクラス名
	w.hInstance = GetModuleHandle(nullptr);//ウィンドウハンドル
	w.hCursor = LoadCursor(NULL, IDC_ARROW);//カーソル指定

	//ウィンドウクラスをOSに登録
	RegisterClassEx(&w);
	//ウィンドウサイズ｛X座標　Y座標　横幅　縦幅｝
	RECT wrc = { 0,0,window_width,window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);//自動でサイズ補正

	//ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindow(w.lpszClassName,//クラス名
		L"DirectXGame",                        //タイトルバーの文字
		WS_OVERLAPPEDWINDOW,                   //標準的なウィンドウスタイル
		CW_USEDEFAULT,                         //表示X座標（OSに任せる）
		CW_USEDEFAULT,                         //表示Y座標（OSに任せる）
		wrc.right - wrc.left,                  //ウィンドウ横幅
		wrc.bottom - wrc.top,                  //ウィンドウ縦幅
		nullptr,                               //親ウィンドウハンドル
		nullptr,                               //メニューハンドル
		w.hInstance,                           //呼び出しアプリケーションハンドル
		nullptr);                              //オプション

	//コンソールへの文字出力
	OutputDebugStringA("Hello,DirectX!!\n");

	//ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);

	MSG msg{};

	//DirectX初期化処理

	//DXGIファクトリーの生成
	result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	//アダプターの列挙用
	std::vector < IDXGIAdapter*>adapters;
	//ここに特定の名前を持つアダプターオブジェクトが入る
	IDXGIAdapter*tmpAdapter = nullptr;
	for (int i = 0;
		dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		i++) {
		adapters.push_back(tmpAdapter);//動的は配列に追加する
	}

	for (int i = 0; i, adapters.size(); i++) {
		DXGI_ADAPTER_DESC adesc{};
		adapters[i]->GetDesc(&adesc);   //アダプターの情報を取得
		std::wstring strDesc = adesc.Description;  //アダプター名
		//Microsoft Basic Render Driver を回避
		if (strDesc.find(L"Microsoft") == std::wstring::npos)
		{
			tmpAdapter = adapters[i];   //採用
			break;
		}
	}

	//対応レベルの配列
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (int i = 0; i < _countof(levels); i++) {
		//採用したアダプターでデバイスを生成
		result = D3D12CreateDevice(tmpAdapter, levels[i], IID_PPV_ARGS(&dev));
		if (result == S_OK) {
			//デバイスを生成できた時点でループを抜ける
			featureLevel = levels[i];
			break;
		}
	}

	//コマンドアロケータを生成
	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAllocator));
	//コマンドリストを生成
	result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList));

	//標準設定でコマンドキューを生成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));

	//各種設定してスワップチェーンを生成
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
	swapchainDesc.Width = 1280;
	swapchainDesc.Height = 720;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     //色情報の書式
	swapchainDesc.SampleDesc.Count = 1;  //マルチサンプルしない
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;//バックバッファ用
	swapchainDesc.BufferCount = 2;       //バッファ数を2つに設定
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//フリップ後は破棄
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	dxgiFactory->CreateSwapChainForHwnd(
		cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&swapchain
	);

	//各種設定をしてディスクリプタヒープを生成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;  //レンダーターゲットビュー
	heapDesc.NumDescriptors = 2;   //裏表の2つ
	dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	//裏表の2つ分について
	std::vector<ID3D12Resource*>backBuffer(2);

	for (int i = 0; i < 2; i++) {
		//スワップチェーンからバッファを取得
		result = swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffer[i]));
		//ディスクリプタヒープのハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE handle =
			rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		//裏か表かでアドレスがずれる
		handle.ptr += i * dev->GetDescriptorHandleIncrementSize(heapDesc.Type);
		//レンダーターゲットビューの生成
		dev->CreateRenderTargetView(
			backBuffer[i],
			nullptr,
			handle
		);
	}

	//フェンスの生成
	ID3D12Fence*fence = nullptr;
	UINT64 fenceVal = 0;
	result = dev->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	//キーボード入力
	//DirectInputオブジェクト生成
	IDirectInput8*dinput = nullptr;
	result = DirectInput8Create(
		w.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);

	//キーボードデバイスの生成
	IDirectInputDevice8*devkeyboard = nullptr;
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);

	//入力データ形式セット
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard);//標準形式

	//排他制御レベルのセット
	result = devkeyboard->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);

	//描画初期化処理

	//頂点データ(3頂点分の座標)
	XMFLOAT3 vertices[] = {
		{-0.5f,-0.5f,0.0f},   //左下
		{-0.5f,+0.5f,0.0f},   //左上
		{+0.5f,-0.5f,0.0f},   //右下
	};

	//頂点バッファの確保
	D3D12_HEAP_PROPERTIES heapprop{};  //頂点ヒープ設定
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;//GPUへの転送用

	D3D12_RESOURCE_DESC resdesc{};     //リソース設定
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);  //頂点情報が入る分のサイズ
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.SampleDesc.Count = 1;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//GPUリソースの生成
	ID3D12Resource*vertBuff = nullptr;
	result = dev->CreateCommittedResource(
		&heapprop,              //ヒープ設定
		D3D12_HEAP_FLAG_NONE,
		&resdesc,               //リソース設定
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	//頂点バッファへのデータ転送
	//GPU上のバッファに対応した仮想メモリを取得
	XMFLOAT3*vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);

	//全頂点に対して
	for (int i = 0; i < _countof(vertices); i++)
	{
		vertMap[i] = vertices[i];   //座標をコピー
	}
	//マップを解除
	vertBuff->Unmap(0, nullptr);

	//頂点バッファビューの作成
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(vertices);
	vbView.StrideInBytes = sizeof(vertices[0]);

	//各シェーダファイルの読み込みとコンパイル
	ID3DBlob*vsBlob = nullptr;     //頂点シェーダオブジェクト
	ID3DBlob*psBlob = nullptr;     //ピクセルシェーダオブジェクト
	ID3DBlob*errorBlob = nullptr;  //エラーオブジェクト

	//頂点シェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",  //シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //インクルード可能にする
		"VSmain", "vs_5_0",          //エントリーポイント名,シェーダモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,//デバッグ用設定
		0,
		&vsBlob, &errorBlob
	);

	//ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",  //シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //インクルード可能にする
		"PSmain", "ps_5_0",          //エントリーポイント名,シェーダモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,//デバッグ用設定
		0,
		&psBlob, &errorBlob
	);

	//シェーダのエラー内容を表示
	if (FAILED(result)) {
		//errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		//エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	//頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
	};

	//グラフィックスパイプライン設定
	//グラフィックスパイプラインの各ステージの設定をする構造体を用意する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	//頂点シェーダ、ピクセルシェーダをパイプラインに設定
	gpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = psBlob->GetBufferSize();
	//サンプルマスクとラスタライザステートの設定
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//標準設定
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  //カリングしない
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;//ワイヤーフレーム表示設定
	gpipeline.RasterizerState.DepthClipEnable = true;  //深度クリッピングを有効に
	//ブレンドステートの設定
	gpipeline.BlendState.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;  //RGBA全てのチャンネルを描画
	//頂点レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);
	//図形の形状を三角形に設定
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//その他のの設定
	gpipeline.NumRenderTargets = 1;   //描画対象は1つ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0〜255指定のRGBA
	gpipeline.SampleDesc.Count = 1;//1ピクセルにつき1回サンプリング

	//ルートシグネチャの生成
	ID3D12RootSignature*rootsignature;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob*rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	result = dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
	rootSigBlob->Release();
	//パイプラインにルートシグネチャをセット
	gpipeline.pRootSignature = rootsignature;

	//パイプラインステートの生成
	ID3D12PipelineState*pipelinestate = nullptr;
	result = dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelinestate));
	while (true)
	{
		//キーボード情報の取得開始
		result = devkeyboard->Acquire();

		//全キーの入力状態を取得する
		BYTE key[256] = {};
		result = devkeyboard->GetDeviceState(sizeof(key), key);

		//キーが押されている時の処理
		if (key[DIK_0]) {   //数字の0キーが押されていたら
			OutputDebugStringA("Hit 0\n");  //出力ウィンドウに「Hit 0」と表示
		}


		//メッセージがあるか？
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//バックバッファの番号を取得(2つなので0番か1番)
		UINT bbIndex = swapchain->GetCurrentBackBufferIndex();
		//1.リソースバリアを変更
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffer[bbIndex];//バックバッファを指定
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;//表示から
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;//描画
		cmdList->ResourceBarrier(1, &barrierDesc);

		//２.画面クリアコマンドここから

		//レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvH =
			rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIndex * dev->GetDescriptorHandleIncrementSize(heapDesc.Type);
		cmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);
		//全画面クリア          R    G     B    A
		float clearColor[] = { 0.1f,0.25f,0.5f,0.0f };//青っぽい色

		//画面クリアカラーを変更してみる例
		if (key[DIK_SPACE]) {
			clearColor[0] = { 1.0f };
		}

		cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);


		//２.画面クリアコマンドここまで

		//３.描画コマンドここから

		//パイプラインステートの設定コマンド
		cmdList->SetPipelineState(pipelinestate);
		//ビューポートの設定コマンド
		D3D12_VIEWPORT viewport{};
		viewport.Width = window_width;
		viewport.Height = window_height;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		cmdList->RSSetViewports(1, &viewport);
		//シザー矩形の設定コマンド
		D3D12_RECT scissorrect{};
		scissorrect.left = 0;                                 //切り抜き座標左
		scissorrect.right = scissorrect.left + window_width;  //切り抜き座標右
		scissorrect.top = 0;                                  //切り抜き座標上
		scissorrect.bottom = scissorrect.top + window_height; //切り抜き座標下
		cmdList->RSSetScissorRects(1, &scissorrect);
		//ルートシグネチャの設定コマンド
		cmdList->SetGraphicsRootSignature(rootsignature);
		//プリミティブ形状の設定コマンド(三角形リスト)
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//頂点バッファの設定コマンド
		cmdList->IASetVertexBuffers(0, 1, &vbView);
		//描画コマンド
		cmdList->DrawInstanced(3, 1, 0, 0);

		//３.描画コマンドここまで

		//４.リソースバリアを戻す
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;//描画 
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;//表示から
		cmdList->ResourceBarrier(1, &barrierDesc);

		//命令のクローズ
		cmdList->Close();
		//コマンドリストの実行
		ID3D12CommandList*cmdLists[] = { cmdList };//コマンドリストの配列
		cmdQueue->ExecuteCommandLists(1, cmdLists);
		//コマンドリストの実行完了を待つ
		cmdQueue->Signal(fence, ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal)
		{
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		cmdAllocator->Reset();//キューをクリア
		cmdList->Reset(cmdAllocator, nullptr);   //再びコマンドリストをためる準備
		//バッファをフリップ(裏表の入れ替え)
		swapchain->Present(1, 0);
		//終了メッセージが来たらループを抜ける
		if (msg.message == WM_QUIT) {
			break;
		}
	}

	//ウィンドウクラスを登録解除
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}

