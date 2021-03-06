////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 RWS Inc, All Rights Reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as published by
// the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
// weapon.cpp
// Project: Nostril (aka Postal)
//
// This module implements the CWeapon class which is base class for the weapons
//
// History:
//		02/27/97 BRH	Started this file from CDoofus and modified it
//							to be a base class for the weapons.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/19/97 BRH	Added virtual functions for processing messages and
//							virtual OnMessage handler functions so that it follows
//							the model of the CThing3d base class object.  
//
//		06/25/97 BRH	Added rendering of 2D shadow sprite to the Render
//							function.  Also added PrepareShadow function to
//							load the default shadow resource if no resource is 
//							loaded for the shadow and then make the shadow visible.
//
//		06/30/97	JMI	Now maps the Z to 3D when loading fileversions previous to
//							24.
//
//		07/09/97	JMI	Now uses realm()->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/21/97	JMI	Now checks upper bound on m_sAlphaLevel of shadow sprite.
//	
//		07/30/97	JMI	Now hides shadow if mainsprite is hidden.
//
////////////////////////////////////////////////////////////////////////////////

#include "weapon.h"

#include "realm.h"
#include "reality.h"
#include "Thing3d.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define SHADOW_FILE	"shadow.img"

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Let this auto-init to 0
int16_t CWeapon::ms_sFileCount;



CWeapon::CWeapon(void)
{
  m_sSuspend = 0;
  position.x = position.y = position.z = rotation.y = m_dVertVel = m_dHorizVel = 0.0;
  m_eState = State_Idle;
  m_spriteShadow.flags.Hidden = true;
  m_spriteShadow.m_pImage = nullptr;
  //			m_spriteShadow.m_pthing = this;
  m_lPrevTime = 0;  // valgrind fix.  --ryan.
}

CWeapon::~CWeapon(void)
{
  // Remove sprite from scene (this is safe even if it was already removed!)
  realm()->Scene()->RemoveSprite(&m_spriteShadow);
  // Release the shadow resource
  if (m_spriteShadow.m_pImage)
    rspReleaseResource(&g_resmgrGame, &(m_spriteShadow.m_pImage));
}

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CWeapon::Load(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	int16_t sFileCount,										// In:  File count (unique per file, never 0)
	uint32_t	ulFileVersion)									// In:  Version of file format to load.
	{
	int16_t sResult = SUCCESS;

	// Call the CThing base class load to get the instance ID
	sResult	= CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == SUCCESS)
		{
		// Load common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
			{
			ms_sFileCount = sFileCount;

			// Load static data.
			switch (ulFileVersion)
				{
				default:
				case 1:
					break;
				}
			}

		switch (ulFileVersion)
			{
			default:
			case 1:
				// Load object data
            pFile->Read(&position.x);
            pFile->Read(&position.y);
            pFile->Read(&position.z);
            pFile->Read(&rotation.y);
				pFile->Read(&m_dVertVel);
				pFile->Read(&m_dHorizVel);
            pFile->Read(reinterpret_cast<uint8_t*>(&m_eState));
				break;
			}

		
		// If the file version is earlier than the change to real 3D coords . . .
      if (ulFileVersion < 24)
        realm()->MapY2DtoZ3D(position.z, position.z); // Convert to 3D.

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == SUCCESS)
			{
			// Get resources
	//		sResult = GetResources();
			}
		else
			{
			sResult = FAILURE;
			TRACE("CWeapon::Load(): Error reading from file!\n");
			}
		}
	else
		{
		TRACE("CWeapon::Load():  CThing::Load() failed.\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CWeapon::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	int16_t sFileCount)										// In:  File count (unique per file, never 0)
	{
	// Call the base class save to save the u16InstanceID
	CThing::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
		{
		ms_sFileCount = sFileCount;

		// Save static data
		}

	// Save object data
   pFile->Write(&position.x);
   pFile->Write(&position.y);
   pFile->Write(&position.z);
   pFile->Write(&rotation.y);
	pFile->Write(&m_dVertVel);
	pFile->Write(&m_dHorizVel);
   pFile->Write(uint8_t(m_eState));

   return SUCCESS;
	}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
void CWeapon::Startup(void)								// Returns 0 if successfull, non-zero otherwise
	{

	// Init other stuff
	m_lPrevTime = realm()->m_time.GetGameTime();
	}


////////////////////////////////////////////////////////////////////////////////
// Setup object
////////////////////////////////////////////////////////////////////////////////

int16_t CWeapon::Setup(int16_t sX, int16_t sY, int16_t sZ)
	{
   position.x = sX;
   position.y = sY;
   position.z = sZ;
   return SUCCESS;
	}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CWeapon::Suspend(void)
	{
	m_sSuspend++;
	}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CWeapon::Resume(void)
	{
	m_sSuspend--;

	// If we're actually going to start updating again, we need to reset
	// the time so as to ignore any time that passed while we were suspended.
	// This method is far from precise, but I'm hoping it's good enough.
	if (m_sSuspend == 0)
		m_lPrevTime = realm()->m_time.GetGameTime();
	}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CWeapon::Render(void)
{
	// If the shadow is enabled and the main sprite is visible . . .
   if (m_spriteShadow.m_pImage && !flags.Hidden)
	{
		// Get the height of the terrain from the attribute map
      int16_t sY = realm()->GetHeight((int16_t) position.x, (int16_t) position.z);
		// Map from 3d to 2d coords
      realm()->Map3Dto2D(position.x, double(sY), position.z,
                         m_spriteShadow.m_sX2, m_spriteShadow.m_sY2);
		// Offset hotspot to center of image.
		m_spriteShadow.m_sX2 -= m_spriteShadow.m_pImage->m_sWidth / 2;
		m_spriteShadow.m_sY2 -= m_spriteShadow.m_pImage->m_sHeight / 2;

		// Priority is based on bottom edge of sprite on X/Z plane!
      m_spriteShadow.m_sPriority = MAX(m_sPriority - 1, 0);//position.z;

		// Layer should be based on info we get from attribute map.
      m_spriteShadow.m_sLayer = m_sLayer;

		// Set the alpha level based on the height difference
      m_spriteShadow.m_sAlphaLevel = 200 - ((int16_t) position.y - sY);
		// Check bounds . . .
		if (m_spriteShadow.m_sAlphaLevel < 0)
			{
			m_spriteShadow.m_sAlphaLevel	= 0;
			}
		else if (m_spriteShadow.m_sAlphaLevel > 255)
			{
			m_spriteShadow.m_sAlphaLevel	= 255;
			}

		// If the main sprite is on the ground, then hide the shadow.
      m_spriteShadow.flags.Hidden = int16_t(position.y) == sY;

		// Update sprite in scene
		realm()->Scene()->UpdateSprite(&m_spriteShadow);
	}
	else
	{
     m_spriteShadow.flags.Hidden = true;
	}
}

#if !defined(EDITOR_REMOVED)
////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
int16_t CWeapon::EditNew(									// Returns 0 if successfull, non-zero otherwise
	int16_t sX,												// In:  New x coord
	int16_t sY,												// In:  New y coord
	int16_t sZ)												// In:  New z coord
	{
	int16_t sResult = SUCCESS;
	
	// Use specified position
   position.x = (double)sX;
   position.y = (double)sY;
   position.z = (double)sZ;

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
int16_t CWeapon::EditModify(void)
	{
   return SUCCESS;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
int16_t CWeapon::EditMove(									// Returns 0 if successfull, non-zero otherwise
	int16_t sX,												// In:  New x coord
	int16_t sY,												// In:  New y coord
	int16_t sZ)												// In:  New z coord
	{
   position.x = (double)sX;
   position.y = (double)sY;
   position.z = (double)sZ;

   return SUCCESS;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CWeapon::EditUpdate(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CWeapon::EditRender(void)
	{
	// In some cases, object's might need to do a special-case render in edit
	// mode because Startup() isn't called.  In this case it doesn't matter, so
	// we can call the normal Render().
	Render();
	}
#endif // !defined(EDITOR_REMOVED)

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
int16_t CWeapon::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
   return SUCCESS;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
int16_t CWeapon::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
   return SUCCESS;
	}

////////////////////////////////////////////////////////////////////////////////
// BounceAngle
////////////////////////////////////////////////////////////////////////////////

double CWeapon::BounceAngle(double dRot)
{
	int16_t sRot = (int16_t) dRot;
	int16_t sBounceAngle = (((((sRot / 90) + 1) * 180) - sRot) % 360);
	return (double) sBounceAngle;
}

////////////////////////////////////////////////////////////////////////////////
// Process all messages currently in the message queue through 
// ProcessMessage().
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CWeapon::ProcessMessages(void)
{
  // Check queue of messages.
  while(!m_MessageQueue.empty())
  {
    GameMessage& msg = m_MessageQueue.front();
    ProcessMessage(&msg);
    m_MessageQueue.pop_front();
  }
}

////////////////////////////////////////////////////////////////////////////////
// Process the specified message.  For most messages, this function
// will call the equivalent On* function.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CWeapon::ProcessMessage(		// Returns nothing.
	GameMessage* pmsg)					// Message to process.
	{
	switch (pmsg->msg_Generic.eType)
		{
		case typeShot:
			OnShotMsg(&(pmsg->msg_Shot) );
			break;
		
		case typeExplosion:
			OnExplosionMsg(&(pmsg->msg_Explosion) );
			break;
		
		case typeBurn:
			OnBurnMsg(&(pmsg->msg_Burn) );
         break;
		case typeTrigger:
			OnTriggerMsg(&(pmsg->msg_Trigger) );
			break;
		
		default:
			// Should this complain when it doesn't know a message type?
			break;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a msg_Shot.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CWeapon::OnShotMsg(	// Returns nothing.
	Shot_Message* pshotmsg)		// In:  Message to handle.
{
  UNUSED(pshotmsg);
}

////////////////////////////////////////////////////////////////////////////////
// Handles an Explosion_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CWeapon::OnExplosionMsg(			// Returns nothing.
	Explosion_Message* pexplosionmsg)	// In:  Message to handle.
{
  UNUSED(pexplosionmsg);
}

////////////////////////////////////////////////////////////////////////////////
// Handles a Burn_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CWeapon::OnBurnMsg(	// Returns nothing.
	Burn_Message* pburnmsg)		// In:  Message to handle.
{
  UNUSED(pburnmsg);
}

////////////////////////////////////////////////////////////////////////////////
// Handles a Trigger_Message
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CWeapon::OnTriggerMsg(			// Returns nothing
	Trigger_Message* ptriggermsg)		// In: Message to handle
{
  UNUSED(ptriggermsg);
}


////////////////////////////////////////////////////////////////////////////////
// PrepareShadow
////////////////////////////////////////////////////////////////////////////////

int16_t CWeapon::PrepareShadow(void)
{
	int16_t sResult = SUCCESS;

	// If the shadow doesn't have resource loaded yet, load the default
	if (m_spriteShadow.m_pImage == nullptr)
	{
		sResult = rspGetResource(&g_resmgrGame, realm()->Make2dResPath(SHADOW_FILE), &(m_spriteShadow.m_pImage), RFile::LittleEndian);
	}

	// If a resource is available, set the shadow to visible.
	if (sResult == SUCCESS)
      m_spriteShadow.flags.Hidden = false;

	return sResult;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
