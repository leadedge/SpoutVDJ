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
//      ===================== end of 32 bit DX9 version ====================
//
//		Create separate VDJSpoutSender64 project for 64bit version
//
//		29.12.18 - Revise for DX11 for VirtualDJ 64 bit
//		11.05.19 - Rebuild 64bit for 2.007 VS2017 /MT - Version 2.00
//		02.12.19 - NULL variables on device close
//		11.05.19 - Rebuild 64bit for 2.007 VS2017 /MT - Version 2.01
//		10.01.19 - Modifications for updated VDJ SDK by by Nicotux 
//				   See vdjVideo8.h - #define VDJFLAG_VIDEO_FORRECORDING 0x1000000
//				   Rebuild 64bit for 2.007 VS2017 /MT - Version 2.02
//		22.04.20 - Use shader resource view as source
//				   Add SpoutCleanup function
//				   Rebuild 64bit for 2.007 VS2017 /MT - Version 2.03
//
//		------------------------------------------------------------
//
//		Copyright (C) 2015-2020. Lynn Jarvis, Leading Edge. Pty. Ltd.
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

//#include "stdafx.h"
#include "VDJSpoutSender64.h"
#include <array>


VDJ_EXPORT HRESULT VDJ_API DllGetClassObject(const GUID &rclsid, const GUID &riid, void** ppObject)
{ 
	// VDJ 8
	if(memcmp(&rclsid, &CLSID_VdjPlugin8, sizeof(GUID)) != 0) return CLASS_E_CLASSNOTAVAILABLE; 
    if(memcmp(&riid, &IID_IVdjPluginVideoFx8, sizeof(GUID)) != 0) return CLASS_E_CLASSNOTAVAILABLE; 

	*ppObject = new SpoutSenderPlugin(); 

    return NO_ERROR; 
}


SpoutSenderPlugin::SpoutSenderPlugin()
{
	m_Width = 0;
	m_Height = 0;
	m_SenderName[0] = 0;
	m_pSharedTexture = nullptr;
	bInitialized = false;
	bSpoutOut = false; // toggle for plugin start and stop
	
	// Enable logging to show Spout warnings and errors
	// Log file saved in AppData>Roaming>Spout
	EnableSpoutLogFile("VDJSpoutSender64.log");
	// SetSpoutLogLevel(SPOUT_LOG_WARNING); // to show only warnings and errors
	// OpenSpoutConsole(); // For debugging

}

SpoutSenderPlugin::~SpoutSenderPlugin()
{

}

HRESULT VDJ_API SpoutSenderPlugin::OnLoad()
{
	double query = NAN;
	// The deck the plugin was loaded
	// is it a standard one ? the master one ? yes master, no, standard, else it is special one or fail
	deck = (!GetInfo("get_plugindeck", &query)) ? (int)query : MININT;
	return NO_ERROR;
}

HRESULT VDJ_API SpoutSenderPlugin::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
	infos->Author = "Lynn Jarvis";
    infos->PluginName = (char *)"VDJSpoutSender64";
    infos->Description = (char *)"Sends frames to a Spout Receiver\nSpout : http://Spout.zeal.co/";
	infos->Version = (char *)"v2.03";
    infos->Bitmap = NULL;

	// A sender is an effect - process last so all other effects are shown
	// As it"s a global sender, process for recording and output resolution
	infos->Flags = VDJFLAG_PROCESSLAST | VDJFLAG_VIDEO_FORRECORDING | VDJFLAG_VIDEO_OUTPUTRESOLUTION;

    return NO_ERROR;
}


HRESULT VDJ_API SpoutSenderPlugin::OnStart()
{
	bSpoutOut = true;
	return NO_ERROR;
}

HRESULT VDJ_API SpoutSenderPlugin::OnStop()
{
	bSpoutOut = false;
	SpoutCleanup();
	return NO_ERROR;
}

HRESULT VDJ_API  SpoutSenderPlugin::OnDeviceInit()
{
	return S_OK;
}

HRESULT VDJ_API SpoutSenderPlugin::OnDeviceClose()
{
	SpoutCleanup();
	return S_OK;
}

ULONG VDJ_API SpoutSenderPlugin::Release()
{
	delete this; 
	return S_OK;
}


HRESULT VDJ_API SpoutSenderPlugin::OnDraw()
{
	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pImmediateContext = nullptr;
	DXGI_FORMAT dxformat = DXGI_FORMAT_B8G8R8A8_UNORM;
	HANDLE dxShareHandle = NULL;

	// Draw now so we have a backbuffer
	DrawDeck();

	if (bSpoutOut) {
		// Get the VirtualDJ DX11 device
		HRESULT hr = GetDevice(VdjVideoEngineDirectX11, (void **)&pDevice);
		if (hr == S_OK && pDevice) {
			// Get immediate context
			pDevice->GetImmediateContext(&pImmediateContext);
			if (pImmediateContext) {
				// Get the VDJ shader resource view
				ID3D11ShaderResourceView* pShaderRV = nullptr;
				hr = GetTexture(VdjVideoEngineDirectX11, (void **)&pShaderRV, nullptr);
				if (hr == S_OK && pShaderRV) {
					// Get the texture resource of the shader resource view
					ID3D11Resource* texResource = nullptr;
					pShaderRV->GetResource(&texResource);
					// Get the texture for size and format
					ID3D11Texture2D* pTexture = nullptr;
					hr = texResource->QueryInterface(&pTexture);
					if (hr == S_OK && pTexture) {
						D3D11_TEXTURE2D_DESC desc;
						ZeroMemory(&desc, sizeof(desc));
						pTexture->GetDesc(&desc);
						// Get it's format for texture creation and copy
						DXGI_FORMAT dxformat = desc.Format;
						if (dxformat > 0) {
							// If a sender has not been initialized yet, create one
							if (!bInitialized) {
								// Save width and height to test for sender size changes
								m_Width = desc.Width;
								m_Height = desc.Width;
								// Create a local shared texture the same size and format as the texture
								spoutdx.CreateSharedDX11Texture(pDevice, m_Width, m_Height, dxformat, &m_pSharedTexture, dxShareHandle);
								// printf("Created shared texture %dx%d, sharehandle = 0x%x\n", m_Width, m_Height, dxShareHandle);
								// Create a sender, specifying the texture format and share handle
								sprintf_s(m_SenderName, 256, "VDJSpoutSender64 Deck %s", (deck <= 0 ? (deck >= -3 ? std::array <std::string, 4> { "master", "sampler", "mic", "aux" }.at(-(int)deck) : std::to_string((int)deck)) : std::to_string((int)deck)).c_str()); // add deck n
								bInitialized = spoutsender.CreateSender(m_SenderName, m_Width, m_Height, dxShareHandle, (DWORD)dxformat);
								// Create a sender mutex for access to the shared texture
								frame.CreateAccessMutex(m_SenderName);
								// Enable frame counting so the receiver gets frame number and fps
								frame.EnableFrameCount(m_SenderName);
							}
							else if (m_Width != desc.Width || m_Height != desc.Height) {
								// Source texture changed size
								m_Width = desc.Width;
								m_Height = desc.Height;
								// Release and re-create the local shared texture to match
								if (m_pSharedTexture) m_pSharedTexture->Release();
								spoutdx.CreateSharedDX11Texture(pDevice, m_Width, m_Height, dxformat, &m_pSharedTexture, dxShareHandle);
								// Update the sender with the new share handle
								spoutsender.UpdateSender(m_SenderName, m_Width, m_Height, dxShareHandle, dxformat);
							}
							else {
								// Access and lock the sender shared texture
								if (frame.CheckAccess()) {
									// Copy the Virtual DJ texture to the sender's shared texture
									pImmediateContext->CopyResource(m_pSharedTexture, pTexture);
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
	}
	return S_OK;
}

void SpoutSenderPlugin::SpoutCleanup()
{
	if (m_pSharedTexture)
		m_pSharedTexture->Release();

	if (bInitialized)
		spoutsender.ReleaseSenderName(m_SenderName);

	m_pSharedTexture = nullptr;
	bInitialized = false;
	m_Width = 0;
	m_Height = 0;
	m_SenderName[0] = 0;
}
