#include<Windows.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<vector>
#include<DirectXMath.h>
#include<d3dcompiler.h>
#include"Input.h"

using namespace DirectX;

#define DIRECTINPUT_VERSION  0x0800//DirectInput�̃o�[�W�����w��

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
D3D12_VIEWPORT viewports[4];

bool triangle = true;
int vertexNum = 3;

int keyCnt, keyCnt2;
bool wireframe;

float moveX = 0.0f, moveY = 0.0f;

LRESULT WindowsProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//���b�Z�[�W�ŕ���
	switch (msg) {
	case WM_DESTROY://�E�B���h�E���j�����ꂽ
		PostQuitMessage(0);//OS�ɑ΂��āA�A�v���̏I����`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);//�W���̏������s��
}

//Windows�A�v���ł̃G���g���[�|�C���g(main�֐�)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	const int window_width = 1280;//����
	const int window_height = 720;//�c��

	WNDCLASSEX w{};//�E�B���h�E�N���X�̐ݒ�
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowsProc;//�E�B���h�E�v���V�[�W����ݒ�
	w.lpszClassName = L"DirectXGame";//�E�B���h�E�N���X��
	w.hInstance = GetModuleHandle(nullptr);//�E�B���h�E�n���h��
	w.hCursor = LoadCursor(NULL, IDC_ARROW);//�J�[�\���w��

	//�E�B���h�E�N���X��OS�ɓo�^
	RegisterClassEx(&w);
	//�E�B���h�E�T�C�Y�oX���W�@Y���W�@�����@�c���p
	RECT wrc = { 0,0,window_width,window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);//�����ŃT�C�Y�␳

	//�E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName,//�N���X��
		L"DirectXGame",                        //�^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,                   //�W���I�ȃE�B���h�E�X�^�C��
		CW_USEDEFAULT,                         //�\��X���W�iOS�ɔC����j
		CW_USEDEFAULT,                         //�\��Y���W�iOS�ɔC����j
		wrc.right - wrc.left,                  //�E�B���h�E����
		wrc.bottom - wrc.top,                  //�E�B���h�E�c��
		nullptr,                               //�e�E�B���h�E�n���h��
		nullptr,                               //���j���[�n���h��
		w.hInstance,                           //�Ăяo���A�v���P�[�V�����n���h��
		nullptr);                              //�I�v�V����

	//�R���\�[���ւ̕����o��
	OutputDebugStringA("Hello,DirectX!!\n");

	//�E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	MSG msg{};

#pragma region DirectX����������
	//DXGI�t�@�N�g���[�̐���
	result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	//�A�_�v�^�[�̗񋓗p
	std::vector < IDXGIAdapter*>adapters;
	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter*tmpAdapter = nullptr;
	for (int i = 0;
		dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		i++) {
		adapters.push_back(tmpAdapter);//���I�͔z��ɒǉ�����
	}

	for (int i = 0; i, adapters.size(); i++) {
		DXGI_ADAPTER_DESC adesc{};
		adapters[i]->GetDesc(&adesc);   //�A�_�v�^�[�̏����擾
		std::wstring strDesc = adesc.Description;  //�A�_�v�^�[��
		//Microsoft Basic Render Driver �����
		if (strDesc.find(L"Microsoft") == std::wstring::npos)
		{
			tmpAdapter = adapters[i];   //�̗p
			break;
		}
	}

	//�Ή����x���̔z��
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (int i = 0; i < _countof(levels); i++) {
		//�̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
		result = D3D12CreateDevice(tmpAdapter, levels[i], IID_PPV_ARGS(&dev));
		if (result == S_OK) {
			//�f�o�C�X�𐶐��ł������_�Ń��[�v�𔲂���
			featureLevel = levels[i];
			break;
		}
	}

	//�R�}���h�A���P�[�^�𐶐�
	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAllocator));
	//�R�}���h���X�g�𐶐�
	result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList));

	//�W���ݒ�ŃR�}���h�L���[�𐶐�
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));

	//�e��ݒ肵�ăX���b�v�`�F�[���𐶐�
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
	swapchainDesc.Width = 1280;
	swapchainDesc.Height = 720;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     //�F���̏���
	swapchainDesc.SampleDesc.Count = 1;  //�}���`�T���v�����Ȃ�
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;//�o�b�N�o�b�t�@�p
	swapchainDesc.BufferCount = 2;       //�o�b�t�@����2�ɐݒ�
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//�t���b�v��͔j��
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	dxgiFactory->CreateSwapChainForHwnd(
		cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&swapchain
	);

	//�e��ݒ�����ăf�B�X�N���v�^�q�[�v�𐶐�
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;  //�����_�[�^�[�Q�b�g�r���[
	heapDesc.NumDescriptors = 2;   //���\��2��
	dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	//���\��2���ɂ���
	std::vector<ID3D12Resource*>backBuffer(2);

	for (int i = 0; i < 2; i++) {
		//�X���b�v�`�F�[������o�b�t�@���擾
		result = swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffer[i]));
		//�f�B�X�N���v�^�q�[�v�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE handle =
			rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		//�����\���ŃA�h���X�������
		handle.ptr += i * dev->GetDescriptorHandleIncrementSize(heapDesc.Type);
		//�����_�[�^�[�Q�b�g�r���[�̐���
		dev->CreateRenderTargetView(
			backBuffer[i],
			nullptr,
			handle
		);
	}

	//�t�F���X�̐���
	ID3D12Fence*fence = nullptr;
	UINT64 fenceVal = 0;
	result = dev->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
#pragma endregion

#pragma region �L�[�{�[�h����
	//DirectInput�I�u�W�F�N�g����
	IDirectInput8*dinput = nullptr;
	result = DirectInput8Create(
		w.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);

	//�L�[�{�[�h�f�o�C�X�̐���
	IDirectInputDevice8*devkeyboard = nullptr;
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);

	//���̓f�[�^�`���Z�b�g
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard);//�W���`��

	//�r�����䃌�x���̃Z�b�g
	result = devkeyboard->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
#pragma endregion

#pragma region �`�揉��������
	//���_�f�[�^(4���_���̍��W)
	XMFLOAT3 vertices[] = {
		{-0.5f,-0.5f,0.0f},   //����
		{-0.5f,+0.5f,0.0f},   //����
		{+0.5f,-0.5f,0.0f},   //�E��
		{+0.5f,+0.5f,0.0f},   //�E��
	};

	//���_�o�b�t�@�̊m��
	D3D12_HEAP_PROPERTIES heapprop{};  //���_�q�[�v�ݒ�
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;//GPU�ւ̓]���p

	D3D12_RESOURCE_DESC resdesc{};     //���\�[�X�ݒ�
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);  //���_��񂪓��镪�̃T�C�Y
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.SampleDesc.Count = 1;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//GPU���\�[�X�̐���
	ID3D12Resource*vertBuff = nullptr;
	result = dev->CreateCommittedResource(
		&heapprop,              //�q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resdesc,               //���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	//���_�o�b�t�@�ւ̃f�[�^�]��
	//GPU��̃o�b�t�@�ɑΉ��������z���������擾
	XMFLOAT3*vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	//�S���_�ɑ΂���
	for (int i = 0; i < _countof(vertices); i++)
	{
		vertMap[i] = vertices[i];   //���W���R�s�[
	}
	//�}�b�v������
	vertBuff->Unmap(0, nullptr);

	//���_�o�b�t�@�r���[�̍쐬
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(vertices);
	vbView.StrideInBytes = sizeof(vertices[0]);

	//�e�V�F�[�_�t�@�C���̓ǂݍ��݂ƃR���p�C��
	ID3DBlob*vsBlob = nullptr;     //���_�V�F�[�_�I�u�W�F�N�g
	ID3DBlob*psBlob = nullptr;     //�s�N�Z���V�F�[�_�I�u�W�F�N�g
	ID3DBlob*errorBlob = nullptr;  //�G���[�I�u�W�F�N�g

	//���_�V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",  //�V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //�C���N���[�h�\�ɂ���
		"VSmain", "vs_5_0",          //�G���g���[�|�C���g��,�V�F�[�_���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,//�f�o�b�O�p�ݒ�
		0,
		&vsBlob, &errorBlob
	);

	//�s�N�Z���V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",  //�V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //�C���N���[�h�\�ɂ���
		"PSmain", "ps_5_0",          //�G���g���[�|�C���g��,�V�F�[�_���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,//�f�o�b�O�p�ݒ�
		0,
		&psBlob, &errorBlob
	);

	//�V�F�[�_�̃G���[���e��\��
	if (FAILED(result)) {
		//errorBlob����G���[���e��string�^�ɃR�s�[
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		//�G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	//���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
	};

	//�O���t�B�b�N�X�p�C�v���C���ݒ�
	//�O���t�B�b�N�X�p�C�v���C���̊e�X�e�[�W�̐ݒ������\���̂�p�ӂ���
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	//���_�V�F�[�_�A�s�N�Z���V�F�[�_���p�C�v���C���ɐݒ�
	gpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = psBlob->GetBufferSize();

	//�T���v���}�X�N�ƃ��X�^���C�U�X�e�[�g�̐ݒ�
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//�W���ݒ�
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  //�J�����O���Ȃ�
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//���C���[�t���[���\���ݒ�
	gpipeline.RasterizerState.DepthClipEnable = true;  //�[�x�N���b�s���O��L����

	//�����_�[�^�[�Q�b�g�̃u�����h�ݒ�(8���邪�A���͈�����g��Ȃ�)
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};

	//�u�����h�X�e�[�g�̐ݒ�
	//gpipeline.BlendState.RenderTarget[0] = blenddesc;
	gpipeline.BlendState.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;


	blenddesc.BlendEnable = true;                   //�u�����h��L���ɂ���
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;    //���Z
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;      //�\�[�X�̒l��100���g��
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;    //�f�X�g�̒l��  0���g��

	//���_���C�A�E�g�̐ݒ�
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);
	//�}�`�̌`����O�p�`�ɐݒ�
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//���̑��̂̐ݒ�
	gpipeline.NumRenderTargets = 1;   //�`��Ώۂ�1��
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0�`255�w���RGBA
	gpipeline.SampleDesc.Count = 1;//1�s�N�Z���ɂ�1��T���v�����O

	//���[�g�V�O�l�`���̐���
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
	//�p�C�v���C���Ƀ��[�g�V�O�l�`�����Z�b�g
	gpipeline.pRootSignature = rootsignature;

	//�p�C�v���C���X�e�[�g�̐���
	ID3D12PipelineState*pipelinestate = nullptr;
	result = dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelinestate));
#pragma endregion


	while (true)
	{
		//�L�[�{�[�h���̎擾�J�n
		result = devkeyboard->Acquire();

		//�S�L�[�̓��͏�Ԃ��擾����
		BYTE key[256] = {};
		result = devkeyboard->GetDeviceState(sizeof(key), key);
		//���b�Z�[�W�����邩�H
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}


		//�o�b�N�o�b�t�@�̔ԍ����擾(2�Ȃ̂�0�Ԃ�1��)
		UINT bbIndex = swapchain->GetCurrentBackBufferIndex();
		//1.���\�[�X�o���A��ύX
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffer[bbIndex];//�o�b�N�o�b�t�@���w��
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;//�\������
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;//�`��
		cmdList->ResourceBarrier(1, &barrierDesc);

		//�Q.��ʃN���A�R�}���h��������

		//�����_�[�^�[�Q�b�g�r���[�p�f�B�X�N���v�^�q�[�v�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvH =
			rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIndex * dev->GetDescriptorHandleIncrementSize(heapDesc.Type);
		cmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);
		//�S��ʃN���A          R    G     B    A
		float clearColor[] = { 1.0f,0.2f,1.0f,0.0f };//���ۂ��F

		cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
		//�Q.��ʃN���A�R�}���h�����܂�


		//�L�[����
		if (key[DIK_1]) {
			keyCnt++;
		}
		else {
			keyCnt = 0;
		}
		if (keyCnt == 1) {
			switch (vertexNum)
			{
			case 3:
				vertexNum = 4;
				break;
			case 4:
				vertexNum = 3;
				break;
			default:
				break;
			}
		}

		if (key[DIK_2]) {
			keyCnt2++;

		}
		else {
			keyCnt2 = 0;
		}

		if (keyCnt2 == 1) {
			wireframe = !wireframe;
			if (wireframe) {
				gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;//���C���[�t���[���\���ݒ�
			}
			else {
				gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//���C���[�t���[���\���ݒ�
			}
			result = dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelinestate));
		}

		if (key[DIK_UP]) {
			moveY += 0.01f;
		}

		if (key[DIK_DOWN]) {
			moveY -= 0.01f;
		}

		if (key[DIK_RIGHT]) {
			moveX += 0.01f;
		}

		if (key[DIK_LEFT]) {
			moveX -= 0.01f;
		}

		//���_
		XMFLOAT3 vertices[] = {
		{-0.5f + moveX,-0.5f + moveY,0.0f},   //����
		{-0.5f + moveX,+0.5f + moveY,0.0f},   //����
		{+0.5f + moveX,-0.5f + moveY,0.0f},   //�E��
		{+0.5f + moveX,+0.5f + moveY,0.0f},   //�E��
		};
		//�S���_�ɑ΂���
		for (int i = 0; i < _countof(vertices); i++)
		{
			vertMap[i] = vertices[i];   //���W���R�s�[
		}
		//�}�b�v������
		vertBuff->Unmap(0, nullptr);


		//�R.�`��R�}���h��������

		//�p�C�v���C���X�e�[�g�̐ݒ�R�}���h
		cmdList->SetPipelineState(pipelinestate);

		//�r���[�|�[�g������
		{
			viewports[0].Width = window_width / 2 + 200;
			viewports[0].Height = window_height / 2;
			viewports[0].TopLeftX = 0;
			viewports[0].TopLeftY = 0;
			viewports[0].MinDepth = 0.0f;
			viewports[0].MaxDepth = 1.0f;

			viewports[1].Width = window_width / 2 - 200;
			viewports[1].Height = window_height / 2 + 100;
			viewports[1].TopLeftX = 840;
			viewports[1].TopLeftY = 0;
			viewports[1].MinDepth = 0.0f;
			viewports[1].MaxDepth = 1.0f;

			viewports[2].Width = window_width / 2 + 200;
			viewports[2].Height = window_height / 2 - 100;
			viewports[2].TopLeftX = 0;
			viewports[2].TopLeftY = 460;
			viewports[2].MinDepth = 0.0f;
			viewports[2].MaxDepth = 1.0f;

			viewports[3].Width = window_width / 2 - 200;
			viewports[3].Height = window_height / 2 - 100;
			viewports[3].TopLeftX = 840;
			viewports[3].TopLeftY = 460;
			viewports[3].MinDepth = 0.0f;
			viewports[3].MaxDepth = 1.0f;
		}

		//�V�U�[��`�̐ݒ�R�}���h
		D3D12_RECT scissorrect{};
		scissorrect.left = 0;                                 //�؂蔲�����W��
		scissorrect.right = scissorrect.left + window_width;  //�؂蔲�����W�E
		scissorrect.top = 0;                                  //�؂蔲�����W��
		scissorrect.bottom = scissorrect.top + window_height; //�؂蔲�����W��
		cmdList->RSSetScissorRects(1, &scissorrect);

		//���[�g�V�O�l�`���̐ݒ�R�}���h
		cmdList->SetGraphicsRootSignature(rootsignature);

		//�v���~�e�B�u�`��̐ݒ�R�}���h(�O�p�`���X�g)
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		//���_�o�b�t�@�̐ݒ�R�}���h
		cmdList->IASetVertexBuffers(0, 1, &vbView);

		for (int i = 0; i < sizeof(viewports) / sizeof(viewports[0]); i++) {
			//�r���[�|�[�g��ݒ�
			cmdList->RSSetViewports(1, &viewports[i]);

			//�`��R�}���h
			cmdList->DrawInstanced(vertexNum, 1, 0, 0);
		}
		//�R.�`��R�}���h�����܂�

		//�S.���\�[�X�o���A��߂�
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;//�`�� 
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;//�\������
		cmdList->ResourceBarrier(1, &barrierDesc);

		if (key[DIK_1]) {
			wireframe = !wireframe;
		}
		if (wireframe) {
			gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		}
		else {
			gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		}

		//���߂̃N���[�Y
		cmdList->Close();
		//�R�}���h���X�g�̎��s
		ID3D12CommandList*cmdLists[] = { cmdList };//�R�}���h���X�g�̔z��
		cmdQueue->ExecuteCommandLists(1, cmdLists);
		//�R�}���h���X�g�̎��s������҂�
		cmdQueue->Signal(fence, ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal)
		{
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		cmdAllocator->Reset();//�L���[���N���A
		cmdList->Reset(cmdAllocator, nullptr);   //�ĂуR�}���h���X�g�����߂鏀��
		//�o�b�t�@���t���b�v(���\�̓���ւ�)
		swapchain->Present(1, 0);
		//�I�����b�Z�[�W�������烋�[�v�𔲂���
		if (msg.message == WM_QUIT) {
			break;
		}
	}

	//�E�B���h�E�N���X��o�^����
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}

