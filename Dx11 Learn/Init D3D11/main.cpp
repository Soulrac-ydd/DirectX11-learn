
///////////////**************new**************////////////////////

//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>

//Global Declarations//
IDXGISwapChain* SwapChain;
ID3D11Device* d3d11Device;
ID3D11DeviceContext* d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;

float red = 0.0f;
float green = 0.0f;
float blue = 0.0f;
int colormodr = 1;
int colormodg = 1;
int colormodb = 1;

///////////////**************new**************////////////////////

LPCTSTR WndClassName = L"firstwindow";
HWND hwnd = NULL;

const int Width = 300;
const int Height = 300;

///////////////**************new**************////////////////////

//Function Prototypes//
bool InitializeDirect3d11App(HINSTANCE hInstance); //初始化direct3d
void ReleaseObjects(); //释放不需要防止内存泄漏的对象
bool InitScene(); //初始化  设置场景
void UpdateScene(); //更新场景  用于在每帧的基础上更改场景
void DrawScene(); //绘制场景  用于将场景绘制到屏幕，并且每帧也更新

///////////////**************new**************////////////////////

bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool windowed);

int messageloop();

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)	//Main windows function
{

	if (!InitializeWindow(hInstance, nShowCmd, Width, Height, true))
	{
		MessageBox(0, L"Window Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	///////////////**************new**************////////////////////

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

	ReleaseObjects();

	///////////////**************new**************////////////////////

	return 0;
}

bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool windowed)
{
	typedef struct _WNDCLASS
	{
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
		L"Window Title",
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

///////////////**************new**************////////////////////

bool InitializeDirect3d11App(HINSTANCE hInstance)
{
	HRESULT hr; //创建名为hr的HRESULT对象，用于错误检查。此处没有包含错误检查，以保持代码更清晰和浓缩。

	//Describe our Buffer
	DXGI_MODE_DESC bufferDesc;

	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

	bufferDesc.Width = Width; //分辨率的宽度
	bufferDesc.Height = Height; //分辨率的高度
	//RefreshRate是DXGI_RATIONAL类型，用赫兹描述刷新率，如下为60/1，即60Hz。
	bufferDesc.RefreshRate.Numerator = 60; //分子
	bufferDesc.RefreshRate.Denominator = 1; // 分母
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //显示的格式为一个32位无符号整数，每个都取8位，红、绿、蓝和Alpha。
	//ScanlineOrdering是DXGI_MODE_SCANLINE_ORDER枚举类型，描述光栅化渲染器渲染到表面上的方式。
	//由于我们使用双缓冲，通常看不到，因此我们可以将其设置为DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED，这意味着渲染在曲面上的顺序无关紧要。
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; //Scaling是DXGI_MODE_SCALING类型，表示图像如何拉伸以适应显示器的分辨率。

	//Describe our SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc = bufferDesc; //这是一个DXGI_MODE_DESC结构，它描述了后台缓冲区。我们将把刚刚填写的bufferDesc对象放在这里。
	//多重采样 用于“平滑”线条和边缘的紊乱（抗锯齿）
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	//BufferUsage是DXGI_USAGE枚举类型，描述cpu对后台缓冲区表面的访问。我们指定DXGI_USAGE_RENDER_TARGET_OUTPUT，因为我们将渲染它。
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//使用的后台缓冲区的数量，设置为1表示双缓冲，2表示三缓冲，以此类推。
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hwnd; //输出到窗口的句柄
	swapChainDesc.Windowed = TRUE; //TRUE or FALSE，取决于我们是否需要窗口或全屏。对于窗口设置为TRUE，对于全屏幕设置为FALSE。
	//SwapEffect是DXGI_SWAP_EFFECT枚举类型，描述了显示驱动程序在将前缓冲区交换到后缓冲区后应该对前缓冲区执行的操作。我们设置DXGI_SWAP_EFFECT_DISCARD让显示驱动程序决定最有效的方法是什么
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

void ReleaseObjects()
{
	//Release the COM Objects we created
	SwapChain->Release();
	d3d11Device->Release();
	d3d11DevCon->Release();
}
bool InitScene()
{

	return true;
}

void UpdateScene()
{
	//Update the colors of our scene
	red += colormodr * 0.00005f;
	green += colormodg * 0.00002f;
	blue += colormodb * 0.00001f;

	if (red >= 1.0f || red <= 0.0f)
		colormodr *= -1;
	if (green >= 1.0f || green <= 0.0f)
		colormodg *= -1;
	if (blue >= 1.0f || blue <= 0.0f)
		colormodb *= -1;
}

void DrawScene()
{
	//Clear our backbuffer to the updated color
	D3DXCOLOR bgColor(red, green, blue, 1.0f);

	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);

	//Present the backbuffer to the screen
	SwapChain->Present(0, 0);
}

///////////////**************new**************////////////////////

int messageloop() {
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
		else {
			///////////////**************new**************////////////////////
						// run game code

			UpdateScene();
			DrawScene();

			///////////////**************new**************////////////////////
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
