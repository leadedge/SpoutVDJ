#ifndef __spoutReceiverPlugin__
#define __spoutReceiverPlugin__

#include "stdafx.h"
#include "vdjVideo8.h"
#include "vdjPlugin8.h"

//
// Libraries needed :
//	Winmm.lib
//	d3d11.lib
//  d3d9.lib

#include "SpoutSDK\SpoutSenderNames.h" // for sender creation and update
#include "SpoutSDK\SpoutDirectX.h" // for creating a shared texture
#include "SpoutSDK\SpoutFrameCount.h" // for mutex lock and new frame signal
#include "SpoutSDK\SpoutUtils.h" // for logging utilites

#include <direct.h> // for _getcwd
#include <TlHelp32.h> // for PROCESSENTRY32
#include <tchar.h> // for _tcsicmp

class COverlayD3D11 {
public:
	char _0x0000[76];
	ID3D11Device*			m_pDevice; //0x004C 
	char _0x0050[28];
	IDXGISwapChain*			m_pSwapChain; //0x006C
};

// VDJ VideoFx plugin class
class SpoutReceiverPlugin : public IVdjPluginVideoFx8
{

public:

    SpoutReceiverPlugin();
    ~SpoutReceiverPlugin();
    HRESULT __stdcall OnLoad();
    HRESULT __stdcall OnGetPluginInfo(TVdjPluginInfo8 *infos);
	HRESULT __stdcall OnStart();
	HRESULT __stdcall OnStop();
	HRESULT __stdcall OnDraw();

	// When DirectX is initialized or closed, these functions will be called
	HRESULT __stdcall OnDeviceInit();
	HRESULT __stdcall OnDeviceClose();
	ULONG   __stdcall Release(); // added 11.12.15 - not really necessary
	HRESULT __stdcall OnParameter(int id);

private:

	// Select sender parameter
	int SelectButton;

	spoutSenderNames spoutsender;
	spoutDirectX spoutdx;
	spoutFrameCount frame;
	
	bool bSpoutInitialized; // did Spout initialization work ?
	bool bSpoutOut; // Spout output on or off when plugin is started and stopped
	bool bIsClosing; // Plugin is closing

	char g_SenderName[256]; // The sender name
	unsigned int g_SenderWidth; // Width and height of the sender detected
	unsigned int g_SenderHeight;

	int oldWidth;
	int oldHeight;
	UINT stride;
	UINT offset;

	// DirectX
	ID3D11Device* pDevice; // VirtualDJ D3D Device
	ID3D11DeviceContext* pImmediateContext; // VirtualDJ D3D Device Context
	ID3D11PixelShader* pPixelShader; // Local pixel shader
	ID3D11Buffer* pVertexBuffer; // Texture vertex buffer
	HANDLE g_dxShareHandle;	// Shared texture handle
	ID3D11Texture2D* g_pSharedTexture; // Shared texture pointer
	ID3D11Texture2D* g_pTexture; // Local texture pointer
	ID3D11ShaderResourceView* pSRView; // Shared texture shader resource view
	DWORD g_dwFormat; // Shared texture format
	bool CreateDX11Texture(ID3D11Device* pd3dDevice, unsigned int width, unsigned int height,
							DXGI_FORMAT format, ID3D11Texture2D** ppTexture);
	bool UpdateVertices();

	// Utility
	SHELLEXECUTEINFOA g_ShExecInfo;
	bool ReceiveSpoutTexture();
	bool CheckSpoutPanel(char *sendername, int maxchars = 256);
	bool OpenSpoutPanel();
	bool bSpoutPanelOpened;
	bool bSpoutPanelActive;

};

#endif
