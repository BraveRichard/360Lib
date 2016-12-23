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

/** \file     TSPSNRMetricCalc.cpp
    \brief    SPSNRMetric class
*/

#include "TSPSNRMetricCalc.h"

#if SVIDEO_SPSNR_NN

TSPSNRMetric::TSPSNRMetric()
: m_bSPSNREnabled(false)
, m_pCart2D(NULL)
, m_fpTable(NULL)
{
  m_dSPSNR[0] = m_dSPSNR[1] = m_dSPSNR[2] = 0;
}

TSPSNRMetric::~TSPSNRMetric()
{
  if(m_pCart2D)
  {
    free(m_pCart2D); m_pCart2D = NULL;
  }
  if (m_fpTable)
  {
    free(m_fpTable); m_fpTable = NULL;
  }
}

Void TSPSNRMetric::setOutputBitDepth(Int iOutputBitDepth[MAX_NUM_CHANNEL_TYPE])
{
  for(Int i = 0; i < MAX_NUM_CHANNEL_TYPE; i++)
  {
    m_outputBitDepth[i] = iOutputBitDepth[i];
  }
}

Void TSPSNRMetric::setReferenceBitDepth(Int iReferenceBitDepth[MAX_NUM_CHANNEL_TYPE])
{
  for(Int i = 0; i < MAX_NUM_CHANNEL_TYPE; i++)
  {
    m_referenceBitDepth[i] = iReferenceBitDepth[i];
  }
}

Void TSPSNRMetric::sphSampoints(Char* cSphDataFile)
{
  if(cSphDataFile == NULL)
  {
    m_bSPSNREnabled = false;
    return;
  }

  // read data
  FILE *fp = fopen(cSphDataFile,"r");
  if(!fp)
  {
    printf("SPSNR-NN is disabled because metadata file (%s) cannot be opened!\n", cSphDataFile);
    m_bSPSNREnabled = false;
    return;
  }

  if(fscanf(fp, "%d ", &m_iSphNumPoints)!=1)
  { 
    printf("SphData file does not exist.\n"); 
    exit(EXIT_FAILURE); 
    fclose(fp);
  }

  m_pCart2D = (CPos2D*)malloc(sizeof(CPos2D)*(m_iSphNumPoints));
  memset(m_pCart2D,0,sizeof(CPos2D)*(m_iSphNumPoints));

  for(Int z = 0; z < m_iSphNumPoints; z++)
  {
    if(fscanf(fp, "%lf %lf", &m_pCart2D[z].x, &m_pCart2D[z].y)!=2)
    { 
      printf("Format error SphData in sphSampoints().\n"); 
      exit(EXIT_FAILURE); 
    }
  }
}

void TSPSNRMetric::sphToCart(CPos2D* sph, CPos3D* out)
{
  POSType fLat = (POSType)(sph->x*S_PI/180.0);
  POSType fLon = (POSType)(sph->y*S_PI/180.0);

  out->x =  ssin(fLon) * scos(fLat);
  out->y =               ssin(fLat);
  out->z = -scos(fLon) * scos(fLat);
}

void TSPSNRMetric::createTable(TComPicYuv* pcPicD, TGeometry *pcCodingGeomtry)
{
  if(!m_bSPSNREnabled)
  {
    return;
  }

  Int iNumPoints = m_iSphNumPoints;
  CPos2D In2d;
  CPos3D Out3d;
  SPos posIn, posOut;
  m_fpTable  = (IPos2D*)malloc(iNumPoints*sizeof(IPos2D));

  for (Int np=0; np < iNumPoints; np++)
  {
    In2d.x=m_pCart2D[np].x;
    In2d.y=m_pCart2D[np].y;

    //get cartesian coordinates
    sphToCart(&In2d, &Out3d);
    posIn.x = Out3d.x; posIn.y = Out3d.y; posIn.z = Out3d.z;
    pcCodingGeomtry->map3DTo2D(&posIn, &posOut);

    posOut.x = (POSType)round(posOut.x);
    posOut.y = (POSType)round(posOut.y);
    IPos tmpPos;
    tmpPos.faceIdx = posOut.faceIdx;
    tmpPos.u = (Int)(posOut.x);
    tmpPos.v = (Int)(posOut.y);
    pcCodingGeomtry->clamp(&tmpPos);
    pcCodingGeomtry->geoToFramePack(&tmpPos, &m_fpTable[np]);
  }
}

Void TSPSNRMetric::xCalculateSPSNR( TComPicYuv* pcOrgPicYuv, TComPicYuv* pcPicD )
{
  Int iNumPoints = m_iSphNumPoints;
  Int iBitDepthForPSNRCalc[MAX_NUM_CHANNEL_TYPE];
  Int iReferenceBitShift[MAX_NUM_CHANNEL_TYPE];
  Int iOutputBitShift[MAX_NUM_CHANNEL_TYPE];
  iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] = std::max(m_outputBitDepth[CHANNEL_TYPE_LUMA], m_referenceBitDepth[CHANNEL_TYPE_LUMA]);
  iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] = std::max(m_outputBitDepth[CHANNEL_TYPE_CHROMA], m_referenceBitDepth[CHANNEL_TYPE_CHROMA]);
  iReferenceBitShift[CHANNEL_TYPE_LUMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] - m_referenceBitDepth[CHANNEL_TYPE_LUMA];
  iReferenceBitShift[CHANNEL_TYPE_CHROMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] - m_referenceBitDepth[CHANNEL_TYPE_CHROMA];
  iOutputBitShift[CHANNEL_TYPE_LUMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] - m_outputBitDepth[CHANNEL_TYPE_LUMA];
  iOutputBitShift[CHANNEL_TYPE_CHROMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] - m_outputBitDepth[CHANNEL_TYPE_CHROMA];

  memset(m_dSPSNR, 0, sizeof(Double)*3);
  TComPicYuv &picd=*pcPicD;
  Double SSDspsnr[3]={0, 0 ,0};

  for(Int chan=0; chan<pcPicD->getNumberValidComponents(); chan++)
  {
    const ComponentID ch=ComponentID(chan);
    const Pel*  pOrg       = pcOrgPicYuv->getAddr(ch);
    const Int   iOrgStride = pcOrgPicYuv->getStride(ch);
    const Pel*  pRec       = picd.getAddr(ch);
    const Int   iRecStride = picd.getStride(ch);

    for (Int np = 0; np < iNumPoints; np++)
    {
      if (!chan)
      {
        Int x_loc=(Int)(m_fpTable[np].x);
        Int y_loc=(Int)(m_fpTable[np].y);
        Intermediate_Int iDifflp=  (pOrg[x_loc+(y_loc*iOrgStride)]<<iReferenceBitShift[toChannelType(ch)]) - (pRec[x_loc+(y_loc*iRecStride)]<<iOutputBitShift[toChannelType(ch)]);
        SSDspsnr[chan]+= iDifflp*iDifflp;
      }
      else
      {
        Int x_loc = Int(m_fpTable[np].x/2);
        Int y_loc = Int(m_fpTable[np].y/2);
        Intermediate_Int iDifflp=  (pOrg[x_loc+(y_loc*iOrgStride)]<<iReferenceBitShift[toChannelType(ch)]) - (pRec[x_loc+(y_loc*iRecStride)]<<iOutputBitShift[toChannelType(ch)]);
        SSDspsnr[chan]+= iDifflp*iDifflp;
      }
    }
  }

  for (Int ch_indx = 0; ch_indx < pcPicD->getNumberValidComponents(); ch_indx++)
  {
    const ComponentID ch=ComponentID(ch_indx);
    const Int maxval = 255<<(iBitDepthForPSNRCalc[toChannelType(ch)]-8) ;

    Double fReflpsnr = Double(iNumPoints)*maxval*maxval;
    m_dSPSNR[ch_indx] = ( SSDspsnr[ch_indx] ? 10.0 * log10( fReflpsnr / (Double)SSDspsnr[ch_indx] ) : 999.99 );
  }
}

#endif