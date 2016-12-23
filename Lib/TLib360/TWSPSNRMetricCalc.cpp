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

/** \file     TWSPSNRMetricCalc.cpp
    \brief    WSPSNRMetric class
*/

#include "TWSPSNRMetricCalc.h"

#if SVIDEO_WSPSNR

#define WSPSNR_MTK_MODIFICATION   1

TWSPSNRMetric::TWSPSNRMetric()
: m_bEnabled(false)
, m_fErpWeight_Y(NULL)
, m_fErpWeight_C(NULL)
, m_fCubeWeight_Y(NULL)
, m_fCubeWeight_C(NULL)
, m_fEapWeight_Y(NULL)
, m_fEapWeight_C(NULL)
, m_fOctaWeight_Y(NULL)
, m_fOctaWeight_C(NULL)
, m_fIcoWeight_Y(NULL)
, m_fIcoWeight_C(NULL)
, m_codingGeoType(0)
, m_iCodingFaceWidth(0)
, m_iCodingFaceHeight(0)
#if SVIDEO_WSPSNR_E2E
, m_pcTVideoIOYuvInputFile(NULL)
, m_pRefGeometry(NULL)
, m_pRecGeometry(NULL)
, m_pcOrgPicYuv(NULL)
, m_pcRecPicYuv(NULL)
, m_iLastFrmPOC(0)
, m_temporalSubsampleRatio(1)
#endif
{
  m_dWSPSNR[0] = m_dWSPSNR[1] = m_dWSPSNR[2] = 0;
}

TWSPSNRMetric::~TWSPSNRMetric()
{
  if (m_fErpWeight_Y)
  {
    free(m_fErpWeight_Y);
    m_fErpWeight_Y = NULL;
  }
  if (m_fErpWeight_C)
  {
    free(m_fErpWeight_C);
    m_fErpWeight_C = NULL;
  }
  if (m_fEapWeight_Y)
  {
    free(m_fEapWeight_Y);
    m_fEapWeight_Y = NULL;
  }
  if (m_fEapWeight_C)
  {
    free(m_fEapWeight_C);
    m_fEapWeight_C = NULL;
  }
  if (m_fCubeWeight_Y)
  {
    free(m_fCubeWeight_Y);
    m_fCubeWeight_Y = NULL;
  }
  if (m_fCubeWeight_C)
  {
    free(m_fCubeWeight_C);
    m_fCubeWeight_C = NULL;
  }
  if(m_fOctaWeight_Y)
  {
    free(m_fOctaWeight_Y);
    m_fOctaWeight_Y=NULL;
  }
  if(m_fOctaWeight_C)
  {
    free(m_fOctaWeight_C);
    m_fOctaWeight_C=NULL;
  }
#if SVIDEO_WSPSNR_E2E
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
  if(m_pcOrgPicYuv)
  {
    m_pcOrgPicYuv->destroy();
    delete m_pcOrgPicYuv;
  }
  if(m_pcRecPicYuv)
  {
    m_pcRecPicYuv->destroy();
    delete m_pcRecPicYuv;
  }
#endif
}

Void TWSPSNRMetric::setOutputBitDepth(Int iOutputBitDepth[MAX_NUM_CHANNEL_TYPE])
{
  for(Int i = 0; i < MAX_NUM_CHANNEL_TYPE; i++)
  {
    m_outputBitDepth[i] = iOutputBitDepth[i];
  }
}

Void TWSPSNRMetric::setReferenceBitDepth(Int iReferenceBitDepth[MAX_NUM_CHANNEL_TYPE])
{
  for(Int i = 0; i < MAX_NUM_CHANNEL_TYPE; i++)
  {
    m_referenceBitDepth[i] = iReferenceBitDepth[i];
  }
}

void TWSPSNRMetric::createTable(TComPicYuv* pcPicD, TGeometry *pcCodingGeomtry)
{
  if(!m_bEnabled)
  {
    return;
  }

  SVideoInfo *pCodingSVideoInfo = pcCodingGeomtry->getSVideoInfo();
  Int iFaceWidth = pCodingSVideoInfo->iFaceWidth;
  Int iFaceHeight = pCodingSVideoInfo->iFaceHeight;
  Int iScaleX = pcPicD->getComponentScaleX(COMPONENT_Cb);
  Int iScaleY = pcPicD->getComponentScaleY(COMPONENT_Cb);
  Double dChromaOffset[2] = {0.0, 0.0}; //[0: X; 1: Y];
  if(pcPicD->getChromaFormat() == CHROMA_420)
  {
    dChromaOffset[0] = (m_iChromaSampleLocType == 0 || m_iChromaSampleLocType == 2)? 0 : 0.5;
    dChromaOffset[1] = (m_iChromaSampleLocType == 2 || m_iChromaSampleLocType == 3)? 0 : 0.5;
  }
  if(pcCodingGeomtry->getType()==SVIDEO_EQUIRECT)
  {
    Double fWeightSum_Y=0;
    Double fWeightSum_C=0;
    Int   iWidth = pcPicD->getWidth(COMPONENT_Y) ;
    Int   iHeight = pcPicD->getHeight(COMPONENT_Y) ;
    Int   iWidthC = pcPicD->getWidth(COMPONENT_Cb) ;
    Int   iHeightC = pcPicD->getHeight(COMPONENT_Cb) ;
    m_fErpWeight_Y=(Double*)malloc(iHeight*sizeof(Double));
    m_fErpWeight_C=(Double*)malloc(iHeightC*sizeof(Double));

    for(Int y=0; y< iHeight; y++)
    {
      m_fErpWeight_Y[y]=scos((y-(iHeight/2-0.5))*S_PI/iHeight);//Brave:the purpose of - 0.5,y [0,iHeight - 1],output [-S_PI/2 + (0.5*S_PI)/iH,S_PI/2 - (0.5*S_PI)/iH]
      fWeightSum_Y += m_fErpWeight_Y[y];
    }
    for(Int y=0; y< iHeightC; y++)
    {
      m_fErpWeight_C[y]=scos(((y<<iScaleY)+dChromaOffset[1]+0.5-iHeight/2)*S_PI/iHeight);
      fWeightSum_C += m_fErpWeight_C[y];
    }

    for(Int y=0; y< iHeight; y++)
    {
      m_fErpWeight_Y[y]=m_fErpWeight_Y[y]/fWeightSum_Y/iWidth;
    }
    for(Int y=0; y< iHeightC; y++)
    {
      m_fErpWeight_C[y]=m_fErpWeight_C[y]/fWeightSum_C/(iWidthC);
    }

  }
  else if(pcCodingGeomtry->getType()==SVIDEO_CUBEMAP)
  {
    Double fWeightSum_Y=0;
    Double fWeightSum_C=0;    

    m_fCubeWeight_Y=(Double*)malloc(iFaceHeight * iFaceWidth*sizeof(Double));
    m_fCubeWeight_C=(Double*)malloc((iFaceHeight >> iScaleY) * (iFaceWidth >> iScaleX)*sizeof(Double));
    for(Int y = 0; y < iFaceHeight; y++ )
    {
      for(Int x=0; x < iFaceWidth; x++)
      {
        Int ci, cj, r2;
        Double d2;
        ci= iFaceWidth/2;
        cj= iFaceHeight/2;
        d2 = (x+0.5-ci)*(x+0.5-ci)+(y+0.5-cj)*(y+0.5-cj);
        r2 = (iFaceWidth/2)*(iFaceWidth/2);
        Double weight= 1.0/((1+d2/r2)*ssqrt(1.0*(1+d2/r2)));
        m_fCubeWeight_Y[iFaceWidth*y+x] = weight;
        fWeightSum_Y += weight;
      }
    }
    for(Int y = 0; y < iFaceHeight; y++ )
    {
      for(Int x=0; x<iFaceWidth; x++)
      {
        m_fCubeWeight_Y[iFaceHeight*y+x] = (m_fCubeWeight_Y[iFaceHeight*y+x])/fWeightSum_Y/6.0;
      }
    }

    for(Int y = 0; y < (iFaceHeight>>iScaleY); y++ )
    {
      for(Int x=0; x< (iFaceWidth>>iScaleX); x++)
      {
        Int ci, cj, r2;
        Double d2;
        ci= iFaceWidth/2;
        cj= iFaceHeight/2;
        d2 = (x*(1<<iScaleX)+dChromaOffset[0]+0.5 - ci)*(x*(1<<iScaleX)+dChromaOffset[0]+0.5 - ci) + (y*(1<<iScaleY)+dChromaOffset[1]+0.5 -cj)*(y*(1<<iScaleY)+dChromaOffset[1]+0.5 -cj);
        r2 = (iFaceWidth/2)*(iFaceWidth/2);
        Double weight= 1.0/((1+d2/r2)*sqrt(1.0*(1+d2/r2)));
        m_fCubeWeight_C[(iFaceWidth>>iScaleX)*y+x]=weight;
        fWeightSum_C += weight;
      }
    }

    for(Int y = 0; y < (iFaceHeight>>iScaleY); y++ )
    {
      for(Int x=0; x< (iFaceWidth>>iScaleX); x++)
      {
        m_fCubeWeight_C[(iFaceWidth>>iScaleX)*y+x]=(m_fCubeWeight_C[(iFaceWidth>>iScaleX)*y+x])/fWeightSum_C/6.0;
      }
    }
  } 
  else if(pcCodingGeomtry->getType()==SVIDEO_EQUALAREA)
  {
    Int   iWidth = pcPicD->getWidth(COMPONENT_Y) ;
    Int   iHeight = pcPicD->getHeight(COMPONENT_Y) ;
    Int   iWidthC = pcPicD->getWidth(COMPONENT_Cb) ;
    Int   iHeightC = pcPicD->getHeight(COMPONENT_Cb) ;
    m_fEapWeight_Y = (Double*)malloc(iWidth*iHeight*sizeof(Double));
    m_fEapWeight_C = (Double*)malloc(iWidthC*iHeightC*sizeof(Double));
    for (Int y=0;y<iHeight;y++)
    {
      for (Int x=0;x<iWidth;x++)
      {
        m_fEapWeight_Y[y*iWidth+x] = 1.0/(iWidth*iHeight);
      }
    }
    for (Int y=0; y<iHeightC; y++)
    {
      for (Int x=0; x<iWidthC; x++)
      {
        m_fEapWeight_C[y*iWidthC+x]=1.0/(iWidthC*iHeightC);
      }
    }
  }
  else if(pcCodingGeomtry->getType()==SVIDEO_OCTAHEDRON && pCodingSVideoInfo->iCompactFPStructure ==0 )
  {
    Int iwidth=iFaceWidth*pCodingSVideoInfo->framePackStruct.cols;
    Int iheight=iFaceHeight*pCodingSVideoInfo->framePackStruct.rows;
    Double* fWeightRotZero_Y = (Double*)malloc(iFaceWidth*iFaceHeight*sizeof(Double));
    Double* fWeightRotZero_C = (Double*)malloc((iFaceWidth>>iScaleX) *(iFaceHeight>>iScaleY) *sizeof(Double));
    m_fOctaWeight_Y=(Double*)malloc(iwidth*iheight*sizeof(Double));
    m_fOctaWeight_C=(Double*)malloc((iwidth>>iScaleX)*(iheight>>iScaleY)*sizeof(Double));
    Double weightSum_Y=0;
    Double weightSum_C=0;
    for (Int x=0; x<iFaceWidth; x++)
    {
      for (Int y=0; y<iFaceHeight; y++)
      {
        if(sfabs(x+0.5-iFaceWidth/2) < (y+0.5)/ssqrt(3.0))
        {
          Int ci=0;
          Int cj=0;
          Double r2,d2;
          ci = iFaceWidth/2;
          cj = 2*iFaceHeight/3;
          d2 =(x+0.5-ci)*(x+0.5-ci)+(y+0.5-cj)*(y+0.5-cj);
          r2 =(iFaceWidth/ssqrt(6.0))*(iFaceWidth/ssqrt(6.0));
          Double weight = 1.0/((1+d2/r2)*ssqrt(1.0*(1+d2/r2)));
          fWeightRotZero_Y[iFaceWidth*y+x] = weight;
          weightSum_Y += weight;
        }
        else
        {
          fWeightRotZero_Y[iFaceWidth*y+x]=0;
        }
      }
    }

    //Normalize per face_Y
    for (Int x=0;x<iFaceWidth;x++)
    {
      for (Int y=0;y<iFaceHeight;y++)
      {
        fWeightRotZero_Y[iFaceWidth*y+x]=fWeightRotZero_Y[iFaceWidth*y+x]/weightSum_Y/8.0;
      }
    }

  //weights for the entire frame_Y
    Double weight;
    for (Int rows=0;rows<pCodingSVideoInfo->framePackStruct.rows;rows++)
    {
      Int ypos=rows*iFaceHeight;
      for (Int cols=0;cols<pCodingSVideoInfo->framePackStruct.cols;cols++)
      {
        Int xpos=cols*iFaceWidth;
        for (Int x=0;x<iFaceWidth;x++)
        {
          for (Int y=0;y<iFaceHeight;y++)
          {
            if(pCodingSVideoInfo->framePackStruct.faces[rows][cols].rot==0)
            {
              weight=fWeightRotZero_Y[y*iFaceWidth+x];
            }
            else
            {
              weight=fWeightRotZero_Y[(iFaceHeight-y-1)*iFaceWidth + (iFaceWidth-x-1)];
            }
            m_fOctaWeight_Y[(ypos+y)*iwidth+xpos+x]=weight;
          }
        }
      }
    }
   free(fWeightRotZero_Y);

   for (Int x=0; x<(iFaceWidth>>iScaleX); x++)
   {
     for (Int y=0; y<(iFaceHeight>>iScaleY); y++)
     {
       if(sfabs(x*(1<<iScaleX)+dChromaOffset[0]+0.5-iFaceWidth/2) < (y*(1<<iScaleY)+dChromaOffset[1]+0.5)/ssqrt(3.0))
       {
         Int ci=0;
         Int cj=0;
         Double r2,d2;
         ci = iFaceWidth/2;
         cj = 2*iFaceHeight/3;
         d2 =(x*(1<<iScaleX)+dChromaOffset[0]+0.5-ci)*(x*(1<<iScaleX)+dChromaOffset[0]+0.5-ci) + (y*(1<<iScaleY)+dChromaOffset[1]+0.5-cj)*(y*(1<<iScaleY)+dChromaOffset[1]+0.5-cj);
         r2 =(iFaceWidth/ssqrt(6.0))*(iFaceWidth/ssqrt(6.0));
         Double dweight = 1.0/((1+d2/r2)*sqrt(1.0*(1+d2/r2)));
         fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]=dweight;
         weightSum_C+=dweight;
       }
       else
       {
         fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]=0;
       }
     }
   }

   //Normalize per face_C
   for (Int x=0; x<(iFaceWidth>>iScaleX); x++)
   {
     for (Int y=0; y<(iFaceHeight>>iScaleY); y++)
     {
       fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]=fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]/weightSum_C/8.0;
     }
   }
   //weights for the entire frame_C
   for (Int rows=0;rows<pCodingSVideoInfo->framePackStruct.rows;rows++)
   {
     Int ypos=rows*(iFaceHeight>>iScaleY);
     for (Int cols=0; cols<pCodingSVideoInfo->framePackStruct.cols; cols++)
     {
       Int xpos=cols*(iFaceWidth>>iScaleX);
       for (Int x=0;x<(iFaceWidth>>iScaleX);x++)
       {
         for (Int y=0;y<(iFaceHeight>>iScaleY);y++)
         {
           if(pCodingSVideoInfo->framePackStruct.faces[rows][cols].rot==0)
           {
             weight=fWeightRotZero_C[y*(iFaceWidth>>iScaleX)+x];
           }
           else
           {
             weight=fWeightRotZero_C[((iFaceHeight>>iScaleY)-y-1)*(iFaceWidth>>iScaleX)+((iFaceWidth>>iScaleX)-x-1)];
           }
           m_fOctaWeight_C[(ypos+y)*(iwidth>>iScaleX)+xpos+x]=weight;
         }
       }
     }
   }
   free(fWeightRotZero_C);
  }
  else if(pcCodingGeomtry->getType()==SVIDEO_OCTAHEDRON && pCodingSVideoInfo->iCompactFPStructure ==1)
  {
    Int   iwidth  = pcPicD->getWidth(COMPONENT_Y) ;
    Int   iheight = pcPicD->getHeight(COMPONENT_Y) ;
    Double* fWeightRotZero_Y=(Double*)malloc(iFaceWidth*iFaceHeight*sizeof(Double));
    Double* fWeightRotZero_C=(Double*)malloc((iFaceWidth>>iScaleX)*(iFaceHeight>>iScaleY)*sizeof(Double));
    m_fOctaWeight_Y=(Double*)malloc(iwidth*iheight*sizeof(Double));
    m_fOctaWeight_C=(Double*)malloc((iwidth>>iScaleX)*(iheight>>iScaleY)*sizeof(Double));
    Double weightSum_Y=0;
    Double weightSum_C=0;

    for (Int x=0;x<iwidth;x++)
    {
      for (Int y=0;y<iheight;y++)
      {
        m_fOctaWeight_Y[y*iwidth+x]=0;
      }
    }

    for (Int x=0;x<(iwidth>>iScaleX);x++)
    {
      for (Int y=0;y<(iheight>>iScaleY);y++)
      {
        m_fOctaWeight_C[y*(iwidth>>iScaleX)+x]=0;
      }
    }

    for (Int x=0;x<iFaceWidth;x++)
    {
      for (Int y=0;y<iFaceHeight;y++)
      {
        if(sfabs(x+0.5-iFaceWidth/2) < (y+0.5)/ssqrt(3.0))
        {
          Int ci=0;
          Int cj=0;
          Double r2,d2;
          ci = iFaceWidth/2;
          cj = 2*iFaceHeight/3;
          d2 =(x+0.5-ci)*(x+0.5-ci)+(y+0.5-cj)*(y+0.5-cj);
          r2 =(iFaceWidth/ssqrt(6.0))*(iFaceWidth/ssqrt(6.0));
          Double weight = 1.0/((1+d2/r2)*sqrt(1.0*(1+d2/r2)));
          fWeightRotZero_Y[iFaceWidth*y+x]=weight;
          weightSum_Y+=weight;
        }
        else
        {
          fWeightRotZero_Y[iFaceWidth*y+x]=0;
        }
      }
    }

    //Normalize per face_Y
    for (Int x=0;x<iFaceWidth;x++)
    {
      for (Int y=0;y<iFaceHeight;y++)
      {
        fWeightRotZero_Y[iFaceWidth*y+x]=fWeightRotZero_Y[iFaceWidth*y+x]/weightSum_Y/8.0;
      }
    }
    //weights for the entire frame_Y
    for(Int i=-1;i<2;i++)
      for (Int x=0;x<iFaceWidth;x++)
        for (Int y=0;y<iFaceHeight;y++)
        {
          if(sfabs(x+0.5-iFaceWidth/2) < (y+0.5)/ssqrt(3.0) && 4*i+4+i*iFaceWidth+x+iFaceWidth/2>=0 && 4*i+4+i*iFaceWidth+x+iFaceWidth/2< iwidth)
          {
            m_fOctaWeight_Y[y*iwidth+4*i+4+i*iFaceWidth+x+iFaceWidth/2]=fWeightRotZero_Y[iFaceWidth*y+x];
          }
        }

    for(Int i=0;i<2;i++)
      for(Int x=0;x<iFaceWidth;x++)
        for (Int y=0;y<iFaceHeight;y++)
        {
#if WSPSNR_MTK_MODIFICATION
          if(sfabs(x + 0.5 - iFaceWidth / 2) < (iFaceHeight - (y + 0.5)) / ssqrt(3.0) )
#else
          if(sfabs(x+0.5-iFaceHeight/2) < (iFaceHeight-(y+0.5))/ssqrt(3.0) )
#endif
          {
            m_fOctaWeight_Y[y*iwidth+4*i+2+i*iFaceWidth+x]=fWeightRotZero_Y[(iFaceHeight-y-1)*iFaceWidth+(iFaceWidth-x-1)];
          }
        }

    for(Int x=0;x<iwidth;x++)
      for (Int y=0;y<iheight/2;y++)
      {
#if WSPSNR_MTK_MODIFICATION
        m_fOctaWeight_Y[iwidth * iheight / 2 + y * iwidth + x] = m_fOctaWeight_Y[(iheight / 2 - 1 - y) * iwidth + (iwidth - 1 - x)];
#else
        m_fOctaWeight_Y[iwidth*iheight/2+y*iwidth+x]=m_fOctaWeight_Y[(iheight-1-y)*iwidth+(iwidth-1-x)];
#endif
      }
      free(fWeightRotZero_Y);

    for (Int x=0;x<(iFaceWidth>>iScaleX);x++)
    {
      for (Int y=0;y<(iFaceHeight>>iScaleY);y++)
      {
        if(sfabs(x*(1<<iScaleX)+dChromaOffset[0]+0.5-iFaceWidth/2) < (y*(1<<iScaleY)+dChromaOffset[1]+0.5)/ssqrt(3.0))
        {
          Int ci=0;
          Int cj=0;
          Double r2,d2;
          ci = iFaceWidth/2;
          cj = 2*iFaceHeight/3;
          d2 =(x*(1<<iScaleX)+dChromaOffset[0]+0.5-ci)*(x*(1<<iScaleX)+dChromaOffset[0]+0.5-ci) + (y*(1<<iScaleY)+dChromaOffset[1]+0.5-cj)*(y*(1<<iScaleY)+dChromaOffset[1]+0.5-cj);
          r2 =(iFaceWidth/ssqrt(6.0))*(iFaceWidth/ssqrt(6.0));
          Double weight = 1.0/((1+d2/r2)*ssqrt(1.0*(1+d2/r2)));
          fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]=weight;
          weightSum_C+=weight;
        }
        else
        {
          fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]=0;
        }
      }
    }

      //Normalize per face_C
    for (Int x=0;x<(iFaceWidth>>iScaleX);x++)
    {
      for (Int y=0;y<(iFaceHeight>>iScaleY);y++)
      {
        fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]=fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]/weightSum_C/8.0;
      }
    }
    //weights for the entire frame_C
    for(Int i=-1;i<2;i++)
      for (Int x=0;x<(iFaceWidth>>iScaleX);x++)
        for (Int y=0;y<(iFaceHeight>>iScaleY);y++)
        {
          if(sfabs(x*(1<<iScaleX)+dChromaOffset[0]+0.5-iFaceWidth/2) < (y*(1<<iScaleY)+dChromaOffset[1]+0.5)/sqrt(3.0)&& 4*i+4+i*m_iCodingFaceWidth+2*x+m_iCodingFaceWidth/2>=0 && 4*i+4+i*m_iCodingFaceWidth+2*x+m_iCodingFaceWidth/2< iwidth)
          {
            m_fOctaWeight_C[y * (iwidth>>iScaleX) + ((4 * i + 4) >> iScaleX) + i * (iFaceWidth>>iScaleX)+x+(iFaceWidth>>iScaleX)/2] = fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x];
          }
        }


    for(Int i=0;i<2;i++)
      for(Int x=0;x<(iFaceWidth>>iScaleX);x++)
        for (Int y=0;y<(iFaceHeight>>iScaleY);y++)
        {
          if(sfabs(x*(1<<iScaleX)+dChromaOffset[0]+0.5-iFaceWidth/2) < (iFaceHeight-(y*(1<<iScaleY)+dChromaOffset[1]+0.5))/ssqrt(3.0))
          {
#if WSPSNR_MTK_MODIFICATION
            m_fOctaWeight_C[y * (iwidth >> iScaleX)+((4*i+2)>>iScaleX)+i*(iFaceWidth>>iScaleX)+x]=fWeightRotZero_C[((iFaceHeight>>iScaleY)-y-1)*(iFaceWidth>>iScaleX)+(iFaceWidth>>iScaleX)-x-1];
#else
            m_fOctaWeight_C[y*(iFaceWidth>>iScaleX)+((4*i+2)>>iScaleX)+i*(iFaceWidth>>iScaleX)+x]=fWeightRotZero_C[((iFaceHeight>>iScaleY)-y-1)*(iFaceWidth>>iScaleX)+(iFaceWidth>>iScaleX)+((iwidth>>iScaleX)-x-1)];
#endif
          }
        }

        for(Int x=0;x<(iwidth>>iScaleX);x++)
          for (Int y=0;y<(iheight>>iScaleY)/2;y++)
          {
#if WSPSNR_MTK_MODIFICATION
            m_fOctaWeight_C[(iwidth>>iScaleX)*(iheight>>iScaleY)/2+y*(iwidth>>iScaleX)+x]=m_fOctaWeight_C[((iheight >> iScaleY)/2-1-y)*(iwidth>>iScaleX)+((iwidth>>iScaleX)-1-x)];
#else
            m_fOctaWeight_C[(iwidth>>iScaleX)*(iheight>>iScaleY)/2+y*(iwidth>>iScaleX)+x]=m_fOctaWeight_C[((iwidth>>iScaleY)/2-1-y)*(iwidth>>iScaleX)+((iwidth>>iScaleX)-1-x)];
#endif
          }

          free(fWeightRotZero_C);
  }
  else if(pcCodingGeomtry->getType()==SVIDEO_OCTAHEDRON && pCodingSVideoInfo->iCompactFPStructure ==2)
  {
    Int   iwidth  = pcPicD->getWidth(COMPONENT_Y) ;
    Int   iheight = pcPicD->getHeight(COMPONENT_Y) ;
    Double* fWeightRotZero_Y=(Double*)malloc(iFaceWidth*iFaceHeight*sizeof(Double));
    Double* fWeightRotZero_C=(Double*)malloc((iFaceWidth>>iScaleX)*(iFaceHeight>>iScaleY)*sizeof(Double));
    m_fOctaWeight_Y=(Double*)malloc(iwidth*iheight*sizeof(Double));
    m_fOctaWeight_C=(Double*)malloc((iwidth>>iScaleX)*(iheight>>iScaleY)*sizeof(Double));
    Double weightSum_Y=0;
    Double weightSum_C=0;

    for (Int x=0;x<iwidth;x++)
    {
      for (Int y=0;y<iheight;y++)
      {
        m_fOctaWeight_Y[y*iwidth+x]=0;
      }
    }

    for (Int x=0;x<(iwidth>>iScaleX);x++)
    {
      for (Int y=0;y<(iheight>>iScaleY);y++)
      {
        m_fOctaWeight_C[y*(iwidth>>iScaleX)+x]=0;
      }
    }

    for (Int x=0;x<iFaceWidth;x++)
    {
      for (Int y=0;y<iFaceHeight;y++)
      {
        if(sfabs(x+0.5-iFaceWidth/2) < (y+0.5)/ssqrt(3.0))
        {
          Int ci=0;
          Int cj=0;
          Double r2,d2;
          ci = iFaceWidth/2;
          cj = 2*iFaceHeight/3;
          d2 =(x+0.5-ci)*(x+0.5-ci)+(y+0.5-cj)*(y+0.5-cj);
          r2 =(iFaceWidth/ssqrt(6.0))*(iFaceWidth/ssqrt(6.0));
          Double weight = 1.0/((1+d2/r2)*sqrt(1.0*(1+d2/r2)));
          fWeightRotZero_Y[iFaceWidth*y+x]=weight;
          weightSum_Y+=weight;
        }
        else
        {
          fWeightRotZero_Y[iFaceWidth*y+x]=0;
        }
      }
    }

    //Normalize per face_Y
    for (Int x=0;x<iFaceWidth;x++)
    {
      for (Int y=0;y<iFaceHeight;y++)
      {
        fWeightRotZero_Y[iFaceWidth*y+x]=fWeightRotZero_Y[iFaceWidth*y+x]/weightSum_Y/8.0;
      }
    }
    //weights for the entire frame_Y
    for(Int i=0;i<4;i++)
     for (Int x=0;x<iFaceWidth;x++)
      for (Int y=0;y<iFaceHeight;y++)
      {
        if(sfabs(x+0.5-iFaceWidth/2) < (y+0.5)/ssqrt(3.0))
        {
          m_fOctaWeight_Y[y*iwidth+4*i+2+i*iFaceWidth+x]=fWeightRotZero_Y[iFaceWidth*y+x];
        }
      }

    for(Int i=-1;i<4;i++)
     for(Int x=0;x<iFaceWidth;x++)
      for (Int y=0;y<iFaceHeight;y++)
      {
#if WSPSNR_MTK_MODIFICATION
        if(sfabs(x+0.5-iFaceWidth/2) < (iFaceHeight-(y+0.5))/ssqrt(3.0) && 4*i+4+i*iFaceWidth+x+iFaceWidth/2>=0 && 4*i+4+i*iFaceWidth+x+iFaceWidth/2< iwidth)
#else
        if(sfabs(x+0.5-iFaceHeight/2) < (iFaceHeight-(y+0.5))/ssqrt(3.0) && 4*i+4+i*iFaceWidth+x+iFaceWidth/2>=0 && 4*i+4+i*iFaceWidth+x+iFaceWidth/2< iwidth)
#endif
        {
          m_fOctaWeight_Y[y*iwidth+4*i+4+i*iFaceWidth+x+iFaceWidth/2]=fWeightRotZero_Y[(iFaceHeight-y-1)*iFaceWidth+x];
        }
      }

    free(fWeightRotZero_Y);

    for (Int x=0;x<(iFaceWidth>>iScaleX);x++)
    {
      for (Int y=0;y<(iFaceHeight>>iScaleY);y++)
      {
        if(sfabs(x*(1<<iScaleX)+dChromaOffset[0]+0.5-iFaceWidth/2) < (y*(1<<iScaleY)+dChromaOffset[1]+0.5)/ssqrt(3.0))
        {
          Int ci=0;
          Int cj=0;
          Double r2,d2;
          ci = iFaceWidth/2;
          cj = 2*iFaceHeight/3;
          d2 =(x*(1<<iScaleX)+dChromaOffset[0]+0.5-ci)*(x*(1<<iScaleX)+dChromaOffset[0]+0.5-ci) + (y*(1<<iScaleY)+dChromaOffset[1]+0.5-cj)*(y*(1<<iScaleY)+dChromaOffset[1]+0.5-cj);
          r2 =(iFaceWidth/ssqrt(6.0))*(iFaceWidth/ssqrt(6.0));
          Double weight = 1.0/((1+d2/r2)*ssqrt(1.0*(1+d2/r2)));
          fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]=weight;
          weightSum_C+=weight;
        }
        else
        {
          fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]=0;
        }
      }
    }

    //Normalize per face_C
    for (Int x=0;x<(iFaceWidth>>iScaleX);x++)
    {
      for (Int y=0;y<(iFaceHeight>>iScaleY);y++)
      {
        fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]=fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]/weightSum_C/8.0;
      }
    }
    //weights for the entire frame_C
    for(Int i=0;i<4;i++)
      for (Int x=0;x<(iFaceWidth>>iScaleX);x++)
        for (Int y=0;y<(iFaceHeight>>iScaleY);y++)
        {
          if(sfabs(x*(1<<iScaleX)+dChromaOffset[0]+0.5-iFaceWidth/2) < (y*(1<<iScaleY)+dChromaOffset[1]+0.5)/sqrt(3.0))
          {
#if WSPSNR_MTK_MODIFICATION
            m_fOctaWeight_C[y*(iwidth >> iScaleX)+2*i+1+i*(iFaceWidth>>iScaleX)+x]=fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x];
#else
            m_fOctaWeight_C[y*(iFaceWidth>>iScaleX)+2*i+1+i*(iFaceWidth>>iScaleX)+x]=fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x];
#endif
          }
        }

        for(Int i=-1;i<4;i++)
          //for(Int x=0;x<m_iCodingFaceWidth;x++)             //WS-PSNR checking;
          for(Int x=0;x<(iFaceWidth>>iScaleX);x++)
            for (Int y=0;y<(iFaceHeight>>iScaleY);y++)
            {
              if(sfabs(x*(1<<iScaleX)+dChromaOffset[0]+0.5-iFaceWidth/2) < (iFaceHeight-(y*(1<<iScaleY)+dChromaOffset[1]+0.5))/ssqrt(3.0) && 4*i+4+i*iFaceWidth+(x<<iScaleX)+iFaceWidth/2>=0 && 4*i+4+i*iFaceWidth+(x<<iScaleX)+iFaceWidth/2< iwidth)
              {
#if WSPSNR_MTK_MODIFICATION
                m_fOctaWeight_C[y*(iwidth>>iScaleX)+2*i+2+i*(iFaceWidth>>iScaleX)+x+((iFaceWidth/2)>>iScaleX)]=fWeightRotZero_C[((iFaceHeight>>iScaleY)-y-1)*(iFaceWidth>>iScaleX)+x];
#else
                m_fOctaWeight_C[y*(iFaceWidth>>iScaleX)+2*i+2+i*(iFaceWidth>>iScaleX)+x+((iFaceWidth/2)>>iScaleX)]=fWeightRotZero_C[((iFaceHeight>>iScaleY)-y-1)*(iFaceWidth>>iScaleX)+x];
#endif
              }
            }
    free(fWeightRotZero_C);
  }
  else if(pcCodingGeomtry->getType()==SVIDEO_ICOSAHEDRON && pCodingSVideoInfo->iCompactFPStructure ==0)
  {
    Int iwidth=iFaceWidth*pCodingSVideoInfo->framePackStruct.cols;
    Int iheight=iFaceHeight*pCodingSVideoInfo->framePackStruct.rows;
    Double* fWeightRotZero_Y=(Double*)malloc(iFaceWidth*iFaceHeight*sizeof(Double));
    Double* fWeightRotZero_C=(Double*)malloc((iFaceWidth>>iScaleX)*(iFaceHeight>>iScaleY)*sizeof(Double));
    m_fIcoWeight_Y=(Double*)malloc(iwidth*iheight*sizeof(Double));
    m_fIcoWeight_C=(Double*)malloc((iwidth>>iScaleX)*(iheight>>iScaleY)*sizeof(Double));
    Double Ratio_r= 4*ssqrt(3.0)/(3+ssqrt(5.0));
    Double weightSum_Y=0;
    Double weightSum_C=0;

    for (Int x=0;x<iFaceWidth;x++)
    {
      for (Int y=0;y<iFaceHeight;y++)
      {
        if(sfabs(x+0.5-iFaceWidth/2) < (y+0.5)/ssqrt(3.0))
        {
          Int ci=0;
          Int cj=0;
          Double r2,d2;
          ci = iFaceWidth/2;
          cj = 2*iFaceHeight/3;
          d2 =(x+0.5-ci)*(x+0.5-ci)+(y+0.5-cj)*(y+0.5-cj);
          r2 =(iFaceWidth/Ratio_r)*(iFaceWidth/Ratio_r);
          Double weight = 1.0/((1+d2/r2)*sqrt(1.0*(1+d2/r2)));
          fWeightRotZero_Y[iFaceWidth*y+x]=weight;
          weightSum_Y+=weight;
        }
        else
        {
          fWeightRotZero_Y[iFaceWidth*y+x]=0;
        }
      }
    }

    //Normalize per face_Y
    for (Int x=0;x<iFaceWidth;x++)
    {
      for (Int y=0;y<iFaceHeight;y++)
      {
        fWeightRotZero_Y[iFaceWidth*y+x]=fWeightRotZero_Y[iFaceWidth*y+x]/weightSum_Y/20.0;
      }
    }

    //weights for the entire frame_Y
    Double weight;
    for (Int rows=0;rows<pCodingSVideoInfo->framePackStruct.rows;rows++)
    {
      Int ypos=rows*iFaceHeight;
      for (Int cols=0;cols<pCodingSVideoInfo->framePackStruct.cols;cols++)
      {
        Int xpos=cols*iFaceWidth;
        for (Int x=0;x<iFaceWidth;x++)
        {
          for (Int y=0;y<iFaceHeight;y++)
          {
            if(pCodingSVideoInfo->framePackStruct.faces[rows][cols].rot==0)
            {
              weight=fWeightRotZero_Y[y*iFaceWidth+x];
            }
            else
            {
              weight=fWeightRotZero_Y[(iFaceHeight-y-1)*iFaceWidth+(iFaceWidth-x-1)];
            }
            m_fIcoWeight_Y[(ypos+y)*iwidth+xpos+x]=weight;
          }
        }
      }
    }
    free(fWeightRotZero_Y);

    for (Int x=0;x<(iFaceWidth>>iScaleX);x++)
    {
      for (Int y=0;y<(iFaceHeight>>iScaleY);y++)
      {
        if(sfabs(x*(1<<iScaleX)+dChromaOffset[0]+0.5-iFaceWidth/2) < (y*(1<<iScaleY)+dChromaOffset[1]+0.5)/sqrt(3.0))
        {
          Int ci=0;
          Int cj=0;
          Double r2,d2;
          ci = iFaceWidth/2;
          cj = 2*iFaceHeight/3;
          d2 =(x*(1<<iScaleX)+dChromaOffset[0]+0.5-ci)*(x*(1<<iScaleX)+dChromaOffset[0]+0.5-ci) + (y*(1<<iScaleY)+dChromaOffset[1]+0.5-cj)*(y*(1<<iScaleY)+dChromaOffset[1]+0.5-cj);
          r2 =(iFaceWidth/Ratio_r)*(iFaceWidth/Ratio_r);
          weight = 1.0/((1+d2/r2)*sqrt(1.0*(1+d2/r2)));
          fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]=weight;
          weightSum_C+=weight;
        }
        else
        {
          fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]=0;
        }
      }
    }

    //Normalize per face_C
    for (Int x=0;x<(iFaceWidth>>iScaleX);x++)
    {
      for (Int y=0;y<(iFaceHeight>>iScaleY);y++)
      {
        fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]=fWeightRotZero_C[(iFaceWidth>>iScaleX)*y+x]/weightSum_C/20.0;
      }
    }
    //weights for the entire frame_C
    for (Int rows=0;rows<pCodingSVideoInfo->framePackStruct.rows;rows++)
    {
      Int ypos=rows*(iFaceHeight>>iScaleY);
      for (Int cols=0;cols<pCodingSVideoInfo->framePackStruct.cols;cols++)
      {
        Int xpos=cols*(iFaceWidth>>iScaleX);
        for (Int x=0;x<(iFaceWidth>>iScaleX);x++)
        {
          for (Int y=0;y<(iFaceHeight>>iScaleY);y++)
          {
            if(pCodingSVideoInfo->framePackStruct.faces[rows][cols].rot==0)
            {
              weight=fWeightRotZero_C[y*(iFaceWidth>>iScaleX)+x];
            }
            else
            {
              weight=fWeightRotZero_C[((iFaceHeight>>iScaleY)-y-1)*(iFaceWidth>>iScaleX)+((iFaceWidth>>iScaleX)-1-x)];
            }
            m_fIcoWeight_C[(ypos+y)*(iwidth>>iScaleX)+xpos+x]=weight;
          }
        }
      }
    }
    free(fWeightRotZero_C);
  }
  else
  {
    printf("WS-PSNR is not support for this format: GeoType:%d, FramePackingType:%d!\n", pcCodingGeomtry->getType(), pCodingSVideoInfo->iCompactFPStructure); 
    assert(!"Checking configruation parameters!\n");
  }
}

Void TWSPSNRMetric::xCalculateWSPSNR( TComPicYuv* pcOrgPicYuv, TComPicYuv* pcPicD )
{
  Int iBitDepthForPSNRCalc[MAX_NUM_CHANNEL_TYPE];
  Int iReferenceBitShift[MAX_NUM_CHANNEL_TYPE];
  Int iOutputBitShift[MAX_NUM_CHANNEL_TYPE];
  iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] = std::max(m_outputBitDepth[CHANNEL_TYPE_LUMA], m_referenceBitDepth[CHANNEL_TYPE_LUMA]);
  iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] = std::max(m_outputBitDepth[CHANNEL_TYPE_CHROMA], m_referenceBitDepth[CHANNEL_TYPE_CHROMA]);
  iReferenceBitShift[CHANNEL_TYPE_LUMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] - m_referenceBitDepth[CHANNEL_TYPE_LUMA];
  iReferenceBitShift[CHANNEL_TYPE_CHROMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] - m_referenceBitDepth[CHANNEL_TYPE_CHROMA];
  iOutputBitShift[CHANNEL_TYPE_LUMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_LUMA] - m_outputBitDepth[CHANNEL_TYPE_LUMA];
  iOutputBitShift[CHANNEL_TYPE_CHROMA] = iBitDepthForPSNRCalc[CHANNEL_TYPE_CHROMA] - m_outputBitDepth[CHANNEL_TYPE_CHROMA];

  memset(m_dWSPSNR, 0, sizeof(Double)*3);
  TComPicYuv &picd=*pcPicD;
  //Double SSDspsnr[3]={0, 0 ,0};
  //ChromaFormat chromaFormat = pcPicD->getChromaFormat();

  for(Int chan=0; chan<pcPicD->getNumberValidComponents(); chan++)
  {
    const ComponentID ch=ComponentID(chan);
    const Pel*  pOrg       = pcOrgPicYuv->getAddr(ch);
    const Int   iOrgStride = pcOrgPicYuv->getStride(ch);
    const Pel*  pRec       = picd.getAddr(ch);
    const Int   iRecStride = picd.getStride(ch);
    const Int   iWidth  = pcPicD->getWidth (ch) ;
    const Int   iHeight = pcPicD->getHeight(ch) ;
    Double fWeight =1;
    Double fWeightSum=0;
    //Int   iSize   = iWidth*iHeight;

    Double SSDwpsnr=0;
    //WS-PSNR
    for(Int y = 0; y < iHeight; y++ )
    {
      if (m_codingGeoType==SVIDEO_EQUIRECT)
      {      
        if(!chan)
        {
          fWeight=m_fErpWeight_Y[y];
        }
        else
        {
          fWeight=m_fErpWeight_C[y];
        }
      }
      for(Int x = 0; x < iWidth; x++ )
      {
        Intermediate_Int iDiff = (Intermediate_Int)( (pOrg[x]<<iReferenceBitShift[toChannelType(ch)]) - (pRec[x]<<iOutputBitShift[toChannelType(ch)]) );
        if (m_codingGeoType==SVIDEO_CUBEMAP)
        {
          if(!chan)
          {
            if(iWidth/4 == iHeight/3 && x >= iWidth/4 && (y< iHeight/3 || y>= 2*iHeight/3))
            {
              fWeight=0;
            }
            else 
            {
              fWeight=m_fCubeWeight_Y[(m_iCodingFaceWidth)*(y%(m_iCodingFaceHeight)) +(x%(m_iCodingFaceWidth))];
            }

          }
          else
          {
            if(iWidth/4 == iHeight/3 && x >= iWidth/4 && (y< iHeight/3 || y>= 2*iHeight/3))
            {
              fWeight=0;
            }
            else
            {
              fWeight=m_fCubeWeight_C[(m_iCodingFaceWidth>>(pcPicD->getComponentScaleX(COMPONENT_Cb)))*(y%(m_iCodingFaceHeight>>(pcPicD->getComponentScaleY(COMPONENT_Cb)))) +(x%(m_iCodingFaceWidth>>(pcPicD->getComponentScaleX(COMPONENT_Cb))))];
            }  
          }
        }

        else if (m_codingGeoType==SVIDEO_EQUALAREA)
        {
          if(!chan)
          {
            fWeight=m_fEapWeight_Y[y*iWidth+x];
          }
          else
          {
            fWeight=m_fEapWeight_C[y*iWidth+x];
          }
        }

        else if (m_codingGeoType==SVIDEO_OCTAHEDRON )
        {
          if(!chan)
          {
            fWeight=m_fOctaWeight_Y[iWidth*y +x];
          }
          else
          {
            fWeight=m_fOctaWeight_C[y*iWidth+x];
          }
        }

        else if ( m_codingGeoType==SVIDEO_ICOSAHEDRON)
        {
          if(!chan)
          {
            fWeight=m_fIcoWeight_Y[iWidth*y +x];
          }
          else
          {
            fWeight=m_fIcoWeight_C[y*iWidth+x];
          }
        }

        if(fWeight>0)
          fWeightSum += fWeight;
        SSDwpsnr   += iDiff * iDiff*fWeight;
      }
      pOrg += iOrgStride;
      pRec += iRecStride;

    }
    const Int maxval = 255<<(iBitDepthForPSNRCalc[toChannelType(ch)]-8) ;
    //const Double fRefValue = (Double) maxval * maxval * iSize;

    m_dWSPSNR[ch]         = ( SSDwpsnr ? 10.0 * log10( (maxval * maxval*fWeightSum) / (Double)SSDwpsnr ) : 999.99 );
  }

}

#if SVIDEO_WSPSNR_E2E
Void TWSPSNRMetric::setCodingGeoInfo2(SVideoInfo& sRefVideoInfo, SVideoInfo& sRecVideoInfo, InputGeoParam *pInGeoParam, TVideoIOYuv& yuvInputFile, Int iInputWidth, Int iInputHeight, UInt tempSubsampleRatio)
{
  m_codingGeoType = sRefVideoInfo.geoType; 
  m_iCodingFaceWidth = sRefVideoInfo.iFaceWidth; 
  m_iCodingFaceHeight = sRefVideoInfo.iFaceHeight; 
  m_iChromaSampleLocType = pInGeoParam->iChromaSampleLocType;
  m_temporalSubsampleRatio = tempSubsampleRatio;
  m_iInputWidth = iInputWidth;
  m_iInputHeight = iInputHeight;
  m_inputChromaFomat = sRefVideoInfo.framePackStruct.chromaFormatIDC;

  //Rec geometry to reference geometry;
  if(m_bEnabled)
  {
    m_pcTVideoIOYuvInputFile = &yuvInputFile;
    m_pRefGeometry = TGeometry::create(sRefVideoInfo, pInGeoParam);
    m_pRecGeometry = TGeometry::create(sRecVideoInfo, pInGeoParam);
    m_pcOrgPicYuv = new TComPicYuv;
    m_pcOrgPicYuv->createWithoutCUInfo(iInputWidth, iInputHeight, m_inputChromaFomat, false, S_PAD_MAX, S_PAD_MAX);
    m_pcRecPicYuv = new TComPicYuv;
    m_pcRecPicYuv->createWithoutCUInfo(iInputWidth, iInputHeight, m_inputChromaFomat, true, S_PAD_MAX, S_PAD_MAX);
  }

}
Void TWSPSNRMetric::xCalculateE2EWSPSNR( TComPicYuv *pcPicYuv, Int iPOC )
{
  if(!m_bEnabled)
    return;

  Int iDeltaFrames = iPOC*m_temporalSubsampleRatio - m_iLastFrmPOC;
  Int aiPad[2]={0,0};
  m_pcTVideoIOYuvInputFile->skipFrames(iDeltaFrames, m_iInputWidth, m_iInputHeight, m_inputChromaFomat);
  m_pcTVideoIOYuvInputFile->read(NULL, m_pcOrgPicYuv, IPCOLOURSPACE_UNCHANGED, aiPad, m_inputChromaFomat, false );
  m_iLastFrmPOC = iPOC*m_temporalSubsampleRatio+1;

  //generate the reconstructed picture in source gemoetry domain;
  if((m_pRecGeometry->getType() == SVIDEO_OCTAHEDRON || m_pRecGeometry->getType() == SVIDEO_ICOSAHEDRON) && m_pRecGeometry->getSVideoInfo()->iCompactFPStructure) 
    m_pRecGeometry->compactFramePackConvertYuv(pcPicYuv);
  else
    m_pRecGeometry->convertYuv(pcPicYuv);
  m_pRecGeometry->geoConvert(m_pRefGeometry);
  if((m_pRefGeometry->getType() == SVIDEO_OCTAHEDRON || m_pRefGeometry->getType() == SVIDEO_ICOSAHEDRON) && m_pRefGeometry->getSVideoInfo()->iCompactFPStructure)
    m_pRefGeometry->compactFramePack(m_pcRecPicYuv);
  else
    m_pRefGeometry->framePack(m_pcRecPicYuv);

  xCalculateWSPSNR(m_pcOrgPicYuv, m_pcRecPicYuv);
}
#endif  //SVIDEO_WSPSNR_E2E
#endif