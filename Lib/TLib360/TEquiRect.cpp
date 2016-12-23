/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2015, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TEquiRect.cpp
    \brief    EquiRect class
*/

#include <assert.h>
#include <math.h>
#include "TEquiRect.h"

#if SVIDEO_EXT

/********************************************
Equirectangular geometry related functions;
*********************************************/
TEquiRect::TEquiRect(SVideoInfo& sVideoInfo, InputGeoParam *pInGeoParam) : TGeometry()
{
#if SVIDEO_CPPPSNR
   assert(sVideoInfo.geoType == SVIDEO_EQUIRECT || sVideoInfo.geoType == SVIDEO_EQUALAREA || sVideoInfo.geoType == SVIDEO_CRASTERSPARABOLIC);
#else
   assert(sVideoInfo.geoType == SVIDEO_EQUIRECT || sVideoInfo.geoType == SVIDEO_EQUALAREA);
#endif
   //assert(sVideoInfo.iNumFaces == 1);
   geoInit(sVideoInfo, pInGeoParam);
}

TEquiRect::~TEquiRect()  
{
}

/**************************************
    -180                         180
90                                   0
                                     |
                                     v
                                     |
-90                                  1
    0 ----------u-----------------1
***************************************/
Void TEquiRect::map2DTo3D(SPos& IPosIn, SPos *pSPosOut)
{ 
  POSType u, v;
  //u = IPosIn.x;
  u = IPosIn.x + (POSType)(0.5);
  v = IPosIn.y + (POSType)(0.5);

  if ((u < 0 || u >= m_sVideoInfo.iFaceWidth) && ( v >= 0 && v < m_sVideoInfo.iFaceHeight)) 
  {
    u = u < 0 ? m_sVideoInfo.iFaceWidth+u : (u - m_sVideoInfo.iFaceWidth);
  }
  else if (v < 0)
  {
    v = -v; 
    u = u + (m_sVideoInfo.iFaceWidth>>1);
    u = u >= m_sVideoInfo.iFaceWidth ? u - m_sVideoInfo.iFaceWidth : u;
  }
  else if(v >= m_sVideoInfo.iFaceHeight)
  {
    v = (m_sVideoInfo.iFaceHeight<<1)-v; 
    u = u + (m_sVideoInfo.iFaceWidth>>1);
    u = u >= m_sVideoInfo.iFaceWidth ? u - m_sVideoInfo.iFaceWidth : u;
  }

  pSPosOut->faceIdx =IPosIn.faceIdx;
  POSType yaw = (POSType)(u*S_PI*2/m_sVideoInfo.iFaceWidth - S_PI);
  POSType pitch = (POSType)(S_PI_2 - v*S_PI/m_sVideoInfo.iFaceHeight);

  pSPosOut->x = (POSType)(scos(pitch)*scos(yaw));  
  pSPosOut->y = (POSType)(ssin(pitch));  
  pSPosOut->z = -(POSType)(scos(pitch)*ssin(yaw));  
}

//The output is within [0.0, width)*[0.0, height) in sampling grid;
Void TEquiRect::map3DTo2D(SPos *pSPosIn, SPos *pSPosOut)
{
  POSType x = pSPosIn->x;
  POSType y = pSPosIn->y;
  POSType z = pSPosIn->z;

  pSPosOut->faceIdx = 0;
  pSPosOut->z = 0;
  //yaw;
  pSPosOut->x = (POSType)((S_PI-satan2(z, x))*m_sVideoInfo.iFaceWidth/(2*S_PI));
  pSPosOut->x -= 0.5;

  POSType len = ssqrt(x*x + y*y + z*z);
  //pitch;
  pSPosOut->y = (POSType)((len < S_EPS? 0.5 : sacos(y/len)/S_PI)*m_sVideoInfo.iFaceHeight);
  pSPosOut->y -= 0.5;
}

Void TEquiRect::convertYuv(TComPicYuv *pSrcYuv)//valuate m_pFacesOrig 
{
  Int nWidth = m_sVideoInfo.iFaceWidth;
  Int nHeight = m_sVideoInfo.iFaceHeight;

  assert(pSrcYuv->getWidth(ComponentID(0)) == nWidth && pSrcYuv->getHeight(ComponentID(0)) == nHeight);
  assert(pSrcYuv->getNumberValidComponents() == getNumChannels());

  if(pSrcYuv->getChromaFormat()==CHROMA_420)
  {
    //memory allocation;
    Int nMarginSizeTmpBuf = std::max(std::max(m_filterUps[0].nTaps, m_filterUps[1].nTaps), std::max(m_filterUps[2].nTaps, m_filterUps[3].nTaps))>>1 ; //2, depends on the chroma upsampling filter size;  
    assert(nMarginSizeTmpBuf <= std::min(pSrcYuv->getMarginX((ComponentID)1), pSrcYuv->getMarginY((ComponentID)1)));

    for(Int ch=0; ch<pSrcYuv->getNumberValidComponents(); ch++)
    {
      ComponentID chId = ComponentID(ch);
      Int iStrideTmpBuf = pSrcYuv->getStride(chId);
      nWidth = m_sVideoInfo.iFaceWidth >>pSrcYuv->getComponentScaleX(chId);
      nHeight = m_sVideoInfo.iFaceHeight >>pSrcYuv->getComponentScaleY(chId);

      //fill;
      Pel *pSrc = pSrcYuv->getAddr(chId);
      Pel *pDst;
      if(!ch || (m_chromaFormatIDC==CHROMA_420 && !m_bResampleChroma))
      {
        pDst = m_pFacesOrig[0][ch];
        for(Int j=0; j<nHeight; j++)
        {
          memcpy(pDst, pSrc, nWidth*sizeof(Pel));
          pDst +=  getStride(chId);
          pSrc += pSrcYuv->getStride(chId);
        }
        continue;
      }

      //padding;
      //left and right; 
      pSrc = pSrcYuv->getAddr(chId);
      pDst = pSrc + nWidth;
      for(Int i=0; i<nHeight; i++)
      {
        sPadH(pSrc, pDst, nMarginSizeTmpBuf);
        pSrc += iStrideTmpBuf;
        pDst += iStrideTmpBuf;
      }
      //top;
      pSrc = pSrcYuv->getAddr(chId) - nMarginSizeTmpBuf;
      pDst = pSrc + nWidth/2;
      for(Int i=-nMarginSizeTmpBuf; i<nWidth/2+nMarginSizeTmpBuf; i++)
      {
        sPadV(pSrc, pDst, iStrideTmpBuf, nMarginSizeTmpBuf);
        pSrc ++;
        pDst ++;
      }
      //bottom;
      pSrc = pSrcYuv->getAddr(chId) + (nHeight-1)*iStrideTmpBuf-nMarginSizeTmpBuf;
      pDst = pSrc + nWidth/2;
      for(Int i=-nMarginSizeTmpBuf; i<nWidth/2+nMarginSizeTmpBuf; i++)
      {
        sPadV(pSrc, pDst, -iStrideTmpBuf, nMarginSizeTmpBuf);
        pSrc ++;
        pDst ++;
      }
      if(m_chromaFormatIDC == CHROMA_444)
      {
        //420->444;
        chromaUpsample(pSrcYuv->getAddr(chId), nWidth, nHeight, iStrideTmpBuf, 0, chId);
      }
      else
      {
        chromaResampleType0toType2(pSrcYuv->getAddr(chId), nWidth, nHeight, iStrideTmpBuf, m_pFacesOrig[0][ch], getStride(chId));
      }
    }
  }
  else if(pSrcYuv->getChromaFormat()==CHROMA_444 || pSrcYuv->getChromaFormat()==CHROMA_400)
  {
    if(m_chromaFormatIDC == pSrcYuv->getChromaFormat())
    {
      //copy;      
      for(Int ch=0; ch<pSrcYuv->getNumberValidComponents(); ch++)
      {
        ComponentID chId = ComponentID(ch);
        Pel *pSrc = pSrcYuv->getAddr(chId); 
        Pel *pDst = m_pFacesOrig[0][ch];
        for(Int j=0; j<nHeight; j++)
        {
          memcpy(pDst, pSrc, nWidth*sizeof(Pel));
          pDst +=  getStride(chId);
          pSrc += pSrcYuv->getStride(chId);
        }
      }
    }
    else
      assert(!"Not supported yet");
  }
  else
    assert(!"Not supported yet");
 
  //set padding flag;
  setPaddingFlag(false);
}

Void TEquiRect::sPadH(Pel *pSrc, Pel *pDst, Int iCount)
{
  for(Int i=1; i<=iCount; i++)
  {
    pDst[i-1] = pSrc[i-1];
    pSrc[-i] = pDst[-i];  
  }
}

Void TEquiRect::sPadV(Pel *pSrc, Pel *pDst, Int iStride, Int iCount)
{
  for(Int i=1; i<=iCount; i++)
  {
    pSrc[-i*iStride] = pDst[(i-1)*iStride];
    pDst[-i*iStride] = pSrc[(i-1)*iStride];
  }
}

Void TEquiRect::spherePadding(Bool bEnforced)
{
  if(!bEnforced && m_bPadded)
  {
    return;
  }
  m_bPadded = false;

#if SVIDEO_DEBUG
  //dump to file;
  static Bool bFirstDumpBeforePading = true;
  dumpAllFacesToFile("equirect_before_padding", false, !bFirstDumpBeforePading); 
  bFirstDumpBeforePading = false;
#endif

  for(Int ch=0; ch<(getNumChannels()); ch++)
  {
    ComponentID chId = (ComponentID)ch;
    Int nWidth = m_sVideoInfo.iFaceWidth >> getComponentScaleX(chId);
    Int nHeight = m_sVideoInfo.iFaceHeight >> getComponentScaleY(chId);
    Int nMarginX = m_iMarginX >> getComponentScaleX(chId);
    Int nMarginY = m_iMarginY >> getComponentScaleY(chId);

    //left and right;
    Pel *pSrc = m_pFacesOrig[0][ch];
    Pel *pDst = pSrc + nWidth;
    for(Int i=0; i<nHeight; i++)
    {
      sPadH(pSrc, pDst, nMarginX);
      pSrc += getStride(ComponentID(ch));
      pDst += getStride(ComponentID(ch));
    }
    //top;
    pSrc = m_pFacesOrig[0][ch] - nMarginX;
    pDst = pSrc + (nWidth>>1);
    for(Int i=-nMarginX; i<((nWidth>>1)+nMarginX); i++)  //only top and bottom padding is necessary for the first stage vertical upsampling;
    {
      sPadV(pSrc, pDst, getStride(ComponentID(ch)), nMarginY);
      pSrc ++;
      pDst ++;
    }
    //bottom;
    pSrc = m_pFacesOrig[0][ch] + (nHeight-1)*getStride(ComponentID(ch)) - nMarginX;
    pDst = pSrc + (nWidth>>1);
    for(Int i=-nMarginX; i<((nWidth>>1)+nMarginX); i++) //only top and bottom padding is necessary for the first stage vertical upsampling;
    {
      sPadV(pSrc, pDst, -getStride(ComponentID(ch)), nMarginY);
      pSrc ++;
      pDst ++;
    }
  }
  m_bPadded = true;

#if SVIDEO_DEBUG
  //dump to file;
  static Bool bFirstDumpAfterPading = true;
  dumpAllFacesToFile("equirect_after_padding", true, !bFirstDumpAfterPading); 
  bFirstDumpAfterPading = false;
#endif

}

Void TEquiRect::framePack(TComPicYuv *pDstYuv)
{
  if(pDstYuv->getChromaFormat()==CHROMA_420)
  {
    if( (m_chromaFormatIDC == CHROMA_444) || (m_chromaFormatIDC == CHROMA_420 && m_bResampleChroma) )
       spherePadding();

    assert(m_sVideoInfo.framePackStruct.chromaFormatIDC == CHROMA_420);
    if(m_chromaFormatIDC == CHROMA_444)
    {
      //1: 444->420;  444->422, H:[1, 6, 1]; 422->420, V[1,1];
      //(Wc*2Hc) and (Wc*Hc) temporal buffer; the resulting buffer is for rotation;
      Int nWidthC = m_sVideoInfo.iFaceWidth>>1;
      Int nHeightC = m_sVideoInfo.iFaceHeight>>1;  
      Int nMarginSize = (m_filterDs[1].nTaps-1)>>1; //0, depending on V filter and considering south pole;
      Int nHeightC422 = m_sVideoInfo.iFaceHeight + nMarginSize*2;
      Int iStride422 = nWidthC;
      Int iStride420 = nWidthC;
      if(!m_pDS422Buf)
      {
        m_pDS422Buf = (Pel*)xMalloc(Pel, nHeightC422*iStride422);
      }
      if(!m_pDS420Buf)
      {
        m_pDS420Buf = (Pel*)xMalloc(Pel, nHeightC*iStride420);
      }
      //1: 444->422;
      for(Int ch=1; ch<getNumChannels(); ch++)
      {
        chromaDonwsampleH(m_pFacesOrig[0][ch]-nMarginSize*getStride((ComponentID)ch), m_sVideoInfo.iFaceWidth, nHeightC422, getStride((ComponentID)ch), 1, m_pDS422Buf, iStride422);
        chromaDonwsampleV(m_pDS422Buf + nMarginSize*iStride422, nWidthC, m_sVideoInfo.iFaceHeight, iStride422, 1, m_pDS420Buf, iStride420);
        rotOneFaceChannel(m_pDS420Buf, nWidthC, nHeightC, iStride420, 1, ch, m_sVideoInfo.framePackStruct.faces[0][0].rot, pDstYuv, 0, 0, 0, 0);
      }
      //luma;
      rotOneFaceChannel(m_pFacesOrig[0][0], m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, getStride((ComponentID)0), 1, 0, m_sVideoInfo.framePackStruct.faces[0][0].rot, pDstYuv, 0, 0, 0, (m_nBitDepth-m_nOutputBitDepth));
    }
    else //if(m_chromaFormatIDC == CHROMA_420)
    {
      //the resulting buffer is for chroma resampling;
      Int nWidthC = m_sVideoInfo.iFaceWidth>>getComponentScaleX((ComponentID)1);
      Int nHeightC = m_sVideoInfo.iFaceHeight>>getComponentScaleY((ComponentID)1);  
      Int iStride420 = nWidthC;
      //chroma resample
      if(!m_pDS420Buf && (m_chromaFormatIDC == CHROMA_420 && m_bResampleChroma))
      {
        m_pDS420Buf = (Pel*)xMalloc(Pel, nHeightC*nWidthC);
      }
      for(Int ch=1; ch<getNumChannels(); ch++)
      {
        if(m_bResampleChroma)
        {
          chromaResampleType2toType0(m_pFacesOrig[0][ch], m_pDS420Buf, nWidthC, nHeightC, getStride((ComponentID)ch), nWidthC);
          rotOneFaceChannel(m_pDS420Buf, nWidthC, nHeightC, iStride420, 1, ch, m_sVideoInfo.framePackStruct.faces[0][0].rot, pDstYuv, 0, 0, 0, 0);
        }
        else
          rotOneFaceChannel(m_pFacesOrig[0][ch], nWidthC, nHeightC, getStride((ComponentID)ch), 1, ch, m_sVideoInfo.framePackStruct.faces[0][0].rot, pDstYuv, 0, 0, 0, (m_nBitDepth-m_nOutputBitDepth));
      }
      //luma;
      rotOneFaceChannel(m_pFacesOrig[0][0], m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, getStride((ComponentID)0), 1, 0, m_sVideoInfo.framePackStruct.faces[0][0].rot, pDstYuv, 0, 0, 0, (m_nBitDepth-m_nOutputBitDepth));
    }
  }
  else if(pDstYuv->getChromaFormat()==CHROMA_444 || pDstYuv->getChromaFormat()==CHROMA_400)
  {
     if(m_chromaFormatIDC == pDstYuv->getChromaFormat())
     {
       for(Int ch=0; ch<getNumChannels(); ch++)
       {
         ComponentID chId = (ComponentID)ch;
         rotOneFaceChannel(m_pFacesOrig[0][ch], m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, getStride(chId), 1, ch, m_sVideoInfo.framePackStruct.faces[0][0].rot, pDstYuv, 0, 0, 0, (m_nBitDepth-m_nOutputBitDepth));
       }
     }
     else
       assert(!"Not supported yet");
  }
}

#endif