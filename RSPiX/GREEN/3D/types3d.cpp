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
////////////////////////////////////////////////////////////////////////////////
//
// types3d.cpp
// Project: RSPiX/Green/3d
//
// History:
//		02/01/97 JRD	Started.
//
//		02/05/97 MJR	Filled in.
//
//		02/10/97 MJR	Removed the no-longer-necessary "long" casts for loading
//							and saving of pixel32_t's.
//							Renamed RForm3d to RSop (Sea-Of-Points) since that's what
//							we all think of it as, so it may as well be called that.
//
//		10/06/99	JMI	Added Unmap() and Adjust().
//
// This module impliments the "high level" data types (containers) needed by the
// renderer.
//
////////////////////////////////////////////////////////////////////////////////

#include "types3d.h"


////////////////////////////////////////////////////////////////////////////////
// RTexture Functions
////////////////////////////////////////////////////////////////////////////////

// Allocate specified number of indices and colors
void RTexture::Alloc(int16_t sNum)
	{
	m_sNum = sNum;
	AllocIndices();
	AllocColors();
	}


// Allocate same number of indices as current number of colors
void RTexture::AllocIndices(void)
	{
	FreeIndices();
	m_pIndices = (uint8_t*)calloc(m_sNum, 1);
   ASSERT(m_pIndices != nullptr);
	}


// Allocate same number of colors as current number of indices
void RTexture::AllocColors(void)
	{
	FreeColors();
	m_pColors = (pixel32_t*)calloc(m_sNum, sizeof(pixel32_t));
   ASSERT(m_pColors != nullptr);
	}


// Free indices and colors
void RTexture::Free(void)
	{
	FreeIndices();
	FreeColors();
	Init();
	}


// Free indices only
void RTexture::FreeIndices(void)
	{
	if (m_pIndices)
		{
		free(m_pIndices);
		m_pIndices = 0;
		}
	}


// Free colors only
void RTexture::FreeColors(void)
	{
	if (m_pColors)
		{
		free(m_pColors);
		m_pColors = 0;
		}
	}


int16_t RTexture::Load(RFile* fp)
	{
	Free();

	int16_t sResult = SUCCESS;
	if (fp->Read(&m_sNum) == 1)
		{
		int16_t sFlags;
		if (fp->Read(&sFlags) == 1)
			{
			if (sFlags & HasIndices)
				{
				AllocIndices();
				fp->Read(m_pIndices, m_sNum);
				}
			if (sFlags & HasColors)
				{
				AllocColors();
				fp->Read(m_pColors, m_sNum);
				}
			}
		}

	if (fp->Error())
		{
		sResult = FAILURE;
		TRACE("RTexture::Load(): Error reading from file!\n");
		}
	return sResult;
	}


int16_t RTexture::Save(RFile* fp)
	{
	int16_t sResult = SUCCESS;

	fp->Write(&m_sNum);

	int16_t sFlags = 0;
	if (m_pIndices)
		sFlags |= HasIndices;
	if (m_pColors)
		sFlags |= HasColors;
	fp->Write(&sFlags);

	if (m_pIndices)
		fp->Write(m_pIndices, m_sNum);
	if (m_pColors)
		fp->Write(m_pColors, m_sNum);

	if (fp->Error())
		{
		sResult = FAILURE;
		TRACE("RTexture::Save(): Error writing to file!\n");
		}
	return sResult;
	}


// Map colors onto the specified palette.  For each color, the best
// matching color is found in the  palette, and the associated palette
// index is written to the array of indices.  If the array of indices
// doesn't exist, it will be created.
void RTexture::Remap(
   palindex_t sStartIndex,
   palindex_t sNumIndex,
   channel_t* pr,
   channel_t* pg,
   channel_t* pb,
   uint32_t linc)
	{
	ASSERT(m_pColors);
		
	if (m_pIndices == 0)
		AllocIndices();

	for (int16_t i = 0; i < m_sNum; i++)
		{
		m_pIndices[i] = rspMatchColorRGB(
			int32_t(m_pColors[i].red),
			int32_t(m_pColors[i].green),
			int32_t(m_pColors[i].blue),
			sStartIndex,sNumIndex,
			pr,pg,pb,linc);
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Unmap colors from the specified palette and put them into the colors
// array.  If the array of colors doesn't exist, it will be created.
////////////////////////////////////////////////////////////////////////////////
void 
RTexture::Unmap(
   channel_t* pr,
   channel_t* pg,
   channel_t* pb,
   uint32_t lInc)
	{
  UNUSED(lInc);
	ASSERT(m_pIndices);
		
	if (m_pColors == 0)
		AllocColors();

	uint8_t*			pu8	= m_pIndices;
	pixel32_t*	ppix	= m_pColors;
	int16_t	sCount		= m_sNum;
	while (sCount--)
		{
		ppix->red		= pr[*pu8];
		ppix->green	= pg[*pu8];
		ppix->blue	= pb[*pu8];

		ppix++;
		pu8++;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Muddy or brighten or darken.  Applies the specified brightness value
// to every nth color (where n == lInc).
////////////////////////////////////////////////////////////////////////////////
void
RTexture::Adjust(
	float fAdjustment,	// In:  Adjustment factor (1.0 == same, < 1 == dimmer, > 1 == brighter).
   uint32_t lInc)				// In:  Number of colors to skip.
	{
	ASSERT(m_pColors);
	ASSERT(fAdjustment >= 0.0f);

#define CLAMP8BIT(u8Color, fColor)	u8Color = fColor < 0xFF ? uint8_t(fColor + 0.5f) : 0xFF

	pixel32_t*	ppix	= m_pColors;
	int16_t	sCount		= m_sNum / lInc;
	float	fColor;
	while (sCount--)
		{
      fColor = ppix->red * fAdjustment;
      CLAMP8BIT(ppix->red, fColor);

      fColor = ppix->green * fAdjustment;
      CLAMP8BIT(ppix->green, fColor);

      fColor = ppix->blue * fAdjustment;
      CLAMP8BIT(ppix->blue, fColor);

		ppix += lInc;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// RMesh Functions
////////////////////////////////////////////////////////////////////////////////
void RMesh::Alloc(int16_t sNum)
	{
	Free();
	m_sNum = sNum;
	m_pArray = (uint16_t*)calloc((int32_t)m_sNum * 3, sizeof(uint16_t));
   ASSERT(m_pArray != nullptr);
	}


void RMesh::Free(void)
	{
	if (m_pArray)
		free(m_pArray);
	Init();
	}


int16_t RMesh::Load(RFile* fp)
	{
	Free();

	int16_t sResult = SUCCESS;
	if (fp->Read(&m_sNum) == 1)
		{
		Alloc(m_sNum);
		fp->Read(m_pArray, (int32_t)m_sNum * 3);
		}
	if (fp->Error())
		{
		sResult = FAILURE;
		TRACE("RMesh::Load(): Error reading from file!\n");
		}
	return sResult;
	}


int16_t RMesh::Save(RFile* fp)
	{
	int16_t sResult = SUCCESS;
	fp->Write(&m_sNum);
   fp->Write(m_pArray, int32_t(m_sNum) * 3);
	if (fp->Error())
		{
		sResult = FAILURE;
		TRACE("RMesh::Save(): Error writing to file!\n");
		}
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// RSop Functions
////////////////////////////////////////////////////////////////////////////////
void RSop::Alloc(size_t lNum)
	{
	Free();
	m_lNum = lNum;
	m_pArray = (RP3d*)calloc(m_lNum, sizeof(RP3d));
   ASSERT(m_pArray != nullptr);
	}


void RSop::Free(void)
	{
	if (m_pArray)
		free(m_pArray);
	Init();
	}


int16_t RSop::Load(RFile* fp)
	{
	Free();

	int16_t sResult = SUCCESS;
   uint32_t val;
   if (fp->Read(&val) == 1)
		{
     m_lNum = val;
		Alloc(m_lNum);
      ASSERT(sizeof(RP3d) == sizeof(real_t) * 4);
      fp->Read((real_t*)m_pArray, m_lNum * 4);
		}
	if (fp->Error())
		{
		sResult = FAILURE;
		TRACE("RSop::Load(): Error reading from file!\n");
		}
	return sResult;
	}


int16_t RSop::Save(RFile* fp)
	{
	int16_t sResult = SUCCESS;
	fp->Write(&m_lNum);
   ASSERT(sizeof(RP3d) == (sizeof(real_t) * 4));
   fp->Write((real_t*)m_pArray, m_lNum * 4);
	if (fp->Error())
		{
		sResult = FAILURE;
		TRACE("RSop::Save(): Error writing to file!\n");
		}
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
