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

/** \file     TCPPPSNRMetricCalc.h
    \brief    CPPPSNRMetric class
*/

#include "TCPPPSNRMetricCalc.h"

#if SVIDEO_CPPPSNR

TCPPPSNRMetric::TCPPPSNRMetric()
: m_bCPPPSNREnabled(false)
, m_pCart2D(NULL)
, m_fpTable(NULL)
{
  m_dCPPPSNR[0] = m_dCPPPSNR[1] = m_dCPPPSNR[2] = 0;
  m_pcOutputGeomtry    = NULL;
  m_pcReferenceGeomtry = NULL;
  m_pcOutputCPPGeomtry = NULL;
  m_pcRefCPPGeomtry    = NULL;
}

TCPPPSNRMetric::~TCPPPSNRMetric()
{
  if(m_pCart2D)
  {
    free(m_pCart2D); m_pCart2D = NULL;
  }
  if (m_fpTable)
  {
    free(m_fpTable); m_fpTable = NULL;
  }

  if (m_pcOutputGeomtry)
  {
    delete m_pcOutputGeomtry; m_pcOutputGeomtry = NULL;
  }
  if (m_pcReferenceGeomtry)
  {
    delete m_pcReferenceGeomtry; m_pcReferenceGeomtry = NULL;
  }
  if (m_pcOutputCPPGeomtry)
  {
    delete m_pcOutputCPPGeomtry; m_pcOutputCPPGeomtry = NULL;
  }
  if (m_pcRefCPPGeomtry)
  {
    delete m_pcRefCPPGeomtry; m_pcRefCPPGeomtry = NULL;
  }
}

Void TCPPPSNRMetric::setOutputBitDepth(Int iOutputBitDepth[MAX_NUM_CHANNEL_TYPE])
{
  for(Int i = 0; i < MAX_NUM_CHANNEL_TYPE; i++)
  {
    m_outputBitDepth[i] = iOutputBitDepth[i];
  }
}

Void TCPPPSNRMetric::setReferenceBitDepth(Int iReferenceBitDepth[MAX_NUM_CHANNEL_TYPE])
{
  for(Int i = 0; i < MAX_NUM_CHANNEL_TYPE; i++)
  {
    m_referenceBitDepth[i] = iReferenceBitDepth[i];
  }
}

Void TCPPPSNRMetric::setCPPWidth(Int iCPPWidth)
{
    m_cppWidth = iCPPWidth;
}

Void TCPPPSNRMetric::setCPPHeight(Int iCPPheight)
{
   m_cppHeight = iCPPheight;
}

Void TCPPPSNRMetric::setCPPVideoInfo(SVideoInfo iCppCodingVdideoInfo, SVideoInfo iCppRefVdideoInfo)
{
    m_cppCodingVideoInfo = iCppCodingVdideoInfo;
    m_cppRefVideoInfo    = iCppRefVdideoInfo;
}

Void TCPPPSNRMetric::setDefaultFramePackingParam(SVideoInfo& sVideoInfo)
{
  if(sVideoInfo.geoType == SVIDEO_EQUIRECT || sVideoInfo.geoType == SVIDEO_EQUALAREA || sVideoInfo.geoType == SVIDEO_VIEWPORT || sVideoInfo.geoType == SVIDEO_CRASTERSPARABOLIC)
  {
      SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
      frmPack.chromaFormatIDC = CHROMA_420;
      frmPack.rows = 1;
      frmPack.cols = 1;
      frmPack.faces[0][0].id = 0; frmPack.faces[0][0].rot = 0;
  }
  else if(sVideoInfo.geoType == SVIDEO_CUBEMAP)
  {
    if(sVideoInfo.framePackStruct.rows ==0 || sVideoInfo.framePackStruct.cols ==0)
    {
      //set default frame packing format as CMP 3x2;
      SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
      frmPack.chromaFormatIDC = CHROMA_420;
      frmPack.rows = 2;
      frmPack.cols = 3;
      frmPack.faces[0][0].id = 4; frmPack.faces[0][0].rot = 0;
      frmPack.faces[0][1].id = 0; frmPack.faces[0][1].rot = 0;
      frmPack.faces[0][2].id = 5; frmPack.faces[0][2].rot = 0;
      frmPack.faces[1][0].id = 3; frmPack.faces[1][0].rot = 180;
      frmPack.faces[1][1].id = 1; frmPack.faces[1][1].rot = 270;
      frmPack.faces[1][2].id = 2; frmPack.faces[1][2].rot = 0;
    }
  }
  else if(sVideoInfo.geoType == SVIDEO_OCTAHEDRON)
  {
    if(sVideoInfo.framePackStruct.rows ==0 || sVideoInfo.framePackStruct.cols ==0)
    {
      if (sVideoInfo.iCompactFPStructure == 1)
      {
        SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
        frmPack.chromaFormatIDC = CHROMA_420;
        frmPack.rows = 4;
        frmPack.cols = 2;
        frmPack.faces[0][0].id = 5; frmPack.faces[0][0].rot = 0;
        frmPack.faces[0][1].id = 0; frmPack.faces[0][1].rot = 0;
        frmPack.faces[0][2].id = 7; frmPack.faces[0][2].rot = 0;
        frmPack.faces[0][3].id = 2; frmPack.faces[0][3].rot = 0;
        frmPack.faces[1][0].id = 4; frmPack.faces[1][0].rot = 180;
        frmPack.faces[1][1].id = 1; frmPack.faces[1][1].rot = 180;
        frmPack.faces[1][2].id = 6; frmPack.faces[1][2].rot = 180;
        frmPack.faces[1][3].id = 3; frmPack.faces[1][3].rot = 180;
      }
      else 
      {
        assert(sVideoInfo.iCompactFPStructure == 0 || sVideoInfo.iCompactFPStructure == 2);
        SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
        frmPack.chromaFormatIDC = CHROMA_420;
        frmPack.rows = 4;
        frmPack.cols = 2;
        frmPack.faces[0][0].id = 4; frmPack.faces[0][0].rot = 0;
        frmPack.faces[0][1].id = 0; frmPack.faces[0][1].rot = 0;
        frmPack.faces[0][2].id = 6; frmPack.faces[0][2].rot = 0;
        frmPack.faces[0][3].id = 2; frmPack.faces[0][3].rot = 0;
        frmPack.faces[1][0].id = 5; frmPack.faces[1][0].rot = 180;
        frmPack.faces[1][1].id = 1; frmPack.faces[1][1].rot = 180;
        frmPack.faces[1][2].id = 7; frmPack.faces[1][2].rot = 180;
        frmPack.faces[1][3].id = 3; frmPack.faces[1][3].rot = 180;
      }
    }
  }
  else if(sVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
  {
    if(sVideoInfo.framePackStruct.rows ==0 || sVideoInfo.framePackStruct.cols ==0)
    {
      //set default frame packing format;
      SVideoFPStruct &frmPack = sVideoInfo.framePackStruct;
      frmPack.chromaFormatIDC = CHROMA_420;
      frmPack.rows = 4;
      frmPack.cols = 5;
      frmPack.faces[0][0].id = 0; frmPack.faces[0][0].rot = 0;
      frmPack.faces[0][1].id = 2; frmPack.faces[0][1].rot = 0;
      frmPack.faces[0][2].id = 4; frmPack.faces[0][2].rot = 0;
      frmPack.faces[0][3].id = 6; frmPack.faces[0][3].rot = 0;
      frmPack.faces[0][4].id = 8; frmPack.faces[0][4].rot = 0;
      frmPack.faces[1][0].id = 1; frmPack.faces[1][0].rot = 180;
      frmPack.faces[1][1].id = 3; frmPack.faces[1][1].rot = 180;
      frmPack.faces[1][2].id = 5; frmPack.faces[1][2].rot = 180;
      frmPack.faces[1][3].id = 7; frmPack.faces[1][3].rot = 180;
      frmPack.faces[1][4].id = 9; frmPack.faces[1][4].rot = 180;
      frmPack.faces[2][0].id = 11; frmPack.faces[1][0].rot = 0;
      frmPack.faces[2][1].id = 13; frmPack.faces[1][1].rot = 0;
      frmPack.faces[2][2].id = 15; frmPack.faces[1][2].rot = 0;
      frmPack.faces[2][3].id = 17; frmPack.faces[1][3].rot = 0;
      frmPack.faces[2][4].id = 19; frmPack.faces[1][4].rot = 0;
      frmPack.faces[3][0].id = 10; frmPack.faces[3][0].rot = 180;
      frmPack.faces[3][1].id = 12; frmPack.faces[3][1].rot = 180;
      frmPack.faces[3][2].id = 14; frmPack.faces[3][2].rot = 180;
      frmPack.faces[3][3].id = 16; frmPack.faces[3][3].rot = 180;
      frmPack.faces[3][4].id = 18; frmPack.faces[3][4].rot = 180;
    }
  }
}

Void TCPPPSNRMetric::fillSourceSVideoInfo(SVideoInfo& sVidInfo, Int inputWidth, Int inputHeight)
{
  if(sVidInfo.geoType == SVIDEO_EQUIRECT || sVidInfo.geoType == SVIDEO_EQUALAREA || sVidInfo.geoType == SVIDEO_VIEWPORT || sVidInfo.geoType == SVIDEO_CRASTERSPARABOLIC)
  {
    //assert(sVidInfo.framePackStruct.rows == 1);
    //assert(sVidInfo.framePackStruct.cols == 1);
    //enforce;
    sVidInfo.framePackStruct.rows = 1;
    sVidInfo.framePackStruct.cols = 1;
    sVidInfo.framePackStruct.faces[0][0].id = 0; 
    //sVidInfo.framePackStruct.faces[0][0].rot = 0;
    sVidInfo.iNumFaces =1;
    if(sVidInfo.framePackStruct.faces[0][0].rot == 90 || sVidInfo.framePackStruct.faces[0][0].rot == 270)
    {
      sVidInfo.iFaceWidth = inputHeight;
      sVidInfo.iFaceHeight = inputWidth;
    }
    else
    {
      sVidInfo.iFaceWidth = inputWidth;
      sVidInfo.iFaceHeight = inputHeight;
    }
  }
  else if(sVidInfo.geoType == SVIDEO_CUBEMAP)
  {
    sVidInfo.iNumFaces = 6;
    sVidInfo.iFaceWidth = inputWidth/sVidInfo.framePackStruct.cols;
    sVidInfo.iFaceHeight = inputHeight/sVidInfo.framePackStruct.rows;
    assert(sVidInfo.iFaceWidth == sVidInfo.iFaceHeight);
  }
  else if(sVidInfo.geoType == SVIDEO_OCTAHEDRON
         || (sVidInfo.geoType == SVIDEO_ICOSAHEDRON)
    )
  {
    sVidInfo.iNumFaces = (sVidInfo.geoType == SVIDEO_OCTAHEDRON)? 8 : 20;
    if(sVidInfo.geoType == SVIDEO_OCTAHEDRON && sVidInfo.iCompactFPStructure)
    {
      sVidInfo.iFaceWidth = (inputWidth/sVidInfo.framePackStruct.cols) - 4;
      sVidInfo.iFaceHeight = (inputHeight<<1)/sVidInfo.framePackStruct.rows;
    }
    else if(sVidInfo.geoType == SVIDEO_ICOSAHEDRON && sVidInfo.iCompactFPStructure)
    {
      Int halfCol = (sVidInfo.framePackStruct.cols>>1);
      sVidInfo.iFaceWidth = (2*inputWidth - 8*halfCol - 4 )/(2*halfCol + 1);
      sVidInfo.iFaceHeight = inputHeight/sVidInfo.framePackStruct.rows;
    }
    else
    {
      sVidInfo.iFaceWidth = inputWidth/sVidInfo.framePackStruct.cols;
      sVidInfo.iFaceHeight = inputHeight/sVidInfo.framePackStruct.rows;
    }
  }
  else
    assert(!"Not supported yet");
}

Void TCPPPSNRMetric::setChromaFormatIDC(ChromaFormat iChromaFormatIDC)
{
   m_chromaFormatIDC = iChromaFormatIDC;
}

Void TCPPPSNRMetric::setCPPGeoParam(InputGeoParam iCPPGeoParam)
{
    m_cppGeoParam = iCPPGeoParam;
}

Void TCPPPSNRMetric::initCPPPSNR(InputGeoParam inputGeoParam, Int cppWidth, Int cppHeight, SVideoInfo codingvideoInfo, SVideoInfo referenceVideoInfo)
{
  setCPPGeoParam(inputGeoParam);
  setCPPWidth(cppWidth);
  setCPPHeight(cppHeight);
  setChromaFormatIDC(referenceVideoInfo.framePackStruct.chromaFormatIDC);
  setCPPVideoInfo(codingvideoInfo, referenceVideoInfo);

  memset(&m_cppVideoInfo, 0, sizeof(m_cppVideoInfo));

  m_cppVideoInfo.geoType = SVIDEO_CRASTERSPARABOLIC;
  setDefaultFramePackingParam(m_cppVideoInfo);
  fillSourceSVideoInfo(m_cppVideoInfo, m_cppWidth, m_cppHeight);

  m_pcOutputGeomtry = TGeometry::create(m_cppCodingVideoInfo, &m_cppGeoParam);
  m_pcOutputCPPGeomtry = TGeometry::create(m_cppVideoInfo, &m_cppGeoParam);
  m_pcRefCPPGeomtry = TGeometry::create(m_cppVideoInfo, &m_cppGeoParam);
  m_pcReferenceGeomtry = TGeometry::create(m_cppRefVideoInfo, &m_cppGeoParam);
}

Void TCPPPSNRMetric::sphSampoints(Char* cSphDataFile)
{
  if(cSphDataFile == NULL)
  {
    m_bCPPPSNREnabled = false;
    return;
  }

  // read data
  FILE *fp = fopen(cSphDataFile,"r");
  if(!fp)
  {
    printf("SPSNR-NN is disabled because metadata file (%s) cannot be opened!\n", cSphDataFile);
    m_bCPPPSNREnabled = false;
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

Void TCPPPSNRMetric::xCalculateCPPPSNR( TComPicYuv* pcOrgPicYuv, TComPicYuv* pcPicD)
{
  Int iBitDepthForPSNRCalc[MAX_NUM_CHANNEL_TYPE];
  Int iReferenceBitShift[MAX_NUM_CHANNEL_TYPE];
  Int iOutputBitShift[MAX_NUM_CHANNEL_TYPE];

  TComPicYuv *TPicYUVRefCPP;
  TComPicYuv *TPicYUVOutCPP;

  iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] = std::max(m_outputBitDepth[CHANNEL_TYPE_LUMA], m_referenceBitDepth[CHANNEL_TYPE_LUMA]);
  iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] = std::max(m_outputBitDepth[CHANNEL_TYPE_CHROMA], m_referenceBitDepth[CHANNEL_TYPE_CHROMA]);
  iReferenceBitShift[CHANNEL_TYPE_LUMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] - m_referenceBitDepth[CHANNEL_TYPE_LUMA];
  iReferenceBitShift[CHANNEL_TYPE_CHROMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] - m_referenceBitDepth[CHANNEL_TYPE_CHROMA];
  iOutputBitShift[CHANNEL_TYPE_LUMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] - m_outputBitDepth[CHANNEL_TYPE_LUMA];
  iOutputBitShift[CHANNEL_TYPE_CHROMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] - m_outputBitDepth[CHANNEL_TYPE_CHROMA];

  memset(m_dCPPPSNR, 0, sizeof(Double)*3);
  Double SCPPDspsnr[3]={0, 0 ,0};

  // Convert Output and Ref to CPP_Projection
  TPicYUVRefCPP = new TComPicYuv;
  TPicYUVRefCPP->createWithoutCUInfo  ( m_cppWidth, m_cppHeight, m_chromaFormatIDC, true );

  TPicYUVOutCPP = new TComPicYuv;
  TPicYUVOutCPP->createWithoutCUInfo  ( m_cppWidth, m_cppHeight, m_chromaFormatIDC, true );

  // Converting Reference to CPP
  if ((m_pcReferenceGeomtry->getSVideoInfo()->geoType == SVIDEO_OCTAHEDRON || m_pcReferenceGeomtry->getSVideoInfo()->geoType == SVIDEO_ICOSAHEDRON) && m_pcReferenceGeomtry->getSVideoInfo()->iCompactFPStructure)
  {
    m_pcReferenceGeomtry->compactFramePackConvertYuv(pcOrgPicYuv);
  }
  else
  {
    m_pcReferenceGeomtry->convertYuv(pcOrgPicYuv);
  }
  m_pcReferenceGeomtry->geoConvert(m_pcRefCPPGeomtry);
  m_pcRefCPPGeomtry->framePack(TPicYUVRefCPP);

  // Converting Output to CPP
  if ((m_pcOutputGeomtry->getSVideoInfo()->geoType == SVIDEO_OCTAHEDRON || m_pcOutputGeomtry->getSVideoInfo()->geoType == SVIDEO_ICOSAHEDRON) && m_pcOutputGeomtry->getSVideoInfo()->iCompactFPStructure)
  {
    m_pcOutputGeomtry->compactFramePackConvertYuv(pcPicD);
  }
  else
  {
    m_pcOutputGeomtry->convertYuv(pcPicD);
  }
  m_pcOutputGeomtry->geoConvert(m_pcOutputCPPGeomtry);
  m_pcOutputCPPGeomtry->framePack(TPicYUVOutCPP);

 for(Int chan=0; chan<pcPicD->getNumberValidComponents(); chan++)
  {
    const ComponentID ch=ComponentID(chan);
    const Pel*  pOrg       = TPicYUVOutCPP->getAddr(ch);
    const Int   iOrgStride = TPicYUVOutCPP->getStride(ch);
    const Pel*  pRec       = TPicYUVRefCPP->getAddr(ch);
    const Int   iRecStride = TPicYUVRefCPP->getStride(ch);
    const Int   iWidth     = TPicYUVRefCPP->getWidth (ch);
    const Int   iHeight    = TPicYUVRefCPP->getHeight(ch);

    Int   iSize            = 0;
    double fPhi, fLambda;
    double fIdxX, fIdxY;
    double fLamdaX, fLamdaY;

    for(Int y=0;y<iHeight;y++)
    {
      for(Int x=0;x<iWidth;x++)
      {
        fLamdaX = ((double)x / (iWidth)) * (2 * S_PI) - S_PI;
        fLamdaY = ((double)y / (iHeight)) * S_PI - (S_PI_2);

        fPhi = 3 * sasin(fLamdaY / S_PI);
        fLambda = fLamdaX / (2 * scos(2 * fPhi / 3) - 1);

        fLamdaX = (fLambda + S_PI) / 2 / S_PI * (iWidth);
        fLamdaY = (fPhi + (S_PI / 2)) / S_PI *  (iHeight);

        fIdxX = (int)((fLamdaX < 0) ? fLamdaX - 0.5 : fLamdaX + 0.5);
        fIdxY = (int)((fLamdaY < 0) ? fLamdaY - 0.5 : fLamdaY + 0.5);

        if(fIdxY >= 0 && fIdxX >= 0 && fIdxX < iWidth && fIdxY < iHeight)
        {
          Intermediate_Int iDifflp  = (Intermediate_Int)((pOrg[x]<<iReferenceBitShift[toChannelType(ch)]) - (pRec[x]<<iOutputBitShift[toChannelType(ch)]) );
          SCPPDspsnr[chan]         += iDifflp * iDifflp;
          iSize++;
        }
      }
      pOrg += iOrgStride;
      pRec += iRecStride;
    }
    SCPPDspsnr[chan] /= iSize;
  }

  for (Int ch_indx = 0; ch_indx < pcPicD->getNumberValidComponents(); ch_indx++)
  {
    const ComponentID ch=ComponentID(ch_indx);
    const Int maxval = 255<<(iBitDepthForPSNRCalc[toChannelType(ch)]-8) ;

    Double fReflpsnr = maxval*maxval;
    m_dCPPPSNR[ch_indx] = ( SCPPDspsnr[ch_indx] ? 10.0 * log10( fReflpsnr / (Double)SCPPDspsnr[ch_indx] ) : 999.99 );
  }

  if(TPicYUVRefCPP)
  {
    TPicYUVRefCPP->destroy();
    delete TPicYUVRefCPP;
    TPicYUVRefCPP = NULL;
  }
  if(TPicYUVOutCPP)
  {
    TPicYUVOutCPP->destroy();
    delete TPicYUVOutCPP;
    TPicYUVOutCPP = NULL;
  }
}

#endif // SVIDEO_CPPPSNR