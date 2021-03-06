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
// goaltimer.cpp
// Project: Postal
//
//	This module is a CThing that can be put in a level to keep track of a kill
//	goal or time limit goal.  It can be set to keep the amount of time it takes
//	to kill a set number of people, or it can be set to count down to zero and
//	end the realm when the timer expires.
//
//
// History:
//
//		06/30/97 BRH	Started this file as part of the challenge levels.
//
//		07/09/97	JMI	Now uses realm()->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		08/05/97	JMI	Changed priority to use Z position rather than 2D 
//							projected Y position.
//
////////////////////////////////////////////////////////////////////////////////

#include "goaltimer.h"

#include "realm.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define IMAGE_FILE			"clock.bmp"
#define TIMER_VALUE_GUI_ID	10
#define KILL_VALUE_GUI_ID	11
#define UPDOWN_GUI_ID		12


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!

// Let this auto-init to 0
int16_t CGoalTimer::ms_sFileCount;


CGoalTimer::CGoalTimer(void)
{
  m_sSuspend = 0;
  m_sUpDown = 1;
  m_sKillGoal = 0;
  m_lTimerMS = 120000;
}

CGoalTimer::~CGoalTimer(void)
{
  // Free resources
  FreeResources();
}

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CGoalTimer::Load(							// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	int16_t sFileCount,										// In:  File count (unique per file, never 0)
	uint32_t	ulFileVersion)									// In:  Version of file format to load.
{
	// Call the base class load to get the instance ID
	int16_t sResult = CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == SUCCESS)
	{
		// Load common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
		{
			ms_sFileCount = sFileCount;

			// Load static data
			switch (ulFileVersion)
			{
				default:
				case 1:
					break;
			}
		}

		// Load object data
		switch (ulFileVersion)
		{
			default:
			case 1:
            pFile->Read(&position.x);
            pFile->Read(&position.y);
            pFile->Read(&position.z);
				pFile->Read(&m_lTimerMS);
				pFile->Read(&m_sKillGoal);
				pFile->Read(&m_sUpDown);
				break;
		}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == SUCCESS)
		{
			// Get resources
			sResult = GetResources();
		}
		else
		{
			sResult = FAILURE;
			TRACE("CGoalTimer::Load(): Error reading from file!\n");
		}
	}
	else
	{
		TRACE("CGoalTimer::Load(): CThing::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CGoalTimer::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	int16_t sFileCount)										// In:  File count (unique per file, never 0)
{
	// Call the base class save to save the instance ID
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
	pFile->Write(&m_lTimerMS);
	pFile->Write(&m_sKillGoal);
	pFile->Write(&m_sUpDown);

   return SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
void CGoalTimer::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
	// At this point we can assume the CHood was loaded, so we init our height
   position.y = realm()->GetHeight((int16_t) position.x, (int16_t) position.z);
}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CGoalTimer::Suspend(void)
{
	m_sSuspend++;
}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CGoalTimer::Resume(void)
{
	m_sSuspend--;

	// If we're actually going to start updating again, we need to reset
	// the time so as to ignore any time that passed while we were suspended.
	// This method is far from precise, but I'm hoping it's good enough.
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CGoalTimer::Update(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CGoalTimer::Render(void)
{
}

#if !defined(EDITOR_REMOVED)
////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
int16_t CGoalTimer::EditNew(									// Returns 0 if successfull, non-zero otherwise
	int16_t sX,												// In:  New x coord
	int16_t sY,												// In:  New y coord
	int16_t sZ)												// In:  New z coord
{
   int16_t sResult = SUCCESS;
	
	// Use specified position
   position.x = (double)sX;
   position.y = (double)sY;
   position.z = (double)sZ;

	// Load resources
	sResult = GetResources();

	if (sResult == SUCCESS)
	{
/*
      CListNode<CThing>* pEditorList = realm()->m_aclassHeads[CGameEditThingID].m_pnNext;
		CGameEditThing* peditor = (CGameEditThing*) pEditorList->m_powner;
		RListBox* plb = peditor->m_plbNavNetList;
		if (plb != nullptr)
		{
         RGuiItem* pgui = plb->AddString(m_rstrNetName);
			pgui->m_lId = GetInstanceID();
			pgui->m_bcUser = NavNetListPressedCall;
			pgui->m_ulUserInstance = (unsigned long) this;
			plb->AdjustContents();
			plb->SetSel(pgui);
		}
*/
	}

	return sResult;
}
#endif // !defined(EDITOR_REMOVED)

////////////////////////////////////////////////////////////////////////////////
// Helper inline to get a GUI, set its text to the value, and recompose it.
////////////////////////////////////////////////////////////////////////////////
inline
void SetText(					// Returns nothing.
	RGuiItem*	pguiRoot,	// In:  Root GUI.
	int32_t			lId,			// In:  ID of GUI to set text.
	int32_t			lVal)			// In:  Value to set text to.
	{
	RGuiItem*	pgui	= pguiRoot->GetItemFromId(lId);
	if (pgui != nullptr)
		{
		pgui->SetText("%i", lVal);
		pgui->Compose(); 
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Helper inline to get a RMultiBtn GUI, and set its state.
////////////////////////////////////////////////////////////////////////////////
inline
void CheckMultiBtn(			// Returns nothing.
	RGuiItem*	pguiRoot,	// In:  Root GUI.
	int32_t			lId,			// In:  ID of GUI to set text.
	int16_t			sChecked)	// In:  1 to check, 0 to uncheck.
	{
	RMultiBtn*	pmb	= (RMultiBtn*)pguiRoot->GetItemFromId(lId);
	if (pmb != nullptr)
		{
		ASSERT(pmb->m_type == RGuiItem::MultiBtn);

		pmb->m_sState	 = sChecked + 1;

		pmb->Compose();
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Helper inline to get a RMultiBtn GUI, and return its state.
////////////////////////////////////////////////////////////////////////////////
inline
int16_t IsMultiBtnChecked(	// Returns multibtn's state.
	RGuiItem*	pguiRoot,	// In:  Root GUI.
	int32_t			lId)			// In:  ID of GUI to set text.
	{
   int16_t sResult = SUCCESS;	// Assume nothing;

	RMultiBtn*	pmb	= (RMultiBtn*)pguiRoot->GetItemFromId(lId);
	if (pmb != nullptr)
		{
		ASSERT(pmb->m_type == RGuiItem::MultiBtn);

		sResult	= (pmb->m_sState == 1) ? 0 : 1;
		}

	return sResult;
	}

#if !defined(EDITOR_REMOVED)
////////////////////////////////////////////////////////////////////////////////
// Edit Modify
////////////////////////////////////////////////////////////////////////////////

int16_t CGoalTimer::EditModify(void)
{
   int16_t sResult = SUCCESS;
	RGuiItem* pgui = RGuiItem::LoadInstantiate(FullPathVD("res/editor/clock.gui"));
	if (pgui)
	{
		SetText(pgui, TIMER_VALUE_GUI_ID, m_lTimerMS);
		SetText(pgui, KILL_VALUE_GUI_ID,	m_sKillGoal);
		CheckMultiBtn(pgui, UPDOWN_GUI_ID, m_sUpDown);

		sResult = DoGui(pgui);
		if (sResult == 1)
		{
			m_lTimerMS = pgui->GetVal(TIMER_VALUE_GUI_ID);
			m_sKillGoal = pgui->GetVal(KILL_VALUE_GUI_ID);
			m_sUpDown = IsMultiBtnChecked(pgui, UPDOWN_GUI_ID);
		}
		delete pgui;
	}

   return SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
int16_t CGoalTimer::EditMove(									// Returns 0 if successfull, non-zero otherwise
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
void CGoalTimer::EditUpdate(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CGoalTimer::EditRender(void)
{
	// No special flags
   flags.clear();

	// Map from 3d to 2d coords
   realm()->Map3Dto2D(position.x, position.y, position.z,
                      m_sX2, m_sY2);

	// Priority is based on bottom edge of sprite
   m_sPriority = position.z;

	// Center on image.
   m_sX2	-= m_pImage->m_sWidth / 2;
   m_sY2	-= m_pImage->m_sHeight;

	// Layer should be based on info we get from attribute map.
   m_sLayer = CRealm::GetLayerViaAttrib(realm()->GetLayer((int16_t) position.x, (int16_t) position.z));

   Object::enqueue(SpriteUpdate); // Update sprite in scene
}

////////////////////////////////////////////////////////////////////////////////
// Give Edit a rectangle around this object
////////////////////////////////////////////////////////////////////////////////
void CGoalTimer::EditRect(RRect* pRect)
{
  realm()->Map3Dto2D(position.x, position.y, position.z,
                     pRect->sX, pRect->sY);

	pRect->sW	= 10;	// Safety.
	pRect->sH	= 10;	// Safety.

	if (m_pImage != nullptr)
	{
		pRect->sW	= m_pImage->m_sWidth;
		pRect->sH	= m_pImage->m_sHeight;
	}

	pRect->sX	-= pRect->sW / 2;
	pRect->sY	-= pRect->sH;
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
////////////////////////////////////////////////////////////////////////////////
void CGoalTimer::EditHotSpot(	// Returns nothiing.
	int16_t*	psX,							// Out: X coord of 2D hotspot relative to
												// EditRect() pos.
	int16_t*	psY)							// Out: Y coord of 2D hotspot relative to
												// EditRect() pos.
{
	// Base of navnet is hotspot.
	*psX	= (m_pImage->m_sWidth / 2);
	*psY	= m_pImage->m_sHeight;
}
#endif // !defined(EDITOR_REMOVED)

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
int16_t CGoalTimer::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
{
   int16_t sResult = SUCCESS;

   if (m_pImage == nullptr)
	{
		sResult	= rspGetResource(&g_resmgrGame, realm()->Make2dResPath(IMAGE_FILE), &m_pImage);
		if (sResult == SUCCESS)
		{
			// This is a questionable action on a resource managed item, but it's
			// okay if EVERYONE wants it to be an FSPR8.
			if (m_pImage->Convert(RImage::FSPR8) != RImage::FSPR8)
			{
				sResult = FAILURE;
				TRACE("CGoalTimer::GetResource(): Couldn't convert to FSPR8!\n");
			}
		}
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
int16_t CGoalTimer::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
   int16_t sResult = SUCCESS;

   if (m_pImage != nullptr)
	{
		rspReleaseResource(&g_resmgrGame, &m_pImage);
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
