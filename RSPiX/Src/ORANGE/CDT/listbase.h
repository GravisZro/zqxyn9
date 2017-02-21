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
//////////////////////////////////////////////////////////////////////////////
//
// listbase.h
// 
// History:
//		07/09/96 JMI	Started.  Copied from list.h.  Making version of CBList
//							that basically supports types that aren't pointers.  This
//							requires a different calling interface.  CList will be
//							based on this.
//
//		10/09/96	JMI	CBList() constructor now has option to skip initialization
//							and automatic dealloaction.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CBList			RBList
//
//		01/05/97	JMI	Added standard forms of GetLogicalNext()/Prev().
//
//		03/31/97	JMI	Remove() had default parameter that was inappropriate for
//							other than pointer types.  Changed it to overload with
//							no parameters.
//
//////////////////////////////////////////////////////////////////////////////
//
// This module provides dynamic linked list services.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef H_LISTBASE
#define H_LISTBASE

#include <BLUE/System.h>

template <class T> class RBList
	{
	protected:	// Internal types.
		typedef T LISTDATA, *PLISTDATA; 

		typedef struct	tagNODE
			{
			LISTDATA	ldData;
			tagNODE*	pNext;
			tagNODE*	pPrev;
			} NODE, *PNODE, *NODELIST;

	public:
		// Add a node to the list at the tail (it will be the new tail)
		int16_t AddTail(LISTDATA ldNew)
			{
			int16_t sResult = SUCCESS; // Assume success.
			// Create new node.
			PNODE pnNew = new NODE;
			// If successful . . .
			if (pnNew != nullptr)
				{
				// Point to provided data.
				pnNew->ldData = ldNew;
				pnNew->pNext = nullptr;

				// If a tail exists . . .
				if (m_pnTail != nullptr)
					{
					// Make the tail's next point to the new.
					m_pnTail->pNext = pnNew;
					// Make the new's previous point to the tail.
					pnNew->pPrev    = m_pnTail;
					}
				else
					{
					// Make the new's previous nullptr.
					pnNew->pPrev	= nullptr;
					// Make the head the tail, since, if no tail exists,
					// there is no head.
					m_pnHead			= pnNew;
					}                 
	
				// Update current node info.
				m_pnPrev		= pnNew->pPrev;
				m_pnCurrent = pnNew;
				m_pnNext		= pnNew->pNext;
				m_pnTail 	= pnNew;
				}
			else
				{
				TRACE("RBList::AddTail(): Unable to allocate new node.\n");
				sResult = FAILURE;
				}

			return sResult;
			}
		
		// Insert a node at the Head (it will be the new head)
		int16_t InsertHead(LISTDATA ldNew)
			{
			int16_t sResult = SUCCESS; // Assume success.
			// Allocate new node.
			PNODE pnNew = new NODE;
			// If successful . . .
			if (pnNew != nullptr)
				{
				// Point to data provided.
				pnNew->ldData = ldNew;
				pnNew->pPrev = nullptr;
				// If there is currently a head node . . .
				if (m_pnHead != nullptr)
					{
					// Point the head's previous to the new.
					m_pnHead->pPrev = pnNew;
					// Point the new's next to the head.
					pnNew->pNext    = m_pnHead;
					}
				else                     
					{
					// Make the new's next nullptr.
					pnNew->pNext    = nullptr;
					// Make the tail the new since, if there is no head,
					// there must not be a tail.
					m_pnTail        = pnNew;
					}

				// Update current node info.
				m_pnHead		= pnNew;
				m_pnPrev		= pnNew->pPrev;
				m_pnCurrent = pnNew;
				m_pnNext		= pnNew->pNext;
				}
			else
				{
				TRACE("RBList::InsertHead(): Unable to allocate new node.\n");
				sResult = FAILURE;
				}

			return sResult;
			}


		// Insert ldNew after ldAfter
		int16_t InsertAfter(LISTDATA ldAfter, LISTDATA ldNew)
			{
			int16_t sResult = SUCCESS; // Assume success.
			// Make sure the list is not empty
			if (IsEmpty() == FALSE)
				{
				// Find the node to insert after.
				PNODE pnAfter = Find(ldAfter);
				// If such a node exists . . .
				if (pnAfter != nullptr)
					{
					// Allocate new node.
					PNODE pnNew  = new NODE;
					// If successful . . .
					if (pnNew != nullptr)
						{
						// Point to provided data.
						pnNew->ldData = ldNew;
						// Point next to node to insert after's next.
						pnNew->pNext = pnAfter->pNext;
						// If there is a next node . . .
						if (pnNew->pNext != nullptr)
							{
							// Have the next's previous point to new.
							pnNew->pNext->pPrev = pnNew;
							}
						else
							{
							// Make new the tail.
							m_pnTail = pnNew;
							}

						// Have the new's previous point the one to insert after.
						pnNew->pPrev = pnAfter;
						// Have the node to insert after's next point to the new.
						pnAfter->pNext = pnNew;
						// Update current info.
						m_pnPrev		= pnNew->pPrev;
						m_pnCurrent = pnNew;       
						m_pnNext		= pnNew->pNext;
						}
					else
						{
						TRACE("RBList::InsertAfter(): Unable to allocate new node.\n");
						sResult = FAILURE * 3;
						}
					}
				else
					{
					TRACE("RBList::InsertAfter():  Unable to locate node to insert after.\n");
					sResult = FAILURE * 2;
					}
				}
			else
				{
				TRACE("RBList::InsertAfter():  The list is empty.\n");
				sResult = FAILURE;
				}

			return sResult;
			}

		// Insert pnNew before lnBefore
		int16_t InsertBefore(LISTDATA ldBefore, LISTDATA ldNew)
			{
			int16_t sResult = SUCCESS; // Assume success.
			// Make sure the list is not empty
			if (IsEmpty() == FALSE)
				{
				// Find the node to insert before.
				PNODE pnBefore = Find(ldBefore);
				// If found a node . . .
				if (pnBefore != nullptr)
					{
					// Allocate new node.
					PNODE pnNew = new NODE;
					// If successful . . .
					if (pnNew != nullptr)
						{
						// Point to supplied data.
						pnNew->ldData = ldNew;
						// Point previous to node to insert before's previous.
						pnNew->pPrev = pnBefore->pPrev;
						// If there is a previous . . .
						if (pnNew->pPrev != nullptr)
							{
							// Have previous' next point to new.
							pnNew->pPrev->pNext = pnNew;
							}
						else
							{
							// Make new the head.
							m_pnHead = pnNew;
							}
						// Point new's next to node to insert before.
						pnNew->pNext = pnBefore;
						// Point before's previous to new.
						pnBefore->pPrev = pnNew;
				
						// Update current info.
						m_pnPrev		= pnNew->pPrev;
						m_pnCurrent = pnNew;       
						m_pnNext		= pnNew->pNext;
						}
					else
						{
						sResult = FAILURE * 3;
						TRACE("RBList::InsertBefore():  Unable to allocate new node.\n");
						}
					}
				else
					{
					sResult = FAILURE * 2;
					TRACE("RBList::InsertBefore():  Unable to find node to insert before.\n");
					}
				}
			else
				{
				sResult = FAILURE;
				TRACE("RBList::InsertBefore():  List is empty.\n");
				}

			return sResult;
			}

		// Add pnNew after current
		int16_t Add(LISTDATA ldNew)
		{ return (m_pnCurrent ? InsertAfter(m_pnCurrent->ldData, ldNew) : 
				(m_pnNext ? InsertAfter(m_pnNext->ldData, ldNew) : AddTail(ldNew) ) ); }

		// Insert pnNew before current
		int16_t Insert(LISTDATA ldNew)
		{ return (m_pnCurrent ? InsertBefore(m_pnCurrent->ldData, ldNew) : 
				(m_pnNext ? InsertBefore(m_pnNext->ldData, ldNew) : AddTail(ldNew) ) ); }

		// Remove current node from the list.
		int16_t Remove(void)
			{
			return Remove(m_pnCurrent);
			}

		// Remove a node from the list.
		int16_t Remove(LISTDATA ldRem)
			{
			return Remove(Find(ldRem));
			}

		int16_t GetHead(			// Returns 0 on success.
			PLISTDATA pldHead)	// Where to store head data.
			{
			int16_t sResult = SUCCESS;	// Assume success.

			m_pnCurrent = m_pnHead;
			// If there is a head . . .
			if (m_pnCurrent != nullptr)
				{
				m_pnPrev = m_pnCurrent->pPrev; 
				m_pnNext = m_pnCurrent->pNext;

				// Store.
				*pldHead	= m_pnCurrent->ldData;
				}
			else
				{
				sResult = 1;
				}

			return sResult;
			}

		int16_t GetTail(			// Returns 0 on success.
			PLISTDATA pldTail)	// Where to store tail data.
			{
			int16_t sResult = SUCCESS;	// Assume success.

			// If there is a tail . . .
			m_pnCurrent = m_pnTail;
			if (m_pnCurrent != nullptr)
				{
				m_pnPrev = m_pnCurrent->pPrev; 
				m_pnNext = m_pnCurrent->pNext;

				*pldTail	= m_pnCurrent->ldData;
				}
			else
				{
				sResult = 1;
				}

			return sResult;
			}

		// Get node following last GetX
		int16_t GetNext(			// Returns 0 on success.
			PLISTDATA pldNext)	// Where to store next data.
			{
			int16_t sResult = SUCCESS;	// Assume success.

			m_pnCurrent = m_pnNext;
			if (m_pnCurrent != nullptr)
				{
				m_pnPrev = m_pnCurrent->pPrev; 
				m_pnNext = m_pnCurrent->pNext;

				*pldNext	= m_pnCurrent->ldData;
				}
			else
				{
				sResult = 1;
				}

			return sResult;
			}

		// Get node following ldData
		int16_t GetNext(			// Returns 0 on success.
			LISTDATA ldData, 	// Node to get next of.
			PLISTDATA pldNext)	// Where to store next data.
			{
			int16_t sResult = SUCCESS;	// Assume success.

			// Make sure the list is not empty
			if (IsEmpty() == FALSE)
				{
				// Attempt to find node.
				PNODE pn = Find(ldData);
				// If node found . . .
				if (pn != nullptr)
					{
					// Make global previous node found.
					m_pnPrev		= pn;
					// Make global current node's next.
					m_pnCurrent = pn->pNext;
					// If new current exists . . .
					if (m_pnCurrent != nullptr)
						{
						// Make global next current's next.
						m_pnNext	= m_pnCurrent->pNext;       
		      		// Return current's data.
						*pldNext = m_pnCurrent->ldData;
						}
					else
						{
						// There is no next to supplied node.
						sResult = 1;
						}
					}
				else
					{
					TRACE("RBList::GetNext(): Unable to find supplied node.\n");
					sResult = FAILURE * 2;
					}
				}
			else
				{
				TRACE("RBList::GetNext():  The list is empty.\n");
				sResult = FAILURE;
				}

			return sResult;
			}

		// Get node logically following last GetX
		int16_t GetLogicalNext(	// Returns 0 on success.
			PLISTDATA pldNext)	// Where to store next data.
			{
			int16_t sResult	= GetNext(pldNext);
			if (sResult != SUCCESS)
				{
				sResult	= GetHead(pldNext);
				}

			return sResult;
			}

		// Get node logically following ldData
		int16_t GetLogicalNext(	// Returns 0 on success.
			LISTDATA ldData,	 	// Node to get next of.
			PLISTDATA pldNext)	// Where to store next data.
			{
			int16_t sResult	= GetNext(ldData, pldNext);
			if (sResult != SUCCESS)
				{
				sResult	= GetHead(pldNext);
				}

			return sResult;
			}

		// Get node preceding last GetX
		int16_t GetPrev(			// Returns 0 on success.
			PLISTDATA pldPrev)	// Where to store previous data.
			{
			int16_t sResult = SUCCESS;	// Assume success.

			m_pnCurrent = m_pnPrev;
			if (m_pnCurrent != nullptr)
				{
				m_pnPrev = m_pnCurrent->pPrev; 
				m_pnNext = m_pnCurrent->pNext;

				*pldPrev	= m_pnCurrent->ldData;
				}
			else
				{
				sResult = 1;
				}

			return sResult;
			}

		// Get node preceding ldData
		int16_t GetPrev(			// Returns 0 on success.
			LISTDATA ldData,	// Node to get previous of.
			PLISTDATA pldPrev)	// Where to store previous data.
			{
			int16_t sResult = SUCCESS;	// Assume success.

			// Make sure the list is not empty.
			if (IsEmpty() == FALSE)
				{
				// Attempt to find node.
				PNODE pn = Find(ldData);
				// If node found . . .
				if (pn != nullptr)
					{
					// Make global next found node.
					m_pnNext		= pn;
					// Make global current node's previous.
					m_pnCurrent = pn->pPrev;
					// If new current exists . . .
					if (m_pnCurrent)
						{
						// Make global previous current's previous.
						m_pnPrev	= m_pnCurrent->pPrev;       
		      		// Return current's data.
						*pldPrev	= m_pnCurrent->ldData;
						}
					else
						{
						// There is no previous to supplied node.
						sResult = FAILURE;
						}
					}
				else
					{
					TRACE("RBList::GetPrev():  Unable to find supplied node.\n");
					sResult = FAILURE;
					}
				}
			else
				{
				TRACE("RBList::GetPrev():  The list is empty.\n");
				sResult = FAILURE * 2;
				}

			return sResult;
			}


		// Get node logically preceding last GetX
		int16_t GetLogicalPrev(	// Returns 0 on success.
			PLISTDATA pldPrev)	// Where to store prev data.
			{
			int16_t sResult	= GetPrev(pldPrev);
			if (sResult != SUCCESS)
				{
				sResult	= GetTail(pldPrev);
				}

			return sResult;
			}

		// Get node logically preceding ldData
		int16_t GetLogicalPrev(	// Returns 0 on success.
			LISTDATA ldData,	 	// Node to get prev of.
			PLISTDATA pldPrev)	// Where to store prev data.
			{
			int16_t sResult	= GetPrev(ldData, pldPrev);
			if (sResult != SUCCESS)
				{
				sResult	= GetTail(pldPrev);
				}

			return sResult;
			}

		int16_t GetCurrent(		// Returns 0 on success.
			PLISTDATA	pldCur)	// Where to store current data.
			{
			int16_t sResult = SUCCESS;	// Assume success.

			if (m_pnCurrent != nullptr)
				{
				*pldCur	= m_pnCurrent->ldData;
				}
			else
				{
				sResult = 1;
				}

			return sResult; 
			}

		int16_t IsEmpty(void)	// Returns TRUE if empty, FALSE otherwise.
			{ 
			return (m_pnHead == nullptr) ? TRUE : FALSE; 
			}

		// Find the node with the value LISTDATA.
		PNODE Find(LISTDATA ldFind)
			{
			PNODE pn, pnTemp;
	
			// Start at head.
			pn = m_pnHead;
			while (pn && pn->ldData != ldFind)
				{
				pnTemp = pn;
				pn = pn->pNext;
				}                             

			return pn;               
			}

		void Reset(void)
			{ Free(); }

	public:
		RBList(
			int16_t sInitialize	= TRUE)	// If this flag is FALSE, no initialization
												// or freeing will be done.  It will be the
												// user's responsibility!
			{
			if (sInitialize != FALSE)
				{
				m_pnHead    = nullptr;  
				m_pnPrev		= nullptr;
				m_pnCurrent = nullptr;
				m_pnNext		= nullptr;
				m_pnTail    = nullptr;
				}

			m_sInitialize	= sInitialize;
			}

		~RBList()
			{
			if (m_sInitialize != FALSE)
				{
				// Free all nodes.
				Free();
				}
			}

	protected:

		// Removes supplied node.
		int16_t Remove(			// Returns 0 on success.
			NODE*	pn)			// In:  Node to remove.
			{
			int16_t sResult = SUCCESS;

			// Make sure the list is not empty
			if (IsEmpty() == FALSE)
				{
				// If we have a valid node to remove . . .
				if (pn != nullptr)
					{
					// If there is a node previous to pn . . .
					if (pn->pPrev != nullptr)
						{
						// Make pn's previous' next point to pn's next.
						pn->pPrev->pNext = pn->pNext;
						}
					else
						{
						// pn is the head...make pn's next the head.
						m_pnHead = pn->pNext;
						}
			
					// If there is a node after pn . . .
					if (pn->pNext != nullptr)
						{
						// Make pn's next's previous point to pn's previous.
						pn->pNext->pPrev = pn->pPrev;
						}
					else
						{
						// pn is the tail...make pn's previous the tail.
						m_pnTail = pn->pPrev;
						}
			
					// Update current info.
					m_pnPrev		= pn->pPrev;
					m_pnCurrent = nullptr;     
					m_pnNext		= pn->pNext;
			
					delete pn;
					}
				else
					{
					sResult = FAILURE * 2;
					TRACE("RBList::Remove():  Unable to find supplied node or no current node.\n");
					}
				}
			else
				{
				sResult = FAILURE;
				TRACE("RBList::Remove():  The list is empty.\n");
				}

			return sResult;
			}

		// Free the entire list.
		void Free(void)
			{
			PNODE pn, pnTemp;

			// Start at the head.
			pn = m_pnHead;
			while (pn)
				{
				pnTemp = pn;
				pn = pn->pNext;

				delete pnTemp;  
				}                                            
	
			// Clear all node pointers.
			m_pnHead    = nullptr;  
			m_pnPrev		= nullptr;
			m_pnCurrent = nullptr;
			m_pnNext		= nullptr;
			m_pnTail    = nullptr;
			}

		PNODE		m_pnHead;
		PNODE		m_pnPrev;
		PNODE		m_pnCurrent;
		PNODE		m_pnNext;
		PNODE		m_pnTail;
		int16_t		m_sInitialize;		// TRUE if this item should handle intializing
											// and freeing the list and members.
	};
   
#endif // H_LISTBASE
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
