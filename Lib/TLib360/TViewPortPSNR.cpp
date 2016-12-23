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

/** \file     TViewPortPSNR.cpp
    \brief    ViewPortPSNR class
*/

#include <assert.h>
#include <math.h>
#include "TViewPortPSNR.h"

#if SVIDEO_VIEWPORT_PSNR
TViewPortPSNR::TViewPortPSNR() 
: m_pRefGeometry(NULL)
, m_pRefViewPortList(NULL)
, m_pRecGeometry(NULL)
, m_pRecViewPortList(NULL)
, m_pcOrgPicYuv(NULL)
, m_pRefViewPortYuv(NULL)
, m_pRecViewPortYuv(NULL)
, m_pdPSNRSum(NULL)
, m_pdMSESum(NULL)
, m_pcTVideoIOYuvInputFile(NULL)
, m_iLastFrmPOC(0)
, m_temporalSubsampleRatio(1)
{
  m_viewPortPSNRParam.bViewPortPSNREnabled = false;
  m_viewPortPSNRParam.viewPortSettingsList.clear();
}

TViewPortPSNR::~TViewPortPSNR()  
{
  if(m_pRefGeometry)
  {
    delete m_pRefGeometry;
    m_pRefGeometry = NULL;
  }
  if(m_pRecGeometry)
  {
    delete m_pRecGeometry;
    m_pRecGeometry = NULL;
  }
  Int iNumViewPorts = (Int)m_viewPortPSNRParam.viewPortSettingsList.size();
  for(Int i=0; i<iNumViewPorts; i++)
  {
    if(m_pRefViewPortList[i])
    {
      delete m_pRefViewPortList[i];
      m_pRefViewPortList[i] = NULL;
    }
    if(m_pRecViewPortList[i])
    {
      delete m_pRecViewPortList[i];
      m_pRecViewPortList[i] = NULL;
    }
  }
  if(m_pRefViewPortList)
  {
    delete[] m_pRefViewPortList;
    m_pRefViewPortList = NULL;
  }
  if(m_pRecViewPortList)
  {
    delete[] m_pRecViewPortList;
    m_pRecViewPortList = NULL;
  }
  if(m_pcOrgPicYuv)
  {
    m_pcOrgPicYuv->destroy();
    delete m_pcOrgPicYuv;
  }
  if(m_pRefViewPortYuv)
  {
    m_pRefViewPortYuv->destroy();
    delete m_pRefViewPortYuv;
    m_pRefViewPortYuv = NULL;
  }
  if(m_pRecViewPortYuv)
  {
    m_pRecViewPortYuv->destroy();
    delete m_pRecViewPortYuv;
    m_pRecViewPortYuv = NULL;
  }

  if(m_pdPSNRSum)
  {
    delete[] m_pdPSNRSum;
    m_pdPSNRSum = NULL;
  }
  if(m_pdMSESum)
  {
    delete[] m_pdMSESum;
    m_pdMSESum = NULL;
  }
  m_pcTVideoIOYuvInputFile = NULL;
}

Void TViewPortPSNR::init(SVideoInfo& sRefVideoInfo, SVideoInfo& sRecVideoInfo, InputGeoParam *pInGeoParam, ViewPortPSNRParam& param, TVideoIOYuv& yuvInputFile, Int iInputWidth, Int iInputHeight, UInt tempSubsampleRatio)
{
  m_viewPortPSNRParam = param;
  if(m_viewPortPSNRParam.bViewPortPSNREnabled)
  {
    Int iNumViewPorts = (Int)m_viewPortPSNRParam.viewPortSettingsList.size();
    m_pRefGeometry = TGeometry::create(sRefVideoInfo, pInGeoParam);
    m_pRecGeometry = TGeometry::create(sRecVideoInfo, pInGeoParam);
    m_pRefViewPortList = new TGeometry*[iNumViewPorts];
    m_pRecViewPortList = new TGeometry*[iNumViewPorts];
    SVideoInfo sViewPortInfo;
    memset(&sViewPortInfo, 0, sizeof(sViewPortInfo));
    sViewPortInfo.geoType = SVIDEO_VIEWPORT;
    sViewPortInfo.framePackStruct.chromaFormatIDC = sRecVideoInfo.framePackStruct.chromaFormatIDC;
    sViewPortInfo.framePackStruct.rows = sViewPortInfo.framePackStruct.cols = 1;
    sViewPortInfo.iNumFaces = 1;
    sViewPortInfo.iFaceWidth = m_viewPortPSNRParam.iViewPortWidth;
    sViewPortInfo.iFaceHeight = m_viewPortPSNRParam.iViewPortHeight;
    for(Int i=0; i<iNumViewPorts; i++)
    {
      sViewPortInfo.viewPort = m_viewPortPSNRParam.viewPortSettingsList[i];
      m_pRefViewPortList[i] = TGeometry::create(sViewPortInfo, pInGeoParam);
      m_pRecViewPortList[i] = TGeometry::create(sViewPortInfo, pInGeoParam);
    }
    m_pdPSNRSum = new Double[iNumViewPorts][3];
    memset(m_pdPSNRSum[0], 0, sizeof(Double)*iNumViewPorts*3); 
    m_pdMSESum = new Double[iNumViewPorts][3];
    memset(m_pdMSESum[0], 0, sizeof(Double)*iNumViewPorts*3);

    m_pRefViewPortYuv = new TComPicYuv;
    m_pRefViewPortYuv->createWithoutCUInfo( m_viewPortPSNRParam.iViewPortWidth, m_viewPortPSNRParam.iViewPortHeight, sViewPortInfo.framePackStruct.chromaFormatIDC, true, S_PAD_MAX, S_PAD_MAX);
    m_pRecViewPortYuv = new TComPicYuv;
    m_pRecViewPortYuv->createWithoutCUInfo( m_viewPortPSNRParam.iViewPortWidth, m_viewPortPSNRParam.iViewPortHeight, sViewPortInfo.framePackStruct.chromaFormatIDC, true, S_PAD_MAX, S_PAD_MAX);
    
    m_iInputWidth = iInputWidth;
    m_iInputHeight = iInputHeight;
    m_iViewPortBitDepth = pInGeoParam->nOutputBitDepth;
    m_iRefBitDepth = pInGeoParam->nOutputBitDepth;
    m_inputChromaFomat = sRefVideoInfo.framePackStruct.chromaFormatIDC;
    m_pcTVideoIOYuvInputFile = &yuvInputFile;
    m_pcOrgPicYuv = new TComPicYuv;
    m_pcOrgPicYuv->createWithoutCUInfo(iInputWidth, iInputHeight, m_inputChromaFomat, true, S_PAD_MAX, S_PAD_MAX);
    m_temporalSubsampleRatio = tempSubsampleRatio;
  }
}

Void TViewPortPSNR::xCalculatePSNRInternal(TComPicYuv *pcOrgPicYuv, TComPicYuv *pcPicD, Double *pdPSNR, Double *pdMSE)
{
  for(Int i=0; i<MAX_NUM_COMPONENT; i++)
  {
    pdPSNR[i] = pdMSE[i] = 0.0;
  }
  Int iBitDepthForPSNRCalc = std::max(m_iViewPortBitDepth, m_iRefBitDepth);
  Int iReferenceBitShift = iBitDepthForPSNRCalc - m_iRefBitDepth;
  Int iOutputBitShift = iBitDepthForPSNRCalc - m_iViewPortBitDepth;

  for(Int chan=0; chan<pcPicD->getNumberValidComponents(); chan++)
  {
    const ComponentID ch=ComponentID(chan);
    const Pel*  pOrg       = pcOrgPicYuv->getAddr(ch);
    const Int   iOrgStride = pcOrgPicYuv->getStride(ch);
    Pel*  pRec             = pcPicD->getAddr(ch);
    const Int   iRecStride = pcPicD->getStride(ch);
    const Int   iWidth  = pcPicD->getWidth (ch) ;
    const Int   iHeight = pcPicD->getHeight(ch) ;
    Int   iSize   = iWidth*iHeight;

    Double SSDpsnr=0;
    for(Int y = 0; y < iHeight; y++ )
    {
      for(Int x = 0; x < iWidth; x++ )
      {
        Intermediate_Int iDiff = (Intermediate_Int)( (pOrg[x]<<iReferenceBitShift) - (pRec[x]<<iOutputBitShift) );
        SSDpsnr += iDiff * iDiff;
      }
      pOrg += iOrgStride;
      pRec += iRecStride;
    }
    const Int maxval = 255<<(iBitDepthForPSNRCalc - 8) ;
    const Double fRefValue = (Double) maxval * maxval * iSize;
    pdPSNR[ch] = ( SSDpsnr ? 10.0 * log10( fRefValue / (Double)SSDpsnr ) : 999.99 );
    pdMSE[ch] = (Double)SSDpsnr/(iSize);
  }
}

Void TViewPortPSNR::calculateCombinedValues(Int vpIdx, UInt uiNumPics, Double &PSNRyuv, Double &MSEyuv)
{
  MSEyuv    = 0;
  Int scale = 0;
  Int maximumBitDepth = std::max(m_iViewPortBitDepth, m_iRefBitDepth);
  ChromaFormat chFmt = m_pRecGeometry->getSVideoInfo()->framePackStruct.chromaFormatIDC;
  const UInt maxval                = 255 << (maximumBitDepth - 8);
  const UInt numberValidComponents = getNumberValidComponents(chFmt);

  for (UInt comp=0; comp<numberValidComponents; comp++)
  {
    const ComponentID compID        = ComponentID(comp);
    const UInt        csx           = getComponentScaleX(compID, chFmt);
    const UInt        csy           = getComponentScaleY(compID, chFmt);
    const Int         scaleChan     = (4>>(csx+csy));
    const UInt        bitDepthShift = 0;

    const Double      channelMSE    = (m_pdMSESum[vpIdx][compID] * Double(1 << bitDepthShift)) / Double(uiNumPics);

    scale  += scaleChan;
    MSEyuv += scaleChan * channelMSE;
  }

  MSEyuv /= Double(scale);  // i.e. divide by 6 for 4:2:0, 8 for 4:2:2 etc.
  PSNRyuv = (MSEyuv==0 ? 999.99 : 10*log10((maxval*maxval)/MSEyuv));
}

Void TViewPortPSNR::xCalculatePSNR( TComPic* pcPic,  Double* pdPSNR)
{
  if(!m_viewPortPSNRParam.bViewPortPSNREnabled)
    return;

  Int iDeltaFrames = pcPic->getPOC()*m_temporalSubsampleRatio - m_iLastFrmPOC;
  Int aiPad[2]={0,0};
  m_pcTVideoIOYuvInputFile->skipFrames(iDeltaFrames, m_iInputWidth, m_iInputHeight, m_inputChromaFomat);
  m_pcTVideoIOYuvInputFile->read(NULL, m_pcOrgPicYuv, IPCOLOURSPACE_UNCHANGED, aiPad, m_inputChromaFomat, false );
  m_iLastFrmPOC = pcPic->getPOC()*m_temporalSubsampleRatio+1;

  Int iNumOfViewPorts = (Int)m_viewPortPSNRParam.viewPortSettingsList.size(); 

  if((m_pRefGeometry->getType() == SVIDEO_OCTAHEDRON || m_pRefGeometry->getType() == SVIDEO_ICOSAHEDRON) && m_pRefGeometry->getSVideoInfo()->iCompactFPStructure) 
    m_pRefGeometry->compactFramePackConvertYuv(m_pcOrgPicYuv);
  else
    m_pRefGeometry->convertYuv(m_pcOrgPicYuv);

  TComPicYuv *pRecPicYuv = pcPic->getPicYuvRec();
  if((m_pRecGeometry->getType() == SVIDEO_OCTAHEDRON || m_pRecGeometry->getType() == SVIDEO_ICOSAHEDRON) && m_pRecGeometry->getSVideoInfo()->iCompactFPStructure) 
    m_pRecGeometry->compactFramePackConvertYuv(pRecPicYuv);
  else
    m_pRecGeometry->convertYuv(pRecPicYuv);

  for(Int i=0; i<iNumOfViewPorts; i++)
  {
    Double dPSNR[MAX_NUM_COMPONENT];
    Double dMSE[MAX_NUM_COMPONENT];

    //generate reference viewport;
    m_pRefGeometry->geoConvert(m_pRefViewPortList[i]);
    if((m_pRefViewPortList[i]->getType() == SVIDEO_OCTAHEDRON || m_pRefViewPortList[i]->getType() == SVIDEO_ICOSAHEDRON) && m_pRefViewPortList[i]->getSVideoInfo()->iCompactFPStructure)
      m_pRefViewPortList[i]->compactFramePack(m_pRefViewPortYuv);
    else
      m_pRefViewPortList[i]->framePack(m_pRefViewPortYuv);
    
    //generate reconstructed viewport;
    m_pRecGeometry->geoConvert(m_pRecViewPortList[i]);
    if((m_pRecViewPortList[i]->getType() == SVIDEO_OCTAHEDRON || m_pRecViewPortList[i]->getType() == SVIDEO_ICOSAHEDRON) && m_pRecViewPortList[i]->getSVideoInfo()->iCompactFPStructure)
      m_pRecViewPortList[i]->compactFramePack(m_pRecViewPortYuv);
    else
      m_pRecViewPortList[i]->framePack(m_pRecViewPortYuv);

    //calculate viewport PSNR;
    xCalculatePSNRInternal(m_pRefViewPortYuv, m_pRecViewPortYuv, dPSNR, dMSE);
    //added frame based metrics;
    for(Int j=0; j<MAX_NUM_COMPONENT; j++)
    {
       m_pdPSNRSum[i][j] += dPSNR[j];
       m_pdMSESum[i][j] += dMSE[j];
    }
#if SVIDEO_VIEWPORT_OUTPUT //the order is encoding order;
    Char fileName[256];
    BitDepths bd;
    bd.recon[CHANNEL_TYPE_LUMA] =bd.recon[CHANNEL_TYPE_CHROMA] = m_iRefBitDepth;
    sprintf(fileName, "ref_viewport%d_%dx%d_BD%d.yuv", i, m_viewPortPSNRParam.iViewPortWidth, m_viewPortPSNRParam.iViewPortHeight, m_iRefBitDepth);
    m_pRefViewPortYuv->dump(fileName, bd, pcPic->getPOC()!=0);
    
    bd.recon[CHANNEL_TYPE_LUMA] =bd.recon[CHANNEL_TYPE_CHROMA] = m_iViewPortBitDepth;
    sprintf(fileName, "rec_viewport%d_%dx%d_BD%d.yuv", i, m_viewPortPSNRParam.iViewPortWidth, m_viewPortPSNRParam.iViewPortHeight, m_iViewPortBitDepth);
    m_pRecViewPortYuv->dump(fileName, bd, pcPic->getPOC()!=0);
#endif
#if SVIDEO_VIEWPORT_PSNR_REPORT_PER_FRAME
    printf("\nVP%d_PSNR|MSE: %.4lf %.4lf %.4lf %.4lf %.4lf %.4lf", i, dPSNR[0], dPSNR[1], dPSNR[2], dMSE[0], dMSE[1], dMSE[2]);
#endif
  }
}

Void TViewPortPSNR::printSummary(UInt uiNumPics)
{
  Int iNumOfViewPorts = (Int)m_viewPortPSNRParam.viewPortSettingsList.size(); 
  printf( "\n\nVIEWPORT SUMMARY --------------------------------------------------------\n" );
  printf( "\tTotal Frames |   "   "ViewPort   "  "Y-PSNR    "  "U-PSNR    "  "V-PSNR    "  "YUV-PSNR " );
  printf( " Y-MSE     "  "U-MSE     "  "V-MSE    "  "YUV-MSE \n" );
  TChar cDelim = 'a';
  for(Int vpIdx=0; vpIdx<iNumOfViewPorts; vpIdx++)
  {
    Double dPSNRYuv, dMSEYuv;
    calculateCombinedValues(vpIdx, uiNumPics, dPSNRYuv, dMSEYuv);
    /*
    Double MSEBasedSNR[MAX_NUM_COMPONENT];
    for (UInt componentIndex = 0; componentIndex < MAX_NUM_COMPONENT; componentIndex++)
    {
      const ComponentID compID = ComponentID(componentIndex);

      if (uiNumPics == 0)
      {
        MSEBasedSNR[compID] = 0; 
      }
      else
      {
        //NOTE: this is not the true maximum value for any bitDepth other than 8. It comes from the original HM PSNR calculation
        const UInt maxval = 255 << ( std::max(m_iViewPortBitDepth, m_iRefBitDepth) - 8);
        const Double MSE = m_pdMSESum[vpIdx][compID];
        MSEBasedSNR[compID] = (MSE == 0) ? 999.99 : (10 * log10((maxval * maxval) / (MSE / (Double)uiNumPics)));
      }
    }*/
    //printf viewport based PSNR;
    printf( "\t %8d    %c "          "%9d  "    "%8.4lf  "   "%8.4lf  "    "%8.4lf  "   "%8.4lf",
            uiNumPics, cDelim,
            vpIdx,
            m_pdPSNRSum[vpIdx][COMPONENT_Y] / (Double)uiNumPics,
            m_pdPSNRSum[vpIdx][COMPONENT_Cb] / (Double)uiNumPics,
            m_pdPSNRSum[vpIdx][COMPONENT_Cr] / (Double)uiNumPics,
            dPSNRYuv );

   printf( "  %8.4lf  "   "%8.4lf  "    "%8.4lf  "   "%8.4lf\n",
            m_pdMSESum[vpIdx][COMPONENT_Y] / (Double)uiNumPics,
            m_pdMSESum[vpIdx][COMPONENT_Cb] / (Double)uiNumPics,
            m_pdMSESum[vpIdx][COMPONENT_Cr] / (Double)uiNumPics,
            dMSEYuv );

  }
}
#endif