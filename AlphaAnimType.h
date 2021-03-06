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
// AlphaAnimType.h
// Project: Postal
//
////////////////////////////////////////////////////////////////////////////////

#ifndef ALPHAANIMTYPE_H
#define ALPHAANIMTYPE_H

#include <BLUE/System.h>
#include <GREEN/Image/Image.h>

// Simple wrapper class for each frame of alpha animation
class CAlphaAnim
	{
	public:
		int16_t m_sNumAlphas;										// Number of alpha images (could be 0!)
		int16_t m_sX;													// Offset from hotspot to upper-left corner of image
		int16_t m_sY;													// Offset from hotspot to upper-left corner of image
		RImage m_imColor;											// "Normal" 8-bit color image
		RImage* m_pimAlphaArray;								// Array of alpha image's (could be empty!)

	public:
      CAlphaAnim(void) noexcept
      {
        m_pimAlphaArray = nullptr;
        m_sNumAlphas = 0;
        m_sX = 0;
        m_sY = 0;
      }

      virtual ~CAlphaAnim(void) noexcept
      {
        if(m_pimAlphaArray != nullptr)
        {
          delete []m_pimAlphaArray;
          m_pimAlphaArray = nullptr;
        }
      }

#ifdef UNUSED_FUNCTIONS
		CAlphaAnim& operator=(const CAlphaAnim& rhs)
			{
			Reset();
			m_sNumAlphas = rhs.m_sNumAlphas;
			m_sX = rhs.m_sX;
			m_sY = rhs.m_sY;
			m_imColor = rhs.m_imColor;
			Alloc(m_sNumAlphas);
         memcpy(m_pimAlphaArray, rhs.m_pimAlphaArray, sizeof(RImage) * m_sNumAlphas); // improper copying
			return *this;
			}
#endif
		bool operator==(const CAlphaAnim& rhs) const
			{
        UNUSED(rhs);
			// Comparing two of these objects is a major undertaking.  Instead,
			// we'll always say that they are different.  This is not a great
			// solution.  In fact, it sucks.  But what the hell...
			return false;
			}


		int16_t Load(RFile* pFile)
			{
			pFile->Read(&m_sNumAlphas);
			pFile->Read(&m_sX);
			pFile->Read(&m_sY);
         m_imColor.Load(pFile);
         if (m_sNumAlphas > 0)
         {
           if(m_pimAlphaArray != nullptr)
             delete[] m_pimAlphaArray;
           m_pimAlphaArray = new RImage[m_sNumAlphas];
           ASSERT(m_pimAlphaArray != nullptr);
         }

			for (int16_t s = 0; s < m_sNumAlphas; s++)
				m_pimAlphaArray[s].Load(pFile);
			return pFile->Error();
			}

		int16_t Save(RFile* pFile)
			{
			pFile->Write(m_sNumAlphas);
			pFile->Write(m_sX);
			pFile->Write(m_sY);
			m_imColor.Save(pFile);
			for (int16_t s = 0; s < m_sNumAlphas; s++)
				m_pimAlphaArray[s].Save(pFile);
			return pFile->Error();
			}
	};

#endif //ALPHAANIMTYPE_H

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
