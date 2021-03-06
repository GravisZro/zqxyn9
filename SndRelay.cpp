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
// SndRelay.cpp
// Project: Nostril (aka Postal)
// 
// History:
//		08/11/97 JMI	Stole infrastructure from SoundThing.
//
//		08/11/97	JMI	Now verifies that the parent ID is that of a CSoundThing
//							before using it.  Don't know what would happen otherwise
//							but I'm sure it would suck.
//
//		09/27/99	JMI	Eliminated boolean performance warnings.
//
//////////////////////////////////////////////////////////////////////////////
//
// This CThing-derived class will relay sound volumes based on 
//	DistanceToVolume() (i.e., the distance to the ear (usually the local dude))
// to the selected CSoundThing.
//
//////////////////////////////////////////////////////////////////////////////

#include "SndRelay.h"

#include "SampleMaster.h"

#include "SoundThing.h"
#include "game.h"
#include "realm.h"

// This class has its own GetRandom() to keep it from de-synching the game.
#ifdef GetRandom
	#undef GetRandom
#endif

#ifdef GetRand
	#undef GetRand
#endif


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define IMAGE_FILE			"soundSatellite.bmp"

#define GUI_FILE_NAME		"res/editor/SndRelay.gui"

////////////////////////////////////////////////////////////////////////////////
// Class statics.
////////////////////////////////////////////////////////////////////////////////

int16_t	CSndRelay::ms_sFileCount			= 0;	// File count.         


CSndRelay::CSndRelay(void)
{
  m_bInitiallyEnabled = true;
  m_bEnabled = m_bInitiallyEnabled;

  m_sSuspend = 0;

  m_state = State_Happy;
}

CSndRelay::~CSndRelay(void)
{

  if (m_pImage != nullptr)
    rspReleaseResource(&g_resmgrGame, &m_pImage);
}

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CSndRelay::Load(								// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	int16_t sFileCount,										// In:  File count (unique per file, never 0)
	uint32_t	ulFileVersion)									// In:  Version of file format to load.
	{
	int16_t sResult = CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == SUCCESS)
		{
		// If new file . . . 
		if (sFileCount != ms_sFileCount)
			{
			ms_sFileCount	= sFileCount;
			
			// Do one time stuff.
			}

		switch (ulFileVersion)
			{
			default:
			case 44:
			case 43:
			case 42:
			case 41:
			case 40:
			case 39:
			case 38:
			case 37:
			case 36:
			case 35:
			case 34:
			case 33:
			case 32:
			case 31:
			case 30:
			case 29:
			case 28:
			case 27:
			case 26:
			case 25:
			case 24:
			case 23:
			case 22:
			case 21:
			case 20:
			case 19:
			case 18:
			case 17:
			case 16:
			case 15:
			case 14:
			case 13:
			case 12:
			case 11:
			case 10:
			case 9:
			case 8:
			case 7:
			case 6:
			case 5:
			case 4:
			case 3:
			case 2:
			case 1:
            pFile->Read(&position.x);
            pFile->Read(&position.y);
            pFile->Read(&position.z);

				int32_t	lBool;
				pFile->Read(&lBool);
				m_bInitiallyEnabled	= lBool ? true : false;

            uint16_t parent_id;
            pFile->Read(&parent_id);
            m_parent = realm()->GetOrAddThingById<CThing>(parent_id, CSoundThingID);

				break;
			}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == SUCCESS)
			{
			sResult = Init();
			}
		else
			{
			sResult = FAILURE;
			TRACE("CSndRelay::Load(): Error reading from file!\n");
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CSndRelay::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	int16_t sFileCount)										// In:  File count (unique per file, never 0)
	{
	int16_t sResult	= CThing::Save(pFile, sFileCount);
	if (sResult == SUCCESS)
		{
      pFile->Write(position.x);
      pFile->Write(position.y);
      pFile->Write(position.z);
      pFile->Write((int32_t)m_bInitiallyEnabled);
      uint16_t parent_id = parent() ? parent()->GetInstanceID() : UINT16_MAX;
      pFile->Write(parent_id);

		// Make sure there were no file errors
		sResult	= pFile->Error();
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::Suspend(void)
	{
	m_sSuspend++;
	}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::Resume(void)
	{
	m_sSuspend--;
	}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::Update(void)
	{
	if (!m_sSuspend)
		{
		// If enabled . . .
		if (m_bEnabled == true)
			{
         // Attempt to get ptr to our parent . . .
         if (parent() && parent()->type() == CSoundThingID)
            {
              managed_ptr<CSoundThing> pst(parent());
					// Report volume based on our distance to the ear.
               pst->RelayVolume(DistanceToVolume(position.x, position.y, position.z, pst->m_lVolumeHalfLife) );
            }
         else if (parent())
            {
               TRACE("Update(): ID %hu is not a \"SoundThing\", it is a \"%s\".\n",
                     parent()->type(),
                     parent()->name());
				}
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::Render(void)
	{
	}

#if !defined(EDITOR_REMOVED)
////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
int16_t CSndRelay::EditNew(								// Returns 0 if successfull, non-zero otherwise
	int16_t sX,												// In:  New x coord
	int16_t sY,												// In:  New y coord
	int16_t sZ)												// In:  New z coord
	{
	int16_t sResult = SUCCESS;
	
	// Use specified position
   position.x = sX;
   position.y = sY;
   position.z = sZ;

	sResult	= EditModify();

	return sResult;
	}
#endif // !defined(EDITOR_REMOVED)

////////////////////////////////////////////////////////////////////////////////
// Helper function/macro for changing a GUIs text value.
////////////////////////////////////////////////////////////////////////////////
inline void SetGuiItemVal(	// Returns nothing.
	RGuiItem*	pguiRoot,	// In:  GUI Root.
	int32_t			lId,			// In:  ID of item whose text we'll change.
	int32_t			lVal)			// In:  New value.
	{
	RGuiItem*	pgui	= pguiRoot->GetItemFromId(lId);
	if (pgui)
		{
      pgui->SetText("%i", lVal);
		pgui->Compose();
		}
	}
#ifdef UNUSED_FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
// Callback from multibtn checkbox.
////////////////////////////////////////////////////////////////////////////////
static void CheckEnableGuiCall(	// Returns nothing.
	RGuiItem*	pgui_pmb)			// In:  GUI pointer to the multi button that was 
											// pressed.
	{
	ASSERT(pgui_pmb->m_type == RGuiItem::MultiBtn);

	RMultiBtn*	pmb	= (RMultiBtn*)pgui_pmb;

	// Show based on value stored in GUI.
	int16_t	sVisible	= pmb->m_ulUserData;
	// If unchecked . . .
	if (pmb->m_sState == 1)
		{
		// Opposite show/hide state.
		sVisible	= !sVisible;
		}

	RGuiItem*	pguiLoopSettingsContainer	= pmb->GetParent()->GetItemFromId(pmb->m_ulUserInstance);
	if (pguiLoopSettingsContainer)
		{
		pguiLoopSettingsContainer->SetVisible(sVisible);
		}
	}
#endif

#if !defined(EDITOR_REMOVED)
////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
int16_t CSndRelay::EditModify(void)
	{
	int16_t sResult = SUCCESS;

	// Load gui dialog
	RGuiItem* pgui = RGuiItem::LoadInstantiate(FullPathVD(GUI_FILE_NAME));
	if (pgui != nullptr)
		{
		// Init "ID" edit
		REdit* peditParentId = (REdit*)pgui->GetItemFromId(100);
		ASSERT(peditParentId != nullptr);
		ASSERT(peditParentId->m_type == RGuiItem::Edit);
      if (parent())
			{
         peditParentId->SetText("%hu", parent()->GetInstanceID());
			}
		else
			{
			peditParentId->SetText("");
			}

		peditParentId->Compose();

		// Init "enable" push button
		RMultiBtn* pmbEnable = (RMultiBtn*)pgui->GetItemFromId(200);
		ASSERT(pmbEnable != nullptr);
		pmbEnable->m_sState = (m_bInitiallyEnabled == true) ? 2 : 1;
		pmbEnable->Compose();

		// Run the dialog using this super-duper helper funciton
		if (DoGui(pgui) == 1)
			{
         // Get new values from dialog
        m_parent = realm()->GetThingById<CThing>(peditParentId->GetVal());
			m_bInitiallyEnabled = (pmbEnable->m_sState == 2) ? true : false;
			}
		else
			{
			sResult = FAILURE;
			}
		
		// Done with GUI.
		delete pgui;
		}
	else
		{
		sResult = FAILURE;
		}

	// If everything's okay, init using new values
	if (sResult == SUCCESS)
		sResult = Init();

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
int16_t CSndRelay::EditMove(									// Returns 0 if successfull, non-zero otherwise
	int16_t sX,												// In:  New x coord
	int16_t sY,												// In:  New y coord
	int16_t sZ)												// In:  New z coord
	{
   position.x = sX;
   position.y = sY;
   position.z = sZ;

   return SUCCESS;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the clickable pos/area of an object in 2D.
// (virtual	(Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::EditRect(	// Returns nothiing.
	RRect*	prc)				// Out: Clickable pos/area of object.
	{
   realm()->Map3Dto2D(position.x, position.y, position.z,
                      prc->sX, prc->sY);

	prc->sW	= 10;	// Safety.
	prc->sH	= 10;	// Safety.

   if (m_pImage)
		{
      prc->sW	= m_pImage->m_sWidth;
      prc->sH	= m_pImage->m_sHeight;
		}

	prc->sX	-= prc->sW / 2;
	prc->sY	-= prc->sH;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
// (virtual	(Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::EditHotSpot(	// Returns nothiing.
	int16_t*	psX,					// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
	int16_t*	psY)					// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.
	{
	*psX	= 5;	// Safety.
	*psY	= 5;	// Safety.

   if (m_pImage != nullptr)
		{
      *psX	= m_pImage->m_sWidth / 2;
      *psY	= m_pImage->m_sHeight;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::EditUpdate(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CSndRelay::EditRender(void)
{
  // Setup simple, non-animating sprite
  flags.clear();

  realm()->Map3Dto2D(position.x, position.y, position.z,
                     m_sX2, m_sY2);

  // Priority is based on bottom edge of sprite
  m_sPriority = position.z;

  // Center on image.
  m_sX2	-= m_pImage->m_sWidth / 2;
  m_sY2	-= m_pImage->m_sHeight;

  m_sLayer = CRealm::GetLayerViaAttrib(realm()->GetLayer((int16_t) position.x, (int16_t) position.z));

  Object::enqueue(SpriteUpdate); // Update sprite in scene
}
#endif // !defined(EDITOR_REMOVED)

////////////////////////////////////////////////////////////////////////////////
// Init object
////////////////////////////////////////////////////////////////////////////////
int16_t CSndRelay::Init(void)							// Returns 0 if successfull, non-zero otherwise
	{
	int16_t sResult = SUCCESS;

	m_bEnabled = m_bInitiallyEnabled;

   if (m_pImage == nullptr)
		{
      sResult = rspGetResource(&g_resmgrGame, realm()->Make2dResPath(IMAGE_FILE), &m_pImage);
		if (sResult == SUCCESS)
			{
			// This is a questionable action on a resource managed item, but it's
			// okay if EVERYONE wants it to be an FSPR8.
         if (m_pImage->Convert(RImage::FSPR8) != RImage::FSPR8)
				{
				sResult = FAILURE;
				TRACE("CSndRelay::GetResource() - Couldn't convert to FSPR8\n");
				}
			}
		}

	return sResult;
	}



////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
