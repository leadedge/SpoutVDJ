#ifndef __spoutSenderPlugin__
#define __spoutSenderPlugin__

#include "stdafx.h"
#include "vdjVideo8.h"

#include "d3d11.h"
#pragma comment (lib, "d3d11.lib")

#include "..\..\SpoutSDK\spoutSenderNames.h"
#include "..\..\SpoutSDK\spoutDirectX.h"
#include "..\..\SpoutSDK\spoutFrameCount.h"
#include "..\..\SpoutSDK\spoutUtils.h"

class SpoutSenderPlugin : public IVdjPluginVideoFx8
{

public:

    SpoutSenderPlugin();
    ~SpoutSenderPlugin();
    
    HRESULT __stdcall OnLoad();
    HRESULT __stdcall OnGetPluginInfo(TVdjPluginInfo8 *infos);
	HRESULT __stdcall OnStart();
	HRESULT __stdcall OnStop();
	HRESULT __stdcall OnDraw();
	HRESULT __stdcall OnDeviceInit();
	HRESULT __stdcall OnDeviceClose();
	ULONG   __stdcall Release();

private:

	bool bInitialized;
	bool bSpoutOut;
	ID3D11Texture2D* g_pSharedTexture;
	HANDLE g_dxShareHandle;
	unsigned int g_Width;
	unsigned int g_Height;
	char g_SenderName[256];

	spoutSenderNames spoutsender;
	spoutDirectX spoutdx;
	spoutFrameCount frame;

};

#endif
