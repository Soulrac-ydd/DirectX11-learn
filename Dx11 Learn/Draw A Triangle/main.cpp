
//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <D3DX10.h>
#include <xnamath.h>

//Global Declarations - Interfaces//
IDXGISwapChain* SwapChain;
ID3D11Device* d3d11Device;
ID3D11DeviceContext* d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;

///////////////**************new**************////////////////////
ID3D11Buffer* triangleVertBuffer;
ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* VS_Buffer;
ID3D10Blob* PS_Buffer;
ID3D11InputLayout* vertLayout;
///////////////**************new**************////////////////////

//Global Declarations - Others//
LPCTSTR WndClassName = L"firstwindow";
HWND hwnd = NULL;
HRESULT hr;

const int Width = 400;
const int Height = 400;

//Function Prototypes//
bool InitializeDirect3d11App(HINSTANCE hInstance);
void CleanUp();
bool InitScene();
void UpdateScene();
void DrawScene();

bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool windowed);
int messageloop();

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

///////////////**************new**************////////////////////
//Vertex Structure and Vertex Layout (Input Layout)//
struct Vertex	//Overloaded Vertex Structure
{
	Vertex() {}
	Vertex(float x, float y, float z) : pos(x, y, z) {}

	XMFLOAT3 pos;
};

//输入数据类型的数组
D3D11_INPUT_ELEMENT_DESC layout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	/*
	D3D11_INPUT_ELEMENT_DESC结构成员说明：

		typedef struct D3D11_INPUT_ELEMENT_DESC
		{
			LPCSTR SemanticName;
			UINT SemanticIndex;
			DXGI_FORMAT Format;
			UINT InputSlot;
			UINT AlignedByteOffset;
			D3D11_INPUT_CLASSIFICATION InputSlotClass;
			UINT InstanceDataStepRate;
		}D3D11_INPUT_ELEMENT_DESC;

		SemanticName：这只是一个与元素关联的字符串。此字符串将用于将顶点结构中的元素映射到顶点着色器中的元素。（即HLSL中的语义）

		SemanticIndex： 这基本上只是用作SemanticName之后的数字。例如，如果我们在顶点结构中有2个纹理元素，而不是创建2个不同的纹理语义名称，我们可以使用2个不同的索引。
						如果顶点着色器代码中的语义名称后面没有索引，则默认为索引0.例如，在我们的着色器代码中，我们的语义名称为“POSITION”，实际上与“POSITION0”相同。

		Format： 对应输入结构中元素的类型格式，必须是DXGI_FORMAT枚举类型的成员。例如输入结构中元素是描述位置的3D矢量，便可使用DXGI_FORMAT_R32G32B32_FLOAT。

		InputSlot： Direct3D允许使用16个不同的元素槽（0-15），可以在其中放置​​顶点数据。如果我们的顶点结构具有位置和颜色，
					我们可以将两个元素放在同一个输入槽中，或者我们可以将位置数据放在第一个槽中，将颜色数据放在第二个槽中。

		AlignedByteOffset： 这是描述的元素的字节偏移量。在单个输入槽中，如果我们有位置和颜色，偏移量可以是0，因为它从顶点结构的开头开始，
							颜色需要是前面的顶点位置的大小，即12个字节（例如顶点位置的格式为DXGI_FORMAT_R32G32B32_FLOAT，
							位置中每个组件（RGB分量）占32位，总共占96位。一个字节有8位，所以96/8 == 12）。

		InputSlotClass： 标识单个输入槽的输入数据类型。

		InstanceDataStepRate： 仅限于InputSlotClass值为D3D11_INPUT_PER_INSTANCE_DATA时使用，InputSlotClass值为D3D11_INPUT_PER_VERTEX_DATA时将该值指定为0。
	*/
};
UINT numElements = ARRAYSIZE(layout);
///////////////**************new**************////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)//Main windows function
{

	if (!InitializeWindow(hInstance, nShowCmd, Width, Height, true))
	{
		MessageBox(0, L"Window Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	if (!InitializeDirect3d11App(hInstance))	//Initialize Direct3D
	{
		MessageBox(0, L"Direct3D Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	if (!InitScene())	//Initialize our scene
	{
		MessageBox(0, L"Scene Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	messageloop();

	CleanUp();

	return 0;
}

bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool windowed)
{
	typedef struct _WNDCLASS {
		UINT cbSize;
		UINT style;
		WNDPROC lpfnWndProc;
		int cbClsExtra;
		int cbWndExtra;
		HANDLE hInstance;
		HICON hIcon;
		HCURSOR hCursor;
		HBRUSH hbrBackground;
		LPCTSTR lpszMenuName;
		LPCTSTR lpszClassName;
	} WNDCLASS;

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WndClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Error registering class", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	hwnd = CreateWindowEx(
		NULL,
		WndClassName,
		L"Lesson 4 - Begin Drawing",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hwnd)
	{
		MessageBox(NULL, L"Error creating window", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

bool InitializeDirect3d11App(HINSTANCE hInstance)
{
	//Describe our Buffer
	DXGI_MODE_DESC bufferDesc;

	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

	bufferDesc.Width = Width;
	bufferDesc.Height = Height;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;


	//Create our SwapChain
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

	//Create our BackBuffer
	ID3D11Texture2D* BackBuffer;
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);

	//Create our Render Target
	hr = d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView);
	BackBuffer->Release();

	//Set our Render Target
	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, NULL);

	return true;
}

void CleanUp()
{
	//Release the COM Objects we created
	SwapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
	renderTargetView->Release();
	///////////////**************new**************////////////////////
	triangleVertBuffer->Release();
	VS->Release();
	PS->Release();
	VS_Buffer->Release();
	PS_Buffer->Release();
	vertLayout->Release();
	///////////////**************new**************////////////////////
}

///////////////**************new**************////////////////////
bool InitScene()
{
	//Compile Shaders from shader file
	hr = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "VS", "vs_5_0", 0, 0, 0, &VS_Buffer, 0, 0);
	hr = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "PS", "ps_5_0", 0, 0, 0, &PS_Buffer, 0, 0);
	/*
	D3DX11CompileFromFile函数说明

		#ifdef UNICODE
		#define D3DX11CompileFromFile D3DX11CompileFromFileW

		HRESULT WINAPI D3DX11CompileFromFileW(
			LPCWSTR pSrcFile,
			CONST D3D10_SHADER_MACRO* pDefines,
			LPD3D10INCLUDE pInclude,
			LPCSTR pFunctionName,
			LPCSTR pProfile,
			UINT Flags1,
			UINT Flags2,
			ID3DX11ThreadPump* pPump,
			ID3D10Blob** ppShader,
			ID3D10Blob** ppErrorMsgs,
			HRESULT* pHResult
		);

		pSrcFile [in]： 包含着色器代码的文件的名称。如果编译器设置需要Unicode，则数据类型LPCTSTR将解析为LPCWSTR。

		pDefines [in]： 指向宏定义数组的指针。

		pInclude [in]： 这是一个指向include接口的指针。如果着色器在文件中包含#include，则将此值设置为NULL将导致编译错误。

		pFunctionName [in]： 这是该文件名中着色器函数的名称。

		pProfile [in]： 要使用的着色器版本。 Direct3D 11支持着色器版本5.0。

		Flags1 [in]： 着色器编译标志，指定编译器如何编译HLSL代码。

		Flags2 [in]： 效果（Effect）编译标志，指示编译器如何编译效果文件或运行时如何处理效果文件。

		pPump [in]： 与多线程有关，使用NULL指定此函数在完成之前不应返回。

		ppShader [out]： 这是返回的着色器。它不是实际的着色器，而更像是包含着色器的缓冲区和有关着色器的信息。然后使用此缓冲区来创建实际的着色器。

		ppErrorMsgs [out]： 返回编译期间发生的错误和警告的列表。这些错误和警告与调试器的调试输出相同。

		pHResult [out]： 指向返回值的指针，如果在向pPump参数提供NULL时向pHResult参数提供非NULL值，则D3DX11CompileFromFile返回E_INVALIDARG。
						 如果pPump不为NULL，则pHResult必须是有效的内存位置（非NULL），直到异步执行完成。
						 总的来说，如果pPump参数为NULL，则pHResult参数也必须为NULL。
	*/

	//Create the Shader Objects
	hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
	hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);
	/*
	Create(Vertex/Pixel)Shader函数说明

		virtual HRESULT STDMETHODCALLTYPE Create(Vertex/Pixel)Shader(
			__in  const void *pShaderBytecode,
			__in  SIZE_T BytecodeLength,
			__in_opt  ID3D11ClassLinkage *pClassLinkage,
			__out_opt  ID3D11(Vertex/Pixel)Shader **pp(Vertex/Pixel)Shader
		) = 0;

		pShaderBytecode [in]： 这是指向着色器缓冲区开始的指针。

		BytecodeLength [in]： 这是缓冲区的大小。

		pClassLinkage [in]： 指向类链接对象的指针。类链接对象是多个着色器可以共享的变量和类型的命名空间。
							 （具体参考：https://docs.microsoft.com/zh-cn/windows/desktop/direct3dhlsl/storing-variables-and-types-for-shaders-to-share）。

		pp(Vertex/Pixel)Shader [out]： 这是返回的（顶点/像素）着色器。
	*/

	//Set Vertex and Pixel Shaders
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);

	//Create the vertex buffer
	//现在我们需要创建顶点缓冲区。
	//我们首先使用Vertex结构制作顶点数组。
	//在我们有一个顶点数组后，我们将通过填写D3D11_BUFFER_DESC结构来描述我们的顶点缓冲区，并通过调用ZeroMemory()确保它是空的。
	//有了缓冲区的描述后，接着需要在缓冲区中填入D3D11_SUBRESOURCE_DATA结构和我们想要的数据。
	//最后使用刚创建的缓冲区描述和缓冲区数据来创建缓冲区。
	Vertex v[] =
	{
		Vertex(-0.5f, 0.5f, 0.5f),
		Vertex(0.5f, 0.5f, 0.5f),
		Vertex(-0.5f, -0.5f, 0.5f),
		Vertex(0.5f, -0.5f, 0.5f),
	};

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;

	hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &triangleVertBuffer);

	//Set the vertex buffer
	//有了一个顶点缓冲区之后，接着需要将它绑定到IA。
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	d3d11DevCon->IASetVertexBuffers(0, 1, &triangleVertBuffer, &stride, &offset);

	//Create the Input Layout
	//创建一个输入布局对象来描述IA阶段的输入缓冲区数据
	d3d11Device->CreateInputLayout(layout, numElements, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);

	//Set the Input Layout
	//创建了输入布局之后，接下来要做的是将它作为活动输入布局绑定到IA。
	d3d11DevCon->IASetInputLayout(vertLayout);

	//Set Primitive Topology
	//接着告诉IA我们发送的图元（primitive）类型是什么。（D3D11_PRIMITIVE_TOPOLOGY枚举类型）
	d3d11DevCon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//Create the Viewport
	//然后创建并设置视口（Viewport）。
	//视口将告诉渲染管线的RS（Rasterizer Stage）阶段要绘制什么。我们可以使用D3D11_VIEWPORT结构创建一个视口。
	//视口是一个以像素为单位的正方形，光栅化器使用该正方形来查找在窗口的客户区域（Win32）上显示几何体的位置。
	//因此,将框的左上角设置为0,0，将框的右下角设置为Width，Height，以像素为单位。
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = Width;
	viewport.Height = Height;

	//Set the Viewport
	//在创建了视口之后，接着需要将它绑定到渲染管线的RS阶段。
	//第一个参数是要绑定的视口数，第二个参数是指向视口数组的指针。
	//这代表可以有多个“窗口”，例如玩家1一个，玩家2一个。
	d3d11DevCon->RSSetViewports(1, &viewport);

	return true;
}
///////////////**************new**************////////////////////

void UpdateScene()
{

}

///////////////**************new**************////////////////////
void DrawScene()
{
	//Clear our backbuffer
	float bgColor[4] = { (0.0f, 0.0f, 0.0f, 0.0f) };
	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);

	//Draw the triangle
	//第一个参数是要绘制的顶点数。第二个参数是要绘制的第一个顶点的索引，通常是顶点缓冲区中的偏移量。
	d3d11DevCon->Draw(4, 0);

	//Present the backbuffer to the screen
	SwapChain->Present(0, 0);
}
///////////////**************new**************////////////////////

int messageloop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (true)
	{
		BOOL PeekMessageL(
			LPMSG lpMsg,
			HWND hWnd,
			UINT wMsgFilterMin,
			UINT wMsgFilterMax,
			UINT wRemoveMsg
		);

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// run game code            
			UpdateScene();
			DrawScene();
		}
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(hwnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

