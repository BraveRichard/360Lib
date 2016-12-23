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

/** \file     TGeometry.cpp
    \brief    Geometry class
*/

#include <assert.h>
#include <math.h>
#include "../TLibCommon/TComChromaFormat.h"
#include "TGeometry.h"
#include "TEquiRect.h"
#include "TEqualArea.h"
#include "TCubeMap.h"
#include "TOctahedron.h"
#include "TIcosahedron.h"
#include "TViewPort.h"
#if SVIDEO_CPPPSNR
#include "TCrastersParabolic.h"
#endif

#if SVIDEO_EXT

template<typename T>
static inline Int filter1D(T *pSrc, Int iStride, Filter1DInfo* pFilter)
{
  Int ret = 0;
  T *pSrcL = pSrc -((pFilter->nTaps-1)>>1)*iStride;
  Int *pFilterCoeff = pFilter->iFilterCoeff;
  for(Int i=0; i<pFilter->nTaps; i++)
  {
    ret += (*pSrcL) * pFilterCoeff[i];
    pSrcL += iStride;
  }
  return ret;
}

Char TGeometry::m_strGeoName[SVIDEO_TYPE_NUM][256] = { {"Equirectangular"},
                                                       {"Cubemap"},
                                                       {"Equal-area"},
                                                       {"Octahedron"},
                                                       {"Viewport"},
                                                       {"Icosahedron"}
#if SVIDEO_CPPPSNR
                                                      ,{"Crastersparabolic"}
#endif
};
static Double sinc(Double x)
{
  x *= S_PI;
  if(x < 0.01 && x > -0.01)
  {
    double x2 = x * x;
    return 1.0f + x2 * (-1.0 / 6.0 + x2 / 120.0);
  }
  else
  {
    return sin(x) / x;
  }
}

TGeometry::TGeometry()
{
  memset(&m_sVideoInfo, 0, sizeof(m_sVideoInfo));
  m_pFacesBuf = m_pFacesOrig = NULL;
  m_chromaFormatIDC = ChromaFormat(CHROMA_444);
  m_bResampleChroma = false;
  m_iMarginX = m_iMarginY =0;
  m_nBitDepth = 8;
  m_pDS422Buf = NULL;
  m_pDS420Buf = NULL;
  m_pFaceRotBuf = NULL;

  m_bPadded = false;
  m_pUpsTempBuf = NULL;
  m_iUpsTempBufMarginSize = 0;
  m_iStrideUpsTempBuf = 0;
  m_InterpolationType[0] = m_InterpolationType[1] = SI_UNDEFINED;
  memset(m_filterDs, 0, sizeof(m_filterDs));
  memset(m_filterUps, 0, sizeof(m_filterUps));
  m_pFacesBufTemp = m_pFacesBufTempOrig = NULL;
  m_nMarginSizeBufTemp = m_nStrideBufTemp =0;

  memset(m_filterDs, 0, sizeof(m_filterDs));
  memset(m_filterUps, 0, sizeof(m_filterUps));
  m_bGeometryMapping = false;
  m_WeightMap_NumOfBits4Faces = 0;
  memset(m_pPixelWeight, 0, sizeof(m_pPixelWeight));
  m_interpolateWeight[0] = m_interpolateWeight[1] = NULL;
  m_iLanczosParamA[0] = m_iLanczosParamA[1] = 0;
  m_pfLanczosFltCoefLut[0] = m_pfLanczosFltCoefLut[1] = NULL;
  m_bGeometryMapping4SpherePadding = false;
  memset(m_pPixelWeight4SherePadding, 0, sizeof(m_pPixelWeight4SherePadding)); 

  memset(m_pWeightLut, 0, sizeof(m_pWeightLut));
  memset(m_iInterpFilterTaps, 0, sizeof(m_iInterpFilterTaps));
  m_bConvOutputPaddingNeeded = false;
}

Void TGeometry::geoInit(SVideoInfo& sVideoInfo, InputGeoParam *pInGeoParam)
{
   m_sVideoInfo = sVideoInfo;
   m_nBitDepth = pInGeoParam->nBitDepth;
   m_nOutputBitDepth = pInGeoParam->nOutputBitDepth;
   m_chromaFormatIDC = pInGeoParam->chromaFormat; 
   m_bResampleChroma = pInGeoParam->bResampleChroma;
   m_bPadded = false;
   m_WeightMap_NumOfBits4Faces = S_log2NumFaces[m_sVideoInfo.iNumFaces];//Brave:iNumFaces in bits
   initInterpolation(pInGeoParam->iInterp);

   initFilterWeightLut();

   setChromaResamplingFilter(pInGeoParam->iChromaSampleLocType);
   m_iMarginX = m_iMarginY = S_PAD_MAX;
   Int nFaces = m_sVideoInfo.iNumFaces;
   m_pFacesBuf = new Pel**[nFaces];
   m_pFacesOrig = new Pel**[nFaces];
   Int nChannels = getNumChannels();
   for(Int i=0; i<nFaces; i++)
   {
     m_pFacesBuf[i] = new Pel*[nChannels];
     memset(m_pFacesBuf[i], 0, sizeof(Pel*)*nChannels);
     m_pFacesOrig[i] = new Pel*[nChannels];
     memset(m_pFacesOrig[i], 0, sizeof(Pel*)*nChannels);
     for(Int j=0; j<(nChannels); j++)
     {
       Int iTotalHeight = (m_sVideoInfo.iFaceHeight+(m_iMarginY<<1)) >> getComponentScaleY(ComponentID(j));
       m_pFacesBuf[i][j] = (Pel *)xMalloc(Pel, getStride(ComponentID(j))*iTotalHeight);
     }
    for(Int j=0; j<nChannels; j++)
      m_pFacesOrig[i][j] = m_pFacesBuf[i][j] +  getStride(ComponentID(j))* getMarginY(ComponentID(j)) + getMarginX(ComponentID(j));
   }

   //optional; map faceId to (row, col) in frame packing structure;
   parseFacePos(m_facePos[0]);
}

TGeometry::~TGeometry()
{
  if(m_pFacesBuf)
  {
    Int nChan = ::getNumberValidComponents(m_chromaFormatIDC);
    for(Int i=0; i<m_sVideoInfo.iNumFaces; i++)
    {
     for(Int j=0; j<nChan; j++)
     {
      if(m_pFacesBuf[i][j])
      {
        xFree(m_pFacesBuf[i][j]);
        m_pFacesBuf[i][j] = NULL;    
      }
      m_pFacesOrig[i][j] = NULL;    
     }
     if(m_pFacesBuf[i])
     {
       delete[] m_pFacesBuf[i];
       m_pFacesBuf[i] = NULL;
     }
     if(m_pFacesOrig[i])
     {
       delete[] m_pFacesOrig[i];
       m_pFacesOrig[i] = NULL;  
     }
    }

    if(m_pFacesBuf)
    {
      delete[] m_pFacesBuf;
      m_pFacesBuf = NULL;
    }
    if(m_pFacesOrig)
    {
      delete[] m_pFacesOrig;
      m_pFacesOrig = NULL;
    }
  }
  if(m_pDS422Buf)
  {
    xFree(m_pDS422Buf);
    m_pDS422Buf = NULL;
  }
  if(m_pDS420Buf)
  {
    xFree(m_pDS420Buf);
    m_pDS420Buf = NULL;
  }
  if(m_pFaceRotBuf) 
  {
    xFree(m_pFaceRotBuf);
    m_pFaceRotBuf = NULL;
  } 
  if(m_pUpsTempBuf)
  {
    xFree(m_pUpsTempBuf);
    m_pUpsTempBuf = NULL;
  }
  for(Int i=0; i<SV_MAX_NUM_FACES; i++)
  {
    if(m_pPixelWeight[i])
    {
      for(Int j=0; j<2; j++)
      {
        if(m_pPixelWeight[i][j])
        {
          delete[] m_pPixelWeight[i][j];
          m_pPixelWeight[i][j] = NULL;
        }
      }
    }
    if(m_pPixelWeight4SherePadding[i])
    {
      for(Int j=0; j<2; j++)
      {
        if(m_pPixelWeight4SherePadding[i][j])
        {
          if(m_pPixelWeight4SherePadding[i][j])
          {
            delete[] m_pPixelWeight4SherePadding[i][j];
            m_pPixelWeight4SherePadding[i][j] = NULL;
          }
        }       
      }
    }
  }

  for(Int j=0; j<2; j++)
  {
    if(m_pWeightLut[j])
    {
      if(m_pWeightLut[j][0])
      {
        delete[] m_pWeightLut[j][0];
        m_pWeightLut[j][0] = NULL;
      }
      delete[] m_pWeightLut[j];
      m_pWeightLut[j] = NULL;
    }
  }

  for(Int ch=0; ch<MAX_NUM_CHANNEL_TYPE; ch++)
  {
    delete[] m_pfLanczosFltCoefLut[ch];
    m_pfLanczosFltCoefLut[ch] = NULL;
  }
}

TGeometry* TGeometry::create(SVideoInfo& sVideoInfo, InputGeoParam *pInGeoParam)
{
  TGeometry *pRet = NULL;
  if(sVideoInfo.geoType == SVIDEO_EQUIRECT)
    pRet = new TEquiRect(sVideoInfo, pInGeoParam);
  else if(sVideoInfo.geoType == SVIDEO_CUBEMAP)
    pRet = new TCubeMap(sVideoInfo, pInGeoParam);
  else if(sVideoInfo.geoType == SVIDEO_EQUALAREA)
    pRet = new TEqualArea(sVideoInfo, pInGeoParam);
  else if(sVideoInfo.geoType == SVIDEO_OCTAHEDRON)
    pRet = new TOctahedron(sVideoInfo, pInGeoParam);
  else if (sVideoInfo.geoType == SVIDEO_VIEWPORT)
    pRet = new TViewPort(sVideoInfo, pInGeoParam);
  else if(sVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
    pRet = new TIcosahedron(sVideoInfo, pInGeoParam);
#if SVIDEO_CPPPSNR
  else if(sVideoInfo.geoType == SVIDEO_CRASTERSPARABOLIC)
    pRet = new TCrastersParabolic(sVideoInfo, pInGeoParam);
#endif
  return pRet;
}

Void TGeometry::clamp(IPos *pIPos)
{
  pIPos->u = Clip3(0, m_sVideoInfo.iFaceWidth-1, (Int)pIPos->u);
  pIPos->v = Clip3(0, m_sVideoInfo.iFaceHeight-1, (Int)pIPos->v);
}

//1->2 upsampling;
Void TGeometry::chromaUpsample(Pel *pSrcBuf, Int nWidthC, Int nHeightC, Int iStrideSrc, Int iFaceId, ComponentID chId)
{
  Int nWidth = nWidthC<<1;
  Int nHeight = nHeightC<<1;
  assert(m_sVideoInfo.iFaceWidth == nWidth);
  assert(m_sVideoInfo.iFaceHeight == nHeight);
  //vertical upsampling;  [-2, 16, 54, -4]; [-4, 54, 16, -2];
  Int iStrideDst = getStride(chId);

  if(!m_pUpsTempBuf)
  {
    m_iUpsTempBufMarginSize = std::max(m_filterUps[0].nTaps, m_filterUps[1].nTaps)>>1;  //2: 4 tap filter;
    m_iStrideUpsTempBuf = nWidthC + m_iUpsTempBufMarginSize*2; 
    m_pUpsTempBuf = (Int *)xMalloc(Int, m_iStrideUpsTempBuf*nHeight); 
  }
  Int *pDst0 = m_pUpsTempBuf + 1;
  Int *pDst1 = pDst0 + m_iStrideUpsTempBuf;
  Pel *pSrc0 = pSrcBuf - m_iUpsTempBufMarginSize+1 + (m_filterUps[2].nTaps>1? - iStrideSrc :0);
  Pel *pSrc1 = pSrcBuf - m_iUpsTempBufMarginSize+1;
  for(Int j=0; j<nHeightC; j++)
  {
    for(Int i=0; i<nWidthC+2*m_iUpsTempBufMarginSize-1; i++)
    {
      pDst0[i] = filter1D(pSrc0 +i, iStrideSrc, m_filterUps+2);
      pDst1[i] = filter1D(pSrc1 +i, iStrideSrc, m_filterUps+3);
    }
    pSrc0 += iStrideSrc;
    pSrc1 += iStrideSrc;

    pDst0 += (m_iStrideUpsTempBuf<<1);
    pDst1 += (m_iStrideUpsTempBuf<<1);
  }
  //horizontal filtering; [-4, 36, 36, -4]
  pDst0 = m_pUpsTempBuf + m_iUpsTempBufMarginSize + (m_filterUps[0].nTaps>1? -1: 0);
  pDst1 = m_pUpsTempBuf + m_iUpsTempBufMarginSize;
  Pel *pOut = m_pFacesOrig[iFaceId][chId];
  Int iBitShift = m_filterUps[0].nlog2Norm + m_filterUps[2].nlog2Norm;
  Int iOffset = iBitShift? (1<<(iBitShift-1)): 0;
  for(Int j=0; j<nHeight; j++)
  {
    for(Int i=0; i<nWidthC; i++)
    {
      Int val = filter1D(pDst0+i, 1, m_filterUps); 
      pOut[(i<<1)] = ClipBD<Int>( (val+iOffset)>>iBitShift, m_nBitDepth);
      val = filter1D(pDst1+i, 1, m_filterUps+1); 
      pOut[((i<<1)+1)] = ClipBD<Int>( (val+iOffset)>>iBitShift, m_nBitDepth);      
    }
    pOut += iStrideDst;
    pDst0 += m_iStrideUpsTempBuf;
    pDst1 += m_iStrideUpsTempBuf;
  }
  
}

//horizontal 2:1 downsampling; //[1,6,1]
Void TGeometry::chromaDonwsampleH(Pel *pSrcBuf, Int iWidth, Int iHeight, Int iStrideSrc, Int iNumPels, Pel *pDstBuf, Int iStrideDst)
{
  Pel *pSrc = pSrcBuf;
  Pel *pDst = pDstBuf;
  for(Int j=0; j<iHeight; j++)
  {
    for(Int i=0; i<(iWidth>>1); i++)
    {
      Int k = i<<1;
      pDst[i] = filter1D(pSrc+k*iNumPels, iNumPels, m_filterDs);
    }
    pSrc += iStrideSrc;
    pDst += iStrideDst;
  }
}

//vertical 2:1 downsampling; //[1, 1]
Void TGeometry::chromaDonwsampleV(Pel *pSrcBuf, Int iWidth, Int iHeight, Int iStrideSrc, Int iNumPels, Pel *pDstBuf, Int iStrideDst)
{
  Int iBitShift = m_filterDs[0].nlog2Norm + m_filterDs[1].nlog2Norm + (m_nBitDepth - m_nOutputBitDepth); //3+1;
  Int iOffset = iBitShift? (1<<(iBitShift-1)) : 0;
  Pel *pSrc = pSrcBuf;
  //Pel *pSrc1 = pSrcBuf+iStrideSrc;
  Pel *pDst = pDstBuf;
  Int iStrideSrc2 = iStrideSrc<<1;

  for(Int j=0; j<(iHeight>>1); j++)
  {
    for(Int i=0; i<iWidth; i++)
    {
      Int val = filter1D(pSrc+ i*iNumPels, iStrideSrc, m_filterDs+1);
      pDst[i] = ClipBD(((val + iOffset)>>iBitShift), m_nOutputBitDepth);
    }
    pSrc += iStrideSrc2;
    //pSrc1 += iStrideSrc2;
    pDst += iStrideDst;
  }
}

//forward resampling;
Void TGeometry::chromaResampleType0toType2(Pel *pSrcBuf, Int nWidthC, Int nHeightC, Int iStrideSrc, Pel *pDstBuf, Int iStrideDst) 
{
  assert(m_iChromaSampleLocType==0);
  //vertical upsampling;  [-2, 16, 54, -4];
  Int iBitShift = m_filterUps[2].nlog2Norm; //6;
  Int iOffset = iBitShift? (1<<(iBitShift-1)) : 0;
  //Int iResampleMarginSize = m_filterUps[2].nTaps>>1;  //2: 4 tap filter;

  Pel *pSrc0 = pSrcBuf - iStrideSrc;
  Pel *pDst0 = pDstBuf;
  for(Int j=0; j<nHeightC; j++)
  {
    for(Int i=0; i<nWidthC; i++)
    {
      Int val = filter1D(pSrc0 +i, iStrideSrc, m_filterUps+2); 
      pDst0[i] = ClipBD<Int>( (val +iOffset)>>iBitShift, m_nBitDepth); 
    }
    pSrc0 += iStrideSrc;
    pDst0 += iStrideDst;
  } 
}

//reverse resampling;
Void TGeometry::chromaResampleType2toType0(Pel *pSrcBuf, Pel *pDst, Int nWidthC, Int nHeightC, Int iStrideSrc, Int iStrideDst)
{
  assert(m_iChromaSampleLocType==0);
  //vertical upsampling;  [-4, 54, 16, -2];
  Int iBitShift = m_filterUps[3].nlog2Norm + (m_nBitDepth - m_nOutputBitDepth); //6;
  Int iOffset = iBitShift? (1<<(iBitShift-1)) : 0;

  Pel *pSrc0 = pSrcBuf;
  Pel *pDst0 = pDst;
  for(Int j=0; j<nHeightC; j++)
  {
    for(Int i=0; i<nWidthC; i++)
    {
      Int val = filter1D(pSrc0 +i, iStrideSrc, m_filterUps+3);
      pDst0[i] = ClipBD<Int>( (val +iOffset)>>iBitShift, m_nOutputBitDepth); 
    }
    pSrc0 += iStrideSrc;
    pDst0 += iStrideDst;
  } 
}

Int TGeometry::getFilterSize(SInterpolationType filterType)
{
  Int iFilterSize =0;
  if(filterType == SI_LANCZOS3)
    iFilterSize = 36;
  else if(filterType == SI_LANCZOS2 || filterType == SI_BICUBIC)
    iFilterSize = 16;
  else if(filterType == SI_BILINEAR)
    iFilterSize = 4;
  else if(filterType == SI_NN)
    iFilterSize = 1;
  else
    assert(!"Not supported yet");
  return iFilterSize;
}

Void TGeometry::initFilterWeightLut()
{
  if(!m_pWeightLut[0])
  {
    //calculate the weight based on sampling pattern and interpolation filter;
    Int iNumWLuts = (m_chromaFormatIDC==CHROMA_400 || (m_InterpolationType[0]==m_InterpolationType[1]))? 1 : 2;
    for(Int i=0; i<iNumWLuts; i++)
    {
        Int iFilterSize = getFilterSize(m_InterpolationType[i]);
        m_pWeightLut[i] = new Int*[(S_LANCZOS_LUT_SCALE+1)*(S_LANCZOS_LUT_SCALE+1)];
        m_pWeightLut[i][0] = new Int[(S_LANCZOS_LUT_SCALE+1)*(S_LANCZOS_LUT_SCALE+1)*iFilterSize];
        for(Int k=1; k<(S_LANCZOS_LUT_SCALE+1)*(S_LANCZOS_LUT_SCALE+1); k++)
          m_pWeightLut[i][k] = m_pWeightLut[i][0] + k*iFilterSize;//Brave:pointer init

        //calculate the weight;
        if(m_InterpolationType[i] == SI_NN)
        {
          Int w = 1<<(S_INTERPOLATE_PrecisionBD);
          assert(iFilterSize == 1);
          for(Int m=0; m<(S_LANCZOS_LUT_SCALE+1); m++)
            for(Int n=0; n<(S_LANCZOS_LUT_SCALE+1); n++)
                m_pWeightLut[i][m*(S_LANCZOS_LUT_SCALE+1)+n][0] = w;
        }
        else if(m_InterpolationType[i] == SI_BILINEAR)
        {
          Int mul = 1<<(S_INTERPOLATE_PrecisionBD);
          assert(iFilterSize == 4);
          Double dScale = 1.0/S_LANCZOS_LUT_SCALE;
          for(Int m=0; m<(S_LANCZOS_LUT_SCALE+1); m++)
          {
            Double fy = m*dScale;
            for(Int n=0; n<(S_LANCZOS_LUT_SCALE+1); n++)
            {              
              Double fx = n*dScale;
              Int *pW = m_pWeightLut[i][m*(S_LANCZOS_LUT_SCALE+1)+n];
              pW[0] = round((1 - fx)*(1 -fy)*mul);
              pW[1] = round((fx)*(1 -fy)*mul);
              pW[2] = round((1 - fx)*(fy)*mul);
              pW[3] = mul - pW[0] -pW[1] -pW[2];
            }
          }
        }
        else if(m_InterpolationType[i] == SI_BICUBIC)
        {
          Int mul = 1<<(S_INTERPOLATE_PrecisionBD);
          assert(iFilterSize == 16);
          Double dScale = 1.0/S_LANCZOS_LUT_SCALE;
          for(Int m=0; m<(S_LANCZOS_LUT_SCALE+1); m++)
          {
            Double t = m*dScale, wy[4];
            wy[0] = (POSType)(0.5*(-t*t*t + 2*t*t - t)); 
            wy[1] = (POSType)(0.5*(3*t*t*t -5*t*t + 2));
            wy[2] = (POSType)(0.5*(-3*t*t*t + 4*t*t + t));
            wy[3] = (POSType)(0.5*(t*t*t - t*t));

            for(Int n=0; n<(S_LANCZOS_LUT_SCALE+1); n++)
            {
              Int *pW = m_pWeightLut[i][m*(S_LANCZOS_LUT_SCALE+1)+n];
              Int sum=0;
              Double wx[4];
              t = n*dScale;
              wx[0] = (POSType)(0.5*(-t*t*t + 2*t*t - t)); 
              wx[1] = (POSType)(0.5*(3*t*t*t -5*t*t + 2));
              wx[2] = (POSType)(0.5*(-3*t*t*t + 4*t*t + t));
              wx[3] = (POSType)(0.5*(t*t*t - t*t));

              for(Int r=0; r<4; r++)
                for(Int c=0; c<4; c++)
                {
                  Int w;
                  if(c!=3 || r!=3)
                    w = round(wy[r]*wx[c]*mul);
                  else
                    w = mul - sum;
                  pW[r*4+c] = w;
                  sum += w;
                }
            }
          }
        }
        else if(m_InterpolationType[i] == SI_LANCZOS2 || m_InterpolationType[i] == SI_LANCZOS3)
        {
          Int mul = 1<<(S_INTERPOLATE_PrecisionBD);
          assert((m_InterpolationType[i] == SI_LANCZOS2 && iFilterSize == 16) || (m_InterpolationType[i] == SI_LANCZOS3 && iFilterSize == 36));
          Double dScale = 1.0/S_LANCZOS_LUT_SCALE;//Brave:min
          POSType *pfLanczosFltCoefLut;
          pfLanczosFltCoefLut = m_pfLanczosFltCoefLut[i];
          for(Int m=0; m<(S_LANCZOS_LUT_SCALE+1); m++)//Brave:Combination of various possibilities.  x 100 y 100
          {
            Double t = m*dScale;
            POSType wy[6];
            for(Int k=-m_iLanczosParamA[i]; k<m_iLanczosParamA[i]; k++)//Brave:y  6 points
              wy[k + m_iLanczosParamA[i]] = pfLanczosFltCoefLut[(Int)((sfabs(t-k-1) + m_iLanczosParamA[i])* S_LANCZOS_LUT_SCALE + 0.5)]; 

            for(Int n=0; n<(S_LANCZOS_LUT_SCALE+1); n++)
            {
              POSType wx[6];
              Int *pW = m_pWeightLut[i][m*(S_LANCZOS_LUT_SCALE+1)+n];
              Int sum=0;
              t = n*dScale;
              for(Int k=-m_iLanczosParamA[i]; k<m_iLanczosParamA[i]; k++)
                wx[k + m_iLanczosParamA[i]] = pfLanczosFltCoefLut[(Int)((sfabs(t-k-1) + m_iLanczosParamA[i])* S_LANCZOS_LUT_SCALE + 0.5)]; 
              //normalize;
              POSType dSum = 0;
              for(Int r=0; r<(m_iLanczosParamA[i]<<1); r++)
                for(Int c=0; c<(m_iLanczosParamA[i]<<1); c++)
                    dSum += wy[r]*wx[c];
              for(Int r=0; r<(m_iLanczosParamA[i]<<1); r++)
                for(Int c=0; c<(m_iLanczosParamA[i]<<1); c++)
                {
                  Int w;
                  if(c!=(m_iLanczosParamA[i]<<1)-1 || r!=(m_iLanczosParamA[i]<<1)-1)
                    w = round((POSType)(wy[r]*wx[c]*mul/dSum));
                  else
                    w = mul - sum;
                  pW[r*(m_iLanczosParamA[i]<<1)+c] = w;
                  sum += w;
                }
            }
          }
        }
    }
  }
}

//nearest neighboring;
Void TGeometry::interpolate_nn_weight(ComponentID chId, SPos *pSPosIn, PxlFltLut &wlist)
{
  //Int iChannels = getNumChannels();
  Int x = round(pSPosIn->x);
  Int y = round(pSPosIn->y);
  assert(x >=- (m_iMarginX >> getComponentScaleX(chId)) && x<(m_sVideoInfo.iFaceWidth + m_iMarginX)>>getComponentScaleX(chId));
  assert(y >=- (m_iMarginY >> getComponentScaleY(chId)) && y<(m_sVideoInfo.iFaceHeight + m_iMarginY)>>getComponentScaleY(chId));
  
  wlist.facePos = ((y*getStride(chId) +x) << m_WeightMap_NumOfBits4Faces) | pSPosIn->faceIdx;
  wlist.weightIdx = 0;
}

//bilinear interpolation;
Void TGeometry::interpolate_bilinear_weight(ComponentID chId, SPos *pSPosIn, PxlFltLut &wlist)
{
  //Int iChannels = getNumChannels();
  Int x = (Int)sfloor(pSPosIn->x);
  Int y = (Int)sfloor(pSPosIn->y);
  assert(x >=- (m_iMarginX >> getComponentScaleX(chId)) && x<((m_sVideoInfo.iFaceWidth + m_iMarginX)>>getComponentScaleX(chId))-1);
  assert(y >=- (m_iMarginY >> getComponentScaleY(chId)) && y<((m_sVideoInfo.iFaceHeight + m_iMarginY)>>getComponentScaleY(chId))-1);

  wlist.facePos = ((y*getStride(chId) +x)<<m_WeightMap_NumOfBits4Faces) | pSPosIn->faceIdx;
  wlist.weightIdx = round((pSPosIn->y -y)*S_LANCZOS_LUT_SCALE)*(S_LANCZOS_LUT_SCALE+1) + round((pSPosIn->x-x)*S_LANCZOS_LUT_SCALE);
}

/******************************************
cubic hermite spline;
(1/2)*[-t^3+2t^2-t,  3t^3-5t^2+2,  -3t^3+4t^2+t,  t^3-t^2] 
*******************************************/
Void TGeometry::interpolate_bicubic_weight(ComponentID chId, SPos *pSPosIn, PxlFltLut &wlist)
{
  //Int iChannels = getNumChannels();
  Int x = (Int)sfloor(pSPosIn->x); 
  Int y = (Int)sfloor(pSPosIn->y); 
  assert(x >=- (m_iMarginX >> getComponentScaleX(chId))+1 && x<((m_sVideoInfo.iFaceWidth + m_iMarginX)>>getComponentScaleX(chId))-2);
  assert(y >=- (m_iMarginY >> getComponentScaleY(chId))+1 && y<((m_sVideoInfo.iFaceHeight + m_iMarginY)>>getComponentScaleY(chId))-2);

  wlist.facePos = ((y*getStride(chId) +x)<<m_WeightMap_NumOfBits4Faces) | pSPosIn->faceIdx;
  wlist.weightIdx = round((pSPosIn->y -y)*S_LANCZOS_LUT_SCALE)*(S_LANCZOS_LUT_SCALE+1) + round((pSPosIn->x-x)*S_LANCZOS_LUT_SCALE);
}

Void TGeometry::interpolate_lanczos_weight(ComponentID chId, SPos *pSPosIn, PxlFltLut &wlist)
{
  //Int iChannels = getNumChannels();
  Int x = (Int)sfloor(pSPosIn->x); 
  Int y = (Int)sfloor(pSPosIn->y); 
  ChannelType chType = toChannelType(chId);
  assert(x >=- (m_iMarginX >> getComponentScaleX(chId))+m_iLanczosParamA[chType]-1 && x<((m_sVideoInfo.iFaceWidth + m_iMarginX)>>getComponentScaleX(chId))-m_iLanczosParamA[chType]);
  assert(y >=- (m_iMarginY >> getComponentScaleY(chId))+m_iLanczosParamA[chType]-1 && y<((m_sVideoInfo.iFaceHeight + m_iMarginY)>>getComponentScaleY(chId))-m_iLanczosParamA[chType]);

  wlist.facePos = ((y*getStride(chId) +x)<<m_WeightMap_NumOfBits4Faces) | pSPosIn->faceIdx;
  wlist.weightIdx = round((pSPosIn->y -y)*S_LANCZOS_LUT_SCALE)*(S_LANCZOS_LUT_SCALE+1) + round((pSPosIn->x-x)*S_LANCZOS_LUT_SCALE);
}

Void TGeometry::convertYuv(TComPicYuv *pSrcYuv)
{
  assert(!"override");
}

Void TGeometry::geometryMapping(TGeometry *pGeoSrc)
{
  assert(!m_bGeometryMapping);
    
    Int iNumMaps = (m_chromaFormatIDC==CHROMA_400 || (m_chromaFormatIDC==CHROMA_444 && m_InterpolationType[0]==m_InterpolationType[1]))? 1 : 2;
    Int *pRot = m_sVideoInfo.sVideoRotation.degree;

    if((m_sVideoInfo.framePackStruct.chromaFormatIDC==CHROMA_420) && ( (m_chromaFormatIDC == CHROMA_444) || (m_chromaFormatIDC == CHROMA_420 && m_bResampleChroma) ) )
      m_bConvOutputPaddingNeeded = true;

    for(Int fIdx=0; fIdx<m_sVideoInfo.iNumFaces; fIdx++)
    {
     for(Int ch=0; ch<iNumMaps; ch++)
     {
       ComponentID chId = (ComponentID)ch;
       Int iWidthPW = getStride(chId);
       Int iHeightPW = (m_sVideoInfo.iFaceHeight+(m_iMarginY<<1))>>getComponentScaleY(chId);

       if(!m_pPixelWeight[fIdx][ch])
       {
         m_pPixelWeight[fIdx][ch] = new PxlFltLut[iWidthPW*iHeightPW];
       }
     }
    }

    //For ViewPort, Set Rotation Matrix and K matrix
    if (m_sVideoInfo.geoType==SVIDEO_VIEWPORT)
    {
      ((TViewPort*)this)->setRotMat();
      ((TViewPort*)this)->setInvK();
    }
    //Brave:add
    braveLocation = (Pel **)xMalloc(Pel*,iNumMaps);
    //Brave:add

    //generate the map;
    for(Int fIdx=0; fIdx<m_sVideoInfo.iNumFaces; fIdx++)
    {
      for(Int ch=0; ch<iNumMaps; ch++)
      {
        //Brave:add
        braveLocation[ch] = (Pel *)xMalloc(Pel,m_sVideoInfo.iFaceHeight >> getComponentScaleY((ComponentID)ch));
        //Brave:add
        ComponentID chId = (ComponentID)ch;
        Int iStridePW = getStride(chId);
        Int iWidth = m_sVideoInfo.iFaceWidth >> getComponentScaleX(chId);
        Int iHeight = m_sVideoInfo.iFaceHeight >> getComponentScaleY(chId);
        Int nMarginX = m_iMarginX >> getComponentScaleX(chId); 
        Int nMarginY = m_iMarginY >> getComponentScaleY(chId);
        for(Int j=-nMarginY; j<iHeight+nMarginY; j++) 
        {
          //Brave:add
          int braveCount = 0;
          if ((j >= 0) && (j < iHeight))
          {
            braveLocation[chId][j] = 0;
          }
          //Brave:add
          for(Int i=-nMarginX; i<iWidth+nMarginX; i++)  
          {
            if(!m_bConvOutputPaddingNeeded && !insideFace(fIdx, (i<<getComponentScaleX(chId)), (j<<getComponentScaleY(chId)), COMPONENT_Y, chId))
              continue;

            //Brave:add
//            if (i < 0 || i > iWidth)
//              continue;
            //Brave:add end
            Int xOrg = (i+nMarginX);
            Int yOrg = (j+nMarginY);
            Int ic = i;
            Int jc = j;
            {
              PxlFltLut& wList = m_pPixelWeight[fIdx][ch][yOrg*iStridePW + xOrg];
              POSType x = (ic) * (1<<getComponentScaleX(chId));
              POSType y = (jc) * (1<<getComponentScaleY(chId)); 
              SPos in(fIdx, x, y, 0), pos3D;
              
              map2DTo3D(in, &pos3D);
              rotate3D(pos3D, pRot[0], pRot[1], pRot[2]);
              //Brave:add
              if((pos3D.x == 1) && (pos3D.y == 0) && (pos3D.z == 0))
              {
                if (i <= iWidth / 2)
                {
                  ++ braveCount;
                  //if (j > 10 || j < iHeight / 2)
                  //{
                  //  
                  //}
                  braveLocation[chId][j] = braveCount;
                }
              }
              //Brave:add
              pGeoSrc->map3DTo2D(&pos3D, &pos3D);

              pos3D.x = pos3D.x/POSType(1<<getComponentScaleX(chId));
              pos3D.y = pos3D.y/POSType(1<<getComponentScaleY(chId));
              (pGeoSrc->*pGeoSrc->m_interpolateWeight[toChannelType(chId)])(chId, &pos3D, wList);
            }    
          }
        }
      }
    }
    m_bGeometryMapping = true;
    
}

/***************************************************
//convert source geometry to destination geometry;
****************************************************/
Void TGeometry::geoConvert(TGeometry *pGeoDst)
{
  //padding;
  spherePadding();

  if(!pGeoDst->m_bGeometryMapping)
    pGeoDst->geometryMapping(this);

  Int nFaces = pGeoDst->m_sVideoInfo.iNumFaces;
  Int iBDPrecision = S_INTERPOLATE_PrecisionBD;
  Int iWeightMapFaceMask = (1<<m_WeightMap_NumOfBits4Faces)-1;
  Int iOffset = 1<<(iBDPrecision-1);
  for(Int fIdx=0; fIdx<nFaces; fIdx++)
  {
    for(Int ch=0; ch<pGeoDst->getNumChannels(); ch++)
    {
      ComponentID chId = (ComponentID)ch;
      Int nWidth = pGeoDst->m_sVideoInfo.iFaceWidth >> pGeoDst->getComponentScaleX(chId);
      Int nHeight = pGeoDst->m_sVideoInfo.iFaceHeight >> pGeoDst->getComponentScaleY(chId);

      Int nMarginX = pGeoDst->m_iMarginX >> pGeoDst->getComponentScaleX(chId);
      Int nMarginY = pGeoDst->m_iMarginY >> pGeoDst->getComponentScaleY(chId);
      Int iWidthPW = pGeoDst->getStride(chId);
      Int mapIdx = (pGeoDst->m_chromaFormatIDC==CHROMA_444 && pGeoDst->m_InterpolationType[CHANNEL_TYPE_LUMA] == pGeoDst->m_InterpolationType[CHANNEL_TYPE_CHROMA])? 0 : (ch>0? 1: 0);
      ChannelType chType = toChannelType(chId);

      for(Int j=-nMarginY; j<nHeight+nMarginY; j++) 
        for(Int i=-nMarginX; i<nWidth+nMarginX; i++)  
        {
          if(!pGeoDst->m_bConvOutputPaddingNeeded && !pGeoDst->insideFace(fIdx, (i<<getComponentScaleX(chId)), (j<<getComponentScaleY(chId)), COMPONENT_Y, chId))
            continue;
          //Brave:add
//          if (i < 0 || i > nWidth)
//            continue;
          //Brave:add end
          Int x = i+nMarginX;
          Int y = j+nMarginY;
          Int sum =0;

          PxlFltLut *pPelWeight = pGeoDst->m_pPixelWeight[fIdx][mapIdx] + y*iWidthPW + x;
          Int face = (pPelWeight->facePos)&iWeightMapFaceMask;
          Int iTLPos = (pPelWeight->facePos)>>m_WeightMap_NumOfBits4Faces;
          Int iWLutIdx = (m_chromaFormatIDC==CHROMA_400 || (m_InterpolationType[0]==m_InterpolationType[1]))? 0 : chType;
          Int *pWLut = m_pWeightLut[iWLutIdx][pPelWeight->weightIdx];
          Pel *pPelLine = m_pFacesOrig[face][ch] +iTLPos -((m_iInterpFilterTaps[chType][1]-1)>>1)*getStride(chId) -((m_iInterpFilterTaps[chType][0]-1)>>1);
          for(Int m=0; m<m_iInterpFilterTaps[chType][1]; m++)
          {
            for(Int n=0; n<m_iInterpFilterTaps[chType][0]; n++)
              sum += pPelLine[n]*pWLut[n];
            pPelLine += getStride(chId);
            pWLut += m_iInterpFilterTaps[chType][0];
          }

          Int iPos = j*pGeoDst->getStride(chId) + i;
          pGeoDst->m_pFacesOrig[fIdx][ch][iPos] = (sum + iOffset)>>iBDPrecision;
        }
      //Brave:add
      for(Int j=0; j<nHeight; j++) 
      {
        int braveWidth = 2 * (nWidth / 2 - pGeoDst->braveLocation[chType][j]);
        for(Int i=0; i<nWidth; i++) 
        {
          if (i < pGeoDst->braveLocation[chType][j])
          {
            Int iDPos = j * pGeoDst->getStride(chId) + i;
            Int iSPos = j * pGeoDst->getStride(chId) + pGeoDst->braveLocation[chType][j] + braveWidth - (pGeoDst->braveLocation[chType][j] - i) % braveWidth;
            pGeoDst->m_pFacesOrig[fIdx][ch][iDPos] = pGeoDst->m_pFacesOrig[fIdx][ch][iSPos];
          }
          else if (i > pGeoDst->braveLocation[chType][j] + braveWidth)
          {
            Int iDPos = j * pGeoDst->getStride(chId) + i;
            Int iSPos = j * pGeoDst->getStride(chId) + (i - pGeoDst->braveLocation[chType][j] - braveWidth) % braveWidth + pGeoDst->braveLocation[chType][j];
            pGeoDst->m_pFacesOrig[fIdx][ch][iDPos] = pGeoDst->m_pFacesOrig[fIdx][ch][iSPos];
          }
        }
      }
      //Brave:add
    }
  }

  pGeoDst->setPaddingFlag(pGeoDst->m_bConvOutputPaddingNeeded ? true : false); 
}

Void TGeometry::geoToFramePack(IPos* posIn, IPos2D* posOut)
{
  Int xoffset=m_facePos[posIn->faceIdx][1]*m_sVideoInfo.iFaceWidth;//[face][0:row, 1:col];
  Int yoffset=m_facePos[posIn->faceIdx][0]*m_sVideoInfo.iFaceHeight;
  Int rot = m_sVideoInfo.framePackStruct.faces[m_facePos[posIn->faceIdx][0]][m_facePos[posIn->faceIdx][1]].rot;
  Int xc=0, yc=0;

  if(!rot)
  {
    xc=posIn->u;
    yc=posIn->v;
  }
  else if(rot==90)
  {
    xc=posIn->v;
    yc=m_sVideoInfo.iFaceWidth-1-posIn->u;
  }
  else if(rot==180)
  {
    xc=m_sVideoInfo.iFaceWidth-posIn->u-1;
    yc=m_sVideoInfo.iFaceHeight-posIn->v-1;
  }
  else if(rot==270)
  {
    xc=m_sVideoInfo.iFaceHeight-1-posIn->v;
    yc=posIn->u;
  }
  else
    assert(!"rotation degree is not supported!\n");

  posOut->x=xc+xoffset;
  posOut->y=yc+yoffset;
}

Void TGeometry::framePack(TComPicYuv *pDstYuv)
{
  Int iTotalNumOfFaces = m_sVideoInfo.framePackStruct.rows * m_sVideoInfo.framePackStruct.cols;

  if(pDstYuv->getChromaFormat()==CHROMA_420)
  {
    if( (m_chromaFormatIDC == CHROMA_444) || (m_chromaFormatIDC == CHROMA_420 && m_bResampleChroma) )
      spherePadding();

    assert(m_sVideoInfo.framePackStruct.chromaFormatIDC == CHROMA_420);
    
    //1: 444->420;  444->422, H:[1, 6, 1]; 422->420, V[1,1];
    //(Wc*2Hc) and (Wc*Hc) temporal buffer; the resulting buffer is for rotation;
    Int nWidthC = m_sVideoInfo.iFaceWidth>>pDstYuv->getComponentScaleX((ComponentID)1);
    Int nHeightC = m_sVideoInfo.iFaceHeight>>pDstYuv->getComponentScaleY((ComponentID)1);  
    Int nMarginSize = (m_filterDs[1].nTaps-1)>>1; //0, depending on V filter and considering south pole;
    Int nHeightC422 = m_sVideoInfo.iFaceHeight + nMarginSize*2;
    Int iStride422 = nWidthC;
    Int iStride420 = nWidthC;
    if((m_chromaFormatIDC == CHROMA_444) && !m_pDS422Buf)
    {
      m_pDS422Buf = (Pel*)xMalloc(Pel, nHeightC422*iStride422);
    }
    if( !m_pDS420Buf && ((m_chromaFormatIDC == CHROMA_444) || (m_chromaFormatIDC == CHROMA_420 && m_bResampleChroma)) )
    {
      m_pDS420Buf = (Pel*)xMalloc(Pel, nHeightC*iStride420);
    }
    for(Int face=0; face<iTotalNumOfFaces; face++)
    {
      Int x = m_facePos[face][1]*m_sVideoInfo.iFaceWidth;
      Int y = m_facePos[face][0]*m_sVideoInfo.iFaceHeight;
      Int rot = m_sVideoInfo.framePackStruct.faces[m_facePos[face][0]][m_facePos[face][1]].rot;
      if(face < m_sVideoInfo.iNumFaces)
      {
        if(m_chromaFormatIDC == CHROMA_444)
        {
          //chroma; 444->420;
          for(Int ch=1; ch<getNumChannels(); ch++)
          {
            ComponentID chId = (ComponentID)ch;
            Int xc = x >> pDstYuv->getComponentScaleX(chId);
            Int yc = y >> pDstYuv->getComponentScaleY(chId);
            chromaDonwsampleH(m_pFacesOrig[face][ch]-nMarginSize*getStride((ComponentID)ch), m_sVideoInfo.iFaceWidth, nHeightC422, getStride(chId), 1, m_pDS422Buf, iStride422);
            chromaDonwsampleV(m_pDS422Buf + nMarginSize*iStride422, nWidthC, m_sVideoInfo.iFaceHeight, iStride422, 1, m_pDS420Buf, iStride420);
            rotOneFaceChannel(m_pDS420Buf, nWidthC, nHeightC, iStride420, 1, ch, rot, pDstYuv, xc, yc, face, 0);
          }
        }
        else
        {
          //m_chromaFormatIDC is CHROMA_420;
          for(Int ch=1; ch<getNumChannels(); ch++)
          {
            ComponentID chId = (ComponentID)ch;
            if(m_bResampleChroma)
            {
              //convert chroma_sample_loc from 2 to 0;
              chromaResampleType2toType0(m_pFacesOrig[face][ch], m_pDS420Buf, nWidthC, nHeightC, getStride(chId), nWidthC);
              rotOneFaceChannel(m_pDS420Buf, nWidthC, nHeightC, nWidthC, 1, ch, rot, pDstYuv, x>>pDstYuv->getComponentScaleX(chId), y>>pDstYuv->getComponentScaleY(chId), face, 0);
            }
            else
              rotOneFaceChannel(m_pFacesOrig[face][ch], nWidthC, nHeightC, getStride(chId), 1, ch, rot, pDstYuv, x>>pDstYuv->getComponentScaleX(chId), y>>pDstYuv->getComponentScaleY(chId), face, (m_nBitDepth-m_nOutputBitDepth));
          }
        }
        //luma;
        rotOneFaceChannel(m_pFacesOrig[face][0], m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, getStride((ComponentID)0), 1, 0, rot, pDstYuv, x, y, face, (m_nBitDepth-m_nOutputBitDepth));
      }
      else
      {
        fillRegion(pDstYuv, x, y, rot, m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight); 
      }
    }    
  }
  else if(pDstYuv->getChromaFormat()==CHROMA_444 || pDstYuv->getChromaFormat()==CHROMA_400)
  {    
    if(m_chromaFormatIDC == pDstYuv->getChromaFormat())
    {
      for(Int face=0; face<iTotalNumOfFaces; face++)
      {
        Int x = m_facePos[face][1]*m_sVideoInfo.iFaceWidth;
        Int y = m_facePos[face][0]*m_sVideoInfo.iFaceHeight;
        Int rot = m_sVideoInfo.framePackStruct.faces[m_facePos[face][0]][m_facePos[face][1]].rot;
        if(face < m_sVideoInfo.iNumFaces)
        {
          for(Int ch=0; ch<getNumChannels(); ch++)
          {
            ComponentID chId = (ComponentID)ch;
            rotOneFaceChannel(m_pFacesOrig[face][ch], m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, getStride(chId), 1, ch, rot, pDstYuv, x, y, face, (m_nBitDepth-m_nOutputBitDepth));
          }
        }
        else
        {
            fillRegion(pDstYuv, x, y, rot, m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight);
        }
      }
    }
    else
      assert(!"Not supported!");
  }

#if SVIDEO_DEBUG
  //dump to file;
  static Bool bFirstDumpFP = true;
  Char fileName[256];
  sprintf(fileName, "framepack_fp_cf%d_%dx%dx%d.yuv", pDstYuv->getChromaFormat(), pDstYuv->getWidth((ComponentID)0), pDstYuv->getHeight((ComponentID)0), m_nBitDepth);
  FILE *fp = fopen(fileName, bFirstDumpFP? "wb" : "ab");
  for(Int ch=0; ch<pDstYuv->getNumberValidComponents(); ch++)
  {
    ComponentID chId = (ComponentID)ch;
    Int iWidth = pDstYuv->getWidth(chId);
    Int iHeight = pDstYuv->getHeight(chId);
    Int iStride = pDstYuv->getStride(chId);  
    dumpBufToFile(pDstYuv->getAddr(chId), iWidth, iHeight, 1, iStride, fp);
  }
  fclose(fp);
  bFirstDumpFP = false;
#endif
}

Void TGeometry::compactFramePackConvertYuv(TComPicYuv * pSrcYuv)
{
  assert(0);
}

Void TGeometry::compactFramePack(TComPicYuv *pDstYuv)
{
  assert(0); 
}

Void TGeometry::spherePadding(Bool bEnforced)
{
  if(!bEnforced && m_bPadded)
  {
    return;
  }
  m_bPadded = false;

#if SVIDEO_DEBUG
  //dump to file;
  static Bool bFirstDumpBeforePading = true;
  dumpAllFacesToFile("geometry_before_padding", false, !bFirstDumpBeforePading); 
  bFirstDumpBeforePading = false;
#endif

  if(!m_bGeometryMapping4SpherePadding)
    geometryMapping4SpherePadding();

  Int iBDPrecision = S_INTERPOLATE_PrecisionBD;
  Int iWeightMapFaceMask = (1<<m_WeightMap_NumOfBits4Faces)-1;
  Int iOffset = 1<<(iBDPrecision-1);

  for(Int fIdx=0; fIdx<m_sVideoInfo.iNumFaces; fIdx++)
  {
   for(Int ch=0; ch<getNumChannels(); ch++)
   {
      ComponentID chId = (ComponentID)ch;
      Int nWidth = m_sVideoInfo.iFaceWidth >> getComponentScaleX(chId);
      Int nHeight = m_sVideoInfo.iFaceHeight >> getComponentScaleY(chId);
      Int nMarginX = m_iMarginX >> getComponentScaleX(chId);
      Int nMarginY = m_iMarginY >> getComponentScaleY(chId);
      Int mapIdx = (m_chromaFormatIDC==CHROMA_444 && m_InterpolationType[CHANNEL_TYPE_LUMA] == m_InterpolationType[CHANNEL_TYPE_CHROMA])? 0: (ch>0? 1: 0);
      ChannelType chType = toChannelType(chId);

      for(Int j=-nMarginY; j<nHeight+nMarginY; j++)
      {
        for(Int i=-nMarginX; i<nWidth+nMarginX; i++)
        {
          if(insideFace(fIdx, (i<<getComponentScaleX(chId)), (j<<getComponentScaleY(chId)), COMPONENT_Y, chId))
            continue;

          Int iLutIdx;
          getSPLutIdx(ch, i, j, iLutIdx);
          Int sum =0;

          PxlFltLut *pPelWeight = m_pPixelWeight4SherePadding[fIdx][mapIdx] + iLutIdx;
          Int face = (pPelWeight->facePos)&iWeightMapFaceMask;
          Int iTLPos = (pPelWeight->facePos)>>m_WeightMap_NumOfBits4Faces;
          Int iWLutIdx = (m_chromaFormatIDC==CHROMA_400 || (m_InterpolationType[0]==m_InterpolationType[1]))? 0 : chType;
          Int *pWLut = m_pWeightLut[iWLutIdx][pPelWeight->weightIdx] ;
          Pel *pPelLine = m_pFacesOrig[face][ch] +iTLPos -((m_iInterpFilterTaps[chType][1]-1)>>1)*getStride(chId) -((m_iInterpFilterTaps[chType][0]-1)>>1);
          for(Int m=0; m<m_iInterpFilterTaps[chType][1]; m++)
          {
            for(Int n=0; n<m_iInterpFilterTaps[chType][0]; n++)
              sum += pPelLine[n]*pWLut[n];
            pPelLine += getStride(chId);
            pWLut += m_iInterpFilterTaps[chType][0];
          }
          
          m_pFacesOrig[fIdx][ch][j*getStride(chId)+i] = ClipBD((sum + iOffset)>>iBDPrecision, m_nBitDepth);
        }
      }
    }
  }
  m_bPadded = true;

#if SVIDEO_DEBUG
  //dump to file;
  static Bool bFirstDumpAfterPading = true;
  dumpAllFacesToFile("geometry_after_padding", true, !bFirstDumpAfterPading); 
  bFirstDumpAfterPading = false;
#endif

}

Void TGeometry::geometryMapping4SpherePadding()
{
  assert(!m_bGeometryMapping4SpherePadding);

  Int iWidth = m_sVideoInfo.iFaceWidth;
  Int iHeight = m_sVideoInfo.iFaceHeight;

  //allocate the memory;
  Int iNumMaps = (m_chromaFormatIDC==CHROMA_400 || (m_chromaFormatIDC==CHROMA_444 && m_InterpolationType[0]==m_InterpolationType[1]))? 1 : 2;
  for(Int fIdx=0; fIdx<m_sVideoInfo.iNumFaces; fIdx++)
  {
    for(Int ch=0; ch<iNumMaps; ch++)
    {
      ComponentID chId = (ComponentID)ch;
      Int iWidthPW = getStride(chId);
      Int iHeightPW = (m_sVideoInfo.iFaceHeight + (m_iMarginY<<1))>>getComponentScaleY(chId);

      if(!m_pPixelWeight4SherePadding[fIdx][ch])
      {
        if(m_sVideoInfo.geoType == SVIDEO_CUBEMAP)
        {
          m_pPixelWeight4SherePadding[fIdx][ch] = new PxlFltLut[iWidthPW*iHeightPW - (iWidth>>getComponentScaleX(chId))*(iHeight>>getComponentScaleY(chId))];
        }
        else if(m_sVideoInfo.geoType == SVIDEO_OCTAHEDRON
          || (m_sVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
          )
        {
          m_pPixelWeight4SherePadding[fIdx][ch] = new PxlFltLut[iWidthPW*iHeightPW];
        }
        else
          assert(!"Not supported yet!");
      }
    }
  }

  //generate the map;
  Bool bPadded[SV_MAX_NUM_FACES];
  memset(bPadded, 0, sizeof(bPadded));
  for(Int fIdx=0; fIdx<m_sVideoInfo.iNumFaces; fIdx++)
  {
    for(Int ch=0; ch<iNumMaps; ch++)
    {
      ComponentID chId = (ComponentID)ch;
      Int nWidth = iWidth >> getComponentScaleX(chId);
      Int nHeight = iHeight >> getComponentScaleY(chId);
      Int nMarginX = m_iMarginX >> getComponentScaleX(chId);
      Int nMarginY = m_iMarginY >> getComponentScaleY(chId);

      for(Int j=-nMarginY; j<nHeight+nMarginY; j++)
      {
        for(Int i=-nMarginX; i<nWidth+nMarginX; i++)
        {
          if(insideFace(fIdx, (i<<getComponentScaleX(chId)), (j<<getComponentScaleY(chId)), COMPONENT_Y, chId))
            continue;

          Int iLutIdx;
          getSPLutIdx(ch, i, j, iLutIdx);
          
          PxlFltLut& wList = m_pPixelWeight4SherePadding[fIdx][ch][iLutIdx];
          POSType x = (i)*(1<<getComponentScaleX(chId));
          POSType y = (j)*(1<<getComponentScaleY(chId));
          SPos in(fIdx, x, y, 0), pos3D;
          map2DTo3D(in, &pos3D);
          map3DTo2D(&pos3D, &pos3D);

          pos3D.x /= (1<<getComponentScaleX(chId));
          pos3D.y /= (1<<getComponentScaleY(chId)); 
          if( (bPadded[pos3D.faceIdx] || validPosition4Interp(chId, pos3D.x, pos3D.y)))
            (this->*m_interpolateWeight[toChannelType(chId)])(chId, &pos3D, wList);
          else
          {
            pos3D.x = Clip3((POSType)0.0, (POSType)(nWidth-1), pos3D.x);
            pos3D.y = Clip3((POSType)0.0, (POSType)(nHeight-1), pos3D.y);
            interpolate_nn_weight(chId, &pos3D, wList);
          }
        }
      }
    }

    bPadded[fIdx] = true;
  }

  m_bGeometryMapping4SpherePadding = true;
}

//the origin for (x, y) cooridates is the topleft of picture;
Void TGeometry::getSPLutIdx(Int ch, Int x, Int y, Int& iIdx)
{
  if(m_sVideoInfo.geoType == SVIDEO_CUBEMAP)
  {
    ComponentID chId = (ComponentID)ch;
    Int iMarginX = m_iMarginX >> getComponentScaleX(chId);
    Int iMarginY = m_iMarginY >> getComponentScaleY(chId);
    Int iHeight = m_sVideoInfo.iFaceHeight >> getComponentScaleY(chId);
    if(y<0)
      iIdx = (y+iMarginY)*getStride(chId) + (x+iMarginX);
    else if(y<iHeight)
    {
      iIdx = (iMarginY)*getStride(chId) + (iMarginX<<1)*y + (x+iMarginX) + (x<0? 0: -(m_sVideoInfo.iFaceWidth >> getComponentScaleX(chId)));
    }
    else //y>=iHeight && y<iHeight+iMarginY
      iIdx = (iMarginY+(y-iHeight))*getStride(chId) + (iMarginX<<1)*iHeight + (x+iMarginX);
  }
  else if(m_sVideoInfo.geoType == SVIDEO_OCTAHEDRON
          || (m_sVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
    )
  {
    ComponentID chId = (ComponentID)ch;
    iIdx = (y + (m_iMarginY >> getComponentScaleY(chId)))*getStride(chId) + (x+(m_iMarginX >> getComponentScaleX(chId)));
  }
  else
    assert(!"Not supported yet!");
}

//for frame packing;
Void TGeometry::rotOneFaceChannel(Pel *pSrcBuf, Int iWidthSrc, Int iHeightSrc, Int iStrideSrc, Int iNumSamplesPerPixel, Int ch, Int rot, TComPicYuv *pcPicYuvDst, Int offsetX, Int offsetY, Int face, Int iBDAdjust)
{
  assert(iBDAdjust >=0);
  ComponentID chId = (ComponentID)ch;
  Int iStrideDst = pcPicYuvDst->getStride(chId);
  Pel *pDstBuf = pcPicYuvDst->getAddr(chId) + offsetY*iStrideDst + offsetX;
  Pel emptyVal = 1<<(m_nOutputBitDepth-1);
  Int iScaleX = pcPicYuvDst->getComponentScaleX(chId);
  Int iScaleY = pcPicYuvDst->getComponentScaleY(chId);
  Int iOffset = iBDAdjust>0? (1<<(iBDAdjust-1)) : 0; 

  if(!rot)
  {
    Int iWidthDst = iWidthSrc;
    Int iHeightDst = iHeightSrc;
    Pel *pSrcLine = pSrcBuf;
    assert(pcPicYuvDst->getWidth(chId) >= offsetX+iWidthDst);
    assert(pcPicYuvDst->getHeight(chId) >= offsetY+iHeightDst);
    for(Int j=0; j<iHeightDst; j++)
    {
      Pel *pSrc = pSrcLine;
      for(Int i=0; i<iWidthDst; i++, pSrc+= iNumSamplesPerPixel)
      {
        if(insideFace(face, i<<iScaleX, j<<iScaleY, COMPONENT_Y, chId))
          pDstBuf[i] = ClipBD(((*pSrc) + iOffset) >> iBDAdjust, m_nOutputBitDepth);
        else
          pDstBuf[i] = emptyVal;
      }
      pDstBuf += iStrideDst;
      pSrcLine += iStrideSrc;
    }
  }
  else if(rot == 90)
  {
    Int iWidthDst = iHeightSrc;
    Int iHeightDst = iWidthSrc;
    Pel *pSrcLine = pSrcBuf + (iWidthSrc-1)*iNumSamplesPerPixel;

    assert(pcPicYuvDst->getWidth(chId) >= offsetX+iWidthDst);
    assert(pcPicYuvDst->getHeight(chId) >= offsetY+iHeightDst);
    for(Int j=0; j<iHeightDst; j++)
    {
      Pel *pSrc = pSrcLine;
      for(Int i=0; i<iWidthDst; i++, pSrc+= iStrideSrc)
      {
        if(insideFace(face, (iHeightDst-1-j)<<iScaleX, i<<iScaleY, COMPONENT_Y, chId))
          pDstBuf[i] = ClipBD(((*pSrc) + iOffset) >> iBDAdjust, m_nOutputBitDepth);
        else
          pDstBuf[i] = emptyVal;
      }
      pDstBuf += iStrideDst;
      pSrcLine -= iNumSamplesPerPixel;
    }
  }
  else if(rot == 180)
  {
    Int iWidthDst = iWidthSrc;
    Int iHeightDst = iHeightSrc;
    Pel *pSrcLine = pSrcBuf + (iHeightSrc-1)*iStrideSrc + (iWidthSrc-1)*iNumSamplesPerPixel;

    assert(pcPicYuvDst->getWidth(chId) >= offsetX+iWidthDst);
    assert(pcPicYuvDst->getHeight(chId) >= offsetY+iHeightDst);
    for(Int j=0; j<iHeightDst; j++)
    {
      Pel *pSrc = pSrcLine;
      for(Int i=0; i<iWidthDst; i++, pSrc -= iNumSamplesPerPixel )
      {
        if(insideFace(face, (iWidthDst-1-i)<<iScaleX, (iHeightDst-1-j)<<iScaleY, COMPONENT_Y, chId))
          pDstBuf[i] = ClipBD(((*pSrc) + iOffset) >> iBDAdjust, m_nOutputBitDepth);
        else
          pDstBuf[i] = emptyVal;
      }
      pDstBuf += iStrideDst;
      pSrcLine -= iStrideSrc;
    }
  }
  else if(rot == 270)
  {
    Int iWidthDst = iHeightSrc;
    Int iHeightDst = iWidthSrc;
    Pel *pSrcLine = pSrcBuf + (iHeightSrc-1)*iStrideSrc;

    assert(pcPicYuvDst->getWidth(chId) >= offsetX+iWidthDst);
    assert(pcPicYuvDst->getHeight(chId) >= offsetY+iHeightDst);
    for(Int j=0; j<iHeightDst; j++)
    {
      Pel *pSrc = pSrcLine;
      for(Int i=0; i<iWidthDst; i++, pSrc -= iStrideSrc)
      {
        if(insideFace(face, j<<iScaleX, (iWidthDst-1-i)<<iScaleY, COMPONENT_Y, chId))
          pDstBuf[i] = ClipBD(((*pSrc) + iOffset) >> iBDAdjust, m_nOutputBitDepth);
        else
          pDstBuf[i] = emptyVal;
      }
      pDstBuf += iStrideDst;
      pSrcLine += iNumSamplesPerPixel;
    }
  }
  else
    assert(!"Not supported");
}

/*********************************************
for convertYuv and framePack in general;
*********************************************/
Void TGeometry::rotFaceChannelGeneral(Pel *pSrcBuf, Int iWidthSrc, Int iHeightSrc, Int iStrideSrc, Int nSPPSrc, Int rot, Pel *pDstBuf, Int iStrideDst, Int nSPPDst, Bool bInverse)
{
  if(bInverse)
  {
    rot = (360-rot)%360;
  }
  if(!rot)
  {
    Int iWidthDst = iWidthSrc;
    Int iHeightDst = iHeightSrc;
    Pel *pSrcLine = pSrcBuf;
    Pel *pDstLine = pDstBuf;
    for(Int j=0; j<iHeightDst; j++)
    {
      for(Int i=0; i<iWidthDst; i++)
      {
        pDstLine[i*nSPPDst] = pSrcLine[i*nSPPSrc];
      }
      pDstLine += iStrideDst;
      pSrcLine += iStrideSrc;
    }
  }
  else if(rot == 90)
  {
    Int iWidthDst = iHeightSrc;
    Int iHeightDst = iWidthSrc;
    Pel *pSrcLine = pSrcBuf + (iWidthSrc-1)*nSPPSrc;
    Pel *pDstLine = pDstBuf;
    for(Int j=0; j<iHeightDst; j++)
    {
      Pel *pSrc = pSrcLine;
      for(Int i=0; i<iWidthDst; i++, pSrc += iStrideSrc)
      {
        pDstLine[i*nSPPDst] = *pSrc;
      }
      pDstLine += iStrideDst;
      pSrcLine -= nSPPSrc;
    }
  }
  else if(rot == 180)
  {
    Int iWidthDst = iWidthSrc;
    Int iHeightDst = iHeightSrc;
    Pel *pSrcLine = pSrcBuf + (iHeightSrc-1)*iStrideSrc + (iWidthSrc-1)*nSPPSrc;
    Pel *pDstLine = pDstBuf;
    for(Int j=0; j<iHeightDst; j++)
    {
      Pel *pSrc = pSrcLine;
      for(Int i=0; i<iWidthDst; i++, pSrc -= nSPPSrc )
      {
        pDstLine[i*nSPPDst] = *pSrc;
      }
      pDstLine += iStrideDst;
      pSrcLine -= iStrideSrc;
    }
  }
  else if(rot == 270)
  {
    Int iWidthDst = iHeightSrc;
    Int iHeightDst = iWidthSrc;
    Pel *pSrcLine = pSrcBuf + (iHeightSrc-1)*iStrideSrc;
    Pel *pDstLine = pDstBuf;
    for(Int j=0; j<iHeightDst; j++)
    {
      Pel *pSrc = pSrcLine;
      for(Int i=0; i<iWidthDst; i++, pSrc -= iStrideSrc)
      {
        pDstLine[i*nSPPDst] = *pSrc;
      }
      pDstLine += iStrideDst;
      pSrcLine += nSPPSrc;
    }
  }
  else
    assert(!"Not supported");
}

Void TGeometry::rotate3D(SPos& sPos, Int rx, Int ry, Int rz)
{
  POSType x = sPos.x;
  POSType y = sPos.y;
  POSType z = sPos.z;
  if(rx)
  {
    POSType rcos = scos((POSType)(rx*S_PI/180.0));
    POSType rsin = ssin((POSType)(rx*S_PI/180.0));
    POSType t1 = rcos*y - rsin*z;
    POSType t2 = rsin*y + rcos*z;
    y = t1;
    z = t2;
  }
  if(ry)
  {
    POSType rcos = scos((POSType)(ry*S_PI/180.0));
    POSType rsin = ssin((POSType)(ry*S_PI/180.0));
    POSType t1 = rcos*x + rsin*z;
    POSType t2 = -rsin*x + rcos*z;
    x = t1;
    z = t2;
  }
  if(rz)
  {
    POSType rcos = scos((POSType)(rz*S_PI/180.0));
    POSType rsin = ssin((POSType)(rz*S_PI/180.0));
    POSType t1 = rcos*x - rsin*y;
    POSType t2 = rsin*x + rcos*y;
    x = t1;
    y = t2;
  }
  sPos.x = x;
  sPos.y = y;
  sPos.z = z;
}

Void TGeometry::fillRegion(TComPicYuv *pDstYuv, Int x, Int y, Int rot, Int iFaceWidth, Int iFaceHeight)
{
  Int iWidth, iHeight;
  if(rot==90 || rot==270)
  {
    iWidth = iFaceHeight;
    iHeight = iFaceWidth;
  }
  else
  {
    iWidth = iFaceWidth;
    iHeight = iFaceHeight;
  }
  
  for(Int ch=0; ch<pDstYuv->getNumberValidComponents(); ch++)
  {
    ComponentID chId = (ComponentID)ch;
    Int iWidthC = iWidth>>pDstYuv->getComponentScaleX(chId);
    Int iHeightC = iHeight>>pDstYuv->getComponentScaleY(chId);
    Int iXC = x>>pDstYuv->getComponentScaleX(chId);
    Int iYC = y>>pDstYuv->getComponentScaleY(chId);
    Int iStride = pDstYuv->getStride(chId);
    Pel *pDst = pDstYuv->getAddr(chId) + iYC*iStride + iXC;
    Pel val = (1<<(m_nOutputBitDepth-1));
    //Pel val = ch==0? 0: (1<<(m_nOutputBitDepth-1));
    for(Int j=0; j<iHeightC; j++)
    {
      for(Int i=0; i<iWidthC; i++)
        pDst[i] = val;
      pDst += iStride;
    }
  }
}

Void TGeometry::initInterpolation(Int *pInterpolateType)
{
  for(Int ch=CHANNEL_TYPE_LUMA; ch<MAX_NUM_CHANNEL_TYPE; ch++)
  {
    m_InterpolationType[ch] = (SInterpolationType)pInterpolateType[ch];
    switch(pInterpolateType[ch])
    {
      case SI_NN:
        m_interpolateWeight[ch] = &TGeometry::interpolate_nn_weight;
        m_iInterpFilterTaps[ch][0] = m_iInterpFilterTaps[ch][1] = 1;
        break;
      case SI_BILINEAR:
        m_interpolateWeight[ch] = &TGeometry::interpolate_bilinear_weight;
        m_iInterpFilterTaps[ch][0] = m_iInterpFilterTaps[ch][1] = 2;
        break;
      case SI_BICUBIC:
        m_interpolateWeight[ch] = &TGeometry::interpolate_bicubic_weight;
        m_iInterpFilterTaps[ch][0] = m_iInterpFilterTaps[ch][1] = 4;
        break;
      case SI_LANCZOS2:
      case SI_LANCZOS3:
        m_iLanczosParamA[ch] = (pInterpolateType[ch] == SI_LANCZOS2)? 2 : 3;        
        if(m_pfLanczosFltCoefLut[ch])
          delete[] m_pfLanczosFltCoefLut[ch];
        m_pfLanczosFltCoefLut[ch] = new POSType[(m_iLanczosParamA[ch]<<1)*S_LANCZOS_LUT_SCALE+1];//Brave:filterCoefArray  base on windows size
        memset(m_pfLanczosFltCoefLut[ch], 0, sizeof(POSType)*((m_iLanczosParamA[ch]<<1)*S_LANCZOS_LUT_SCALE+1));
        for(Int i=0; i<(m_iLanczosParamA[ch]<<1)*S_LANCZOS_LUT_SCALE; i++)//Brave:calculate base Function's value,1 for 100 samping
        {
          Double x = (Double)i / S_LANCZOS_LUT_SCALE - m_iLanczosParamA[ch];
          m_pfLanczosFltCoefLut[ch][i] = (POSType)(sinc(x) * sinc(x / m_iLanczosParamA[ch]));
        }
        m_interpolateWeight[ch] = &TGeometry::interpolate_lanczos_weight;
        m_iInterpFilterTaps[ch][0] = m_iInterpFilterTaps[ch][1] = m_iLanczosParamA[ch]*2;
        break;
      default:
        assert(!"Not supported yet!");
        break;
    }
  }
}

#if SVIDEO_SPSNR_I
Pel TGeometry::getPelValue(ComponentID chId, SPos inPos)
{
  Int sum =0;
  ChannelType chType = toChannelType(chId);
  Int iWidthPW = getStride(chId);
  Int iWeightMapFaceMask = (1<<m_WeightMap_NumOfBits4Faces)-1;
  Pel pVal;
  Int iBDPrecision = S_INTERPOLATE_PrecisionBD;
  Int iOffset = 1<<(iBDPrecision-1);
  //Int x, y;
  PxlFltLut wList;

  (this->*m_interpolateWeight[chType])(chId, &inPos, wList);

  Int face = (wList.facePos)&iWeightMapFaceMask;
  Int iTLPos = (wList.facePos)>>m_WeightMap_NumOfBits4Faces;
  Int iWLutIdx = (m_chromaFormatIDC==CHROMA_400 || (m_InterpolationType[0]==m_InterpolationType[1]))? 0 : chType;
  Int *pWLut = m_pWeightLut[iWLutIdx][wList.weightIdx];
  Pel *pPelLine = m_pFacesOrig[face][chId] +iTLPos -((m_iInterpFilterTaps[chType][1]-1)>>1)*iWidthPW -((m_iInterpFilterTaps[chType][0]-1)>>1);

  for(Int m=0; m<m_iInterpFilterTaps[chType][1]; m++)
  {
    for(Int n=0; n<m_iInterpFilterTaps[chType][0]; n++)
      sum += pPelLine[n]*pWLut[n];
    pPelLine += iWidthPW;
    pWLut += m_iInterpFilterTaps[chType][0];
  }
  
  pVal = (sum + iOffset)>>iBDPrecision;
  
  
  return pVal;
}
#endif

Void TGeometry::setChromaResamplingFilter(Int iChromaSampleLocType)
{
  m_iChromaSampleLocType = iChromaSampleLocType;
  if(m_iChromaSampleLocType == 0)
  {
    //vertical 0.5 shift, horizontal 0 shift;
    //444->420;  444->422, H:[1, 6, 1]; 422->420, V[1,1];
    m_filterDs[0].nTaps = 3;
    m_filterDs[0].nlog2Norm = 3;
    m_filterDs[0].iFilterCoeff[0] = 1;
    m_filterDs[0].iFilterCoeff[1] = 6;
    m_filterDs[0].iFilterCoeff[2] = 1;
    m_filterDs[1].nTaps = 2;
    m_filterDs[1].nlog2Norm = 1;
    m_filterDs[1].iFilterCoeff[0] = 1;
    m_filterDs[1].iFilterCoeff[1] = 1;

    //horizontal filtering; [64] [-4, 36, 36, -4]; vertical [-2, 16, 54, -4]; [-4, 54, 16, -2];
    m_filterUps[0].nTaps = 1;
    m_filterUps[0].nlog2Norm = 6;
    m_filterUps[0].iFilterCoeff[0] = 64;
    // { -1, 4, -11, 40, 40, -11,  4, -1 }, 
    m_filterUps[1].nTaps = 8;
    m_filterUps[1].nlog2Norm = 6;
    m_filterUps[1].iFilterCoeff[0] = -1;
    m_filterUps[1].iFilterCoeff[1] = 4;
    m_filterUps[1].iFilterCoeff[2] = -11;
    m_filterUps[1].iFilterCoeff[3] = 40;
    m_filterUps[1].iFilterCoeff[4] = 40;
    m_filterUps[1].iFilterCoeff[5] = -11;
    m_filterUps[1].iFilterCoeff[6] =  4;
    m_filterUps[1].iFilterCoeff[7] = -1;

    //{  0, 1,  -5, 17, 58, -10, 4, -1 }
    m_filterUps[2].nTaps = 8;
    m_filterUps[2].nlog2Norm = 6;
    m_filterUps[2].iFilterCoeff[0] = 0;
    m_filterUps[2].iFilterCoeff[1] = 1;
    m_filterUps[2].iFilterCoeff[2] = -5;
    m_filterUps[2].iFilterCoeff[3] = 17;
    m_filterUps[2].iFilterCoeff[4] = 58;
    m_filterUps[2].iFilterCoeff[5] = -10;
    m_filterUps[2].iFilterCoeff[6] = 4;
    m_filterUps[2].iFilterCoeff[7] = -1;
    //{ -1, 4, -10, 58, 17,  -5, 1,  0 },
    m_filterUps[3].nTaps = 8;
    m_filterUps[3].nlog2Norm = 6;
    m_filterUps[3].iFilterCoeff[0] = -1;
    m_filterUps[3].iFilterCoeff[1] =  4;
    m_filterUps[3].iFilterCoeff[2] = -10;
    m_filterUps[3].iFilterCoeff[3] = 58;
    m_filterUps[3].iFilterCoeff[4] = 17;
    m_filterUps[3].iFilterCoeff[5] = -5;
    m_filterUps[3].iFilterCoeff[6] =  1;
    m_filterUps[3].iFilterCoeff[7] =  0;
  }
  else if(m_iChromaSampleLocType == 2)
  {
    //vertical 0 shift, horizontal 0 shift;
    //444->420;  444->422, H:[1, 6, 1]; 422->420, V:[1, 6, 1];
    m_filterDs[0].nTaps = 1;
    m_filterDs[0].nlog2Norm = 0;
    m_filterDs[0].iFilterCoeff[0] = 1;
    m_filterDs[1].nTaps = 1;
    m_filterDs[1].nlog2Norm = 0;
    m_filterDs[1].iFilterCoeff[0] = 1;

    //horizontal filtering; [64], [-4, 36, 36, -4]; vertical [64], [-4, 36, 36, -4];
    m_filterUps[0].nTaps = 1;
    m_filterUps[0].nlog2Norm = 6;
    m_filterUps[0].iFilterCoeff[0] = 64;

    m_filterUps[1].nTaps = 8;
    m_filterUps[1].nlog2Norm = 6;
    m_filterUps[1].iFilterCoeff[0] = -1;
    m_filterUps[1].iFilterCoeff[1] = 4;
    m_filterUps[1].iFilterCoeff[2] = -11;
    m_filterUps[1].iFilterCoeff[3] = 40;
    m_filterUps[1].iFilterCoeff[4] = 40;
    m_filterUps[1].iFilterCoeff[5] = -11;
    m_filterUps[1].iFilterCoeff[6] =  4;
    m_filterUps[1].iFilterCoeff[7] = -1;

    m_filterUps[2].nTaps = 1;
    m_filterUps[2].nlog2Norm = 6;
    m_filterUps[2].iFilterCoeff[0] = 64;
    memcpy(m_filterUps+3, m_filterUps+1, sizeof(m_filterUps[3]));
  }
  else if(m_iChromaSampleLocType == 1)
  {
    //vertical 0.5 shift, horizontal 0.5 shift;
    //444->420;  444->422, H:[1, 1]; 422->420, V[1,1];
    m_filterDs[0].nTaps = 2;
    m_filterDs[0].nlog2Norm = 1;
    m_filterDs[0].iFilterCoeff[0] = 1;
    m_filterDs[0].iFilterCoeff[1] = 1;
    m_filterDs[1].nTaps = 2;
    m_filterDs[1].nlog2Norm = 1;
    m_filterDs[1].iFilterCoeff[0] = 1;
    m_filterDs[1].iFilterCoeff[1] = 1;
    //horizontal filtering; [-2, 16, 54, -4], [-4, 54, 16, -2]; vertical [-2, 16, 54, -4]; [-4, 54, 16, -2];
    m_filterUps[0].nTaps = 4;
    m_filterUps[0].nlog2Norm = 6;
    m_filterUps[0].iFilterCoeff[0] = -2;
    m_filterUps[0].iFilterCoeff[1] = 16;
    m_filterUps[0].iFilterCoeff[2] = 54;
    m_filterUps[0].iFilterCoeff[3] = -4;
    m_filterUps[1].nTaps = 4;
    m_filterUps[1].nlog2Norm = 6;
    m_filterUps[1].iFilterCoeff[0] = -4;
    m_filterUps[1].iFilterCoeff[1] = 54;
    m_filterUps[1].iFilterCoeff[2] = 16;
    m_filterUps[1].iFilterCoeff[3] = -2;
    m_filterUps[2].nTaps = 4;
    m_filterUps[2].nlog2Norm = 6;
    m_filterUps[2].iFilterCoeff[0] = -2;
    m_filterUps[2].iFilterCoeff[1] = 16;
    m_filterUps[2].iFilterCoeff[2] = 54;
    m_filterUps[2].iFilterCoeff[3] = -4;
    m_filterUps[3].nTaps = 4;
    m_filterUps[3].nlog2Norm = 6;
    m_filterUps[3].iFilterCoeff[0] = -4;
    m_filterUps[3].iFilterCoeff[1] = 54;
    m_filterUps[3].iFilterCoeff[2] = 16;
    m_filterUps[3].iFilterCoeff[3] = -2;
  }
  else //if(m_iChromaSampleLocType == 3)
  {
    //vertical 0 shift, horizontal 0.5 shift;
    //444->420;  444->422, H:[1,1]; 422->420, V[1, 6, 1];
    m_filterDs[0].nTaps = 2;
    m_filterDs[0].nlog2Norm = 1;
    m_filterDs[0].iFilterCoeff[0] = 1;
    m_filterDs[0].iFilterCoeff[1] = 1;
    m_filterDs[1].nTaps = 3;
    m_filterDs[1].nlog2Norm = 3;
    m_filterDs[1].iFilterCoeff[0] = 1;
    m_filterDs[1].iFilterCoeff[1] = 6;
    m_filterDs[1].iFilterCoeff[2] = 1;
    //horizontal filtering [-2, 16, 54, -4]; [-4, 54, 16, -2] ; vertical [64]; [-4, 36, 36, -4];
    m_filterUps[0].nTaps = 4;
    m_filterUps[0].nlog2Norm = 6;
    m_filterUps[0].iFilterCoeff[0] = -2;
    m_filterUps[0].iFilterCoeff[1] = 16;
    m_filterUps[0].iFilterCoeff[2] = 54;
    m_filterUps[0].iFilterCoeff[3] = -4;
    m_filterUps[1].nTaps = 4;
    m_filterUps[1].nlog2Norm = 6;
    m_filterUps[1].iFilterCoeff[0] = -4;
    m_filterUps[1].iFilterCoeff[1] = 54;
    m_filterUps[1].iFilterCoeff[2] = 16;
    m_filterUps[1].iFilterCoeff[3] = -2;
    m_filterUps[2].nTaps = 1;
    m_filterUps[2].nlog2Norm = 6;
    m_filterUps[2].iFilterCoeff[0] = 64;
    m_filterUps[3].nTaps = 4;
    m_filterUps[3].nlog2Norm = 6;
    m_filterUps[3].iFilterCoeff[0] = -4;
    m_filterUps[3].iFilterCoeff[1] = 36;
    m_filterUps[3].iFilterCoeff[2] = 36;
    m_filterUps[3].iFilterCoeff[3] = -4;
  }
}

//this function should be overwritten by those gemoetries with triangle faces;
Bool TGeometry::validPosition4Interp(ComponentID chId, POSType x, POSType y)
{
  Int iWidth = m_sVideoInfo.iFaceWidth >> getComponentScaleX(chId);
  Int iHeight = m_sVideoInfo.iFaceHeight >> getComponentScaleY(chId);
  ChannelType chType = toChannelType(chId);
  POSType fLimit;
  if(m_InterpolationType[chType] == SI_NN)
    fLimit = (POSType)0.499;
  else if(m_InterpolationType[chType] == SI_BILINEAR)
    fLimit = 0;
  else if(m_InterpolationType[chType] == SI_BICUBIC || m_InterpolationType[chType] == SI_LANCZOS2)
    fLimit = -1.0;
  else if(m_InterpolationType[chType] == SI_LANCZOS3)
    fLimit = -2.0;
  else
    assert(!"Not supported\n");

  if( x>=-fLimit && x<iWidth-1+fLimit && y>=-fLimit && y<iHeight-1+fLimit)
    return true;
  else
    return false;
}

/***************************************************
map faceId to (row, col) in frame packing structure;
***************************************************/
Void TGeometry::parseFacePos(Int *pFacePos)
{
  Int* pCurrFacePos = pFacePos;
  Int iFound=0;
  FaceProperty (*pFaceProp)[12] = m_sVideoInfo.framePackStruct.faces;
  Int iTotalNumOfFaces = m_sVideoInfo.framePackStruct.rows * m_sVideoInfo.framePackStruct.cols;
  assert(m_sVideoInfo.iNumFaces <= iTotalNumOfFaces);
  
  for(Int faceId=0; faceId<iTotalNumOfFaces; faceId++)
  {
    pCurrFacePos = pFacePos + faceId*2;
    for(Int j=0; j<m_sVideoInfo.framePackStruct.rows; j++)
    {
      Int i=0;
      for(; i<m_sVideoInfo.framePackStruct.cols; i++)
      {
        if(pFaceProp[j][i].id == faceId)
        {
          *pCurrFacePos++ = j;
          *pCurrFacePos = i;
          iFound++;
          break;
        }
      }
      if(i <m_sVideoInfo.framePackStruct.cols)
         break;
    }
  }
  assert(iFound >= m_sVideoInfo.iNumFaces);
}

//for debug;
Void TGeometry::dumpAllFacesToFile(Char *pPrefixFN, Bool bMarginIncluded, Bool bAppended)
{
  for(Int face=0; face<m_sVideoInfo.iNumFaces; face++)
  {
    Int iTotalWidth = m_sVideoInfo.iFaceWidth + (bMarginIncluded? (m_iMarginX<<1): 0);
    Int iTotalHeight = m_sVideoInfo.iFaceHeight + (bMarginIncluded? (m_iMarginY<<1): 0);
    Char fileName[256];
    sprintf(fileName, "%s_%dx%dx%d_f%d_cf%d.yuv", pPrefixFN, iTotalWidth, iTotalHeight, m_nBitDepth, face, (Int)m_chromaFormatIDC);
    FILE *fp = fopen(fileName, bAppended? "ab":"wb");
    assert(fp);
    for(Int ch=0; ch<getNumChannels(); ch++)
    {
      ComponentID chId = (ComponentID)ch;
      Int iWidth = iTotalWidth >> getComponentScaleX(chId);
      Int iHeight = iTotalHeight >> getComponentScaleY(chId);
      Int iOffsetX = -(bMarginIncluded? (m_iMarginX >>getComponentScaleX(chId)) :0);
      Int iOffsetY = -(bMarginIncluded? (m_iMarginY >>getComponentScaleY(chId)) :0);
      Int iStride = getStride(chId);   
      dumpBufToFile(m_pFacesOrig[face][ch] + iOffsetY*iStride + iOffsetX, iWidth, iHeight, 1, iStride, fp);
    }
    fclose(fp);
  }
}
Void TGeometry::dumpBufToFile(Pel *pSrc, Int iWidth, Int iHeight, Int iNumSamples, Int iStride, FILE *fp)
{
  assert(fp && pSrc);
  //output the 16bit format;
  if(fp)
  {
    Pel *pSrcLine = pSrc;
    for(Int j=0; j<iHeight; j++)
    {
      for(Int i=0; i<iWidth; i++)
      {
        fwrite(pSrcLine+i*iNumSamples, sizeof(Pel), 1, fp);
      }
      pSrcLine += iStride;
    }
  }
}

//dump the sampling points on the sphere either to the file, or to the memory for analysis, or both;
Void TGeometry::dumpSpherePoints(Char *pFileName, Bool bAppended, SpherePoints *pSphPoints)
{
  if(!pFileName && !pSphPoints)
    return;

  FILE *fp = NULL;
  if(pFileName)
    fp = fopen(pFileName, bAppended? "a":"w");

  //For ViewPort, Set Rotation Matrix and K matrix
  if (m_sVideoInfo.geoType==SVIDEO_VIEWPORT)
  {
    ((TViewPort*)this)->setRotMat();
    ((TViewPort*)this)->setInvK();
  }
  Int *pRot = m_sVideoInfo.sVideoRotation.degree;
  //dump the points on the sphere (luma component);
  Int iTotalNumOfPoints = 0;
  for(Int fIdx=0; fIdx<m_sVideoInfo.iNumFaces; fIdx++)
  {
    for(Int ch=0; ch<1; ch++)
    {
      ComponentID chId = (ComponentID)ch;
      Int iWidth = m_sVideoInfo.iFaceWidth>>getComponentScaleX(chId);
      Int iHeight = m_sVideoInfo.iFaceHeight>>getComponentScaleY(chId);
      for(Int j=0; j<iHeight; j++) 
        for(Int i=0; i<iWidth; i++)  
        {
          if(insideFace(fIdx, (i<<getComponentScaleX(chId)), (j<<getComponentScaleY(chId)), COMPONENT_Y, chId))
            iTotalNumOfPoints ++;
        }
    }
  }
  if(fp)
    fprintf(fp, "%d\n", iTotalNumOfPoints);
  Int iIdx = 0;
  Double (*pPos)[2] = NULL;
  if(pSphPoints)
  {
    pSphPoints->iNumOfPoints = iTotalNumOfPoints;
    pSphPoints->pPointPos = new Double[iTotalNumOfPoints][2];
    pPos = pSphPoints->pPointPos;
  }
    
  for(Int fIdx=0; fIdx<m_sVideoInfo.iNumFaces; fIdx++)
  {
    for(Int ch=0; ch<1; ch++)
    {
      ComponentID chId = (ComponentID)ch;
      Int iWidth = m_sVideoInfo.iFaceWidth>>getComponentScaleX(chId);
      Int iHeight = m_sVideoInfo.iFaceHeight>>getComponentScaleY(chId);
      for(Int j=0; j<iHeight; j++) 
        for(Int i=0; i<iWidth; i++)  
        {
          if(!insideFace(fIdx, (i<<getComponentScaleX(chId)), (j<<getComponentScaleY(chId)), COMPONENT_Y, chId))
            continue;

          SPos in(fIdx, i, j, 0), pos3D;
          map2DTo3D(in, &pos3D);
          rotate3D(pos3D, pRot[0], pRot[1], pRot[2]);
          Double x = pos3D.x;
          Double y = pos3D.y;
          Double z = pos3D.z;

          //yaw;
          Double yaw = (-satan2(z, x))*180.0/S_PI;
          Double len = ssqrt(x*x + y*y + z*z);
          //pitch;
          Double pitch = 90.0 - (POSType)((len < S_EPS? 0.5 : sacos(y/len)/S_PI)*180.0);
          if(fp)
            fprintf(fp, "%.6lf %.6lf\n", pitch, yaw);
          if(pPos)
          {
            pPos[iIdx][0] = pitch;
            pPos[iIdx][1] = yaw;
          }
          iIdx++;
        }
    }
  }
  if(fp)
    fclose(fp);  
}

//vector multiplication;
Void TGeometry::vecMul(const POSType *v0, const POSType *v1, POSType *nVec, Int bNormalized)
{
  nVec[0] = v0[1]*v1[2] - v0[2]*v1[1];
  nVec[1] = -v0[0]*v1[2] + v0[2]*v1[0];
  nVec[2] = v0[0]*v1[1] - v0[1]*v1[0];
  if(bNormalized)
  {
    POSType d = ssqrt(nVec[0]*nVec[0] + nVec[1]*nVec[1] + nVec[2]*nVec[2]);
    nVec[0] /= d;
    nVec[1] /= d;
    nVec[2] /= d;
  }
}

Void TGeometry::initTriMesh(TriMesh& meshFace)
{
  //calculate the origin;
  POSType (*p)[3] = meshFace.vertex;
  for(Int i=0; i<3; i++)
    meshFace.origin[i]  = p[0][i] + 0.5f*(p[1][i]-p[2][i]);
  //caculate the base;
  for(Int j=0; j<2; j++)
  {
    POSType d=0;
    for(Int i=0; i<3; i++)
    {
      meshFace.baseVec[j][i] = p[j][i] - meshFace.origin[i];
      d += meshFace.baseVec[j][i]*meshFace.baseVec[j][i];
    }
    d = ssqrt(d);
    meshFace.baseVec[j][0] /= d;
    meshFace.baseVec[j][1] /= d;
    meshFace.baseVec[j][2] /= d;
  }
  //calc the norm;
  vecMul(meshFace.baseVec[1], meshFace.baseVec[0], meshFace.normVec, 1); 
}

#endif