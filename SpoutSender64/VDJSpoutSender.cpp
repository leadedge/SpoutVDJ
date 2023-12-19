//
// VDJSpoutSender.cpp : Defines the exported functions for the DLL application.
//
//		10.02.15	Inital testing
//		14.02.15	added Optimus enablement export
//					Changed to /MT compile
//					Version 1.0
//		21.02.15	Removed OptimusEnablement export - does not work for a dll
//					Version 1.01
//		26.05.15	Recompile for revised SpoutPanel registry write of sender name
//					Version 1.02
//		08.07.15	Create an invisible dummy button window for OpenGL due to SetPixelFormat problems noted with Mapio
//		01.08.15	Recompile for 2.004
//					Version 1.03
//		17.12.15	Clean up and rebuild for 2.005 release VS2012
//					Version 1.04
//		23.06.16 - rebuild for Spout 2.005 release Version 1.05
//				   VS2012 /MT
//		12.01.17 - Add CS_OWNDC to OpenGL window creation
//		16.01.17 - Remove destroy OpenGL on Stop
//		23.01.17 - Rebuild for 2.006 VS2012 /MD - Version 1.06
//		01.10.18 - Convert to VS2017 - set up solution to build for 32bit and 64bit
//		23.12.18 - Rebuild Win32 for 2.007 VS2017 /MT - Version 1.07
//      ===================== end of 32 bit DX9 version ====================
//
//		29.12.18 - Revise for DX11 for VirtualDJ 64 bit
//		23.12.18 - Rebuild 64bit for 2.007 VS2017 /MT - Version 2.00
//
//		------------------------------------------------------------
//
//		Copyright (C) 2015-2018. Lynn Jarvis, Leading Edge. Pty. Ltd.
//
//		This program is free software: you can redistribute it and/or modify
//		it under the terms of the GNU Lesser General Public License as published by
//		the Free Software Foundation, either version 3 of the License, or
//		(at your option) any later version.
//
//		This program is distributed in the hope that it will be useful,
//		but WITHOUT ANY WARRANTY; without even the implied warranty of
//		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//		GNU Lesser General Public License for more details.
//
//		You will receive a copy of the GNU Lesser General Public License along 
//		with this program.  If not, see http://www.gnu.org/licenses/.
//		--------------------------------------------------------------
//
//

#include "stdafx.h"
#include "VDJSpoutSender.h"


VDJ_EXPORT HRESULT __stdcall DllGetClassObject(const GUID &rclsid, const GUID &riid, void** ppObject)
{ 
	// VDJ 8
	if(memcmp(&rclsid, &CLSID_VdjPlugin8, sizeof(GUID)) != 0) return CLASS_E_CLASSNOTAVAILABLE; 
    if(memcmp(&riid, &IID_IVdjPluginVideoFx8, sizeof(GUID)) != 0) return CLASS_E_CLASSNOTAVAILABLE; 

	*ppObject = new SpoutSenderPlugin(); 

    return NO_ERROR; 
}


SpoutSenderPlugin::SpoutSenderPlugin()
{
	g_Width = 0;
	g_Height = 0;
	g_SenderName[0] = 0;
	g_pSharedTexture = nullptr;
	g_dxShareHandle = NULL;
	bInitialized = false;
	bSpoutOut = false; // toggle for plugin start and stop
	
	// Enable logging to show Spout warnings and errors
	// Log file saved in AppData>Roaming>Spout
	EnableSpoutLog("VDJSpoutSender64.log");
	SetSpoutLogLevel(SPOUT_LOG_WARNING); // show only warnings and errors
	// OpenSpoutConsole(); // For debugging


}

SpoutSenderPlugin::~SpoutSenderPlugin()
{

}

HRESULT __stdcall SpoutSenderPlugin::OnLoad()
{
    return NO_ERROR;
}

HRESULT __stdcall SpoutSenderPlugin::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
	infos->Author = "Lynn Jarvis";
    infos->PluginName = (char *)"VDJSpoutSender64";
    infos->Description = (char *)"Sends frames to a Spout Receiver\nSpout : http://Spout.zeal.co/";
	infos->Version = (char *)"v2.00";
    infos->Bitmap = NULL;

	// A sender is an effect - process last so all other effects are shown
	infos->Flags = VDJFLAG_PROCESSLAST;

    return NO_ERROR;
}


HRESULT __stdcall SpoutSenderPlugin::OnStart()
{
	bSpoutOut = true;
	return NO_ERROR;
}

HRESULT __stdcall SpoutSenderPlugin::OnStop()
{
	bSpoutOut = false;
	return NO_ERROR;
}

HRESULT __stdcall  SpoutSenderPlugin::OnDeviceInit() 
{
	return S_OK;
}

HRESULT __stdcall SpoutSenderPlugin::OnDeviceClose() 
{

	if(g_pSharedTexture) 
		g_pSharedTexture->Release();

	if (bInitialized)
		spoutsender.ReleaseSenderName(g_SenderName);

	return S_OK;
}

ULONG __stdcall SpoutSenderPlugin::Release()
{
	delete this; 
	return S_OK;
}


HRESULT __stdcall SpoutSenderPlugin::OnDraw()
{
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pImmediateContext = nullptr;
	ID3D11RenderTargetView *pRenderTargetView = nullptr;
	ID3D11Resource *backbufferRes = nullptr;
	D3D11_RENDER_TARGET_VIEW_DESC vDesc;
	DXGI_FORMAT dxformat = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Draw now so we have a backbuffer
	DrawDeck();

	if (bSpoutOut) {
		// Get the VirtualDJ DX11 device
		HRESULT hr = GetDevice(VdjVideoEngineDirectX11, (void **)&pDevice);
		if (hr == S_OK && pDevice) {
			// Get immediate context
			pDevice->GetImmediateContext(&pImmediateContext);
			if (pImmediateContext) {
				// Get render target view
				pImmediateContext->OMGetRenderTargets(1, &pRenderTargetView, 0);
				if (pRenderTargetView) {
					// Get it's format for sender texture and copy
					ZeroMemory(&vDesc, sizeof(vDesc));
					pRenderTargetView->GetDesc(&vDesc);
					dxformat = vDesc.Format;
					if (dxformat > 0) { // possible problem here
						// Get the backbuffer resource
						pRenderTargetView->GetResource(&backbufferRes);
						if (backbufferRes) {
							
							// If a sender has not been initialized yet, create one
							if (!bInitialized) {
								// Save width and height to test for sender size changes
								g_Width = width;
								g_Height = height;
								// Create a local shared texture the same size and format as the backbuffer
								spoutdx.CreateSharedDX11Texture(pDevice, g_Width, g_Height, dxformat, &g_pSharedTexture, g_dxShareHandle);
								// Create a sender, specifying the backbuffer format and returned share handle
								strcpy_s(g_SenderName, 256, "VDJSpoutSender64");
								bInitialized = spoutsender.CreateSender(g_SenderName, g_Width, g_Height, g_dxShareHandle, (DWORD)dxformat);
								// Create a sender mutex for access to the shared texture
								frame.CreateAccessMutex(g_SenderName);
								// Enable frame counting so the receiver gets frame number and fps
								frame.EnableFrameCount(g_SenderName);
							}
							else if (g_Width != width || g_Height != height) {
								// Source texture changed size
								g_Width = width;
								g_Height = height;
								// Release and re-create the local shared texture to match
								g_pSharedTexture->Release();
								spoutdx.CreateSharedDX11Texture(pDevice, g_Width, g_Height, dxformat, &g_pSharedTexture, g_dxShareHandle);
								// Update the sender
								spoutsender.UpdateSender(g_SenderName, g_Width, g_Height, g_dxShareHandle, dxformat);
							}

							// Access and lock the sender shared texture
							if (frame.CheckAccess()) {
								// Copy the backbuffer to the sender's shared texture
								pImmediateContext->CopyResource(g_pSharedTexture, backbufferRes);
								// Flush and wait until CopyResource is done
								spoutdx.FlushWait(pDevice, pImmediateContext);
								// While the mutex is still locked, signal a new frame
								frame.SetNewFrame();
								// Allow access to the shared texture
								frame.AllowAccess();
							}

						}
					}
				}
			}
		}
	}

	return S_OK;

}
