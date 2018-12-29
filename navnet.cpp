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
// navnet.cpp
// Project: Nostril (aka Postal)
//
// This module implements the bouy marker for use with the network navagation
//	system that will help the enemy guys get around the world.
//
// History:
//		01/28/97 BRH	Added navigation net objects as the thing that holds 
//							a group of bouys together in a navigational network.
//
//		02/02/97 BRH	Added Ping routine that gives the miniumum number of hops
//							from src to dst nodes in the network.  Added 
//							FindNearestBouy function to locate the closest Bouy to
//							a given position.  This may later be optimized using 
//							collision regions.  Also added UpdateRoutingTables()
//							function which asks for each destination node from each
//							Bouy so that each Bouy will discover a route and add it
//							to its routing table.  This will give a fully connected
//							routing table for faster access rather than building
//							the routing tables as we go.  
//
//		02/03/97 BRH	Changed Load and Save functions to call the base
//							class load and save to preserve the instance ID's
//							and also saved the number of nodes in the network
//							to be used later to reinitialize the network after
//							a load of the NavNet and the Bouys.  Added the
//							startup code to update the routing tables after the load
//							is done.
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/04/97 BRH	Sets itself to be the current Nav Net in the realm on 
//							construction, Startup, and EditModify.
//
//		02/05/97 BRH	Fixed problem with the FindNearestBouy function and 
//							fixed a problem with Ping where it wasn't adding 1 
//							if it pinged a link from a routing table entry.
//
//		02/06/97 BRH	Added SetAsDefault() function which sets this NavNet
//							as the default one in its realm.  This was needed 
//							by the editor when deleteing the default NavNet, the
//							editor didn't have access to the realm's protected
//							current NavNet pointer.  Also added DeleteNetwork()
//							which frees all of the bouys in this network.  This
//							is also called by the editor when deleting a NavNet.
//
//		02/18/97	JMI	Changed GetResources() to use the resmgr.
//
//		02/23/97 BRH	Moved Render() functionality to EditRender and made
//							Render do nothing so that it isn't drawing during gameplay.
//
//		02/24/97	JMI	No longer sets the m_type member of the m_sprite b/c it
//							is set by m_sprite's constructor.
//
//		03/04/97 BRH	Added a string name to the NavNet so it can be displayed
//							in a listbox int eh editor to indicate the current
//							NavNet and be able to tell them apart.
//
//		03/06/97 BRH	Changed Ping to include a visited nodes list to detect
//							loops faster.  The old method was extremely slow, as in
//							days of checking for a network of 45 bouys.  The new one
//							is still somewhat slow at 3 minutes for that number, but
//							the bouys will save their route tables to save time when
//							the level is loaded.
//
//		03/13/97 BRH	Made a few modifications to Ping and verified that it
//							correctly builds routing tables for several networks. 
//							It is still slow for large looped networks so I will
//							be adding a hops table to the bouys to cut down on the
//							number of pings required to complete the tables.  Then
//							the hops tables can be freed at some point.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		04/10/97 BRH	Updated this to work with the new multi layer attribute
//							maps.
//
//		04/15/97 BRH	Changing over to the new BFS tree method of routing, and
//							getting rid of the old code like Ping().  Also added
//							dialog box that allows you to set the name of the NavNet
//							as well as to set it as the default Network.
//
//		04/18/97 BRH	Fixed problem in Save where it saved the number of nodes
//							using the last index, rather than the size of the node map
//							which would be off if some bouys were deleted from the
//							network.  Changed it to save the size of the m_NodeMap.
//
//		05/29/97	JMI	Removed ASSERT on realm()->m_pAttribMap which no longer
//							exists.
//
//		06/27/97 BRH	Changed FindNearestBouy to use the new bouy tree nodes
//							to build a sorted list of nearest bouys.  It then checks
//							to see if the closest is reachable without terrain
//							terrain blocking the path, and if so returns that bouy.
//							If it is blocked, then it will try the next one in the
//							list.
//
//		06/29/97	JMI	Converted EditRect(), EditRender(), and/or Render() to
//							use Map3Dto2D().
//							Moved EditRect() here from navnet.h and added EditHotSpot().
//
//		06/30/97 BRH	Fixed bug where the wrong overloaded version of IsPathClear
//							was being called.
//
//		07/09/97	JMI	Now uses realm()->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/14/97	JMI	FindNearestBouy() was deleting a NULL tree, if no bouys.  
//							Fixed.
//
//		07/25/97 BRH	Fixed problem of > 254 bouys being created in the editor.
//
//		08/05/97	JMI	Changed priority to use Z position rather than 2D 
//							projected Y position.
//
//		08/08/97 BRH	Added EditPostLoad function which adds the Nav Net name
//							to the editor's list box, just as it does when EditNew is
//							called.  This way the NavNets loaded from the realm file
//							are now correctly displayed.
//
////////////////////////////////////////////////////////////////////////////////

#include "navnet.h"

#include "realm.h"
#include "bouy.h"
#include "gameedit.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define IMAGE_FILE			"nnet.bmp"

// Minimum elapsed time (in milliseconds)
#define MIN_ELAPSED_TIME	10


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!

// Let this auto-init to 0
int16_t CNavigationNet::ms_sFileCount;


CNavigationNet::CNavigationNet(void)
{
  m_sSuspend = 0;
  m_ucNextID = 1;
  // Set yourself to be the new current Nav Net in the realm
  //pRealm->m_pCurrentNavNet = this; // TODO: REPLACE
  // Set default name as NavNetxx where xx is CThing ID
  m_rstrNetName = "Untitled NavNet";
  // Initialize the dummy nodes for the sorted tree/list
  m_BouyTreeListHead.m_powner = nullptr;
  m_BouyTreeListHead.m_pnPrev = nullptr;
  m_BouyTreeListHead.m_pnRight = nullptr;
  m_BouyTreeListHead.m_pnLeft = nullptr;
  m_BouyTreeListHead.m_pnNext = &m_BouyTreeListTail;
  m_BouyTreeListTail.m_powner = nullptr;
  m_BouyTreeListTail.m_pnPrev = &m_BouyTreeListHead;
  m_BouyTreeListTail.m_pnNext = nullptr;
  m_BouyTreeListTail.m_pnRight = nullptr;
  m_BouyTreeListTail.m_pnLeft = nullptr;
}

CNavigationNet::~CNavigationNet(void)
{
  // Free resources
  FreeResources();
}

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CNavigationNet::Load(							// Returns 0 if successfull, non-zero otherwise
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
            pFile->Read(&m_position.x);
            pFile->Read(&m_position.y);
            pFile->Read(&m_position.z);

				// Load the number of bouys that were saved
				pFile->Read(&m_ucNumSavedBouys);

				m_rstrNetName.Load(pFile);
				
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
			TRACE("CNavigationNet::Load(): Error reading from file!\n");
			}
		}
	else
		{
		TRACE("CNavigationNet::Load(): CThing::Load() failed.\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
int16_t CNavigationNet::Save(										// Returns 0 if successfull, non-zero otherwise
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
   pFile->Write(&m_position.x);
   pFile->Write(&m_position.y);
   pFile->Write(&m_position.z);

	// Save the number of nodes so we can check after load to see if all
	// of the Bouys have been loaded yet.
//	pFile->Write(&m_ucNextID);
	uint8_t ucNumNodes = m_NodeMap.size();
	pFile->Write(&ucNumNodes);

	m_rstrNetName.Save(pFile);

   return SUCCESS;
	}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
void CNavigationNet::Startup(void)								// Returns 0 if successfull, non-zero otherwise
   {
	// At this point we can assume the CHood was loaded, so we init our height
   m_position.y = realm()->GetHeight((int16_t) m_position.x, (int16_t) m_position.z);
	// Set yourself to be the new current Nav Net
   realm()->setNavNet(this);

	// Init other stuff
   if (m_ucNextID > m_ucNumSavedBouys)
      UpdateRoutingTables();
	}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CNavigationNet::Suspend(void)
	{
	m_sSuspend++;
	}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CNavigationNet::Resume(void)
	{
	m_sSuspend--;

	// If we're actually going to start updating again, we need to reset
	// the time so as to ignore any time that passed while we were suspended.
	// This method is far from precise, but I'm hoping it's good enough.
	}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CNavigationNet::Update(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CNavigationNet::Render(void)
	{
	}

#if !defined(EDITOR_REMOVED)
////////////////////////////////////////////////////////////////////////////////
// Called by editor after load to set up the nav net list box in the editor
// similar to EditNew but without the positioning.
////////////////////////////////////////////////////////////////////////////////

int16_t CNavigationNet::EditPostLoad(void)
{
   int16_t sResult = SUCCESS;

   RListBox* plb = managed_ptr<CGameEditThing>(realm()->GetThingsByType(CGameEditThingID).front())->m_plbNavNetList;
	if (plb != nullptr)
	{
      RGuiItem* pgui = plb->AddString(m_rstrNetName);
		pgui->m_lId = GetInstanceID();
		pgui->m_bcUser = NavNetListPressedCall;
      pgui->m_ulUserInstance = (uintptr_t) this;
		plb->AdjustContents();
		plb->SetSel(pgui);
	}
	else
	{
		sResult = FAILURE;
		TRACE("CNavigationNet::EditLoad - Error setting up NavNet list box in editor\n");
	}

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
int16_t CNavigationNet::EditNew(									// Returns 0 if successfull, non-zero otherwise
	int16_t sX,												// In:  New x coord
	int16_t sY,												// In:  New y coord
	int16_t sZ)												// In:  New z coord
	{
   int16_t sResult = SUCCESS;
	
	// Use specified position
   m_position.x = (double)sX;
   m_position.y = (double)sY;
   m_position.z = (double)sZ;

	// Load resources
	sResult = GetResources();

	if (sResult == SUCCESS)
	{
     RListBox* plb = managed_ptr<CGameEditThing>(realm()->GetThingsByType(CGameEditThingID).front())->m_plbNavNetList;
		if (plb != nullptr)
		{
         RGuiItem* pgui = plb->AddString(m_rstrNetName);
			pgui->m_lId = GetInstanceID();
			pgui->m_bcUser = NavNetListPressedCall;
         pgui->m_ulUserInstance = (uintptr_t) this;
			plb->AdjustContents();
			plb->SetSel(pgui);
		}
	}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Helper inline to get a GUI, set its text to the value, and recompose it.
////////////////////////////////////////////////////////////////////////////////
inline
void SetText(					// Returns nothing.
	RGuiItem*	pguiRoot,	// In:  Root GUI.
	int32_t			lId,			// In:  ID of GUI to set text.
   const char*			str)			// In:  Value to set text to.
	{
	RGuiItem*	pgui	= pguiRoot->GetItemFromId(lId);
	if (pgui != nullptr)
		{
		pgui->SetText(str);
		pgui->Compose(); 
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Edit Modify
////////////////////////////////////////////////////////////////////////////////

int16_t CNavigationNet::EditModify(void)
{
   int16_t sResult = SUCCESS;
	RGuiItem* pGui = RGuiItem::LoadInstantiate(FullPathVD("res/editor/network.gui"));
	if (pGui)
   {
      SetText(pGui, 3, m_rstrNetName.operator const char *());

		sResult = DoGui(pGui);
		if (sResult == 1)
		{
			SetAsDefault();
			m_rstrNetName.Grow(256);
         pGui->GetText(3, m_rstrNetName.Data(), 255);
			m_rstrNetName.Update();

         RListBox* plb = managed_ptr<CGameEditThing>(realm()->GetThingsByType(CGameEditThingID).front())->m_plbNavNetList;
         SetText(plb, GetInstanceID(), m_rstrNetName.operator const char *());
			plb->SetSel(plb->GetItemFromId(GetInstanceID()));
//			RGuiItem* pguiRemove = plb->GetItemFromId(GetInstanceID());
//			SetText(pguiRemove, 

		}
	}

   return SUCCESS;
}
/*

		RGuiItem*	pguiRemove;
		if (pview != nullptr)
			{
			pguiRemove	= plb->GetItemFromId((long)pview);
			}
		else
			{
			pguiRemove	= plb->GetSel();
			}

		if (pguiRemove != nullptr)
			{
			KillView(static_cast<View*>(pguiRemove->m_lId));
			plb->RemoveItem(pguiRemove);
			plb->AdjustContents();
*/
////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
int16_t CNavigationNet::EditMove(									// Returns 0 if successfull, non-zero otherwise
	int16_t sX,												// In:  New x coord
	int16_t sY,												// In:  New y coord
	int16_t sZ)												// In:  New z coord
	{
   m_position.x = (double)sX;
   m_position.y = (double)sY;
   m_position.z = (double)sZ;

   return SUCCESS;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CNavigationNet::EditUpdate(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CNavigationNet::EditRender(void)
{
  // No special flags
  m_sInFlags = 0;

  // Map from 3d to 2d coords
  realm()->Map3Dto2D(m_position.x, m_position.y, m_position.z,
                     m_sX2, m_sY2);

  // Priority is based on bottom edge of sprite
  m_sPriority = m_position.z;

  // Center on image.
  m_sX2	-= m_pImage->m_sWidth / 2;
  m_sY2	-= m_pImage->m_sHeight;

  // Layer should be based on info we get from attribute map.
  m_sLayer = CRealm::GetLayerViaAttrib(realm()->GetLayer((int16_t) m_position.x, (int16_t) m_position.z));

  // Image would normally animate, but doesn't for now
  m_pImage = m_pImage;

  Object::enqueue(SpriteUpdate); // Update sprite in scene
}

////////////////////////////////////////////////////////////////////////////////
// Give Edit a rectangle around this object
////////////////////////////////////////////////////////////////////////////////
void CNavigationNet::EditRect(RRect* pRect)
{
  realm()->Map3Dto2D(m_position.x, m_position.y, m_position.z,
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
void CNavigationNet::EditHotSpot(	// Returns nothiing.
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
int16_t CNavigationNet::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
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
				TRACE("CNavigationNet::GetResource(): Couldn't convert to FSPR8!\n");
				}
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
int16_t CNavigationNet::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
   int16_t sResult = SUCCESS;

   if (m_pImage != nullptr)
		{
		rspReleaseResource(&g_resmgrGame, &m_pImage);
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// AddBouy - Returns zero if there are no bouys left.
////////////////////////////////////////////////////////////////////////////////

uint8_t CNavigationNet::AddBouy(CBouy* pBouy)
{
	uint8_t ucID = 0;

	if (m_ucNextID < 254)
	{
		pBouy->m_ucID = m_ucNextID;
      pBouy->m_pParentNavNet = this;
      m_NodeMap.emplace(m_ucNextID, pBouy);
		m_ucNextID++;
		ucID = pBouy->m_ucID;
	}

	return ucID;
}

////////////////////////////////////////////////////////////////////////////////
// RemoveBouy
////////////////////////////////////////////////////////////////////////////////

void CNavigationNet::RemoveBouy(uint8_t ucBouyID)
{
	m_NodeMap.erase(ucBouyID);
	UpdateRoutingTables();
}

////////////////////////////////////////////////////////////////////////////////
// GetBouy
////////////////////////////////////////////////////////////////////////////////

managed_ptr<CBouy> CNavigationNet::GetBouy(uint8_t ucBouyID)
{
   managed_ptr<CBouy> pBouy;

   auto iter = m_NodeMap.find(ucBouyID);
   if (iter != m_NodeMap.end())
      pBouy = iter->second;

	return pBouy;
}

////////////////////////////////////////////////////////////////////////////////
// FindNearestBouy - Go through the list and get the ID of the nearest
//							bouy to the given x, z position
//
//							This has been modified to return the closest available
//							bouy - one that is not blocked by terrain.
////////////////////////////////////////////////////////////////////////////////

uint8_t CNavigationNet::FindNearestBouy(int16_t sX, int16_t sZ)
{
	double dSqDist;
	double dX;
	double dZ;
   managed_ptr<CBouy> pBouy;
	TreeListNode* pRoot = nullptr;
	TreeListNode* pCurrent = nullptr;
	uint8_t	ucNode = 0;

	// Build the sorted list of bouys
   for (auto iter = m_NodeMap.begin(); iter != m_NodeMap.end(); ++iter)
	{
      pBouy = iter->second;
      dX = pBouy->m_position.x - sX;
      dZ = pBouy->m_position.z - sZ;
		dSqDist = (dX * dX) + (dZ * dZ);
		pBouy->m_TreeNode.m_sortkey = dSqDist;
      pBouy->m_TreeNode.m_powner = pBouy.pointer();
		if (pRoot == nullptr)
		{
			pRoot = &pBouy->m_TreeNode;
			//m_BouyTreeListHead.AddAfter(&pBouy->m_TreeNode);
			pBouy->m_TreeNode.AddAfter(&m_BouyTreeListHead);
		}
		else
			pRoot->Add(&pBouy->m_TreeNode);
	}

	pCurrent = m_BouyTreeListHead.m_pnNext;
	bool bSearching = true;
	// Get the height at the startling location for path checking
   int16_t sY = realm()->GetHeight(sX, sZ);

	// Go through the sorted list of bouys and check to see if you could
	// get to it from where you are standing.  If not, check the next one.
	while (pCurrent && bSearching)
	{
		pBouy = pCurrent->m_powner;
//		ASSERT(pBouy != nullptr);
		if (pBouy)
		{
         if (realm()->IsPathClear(sX, sY, sZ, 4.0, (int16_t) pBouy->GetX(), (int16_t) pBouy->GetZ()))
			{
				ucNode = pBouy->m_ucID;
				bSearching = false;
			}
			else
				pCurrent = pCurrent->m_pnNext;
		}
		else
			pCurrent = pCurrent->m_pnNext;
	}

	// If we built a tree . . .
	if (pRoot)
	{
		// Get rid of the tree, its useless now
		pRoot->DeleteTree();
	}

	return ucNode;
}

#if 0
uint8_t CNavigationNet::FindNearestBouy(short sX, short sZ)
{
	nodeMap::iterator i;
	double dSqDist;
	double dMinDist = 1.0E+200;
	double dX;
	double dZ;
	CBouy* pBouy;
	uint8_t ucNode = 0;

	for (i = m_NodeMap.begin(); i != m_NodeMap.end(); i++)
	{
		pBouy = (CBouy*) (*i).second;
      dX = pBouy->m_position.x - sX;
      dZ = pBouy->m_position.z - sZ;
		dSqDist = (dX * dX) + (dZ * dZ);
		if (dSqDist < dMinDist)
		{
			dMinDist = dSqDist;
			ucNode = (*i).first;
		}
	}

	return ucNode;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// UpdateRoutingTables - Ping all of the bouys from the list in this network
//								 which will force them all to build up complete
//								 routing tables
////////////////////////////////////////////////////////////////////////////////

void CNavigationNet::UpdateRoutingTables(void)
{
   for (auto iter = m_NodeMap.begin(); iter != m_NodeMap.end(); ++iter)
      iter->second->BuildRoutingTable();

#if 0
	char szLine[256];
	FILE* fp;
	fp = fopen("c:\\temp\\net.txt", "wt");
	if (fp != nullptr)
	{
		for (i = m_NodeMap.begin(); i != m_NodeMap.end(); i++)
		{
			sprintf(szLine, "Node %d\n", ((*i).second)->m_ucID);
			fwrite(szLine, sizeof(char), strlen(szLine), fp);
			((*i).second)->BuildRoutingTable();
		}	 
		fclose(fp);

	}
	else
	{
		TRACE("*********** Nav Net unable to open log file\n");
	}
#endif

//	PrintRoutingTables();	
}

void CNavigationNet::PrintRoutingTables(void)
{
	char szLine[256];
	FILE* fp;
	fp = fopen("c:\\temp\\links.txt", "wt");
	if (fp != nullptr)
	{
      for (auto iter = m_NodeMap.begin(); iter != m_NodeMap.end(); ++iter)
		{
         sprintf(szLine, "\nNode %d\n----------------------\n", iter->second->m_ucID);
			fwrite(szLine, sizeof(char), strlen(szLine), fp);
         iter->second->PrintRouteTable(fp);
//			iter->second->PrintDirectLinks(fp);
		}	 
		fclose(fp);

	}
	else
	{
		TRACE("*********** Nav Net unable to open log file\n");
	}


}


int16_t CNavigationNet::SetAsDefault(void)
   {  int16_t sResult = SUCCESS;
      if (realm())
         realm()->setNavNet(this);
      else
         sResult = FAILURE;
      return sResult;
   }

////////////////////////////////////////////////////////////////////////////////
// DeleteNetwork - Delte all bouys from this network
////////////////////////////////////////////////////////////////////////////////

int16_t CNavigationNet::DeleteNetwork(void)
{
   int16_t sResult = SUCCESS;

	m_NodeMap.erase(m_NodeMap.begin(), m_NodeMap.end());

   return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
