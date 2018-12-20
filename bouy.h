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
// bouy.h
// Project: Postal
//
//	History:
//		03/07/97	JMI	Added an EditHotSpot() to make dragging smoother.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		04/15/97 BRH	Taking out the old routing methods in favor of the new
//							BFS tree method.  
//
//		06/06/97 BRH	Freed m_paucRouteTable in the Destructor which had
//							previously been a source of memory leaks.
//
//		06/25/97 BRH	Took out the STL set "linkset" because it was causing
//							sync problems with the game.  Apparently, the set uses
//							a tree to store its data, and when building that tree, 
//							uses some kind of random function to balance the tree, but
//							not the standard library rand() function, but a different
//							random function that we don't reset from realm to realm.
//							This caused the routing tables to be build differently
//							each time the game was played, and so the demo mode and
//							network mode were out of sync.
//
//		06/27/97 BRH	Added a CTreeListNode to the bouys which the NavNet
//							will use to build a sorted list of nearest bouys when
//							someone asks for a the nearest bouy.  
//
//		06/29/97 MJR	Removed last trace of STL, replacing vector with RFList.
//
//		06/30/97	JMI	Moved definitions of EditRect() and EditHotSpot() into 
//							bouy.cpp.
//
//		07/21/97	JMI	Added GetX(), GetY(), and GetZ().	
//
//		07/30/97 BRH	Added ms_bShowBouys flag and static functions to toggle
//							this flag.
//
//		08/08/97 BRH	Added a Visible() function for the gameedit to be able
//							to determine when the bouy is hiding itself.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef BOUY_H
#define BOUY_H

#include <RSPiX.h>
#include "realm.h"

// Template node class for linked lists
template <class Owner, class K>
class CTreeListNode
{
	typedef CTreeListNode Node;
	typedef K SORTKEY, *PSORTKEY;

//	protected:
	public:
		CTreeListNode()	
			{
				m_pnNext = m_pnPrev = m_pnRight = m_pnLeft = nullptr;
				m_powner = nullptr;
			}	// Do not use.

	public:
		CTreeListNode(Owner* powner)
		{ m_powner	= powner;
		  m_pnNext = m_pnPrev = m_pnRight = m_pnLeft = nullptr; }

		// Note:  This function can only be used with a list that has
		// dummy nodes for head and tail.
		Owner* GetNext(void)
		{ return m_pnNext->m_powner; }
		// Note:  This function can only be used with a list that has
		// dummy nodes for head and tail.
		Owner* GetPrev(void)
		{ return m_pnPrev->m_powner; }

		// Note:  This function can only be used with a list that has
		// dummy nodes for head and tail.
		void InsertBefore(
			Node* pn)	// In:  Node to insert before.
		{
			ASSERT(m_pnNext == nullptr && m_pnPrev == nullptr);
			m_pnNext					= pn;
			m_pnPrev					= pn->m_pnPrev;
			m_pnPrev->m_pnNext	= this;
			pn->m_pnPrev			= this;
		}

		// Note:  This function can only be used with a list that has
		// dummy nodes for head and tail.
		void AddAfter(
			Node* pn)	// In:  Node to add after.
		{
			ASSERT(m_pnNext == nullptr && m_pnPrev == nullptr);
			m_pnNext					= pn->m_pnNext;
			m_pnPrev					= pn;
			m_pnNext->m_pnPrev	= this;
			pn->m_pnNext			= this;
		}

		// Note:  This function can only be used with a list that has
		// dummy nodes for head and tail.
		// Note:  Do not call, if already removed.
		void Remove(void)
		{
			m_pnNext->m_pnPrev		= m_pnPrev;
			m_pnPrev->m_pnNext		= m_pnNext;
			m_pnNext						= nullptr;
			m_pnPrev						= nullptr;
		}

		// Note:  This function adds the item into the tree in its
		// proper place.  It also puts it in the list in the proper place
		// This should usually be called by the root of the tree
		void Add
			(Node* pn)	//In:  Node to add in the tree and list
		{
			if (pn->m_sortkey >= m_sortkey)
			// go right
			{
				// If this is the end of the line, add it here
				if (m_pnRight == nullptr)	
				{
					m_pnRight = pn;
					//AddAfter(pn);
					pn->AddAfter(this);
				}
				else
				{
					m_pnRight->Add(pn);
				}
			}
			else
			// go left
			{
				// If this is the end of the line, add it here
				if (m_pnLeft == nullptr)
				{
					m_pnLeft = pn;
					//InsertBefore(pn);
					pn->InsertBefore(this);
				}
				else
				{
					m_pnLeft->Add(pn);
				}
			}
		}

		// Note:  This function is used to clear the tree - it will remove
		// all nodes.  Call this from the root to delete the whole, or
		// from any other node to delete from there on down.
		// This can only be used if the list has dummy head and tail nodes.
		void DeleteTree(void)
		{
			// Delete left branches
			if (m_pnLeft)
			{
				m_pnLeft->DeleteTree();
				m_pnLeft = nullptr;			
			}	

			// Delete right branches
			if (m_pnRight)
			{
				m_pnRight->DeleteTree();
				m_pnRight = nullptr;
			}

			// Remove yourself from the list
			if (m_pnNext && m_pnPrev)
				Remove();
		}


	public:
		Node*		m_pnNext;
		Node*		m_pnPrev;
		Node*		m_pnLeft;
		Node*		m_pnRight;
		Owner*	m_powner;
		SORTKEY	m_sortkey;
};

// Forward declaration
class CBouy;

typedef CTreeListNode<CBouy, double> TreeListNode;


// CBouy is the class for navigation
class CBouy : public CThing
	{
   friend class CNavigationNet;

	//---------------------------------------------------------------------------
	// Variables
	//---------------------------------------------------------------------------
	public:
		uint8_t	m_ucID;								// Bouy ID (or address)
      std::set<managed_ptr<CBouy>> m_aplDirectLinks;
		TreeListNode m_TreeNode;

	protected:
		double m_dX;								// x coord
		double m_dY;								// y coord
		double m_dZ;								// z coord
      managed_ptr<CNavigationNet> m_pParentNavNet;		// Pointer to its navigation network
													
		RImage* m_pImage;							// Pointer to only image (replace with 3d anim, soon)
		CSprite2 m_sprite;						// Sprite (replace with CSprite3, soon)

		int16_t m_sSuspend;							// Suspend flag

		uint8_t* m_paucRouteTable;				// Routing table (new non-STL way)
      int16_t m_sRouteTableSize;

		// Tracks file counter so we know when to load/save "common" data 
		static int16_t ms_sFileCount;
		static bool  ms_bShowBouys;

		// "Constant" values that we want to be able to tune using the editor

	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
   public:
      CBouy(void)
			{
			m_pImage = 0;
			m_sSuspend = 0;
			m_pParentNavNet = nullptr;
			m_paucRouteTable = nullptr;
         m_sRouteTableSize = 0;
         }

      ~CBouy()
         {
         // Remove sprite from scene (this is safe even if it was already removed!)
         realm()->Scene()->RemoveSprite(&m_sprite);

         if (m_paucRouteTable != nullptr)
            free(m_paucRouteTable);

         // Free resources
         FreeResources();
         }

	//---------------------------------------------------------------------------
	// Required virtual functions (implimenting them as inlines doesn't pay!)
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		int16_t Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			int16_t sFileCount,										// In:  File count (unique per file, never 0)
			uint32_t	ulFileVersion);								// In:  Version of file format to load.

		// Save object (should call base class version!)
		int16_t Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			int16_t sFileCount);									// In:  File count (unique per file, never 0)

		// Startup object
		int16_t Startup(void);										// Returns 0 if successfull, non-zero otherwise

		// Suspend object
		void Suspend(void);

		// Resume object
		void Resume(void);

		// Update object
		void Update(void);

		// Render object
		void Render(void);

#if !defined(EDITOR_REMOVED)
		// Called by editor to init new object at specified position
		int16_t EditNew(												// Returns 0 if successfull, non-zero otherwise
			int16_t sX,												// In:  New x coord
			int16_t sY,												// In:  New y coord
			int16_t sZ);												// In:  New z coord

		// Called by editor to modify object
		int16_t EditModify(void);									// Returns 0 if successfull, non-zero otherwise

		// Called by editor to move object to specified position
		int16_t EditMove(											// Returns 0 if successfull, non-zero otherwise
			int16_t sX,												// In:  New x coord
			int16_t sY,												// In:  New y coord
			int16_t sZ);												// In:  New z coord

		// Called by editor to update object
		void EditUpdate(void);

		// Called by editor to render object
		void EditRender(void);

		// Give Edit a rectangle around this object
		void EditRect(RRect* pRect);

		// Called by editor to get the hotspot of an object in 2D.
		void EditHotSpot(			// Returns nothiing.
			int16_t*	psX,			// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
			int16_t*	psY);			// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.
#endif // !defined(EDITOR_REMOVED)

		// Get the coordinates of this thing.
		virtual					// Overriden here.
      double GetX(void)	const { return m_dX; }

		virtual					// Overriden here.
      double GetY(void)	const { return m_dY; }

		virtual					// Overriden here.
      double GetZ(void)	const { return m_dZ; }

		// Add a link to this bouy - it is directly connected, ie, 1 hop away
      int16_t AddLink(managed_ptr<CBouy> pBouy);

		// Get the next link to follow to get to this destination.  This is
		// normally a routing table lookup unless the entry is not in the routing
		// table, then it is discovered and added as an entry to the routing table.
		uint8_t NextRouteNode(uint8_t dst);

		// Disconnect this node from the network.  This will visit all of its
		// direct links and remove itself from their link list and then free
		// its own link list.  This is called when a Bouy is deleted from the 
		// editor.
		void Unlink(void);

		// Fill in all entries in the routing table.
		int16_t BuildRoutingTable(void);

		// Print the routing table for this bouy - for debugging
		void PrintRouteTable(FILE* fp)
		{
			int16_t i;
			char szLine[256];
			for (i = 0; i < m_sRouteTableSize; i++)
			{
				sprintf(szLine, "%d next %d\n", i, m_paucRouteTable[i]);
				fwrite(szLine, sizeof(char), strlen(szLine), fp);				
			}
		}
		
		// Used by gameedit to know is this bouy is visible or hidden
		bool Visible(void)
		{
			return ((m_sprite.m_sInFlags & CSprite::InHidden) == 0);
		}

		// Toggle the show/hide flag
		static void Show(void)
		{
			ms_bShowBouys = true;
		}

		// Toggle the show/hide flag
		static void Hide(void)
		{
			ms_bShowBouys = false;
		}

	//---------------------------------------------------------------------------
	// Internal functions
	//---------------------------------------------------------------------------
	protected:
		// Get all required resources
		int16_t GetResources(void);						// Returns 0 if successfull, non-zero otherwise
		
		// Free all resources
		int16_t FreeResources(void);						// Returns 0 if successfull, non-zero otherwise
	};


#endif //DOOFUS_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
