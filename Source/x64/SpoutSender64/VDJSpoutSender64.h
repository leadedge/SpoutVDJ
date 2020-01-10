#ifndef __spoutSenderPlugin__
#define __spoutSenderPlugin__

//#include "stdafx.h"
#include "vdjVideo8.h"

#include "d3d11.h"
#pragma comment (lib, "d3d11.lib")

#include "../../SpoutSDK/SpoutSenderNames.h"
#include "../../SpoutSDK/SpoutDirectX.h"
#include "../../SpoutSDK/SpoutFrameCount.h"
#include "../../SpoutSDK/SpoutFrameCount.h"
#include "../../SpoutSDK/Spout.h"

class SpoutSenderPlugin : public IVdjPluginVideoFx8
{

public:

    SpoutSenderPlugin();
    ~SpoutSenderPlugin();
    
    HRESULT VDJ_API OnLoad();
    HRESULT VDJ_API OnGetPluginInfo(TVdjPluginInfo8 *infos);
	HRESULT VDJ_API OnStart();
	HRESULT VDJ_API OnStop();
	HRESULT VDJ_API OnDraw();
	HRESULT VDJ_API OnDeviceInit();
	HRESULT VDJ_API OnDeviceClose();
	ULONG   VDJ_API Release();

private:
	int deck; // the deck the plugin is working on

	bool bInitialized;
	bool bSpoutOut;
	ID3D11Texture2D* m_pSharedTexture;
	unsigned int m_Width;
	unsigned int m_Height;
	char m_SenderName[256];

	spoutSenderNames spoutsender;
	spoutDirectX spoutdx;
	spoutFrameCount frame;

};

#endif
