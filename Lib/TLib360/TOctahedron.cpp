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

/** \file     TOctahedron.cpp
    \brief    Octahedron class
*/

#include <assert.h>
#include <math.h>
#include "TOctahedron.h"

#if SVIDEO_EXT

/*****************************
Octahedron projection
******************************/
const POSType TOctahedron::m_octaVertices[6][3] = {  { 0,  ssqrt(2),  0},
                                                     { 1,     0,      1},
                                                     { 1,     0,     -1},
                                                     { 0, -ssqrt(2),  0},
                                                     {-1,     0,     -1},
                                                     {-1,     0,      1},
                                              };
TOctahedron::TOctahedron(SVideoInfo& sVideoInfo, InputGeoParam *pInGeoParam) : TGeometry()
{
  geoInit(sVideoInfo, pInGeoParam);

  if(sVideoInfo.geoType == SVIDEO_OCTAHEDRON )  //TIcosahedron;
  {
    if((sVideoInfo.iCompactFPStructure == 0) || (sVideoInfo.iCompactFPStructure == 2))
    {
      //trimesh face 0; PX top;
      //assert(sVideoInfo.geoType == SVIDEO_OCTAHEDRON);
      assert(sVideoInfo.iNumFaces == 8);
      memcpy(m_meshFaces[0].vertex[0], m_octaVertices[0], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[0].vertex[1], m_octaVertices[1], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[0].vertex[2], m_octaVertices[2], 3*sizeof(m_octaVertices[0][0]));

      //trimesh face 1;  PX bottom;
      memcpy(m_meshFaces[1].vertex[0], m_octaVertices[3], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[1].vertex[1], m_octaVertices[2], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[1].vertex[2], m_octaVertices[1], 3*sizeof(m_octaVertices[0][0]));

      //trimesh face 2;  NX top;
      memcpy(m_meshFaces[2].vertex[0], m_octaVertices[0], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[2].vertex[1], m_octaVertices[4], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[2].vertex[2], m_octaVertices[5], 3*sizeof(m_octaVertices[0][0]));

      //trimesh face 3;  NX bottom;
      memcpy(m_meshFaces[3].vertex[0], m_octaVertices[3], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[3].vertex[1], m_octaVertices[5], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[3].vertex[2], m_octaVertices[4], 3*sizeof(m_octaVertices[0][0]));

      //trimesh face 4;  PZ top;
      memcpy(m_meshFaces[4].vertex[0], m_octaVertices[0], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[4].vertex[1], m_octaVertices[5], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[4].vertex[2], m_octaVertices[1], 3*sizeof(m_octaVertices[0][0]));

      //trimesh face 5;  PZ bottom;
      memcpy(m_meshFaces[5].vertex[0], m_octaVertices[3], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[5].vertex[1], m_octaVertices[1], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[5].vertex[2], m_octaVertices[5], 3*sizeof(m_octaVertices[0][0]));

      //trimesh face 6;  NZ top;
      memcpy(m_meshFaces[6].vertex[0], m_octaVertices[0], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[6].vertex[1], m_octaVertices[2], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[6].vertex[2], m_octaVertices[4], 3*sizeof(m_octaVertices[0][0]));

      //trimesh face 7;  NZ bottom;
      memcpy(m_meshFaces[7].vertex[0], m_octaVertices[3], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[7].vertex[1], m_octaVertices[4], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[7].vertex[2], m_octaVertices[2], 3*sizeof(m_octaVertices[0][0]));
    }
    else if(sVideoInfo.iCompactFPStructure == 1)
    {
      //trimesh face 0; PX top;
      //assert(sVideoInfo.geoType == SVIDEO_OCTAHEDRON);
      assert(sVideoInfo.iNumFaces == 8);
      memcpy(m_meshFaces[0].vertex[0], m_octaVertices[0], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[0].vertex[1], m_octaVertices[1], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[0].vertex[2], m_octaVertices[2], 3*sizeof(m_octaVertices[0][0]));

      //trimesh face 1;  PX bottom;
      memcpy(m_meshFaces[1].vertex[0], m_octaVertices[3], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[1].vertex[1], m_octaVertices[2], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[1].vertex[2], m_octaVertices[1], 3*sizeof(m_octaVertices[0][0]));

      //trimesh face 2;  NX top;
      memcpy(m_meshFaces[2].vertex[0], m_octaVertices[0], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[2].vertex[1], m_octaVertices[4], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[2].vertex[2], m_octaVertices[5], 3*sizeof(m_octaVertices[0][0]));

      //trimesh face 3;  NX bottom;
      memcpy(m_meshFaces[3].vertex[0], m_octaVertices[3], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[3].vertex[1], m_octaVertices[5], 3*sizeof(m_octaVertices[0][0]));
      memcpy(m_meshFaces[3].vertex[2], m_octaVertices[4], 3*sizeof(m_octaVertices[0][0]));


      //trimesh face 4;  PZ top;
      memcpy(m_meshFaces[4].vertex[0], m_octaVertices[1], 3*sizeof(m_octaVertices[0][0]));      // 3
      memcpy(m_meshFaces[4].vertex[1], m_octaVertices[0], 3*sizeof(m_octaVertices[0][0]));      // 1
      memcpy(m_meshFaces[4].vertex[2], m_octaVertices[5], 3*sizeof(m_octaVertices[0][0]));      // 5

      //trimesh face 5;  PZ bottom;
      memcpy(m_meshFaces[5].vertex[0], m_octaVertices[1], 3*sizeof(m_octaVertices[0][0]));      // 0
      memcpy(m_meshFaces[5].vertex[1], m_octaVertices[5], 3*sizeof(m_octaVertices[0][0]));      // 5
      memcpy(m_meshFaces[5].vertex[2], m_octaVertices[3], 3*sizeof(m_octaVertices[0][0]));      // 1


      //trimesh face 6;  NZ top;
      memcpy(m_meshFaces[6].vertex[0], m_octaVertices[2], 3*sizeof(m_octaVertices[0][0]));      // 3
      memcpy(m_meshFaces[6].vertex[1], m_octaVertices[4], 3*sizeof(m_octaVertices[0][0]));      // 4
      memcpy(m_meshFaces[6].vertex[2], m_octaVertices[0], 3*sizeof(m_octaVertices[0][0]));      // 2


      //trimesh face 7;  NZ bottom;
      memcpy(m_meshFaces[7].vertex[0], m_octaVertices[2], 3*sizeof(m_octaVertices[0][0]));      // 0
      memcpy(m_meshFaces[7].vertex[1], m_octaVertices[3], 3*sizeof(m_octaVertices[0][0]));      // 2
      memcpy(m_meshFaces[7].vertex[2], m_octaVertices[4], 3*sizeof(m_octaVertices[0][0]));      // 4
    }
    //init faces;
    for(Int f=0; f<sVideoInfo.iNumFaces; f++)
      initTriMesh(m_meshFaces[f]);
  }
}

Void TOctahedron::map2DTo3D(SPos& IPosIn, SPos *pSPosOut)
{ 
  pSPosOut->faceIdx = IPosIn.faceIdx;
  POSType u, v;
  POSType pu, pv; //positin in the plane of unit sphere;
  u = IPosIn.x;
  v = IPosIn.y;
  u += (POSType)(0.5);
  v += (POSType)(0.5);
  pu = (POSType)((2.0*u)/m_sVideoInfo.iFaceWidth);
  pv = (POSType)((ssqrt(3.0f)*v)/m_sVideoInfo.iFaceHeight);
  pSPosOut->x = m_meshFaces[pSPosOut->faceIdx].origin[0] + pu*m_meshFaces[pSPosOut->faceIdx].baseVec[0][0] + pv*m_meshFaces[pSPosOut->faceIdx].baseVec[1][0];
  pSPosOut->y = m_meshFaces[pSPosOut->faceIdx].origin[1] + pu*m_meshFaces[pSPosOut->faceIdx].baseVec[0][1] + pv*m_meshFaces[pSPosOut->faceIdx].baseVec[1][1];
  pSPosOut->z = m_meshFaces[pSPosOut->faceIdx].origin[2] + pu*m_meshFaces[pSPosOut->faceIdx].baseVec[0][2] + pv*m_meshFaces[pSPosOut->faceIdx].baseVec[1][2]; 
}

Void TOctahedron::map3DTo2D(SPos *pSPosIn, SPos *pSPosOut)
{
  Int iFaceIdx=0;
  POSType dMax = std::numeric_limits<POSType>::min();
  POSType pu, pv;
  //determine face idx;
  for(Int f=0; f<m_sVideoInfo.iNumFaces; f++)
  {
    POSType d = (pSPosIn->x*m_meshFaces[f].normVec[0] + pSPosIn->y*m_meshFaces[f].normVec[1] + pSPosIn->z*m_meshFaces[f].normVec[2]);
    if(d >dMax)
    {
      iFaceIdx = f;
      dMax = d;
    }
  }
  TriMesh& tmFace = m_meshFaces[iFaceIdx];
  POSType d = tmFace.origin[0] *tmFace.normVec[0] + tmFace.origin[1]*tmFace.normVec[1] + tmFace.origin[2]*tmFace.normVec[2];
  POSType projected[3];
  projected[0] = pSPosIn->x * d/dMax - tmFace.origin[0];
  projected[1] = pSPosIn->y * d/dMax - tmFace.origin[1];
  projected[2] = pSPosIn->z * d/dMax - tmFace.origin[2];
  pu = projected[0]*tmFace.baseVec[0][0] + projected[1]*tmFace.baseVec[0][1] + projected[2]*tmFace.baseVec[0][2];
  pv = projected[0]*tmFace.baseVec[1][0] + projected[1]*tmFace.baseVec[1][1] + projected[2]*tmFace.baseVec[1][2];
  //convert pu, pv to [0, width], [0, height];
  pSPosOut->faceIdx = iFaceIdx;
  pSPosOut->z = 0;
  pSPosOut->x = (POSType)(pu*(m_sVideoInfo.iFaceWidth>>1) + (-0.5));
  pSPosOut->y = (POSType)(pv*(m_sVideoInfo.iFaceHeight)/ssqrt(3.0f)+ (-0.5));
}

Void TOctahedron::clamp(IPos *pIPos)
{
  Int x = pIPos->u;
  Int y = pIPos->v;
  y = Clip3(0, m_sVideoInfo.iFaceHeight-1, y);
  {
    Int x_L = x;
    Int y_L = y;

    Int x0, x1;
    if(y_L <= ((m_sVideoInfo.iFaceHeight>>1)-1))
    {
      Double d = 1.0*y_L/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
      Int  d_i = (Int(d + 1)>>1)<<1;
      x0 = (m_sVideoInfo.iFaceWidth>>1) - 2 - d_i;
      x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
    }
    else
    {
      Int y_m = m_sVideoInfo.iFaceHeight - 1 - y_L;
      Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
      Int  d_i = (Int(d + 1)>>1)<<1;
      x0 = d_i;
      x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
    }
    x = Clip3(x0, x1, x_L);
  }
  pIPos->u = x;
  pIPos->v = y;
}

Bool TOctahedron::insideFace(Int fId, Int x, Int y, ComponentID chId, ComponentID origchId)
{

  if(y<0 || y>=(m_sVideoInfo.iFaceHeight >> getComponentScaleY(chId)))
    return false;

  if(m_chromaFormatIDC == CHROMA_444 || origchId == COMPONENT_Y)
  {
    Int x_L = x<<getComponentScaleX(chId);
    Int y_L = y<<getComponentScaleY(chId);

    Int x0, x1;
    if(y_L <= ((m_sVideoInfo.iFaceHeight>>1)-1))
    {
      Double d = 1.0*y_L/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);//Brave: similar triangles
      Int  d_i = (Int(d + 1)>>1)<<1;
      x0 = (m_sVideoInfo.iFaceWidth>>1) - 2 - d_i;
      x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
    }
    else
    {
      Int y_m = m_sVideoInfo.iFaceHeight - 1 - y_L;
      Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
      Int  d_i = (Int(d + 1)>>1)<<1;
      x0 = d_i;
      x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
    }

    return (x_L >= x0 && x_L <= x1);
  }
  else  // chroma in 420
  {
    Int x_L = x<<getComponentScaleX(chId);
    Int y_L = y<<getComponentScaleY(chId);
    Int x_c = x_L>>getComponentScaleX(origchId);
    //Int y_c = y_L>>getComponentScaleY(origchId);

    if(y_L >= (m_sVideoInfo.iFaceHeight>>1))
    {
      y_L ++; 
    } 
  
    Int x0, x1;
    if(y_L <= ((m_sVideoInfo.iFaceHeight>>1)-1))
    {
      Double d = 1.0*y_L/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
      Int  d_i = (Int(d + 1)>>1)<<1;
      x0 = (m_sVideoInfo.iFaceWidth>>1) - 2 - d_i;
      x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
    }
    else
    {
      Int y_m = m_sVideoInfo.iFaceHeight - 1 - y_L;
      Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
      Int  d_i = (Int(d + 1)>>1)<<1;
      x0 = d_i;
      x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
    }
    x0 = ((x0+1)>>1);
    x1 = (x1>>1);

    return (x_c >= x0 && x_c <= x1);
  }
}

Bool TOctahedron::validPosition4Interp(ComponentID chId, POSType x, POSType y)
{
  ChannelType chType = toChannelType(chId);
  if(m_InterpolationType[chType] == SI_NN)
  {
    Int xi = round(x);
    Int yi = round(y);
    return insideFace(0, xi, yi, chId, chId);
  }
  else if(m_InterpolationType[chType] == SI_BILINEAR)
  {
    Int xi = (Int)sfloor(x);
    Int yi = (Int)sfloor(y);
    return (insideFace(0, xi, yi, chId, chId) && insideFace(0, xi, yi+1, chId, chId) && insideFace(0, xi+1, yi, chId, chId) && insideFace(0, xi+1, yi+1, chId, chId) );
  }
  else if(m_InterpolationType[chType] == SI_BICUBIC)
  {
    Int xi = (Int)sfloor(x);
    Int yi = (Int)sfloor(y);
    return (   insideFace(0, xi-1, yi-1, chId, chId) && insideFace(0, xi, yi-1, chId, chId) && insideFace(0, xi+1, yi-1, chId, chId) && insideFace(0, xi+2, yi-1, chId, chId)
            && insideFace(0, xi-1, yi, chId, chId) && insideFace(0, xi, yi, chId, chId) && insideFace(0, xi+1, yi, chId, chId) && insideFace(0, xi+2, yi, chId, chId)
            && insideFace(0, xi-1, yi+1, chId, chId) && insideFace(0, xi, yi+1, chId, chId) && insideFace(0, xi+1, yi+1, chId, chId) && insideFace(0, xi+2, yi+1, chId, chId)
            && insideFace(0, xi-1, yi+2, chId, chId) && insideFace(0, xi, yi+2, chId, chId) && insideFace(0, xi+1, yi+2, chId, chId) && insideFace(0, xi+2, yi+2, chId, chId)
           );
  }
  else if(m_InterpolationType[chType] == SI_LANCZOS2 || m_InterpolationType[chType] == SI_LANCZOS3)
  {
    Int xi = (Int)sfloor(x);
    Int yi = (Int)sfloor(y);
    Bool bInside = true;
    for(Int j=0; j<(m_iLanczosParamA[chType]<<1); j++)
    {
      for(Int i=0; i<(m_iLanczosParamA[chType]<<1); i++)
        if(!insideFace(0, xi-m_iLanczosParamA[chType]+1+i, yi-m_iLanczosParamA[chType]+1+j, chId, chId))
        {
          bInside = false;
          break;
        }
      if(!bInside)
        break;
    }
    return bInside;
  }
  else
    assert(!"Not supported\n");

  return false;
}

Void TOctahedron::convertYuv(TComPicYuv *pSrcYuv)
{
  Int nWidth = m_sVideoInfo.iFaceWidth;
  Int nHeight = m_sVideoInfo.iFaceHeight;

  assert(pSrcYuv->getWidth(ComponentID(0)) == nWidth*m_sVideoInfo.framePackStruct.cols && pSrcYuv->getHeight(ComponentID(0)) == nHeight*m_sVideoInfo.framePackStruct.rows);
  assert(pSrcYuv->getNumberValidComponents() == getNumChannels());

  if(pSrcYuv->getChromaFormat()==CHROMA_420)
  {
    Int nFaces = m_sVideoInfo.iNumFaces;

    for(Int ch=0; ch<pSrcYuv->getNumberValidComponents(); ch++)
    {
      ComponentID chId = ComponentID(ch);
      nWidth = m_sVideoInfo.iFaceWidth >> pSrcYuv->getComponentScaleX(chId);
      nHeight = m_sVideoInfo.iFaceHeight >> pSrcYuv->getComponentScaleY(chId); 

      //Int iStrideTmpBuf = pSrcYuv->getStride(chId);
      if(!ch || (m_chromaFormatIDC==CHROMA_420 && !m_bResampleChroma))
      {
        for(Int faceIdx=0; faceIdx<nFaces; faceIdx++)
        {
          Int faceX = m_facePos[faceIdx][1]*nWidth;
          Int faceY = m_facePos[faceIdx][0]*nHeight;
          assert(faceIdx == m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].id);
          Int iRot = m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].rot;
          Int iStrideSrc = pSrcYuv->getStride((ComponentID)(ch));
          Pel *pSrc = pSrcYuv->getAddr((ComponentID)ch) + faceY*iStrideSrc + faceX;
          Pel *pDst = m_pFacesOrig[faceIdx][ch];
          rotFaceChannelGeneral(pSrc, nWidth, nHeight, pSrcYuv->getStride((ComponentID)ch), 1, iRot, pDst, getStride((ComponentID)ch), 1, true);
          
          //fill in a rectangular;
          pSrc = m_pFacesOrig[faceIdx][ch];
          //left/right;
          for(Int row=0; row<nHeight; row++)
          {
            Int x0, x1;
            if(ch == 0)
            {
              if(row <= ((nHeight>>1)-1))
              {
                Double d = 1.0*row/(nHeight-1)*((nWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = (nWidth>>1) - 2 - d_i;
                x1 = nWidth - 1 - x0;
              }
              else
              {
                Int y_m = nHeight - 1 - row;
                Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = d_i;
                x1 = nWidth - 1 - x0;
              }
            }
            else
            {
              Int y_L = row<<getComponentScaleY(chId);
              if(y_L >= (m_sVideoInfo.iFaceHeight>>1))
              {
                y_L ++; 
              } 
              if(y_L <= ((m_sVideoInfo.iFaceHeight>>1)-1))
              {
                Double d = 1.0*y_L/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = (m_sVideoInfo.iFaceWidth>>1) - 2 - d_i;
                x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
              }
              else
              {
                Int y_m = m_sVideoInfo.iFaceHeight - 1 - y_L;
                Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = d_i;
                x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
              }

              x0 = ((x0 + 1)>>1);
              x1 = x1>>1;
            }

            //left;
            Pel* pSrcLine = pSrc + x0;
            pDst = pSrcLine - 1;
            for(Int j=0; j<x0; j++)
                *pDst-- = *pSrcLine;
            //right;
            pSrcLine = pSrc + x1;
            pDst = pSrcLine + 1;
            for(Int j=0; j<nWidth-1-x1; j++)
                *pDst++ = *pSrcLine;

            pSrc += getStride((ComponentID)ch);
          }
        }
        continue;
      }

      //memory allocation;
      if(!m_pFacesBufTemp)
      {
        assert(!m_pFacesBufTempOrig);
        m_nMarginSizeBufTemp = 2;  //4 tap filter;
        m_nStrideBufTemp = nWidth + (m_nMarginSizeBufTemp<<1);
        m_pFacesBufTemp = new Pel*[nFaces];
        memset(m_pFacesBufTemp, 0, sizeof(Pel*)*nFaces);
        m_pFacesBufTempOrig = new Pel*[nFaces];
        memset(m_pFacesBufTempOrig, 0, sizeof(Pel*)*nFaces);
        Int iTotalHeight = (nHeight+(m_nMarginSizeBufTemp<<1));
        for(Int i=0; i<nFaces; i++)
        {
          m_pFacesBufTemp[i] = (Pel *)xMalloc(Pel,  m_nStrideBufTemp*iTotalHeight);
          m_pFacesBufTempOrig[i] = m_pFacesBufTemp[i] +  m_nStrideBufTemp * m_nMarginSizeBufTemp + m_nMarginSizeBufTemp;
        }
      }
      //read content first;
      for(Int faceIdx=0; faceIdx<nFaces; faceIdx++)
      {
        Int faceX = m_facePos[faceIdx][1]*nWidth;
        Int faceY = m_facePos[faceIdx][0]*nHeight;
        assert(faceIdx == m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].id);
        Int iRot = m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].rot;

        Int iStrideSrc = pSrcYuv->getStride((ComponentID)(ch));
        Pel *pSrc = pSrcYuv->getAddr((ComponentID)ch) + faceY*iStrideSrc + faceX;
        Pel *pDst = m_pFacesBufTempOrig[faceIdx];
        rotFaceChannelGeneral(pSrc, nWidth, nHeight, pSrcYuv->getStride((ComponentID)ch), 1, iRot, pDst, m_nStrideBufTemp, 1, true);
      }

      //padding;
      {
        //Int iFaceStride = m_nStrideBufTemp;
        for(Int f=0; f<nFaces; f++)
        {
          Pel *pDst;
          Pel *pSrc;
          
          pSrc = m_pFacesBufTempOrig[f];
          //left/right;
          for(Int row=0; row<nHeight; row++)
          {
            Int x0, x1;
            Int y_L = row<<getComponentScaleY(chId);
            if(y_L >= (m_sVideoInfo.iFaceHeight>>1))
            {
              y_L ++; 
            } 
            if(y_L <= ((m_sVideoInfo.iFaceHeight>>1)-1))
            {
              Double d = 1.0*y_L/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
              Int  d_i = (Int(d + 1)>>1)<<1;
              x0 = (m_sVideoInfo.iFaceWidth>>1) - 2 - d_i;
              x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
            }
            else
            {
              Int y_m = m_sVideoInfo.iFaceHeight - 1 - y_L;
              Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
              Int  d_i = (Int(d + 1)>>1)<<1;
              x0 = d_i;
              x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
            }

            x0 = ((x0 + 1)>>1);
            x1 = x1>>1;

            //left;
            Pel* pSrcLine = pSrc + x0;
            pDst = pSrcLine - 1;
            for(Int j=0; j<m_nMarginSizeBufTemp + x0; j++)
                *pDst-- = *pSrcLine;
            //right;
            pSrcLine = pSrc + x1;
            pDst = pSrcLine + 1;
            for(Int j=0; j<nWidth-1-x1 + m_nMarginSizeBufTemp; j++)
                *pDst++ = *pSrcLine;

            pSrc += m_nStrideBufTemp;
          }
          //top;
          pSrc = m_pFacesBufTempOrig[f] - m_nMarginSizeBufTemp;
          pDst = pSrc - m_nStrideBufTemp;
          for(Int j=0; j<m_nMarginSizeBufTemp; j++)
          {
            memcpy(pDst, pSrc, m_nStrideBufTemp*sizeof(Pel));
            pDst -= m_nStrideBufTemp;
          }
          //bottom;
          pSrc = m_pFacesBufTempOrig[f] + (nHeight - 1)*m_nStrideBufTemp -m_nMarginSizeBufTemp;
          pDst = pSrc + m_nStrideBufTemp;
          for(Int j=0; j<m_nMarginSizeBufTemp; j++)
          {
            memcpy(pDst, pSrc, m_nStrideBufTemp*sizeof(Pel));
            pDst += m_nStrideBufTemp;
          }
        }
      }
      if(m_chromaFormatIDC == CHROMA_420)
      {
        //convert chroma_sample_loc from 0 to 2;
        for(Int f=0; f<nFaces; f++)
          chromaResampleType0toType2(m_pFacesBufTempOrig[f], nWidth, nHeight, m_nStrideBufTemp, m_pFacesOrig[f][ch], getStride(chId));
      }
      else
      {
        //420->444;
        for(Int f=0; f<nFaces; f++)
          chromaUpsample(m_pFacesBufTempOrig[f], nWidth, nHeight, m_nStrideBufTemp, f, chId);
      }
    }
  }
  else if(pSrcYuv->getChromaFormat()==CHROMA_400 || pSrcYuv->getChromaFormat()==CHROMA_444)
  {  
     if(m_chromaFormatIDC == pSrcYuv->getChromaFormat())
     {
      for(Int faceIdx=0; faceIdx<m_sVideoInfo.iNumFaces; faceIdx++)
      {
        Int faceX = m_facePos[faceIdx][1]*nWidth;
        Int faceY = m_facePos[faceIdx][0]*nHeight;
        assert(faceIdx == m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].id);
        Int iRot = m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].rot;        

        for(Int ch=0; ch<getNumChannels(); ch++)
        {
          ComponentID chId = ComponentID(ch);
          Int iStrideSrc = pSrcYuv->getStride((ComponentID)(ch));
          Pel *pSrc = pSrcYuv->getAddr((ComponentID)ch) + faceY*iStrideSrc + faceX;
          Pel *pDst = m_pFacesOrig[faceIdx][ch];
          rotFaceChannelGeneral(pSrc, nWidth, nHeight, pSrcYuv->getStride((ComponentID)ch), 1, iRot, pDst, getStride((ComponentID)ch), 1, true);

          //fill in a rectangular;
          pSrc = m_pFacesOrig[faceIdx][ch];
          //left/right;
          for(Int row=0; row<nHeight; row++)
          {
            Int x0, x1;
            if(ch == 0)
            {
              if(row <= ((nHeight>>1)-1))
              {
                Double d = 1.0*row/(nHeight-1)*((nWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = (nWidth>>1) - 2 - d_i;
                x1 = nWidth - 1 - x0;
              }
              else
              {
                Int y_m = nHeight - 1 - row;
                Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = d_i;
                x1 = nWidth - 1 - x0;
              }
            }
            else
            {
              Int y_L = row<<getComponentScaleY(chId);
              if(y_L >= (m_sVideoInfo.iFaceHeight>>1))
              {
                y_L ++; 
              } 
              if(y_L <= ((m_sVideoInfo.iFaceHeight>>1)-1))
              {
                Double d = 1.0*y_L/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = (m_sVideoInfo.iFaceWidth>>1) - 2 - d_i;
                x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
              }
              else
              {
                Int y_m = m_sVideoInfo.iFaceHeight - 1 - y_L;
                Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = d_i;
                x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
              }

              x0 = ((x0 + 1)>>1);
              x1 = x1>>1;
            }

            //left;
            Pel* pSrcLine = pSrc + x0;
            pDst = pSrcLine - 1;
            for(Int j=0; j<x0; j++)
                *pDst-- = *pSrcLine;
            //right;
            pSrcLine = pSrc + x1;
            pDst = pSrcLine + 1;
            for(Int j=0; j<nWidth-1-x1; j++)
                *pDst++ = *pSrcLine;

            pSrc += getStride((ComponentID)ch);
          }

        }
      }
     }
     else
       assert(!"Not supported yet!");
  }
  else
    assert(!"Not supported yet");
 
  //set padding flag;
  setPaddingFlag(false);
}

Void TOctahedron::compactFramePackConvertYuv(TComPicYuv *pSrcYuv)
{
  if(m_sVideoInfo.iCompactFPStructure == 1)
    compactFramePackConvertYuvType1(pSrcYuv);
  else if(m_sVideoInfo.iCompactFPStructure == 2)
    compactFramePackConvertYuvType2(pSrcYuv);
  else
    assert(!"Not supported yet!");
}

Void TOctahedron::recoverFaceType1(Int faceIdx, TComPicYuv *pSrcYuv, ComponentID chId, Int iRot, Int nWidth, Int nHeight)
{
  Pel* pcBufSrc, *pcBufDst;
  Int iStrideSrc = pSrcYuv->getStride(chId);

  if (faceIdx == 2)
  {
    Int iHorFPShift = (nWidth>>1) + 2 + (nWidth + 4);
    pcBufSrc = pSrcYuv->getAddr(chId) + iHorFPShift;
    pcBufDst = m_pFaceRotBuf;
    triangleFaceCopy(nWidth, nHeight, pcBufSrc, 0, (nWidth>>1)+1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
  
    pcBufSrc = pSrcYuv->getAddr(chId);
    pcBufDst = m_pFaceRotBuf + (nWidth>>1) + 2;
    triangleFaceCopy(nWidth, nHeight, pcBufSrc, (nWidth>>1)+2, nWidth-1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
  }
  else if (faceIdx == 3)
  {
    Int iHorFPShift = (nWidth>>1) + 2 + (nWidth + 4);
    pcBufSrc = pSrcYuv->getAddr(chId) + iHorFPShift + pSrcYuv->getStride(chId) * nHeight;
    pcBufDst = m_pFaceRotBuf;
    triangleFaceCopy(nWidth, nHeight, pcBufSrc, 0, (nWidth>>1)+1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
  
    pcBufSrc = pSrcYuv->getAddr(chId) + pSrcYuv->getStride(chId) * nHeight;
    pcBufDst = m_pFaceRotBuf + (nWidth>>1) + 2;
    triangleFaceCopy(nWidth, nHeight, pcBufSrc, (nWidth>>1)+2, nWidth-1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
  }
  else
  {
    pcBufDst = m_pFaceRotBuf;
  
    Int iHorFPShift = 0;
    Int iVerFPShift = 0;
  
    switch (faceIdx)
    {
      case 0:
        iHorFPShift = (nWidth >> 1) + 2;
        iVerFPShift = 0;
        break;
      case 1:
        iHorFPShift = (nWidth>>1) + 2;
        iVerFPShift = nHeight;
        break;
      case 4:
        iHorFPShift = 0;
        iVerFPShift = 0;
        break;
      case 5:
        iHorFPShift = 0;
        iVerFPShift = nHeight;
        break;
      case 6:
        iHorFPShift = (nWidth + 4);
        iVerFPShift = 0;
        break;
      case 7:
      default:
        iHorFPShift = (nWidth + 4);
        iVerFPShift = nHeight;
        break;
    }
  
    pcBufSrc = pSrcYuv->getAddr(chId) + iHorFPShift + pSrcYuv->getStride(chId) * iVerFPShift;
  
    triangleFaceCopy(nWidth, nHeight, pcBufSrc, 0, nWidth - 1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
  }
}

Void TOctahedron::recoverFaceChromaType1(Int faceIdx, TComPicYuv *pSrcYuv, ComponentID chId, Int iRot, Int nWidth, Int nHeight)
{
  Pel* pcBufSrc, *pcBufDst;
  Int iStrideSrc = pSrcYuv->getStride(chId);

  if (faceIdx == 2)
  {
      Int iHorFPShift = (nWidth>>1) + 1 + (nWidth + 2);
      pcBufSrc = pSrcYuv->getAddr(chId) + iHorFPShift;
      pcBufDst = m_pFaceRotBuf;
      triangleFaceCopy(nWidth, nHeight, pcBufSrc, 0, (nWidth>>1), 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
  
      pcBufSrc = pSrcYuv->getAddr(chId);
      pcBufDst = m_pFaceRotBuf + (nWidth>>1) + 1;
      triangleFaceCopy(nWidth, nHeight, pcBufSrc, (nWidth>>1)+1, nWidth-1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
  }
  else if (faceIdx == 3)
  {
      Int iHorFPShift = (nWidth>>1) + 1 + (nWidth + 2);
      pcBufSrc = pSrcYuv->getAddr(chId) + iHorFPShift + pSrcYuv->getStride(chId) * nHeight;
      pcBufDst = m_pFaceRotBuf;
      triangleFaceCopy(nWidth, nHeight, pcBufSrc, 0, (nWidth>>1), 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
  
      pcBufSrc = pSrcYuv->getAddr(chId) + pSrcYuv->getStride(chId) * nHeight;
      pcBufDst = m_pFaceRotBuf + (nWidth>>1) + 1;
      triangleFaceCopy(nWidth, nHeight, pcBufSrc, (nWidth>>1)+1, nWidth-1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
  }
  else
  {
    pcBufDst = m_pFaceRotBuf;
  
    Int iHorFPShift = 0;
    Int iVerFPShift = 0;

    switch (faceIdx)
    {
      case 0:
        iHorFPShift = (nWidth >> 1) + 1;
        iVerFPShift = 0;
        break;
      case 1:
        iHorFPShift = (nWidth>>1) + 1;
        iVerFPShift = nHeight;
        break;
      case 4:
        iHorFPShift = 0;
        iVerFPShift = 0;
        break;
      case 5:
        iHorFPShift = 0;
        iVerFPShift = nHeight;
        break;
      case 6:
        iHorFPShift = (nWidth + 2);
        iVerFPShift = 0;
        break;
      case 7:
      default:
        iHorFPShift = (nWidth + 2);
        iVerFPShift = nHeight;
        break;
    }
  
    pcBufSrc = pSrcYuv->getAddr(chId) + iHorFPShift + pSrcYuv->getStride(chId) * iVerFPShift;
  
    triangleFaceCopy(nWidth, nHeight, pcBufSrc, 0, nWidth - 1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
  }  
}

Void TOctahedron::compactFramePackConvertYuvType1(TComPicYuv *pSrcYuv)
{
  Int nWidth = m_sVideoInfo.iFaceWidth;
  Int nHeight = m_sVideoInfo.iFaceHeight;

  assert(pSrcYuv->getWidth(ComponentID(0)) == (nWidth+4)*m_sVideoInfo.framePackStruct.cols && pSrcYuv->getHeight(ComponentID(0)) == nHeight*(m_sVideoInfo.framePackStruct.rows>>1));
  assert(pSrcYuv->getNumberValidComponents() == getNumChannels());

  if( !m_pFaceRotBuf )
  {
    m_pFaceRotBuf = (Pel*)xMalloc(Pel, m_sVideoInfo.iFaceWidth*m_sVideoInfo.iFaceHeight);
  }

  if(pSrcYuv->getChromaFormat()==CHROMA_420)
  {
    Int nFaces = m_sVideoInfo.iNumFaces;

    for(Int ch=0; ch<pSrcYuv->getNumberValidComponents(); ch++)
    {
      ComponentID chId = ComponentID(ch);
      nWidth = m_sVideoInfo.iFaceWidth >> pSrcYuv->getComponentScaleX(chId);
      nHeight = m_sVideoInfo.iFaceHeight >> pSrcYuv->getComponentScaleY(chId); 

      if(!ch || (m_chromaFormatIDC==CHROMA_420 && !m_bResampleChroma))
      {
        for(Int faceIdx=0; faceIdx<nFaces; faceIdx++)
        {
          assert(faceIdx == m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].id);
          Int iRot = m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].rot;

          if (chId == COMPONENT_Y)
            recoverFaceType1(faceIdx, pSrcYuv, COMPONENT_Y, iRot, nWidth, nHeight);
          else
            recoverFaceChromaType1(faceIdx, pSrcYuv, chId, iRot, nWidth, nHeight);

          Pel *pSrc = m_pFaceRotBuf;
          Pel *pDst = m_pFacesOrig[faceIdx][ch];
          rotFlipFaceChannelGeneral(pSrc, nWidth, nHeight, m_sVideoInfo.iFaceWidth, pDst, getStride((ComponentID)ch), iRot, true, FACE_NO_FLIP);

          //fill in a rectangular
          pSrc = m_pFacesOrig[faceIdx][ch];
          //left/right
          for(Int row=0; row<nHeight; row++)
          {
            Int x0, x1;
            if(ch == 0)
            {
              if(row <= ((nHeight>>1)-1))
              {
                Double d = 1.0*row/(nHeight-1)*((nWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = (nWidth>>1) - 2 - d_i;
                x1 = nWidth - 1 - x0;
              }
              else
              {
                Int y_m = nHeight - 1 - row;
                Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = d_i;
                x1 = nWidth - 1 - x0;
              }
            }
            else
            {
              Int y_L = row<<getComponentScaleY(chId);
              if(y_L >= (m_sVideoInfo.iFaceHeight>>1))
              {
                y_L ++; 
              } 
              if(y_L <= ((m_sVideoInfo.iFaceHeight>>1)-1))
              {
                Double d = 1.0*y_L/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = (m_sVideoInfo.iFaceWidth>>1) - 2 - d_i;
                x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
              }
              else
              {
                Int y_m = m_sVideoInfo.iFaceHeight - 1 - y_L;
                Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = d_i;
                x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
              }

              x0 = ((x0 + 1)>>1);
              x1 = x1>>1;
            }

            //left
            Pel* pSrcLine = pSrc + x0;
            pDst = pSrcLine - 1;
            for(Int j=0; j<x0; j++)
              *pDst-- = *pSrcLine;
            //right
            pSrcLine = pSrc + x1;
            pDst = pSrcLine + 1;
            for(Int j=0; j<nWidth-1-x1; j++)
              *pDst++ = *pSrcLine;

            pSrc += getStride((ComponentID)ch);
          }
        }
        continue;
      }

      //memory allocation
      if(!m_pFacesBufTemp)
      {
        assert(!m_pFacesBufTempOrig);
        m_nMarginSizeBufTemp = 2;  //4 tap filter
        m_nStrideBufTemp = nWidth + (m_nMarginSizeBufTemp<<1);
        m_pFacesBufTemp = new Pel*[nFaces];
        memset(m_pFacesBufTemp, 0, sizeof(Pel*)*nFaces);
        m_pFacesBufTempOrig = new Pel*[nFaces];
        memset(m_pFacesBufTempOrig, 0, sizeof(Pel*)*nFaces);
        Int iTotalHeight = (nHeight+(m_nMarginSizeBufTemp<<1));
        for(Int i=0; i<nFaces; i++)
        {
          m_pFacesBufTemp[i] = (Pel *)xMalloc(Pel,  m_nStrideBufTemp*iTotalHeight);
          m_pFacesBufTempOrig[i] = m_pFacesBufTemp[i] +  m_nStrideBufTemp * m_nMarginSizeBufTemp + m_nMarginSizeBufTemp;
        }
      }
      //read content first
      for(Int faceIdx=0; faceIdx<nFaces; faceIdx++)
      {
        assert(faceIdx == m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].id);
        Int iRot = m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].rot;

        recoverFaceChromaType1(faceIdx, pSrcYuv, chId, iRot, nWidth, nHeight);

        Pel *pSrc = m_pFaceRotBuf;
        Pel *pDst = m_pFacesBufTempOrig[faceIdx];
        rotFlipFaceChannelGeneral(pSrc, nWidth, nHeight, m_sVideoInfo.iFaceWidth, pDst, m_nStrideBufTemp, iRot, true, FACE_NO_FLIP);
      }

      //padding
      {
        for(Int f=0; f<nFaces; f++)
        {
          Pel *pDst;
          Pel *pSrc;

          pSrc = m_pFacesBufTempOrig[f];
          //left/right
          for(Int row=0; row<nHeight; row++)
          {
            Int x0, x1;
            Int y_L = row<<getComponentScaleY(chId);
            if(y_L >= (m_sVideoInfo.iFaceHeight>>1))
            {
              y_L ++; 
            } 
            if(y_L <= ((m_sVideoInfo.iFaceHeight>>1)-1))
            {
              Double d = 1.0*y_L/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
              Int  d_i = (Int(d + 1)>>1)<<1;
              x0 = (m_sVideoInfo.iFaceWidth>>1) - 2 - d_i;
              x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
            }
            else
            {
              Int y_m = m_sVideoInfo.iFaceHeight - 1 - y_L;
              Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
              Int  d_i = (Int(d + 1)>>1)<<1;
              x0 = d_i;
              x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
            }

            x0 = ((x0 + 1)>>1);
            x1 = x1>>1;

            //left
            Pel* pSrcLine = pSrc + x0;
            pDst = pSrcLine - 1;
            for(Int j=0; j<m_nMarginSizeBufTemp + x0; j++)
              *pDst-- = *pSrcLine;
            //right
            pSrcLine = pSrc + x1;
            pDst = pSrcLine + 1;
            for(Int j=0; j<nWidth-1-x1 + m_nMarginSizeBufTemp; j++)
              *pDst++ = *pSrcLine;

            pSrc += m_nStrideBufTemp;
          }
          //top
          pSrc = m_pFacesBufTempOrig[f] - m_nMarginSizeBufTemp;
          pDst = pSrc - m_nStrideBufTemp;
          for(Int j=0; j<m_nMarginSizeBufTemp; j++)
          {
            memcpy(pDst, pSrc, m_nStrideBufTemp*sizeof(Pel));
            pDst -= m_nStrideBufTemp;
          }
          //bottom
          pSrc = m_pFacesBufTempOrig[f] + (nHeight - 1)*m_nStrideBufTemp -m_nMarginSizeBufTemp;
          pDst = pSrc + m_nStrideBufTemp;
          for(Int j=0; j<m_nMarginSizeBufTemp; j++)
          {
            memcpy(pDst, pSrc, m_nStrideBufTemp*sizeof(Pel));
            pDst += m_nStrideBufTemp;
          }
        }
      }
      if(m_chromaFormatIDC == CHROMA_420)
      {
        //convert chroma_sample_loc from 0 to 2
        for(Int f=0; f<nFaces; f++)
          chromaResampleType0toType2(m_pFacesBufTempOrig[f], nWidth, nHeight, m_nStrideBufTemp, m_pFacesOrig[f][ch], getStride(chId));
      }
      else
      {
        //420->444
        for(Int f=0; f<nFaces; f++)
          chromaUpsample(m_pFacesBufTempOrig[f], nWidth, nHeight, m_nStrideBufTemp, f, chId);
      }
    }
  }
  else if(pSrcYuv->getChromaFormat()==CHROMA_400 || pSrcYuv->getChromaFormat()==CHROMA_444)
  {  
    if(m_chromaFormatIDC == pSrcYuv->getChromaFormat())
    {
      for(Int faceIdx=0; faceIdx<m_sVideoInfo.iNumFaces; faceIdx++)
      {
        assert(faceIdx == m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].id);
        Int iRot = m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].rot;

        for(Int ch=0; ch<getNumChannels(); ch++)
        {
          ComponentID chId = (ComponentID)ch;

          recoverFaceType1(faceIdx, pSrcYuv, chId, iRot, nWidth, nHeight);

          Pel *pSrc = m_pFaceRotBuf;
          Pel *pDst = m_pFacesOrig[faceIdx][ch];
          rotFlipFaceChannelGeneral(pSrc, nWidth, nHeight, m_sVideoInfo.iFaceWidth, pDst, getStride((ComponentID)ch), iRot, true, FACE_NO_FLIP);
        }
      }
    }
    else
      assert(!"Not supported yet!");
  }
  else
    assert(!"Not supported yet");

  //set padding flag
  setPaddingFlag(false);
}

Void TOctahedron::compactFramePackConvertYuvType2(TComPicYuv *pSrcYuv)
{
  Int nWidth = m_sVideoInfo.iFaceWidth;
  Int nHeight = m_sVideoInfo.iFaceHeight;

  assert(pSrcYuv->getWidth(ComponentID(0)) == (nWidth+4)*m_sVideoInfo.framePackStruct.cols && pSrcYuv->getHeight(ComponentID(0)) == nHeight*(m_sVideoInfo.framePackStruct.rows>>1));
  assert(pSrcYuv->getNumberValidComponents() == getNumChannels());

  if( !m_pFaceRotBuf )
  {
    m_pFaceRotBuf = (Pel*)xMalloc(Pel, m_sVideoInfo.iFaceWidth*m_sVideoInfo.iFaceHeight);
  }
  Pel* pcBufSrc, *pcBufDst;

  if(pSrcYuv->getChromaFormat()==CHROMA_420)
  {
    Int nFaces = m_sVideoInfo.iNumFaces;

    for(Int ch=0; ch<pSrcYuv->getNumberValidComponents(); ch++)
    {
      ComponentID chId = ComponentID(ch);
      nWidth = m_sVideoInfo.iFaceWidth >> pSrcYuv->getComponentScaleX(chId);
      nHeight = m_sVideoInfo.iFaceHeight >> pSrcYuv->getComponentScaleY(chId); 

      if(!ch || (m_chromaFormatIDC==CHROMA_420 && !m_bResampleChroma))
      {
        for(Int faceIdx=0; faceIdx<nFaces; faceIdx++)
        {
          assert(faceIdx == m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].id);
          Int iRot = m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].rot;
          Int iStrideSrc = pSrcYuv->getStride((ComponentID)(ch));

          if(faceIdx == 3)
          {
            if(chId == COMPONENT_Y)
            {
              Int iHorFPShift = (nWidth>>1) + 2 + 3*(nWidth + 4);
              pcBufSrc = pSrcYuv->getAddr((ComponentID)ch) + iHorFPShift;
              pcBufDst = m_pFaceRotBuf;
              triangleFaceCopy(nWidth, nHeight, pcBufSrc, 0, (nWidth>>1)+1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);

              pcBufSrc = pSrcYuv->getAddr((ComponentID)ch);
              pcBufDst = m_pFaceRotBuf + (nWidth>>1) + 2;
              triangleFaceCopy(nWidth, nHeight, pcBufSrc, (nWidth>>1)+2, nWidth-1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
            }
            else
            {
              Int iHorFPShift = (nWidth>>1) + 1 + 3*(nWidth + 2);
              pcBufSrc = pSrcYuv->getAddr((ComponentID)ch) + iHorFPShift;
              pcBufDst = m_pFaceRotBuf;
              triangleFaceCopy(nWidth, nHeight, pcBufSrc, 0, (nWidth>>1), 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);

              pcBufSrc = pSrcYuv->getAddr((ComponentID)ch);
              pcBufDst = m_pFaceRotBuf + (nWidth>>1) + 1;
              triangleFaceCopy(nWidth, nHeight, pcBufSrc, (nWidth>>1)+1, nWidth-1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
            }
          }
          else
          {
            pcBufDst = m_pFaceRotBuf;
            if(faceIdx%2 == 0)
            {
              Int iHorFPShift = 0;
              if(chId == COMPONENT_Y) 
              {
                if(faceIdx == 4)      iHorFPShift = 0;
                else if(faceIdx == 0) iHorFPShift = (nWidth + 4);
                else if(faceIdx == 6) iHorFPShift = 2*(nWidth + 4);
                else if(faceIdx == 2) iHorFPShift = 3*(nWidth + 4);
              }
              else
              {
                if(faceIdx == 4)      iHorFPShift = 0;
                else if(faceIdx == 0) iHorFPShift = (nWidth + 2);
                else if(faceIdx == 6) iHorFPShift = 2*(nWidth + 2);
                else if(faceIdx == 2) iHorFPShift = 3*(nWidth + 2);
              }
              pcBufSrc = pSrcYuv->getAddr((ComponentID)ch) + iHorFPShift;
            }
            else
            {
              Int iHorFPShift = 0;
              if(chId == COMPONENT_Y)
              {
                if(faceIdx == 5)      iHorFPShift = (nWidth>>1) + 2;
                else if(faceIdx == 1) iHorFPShift = (nWidth>>1) + 2 + (nWidth + 4);
                else if(faceIdx == 7) iHorFPShift = (nWidth>>1) + 2 + 2*(nWidth + 4);
              }
              else
              {
                if(faceIdx == 5)      iHorFPShift = (nWidth>>1) + 1;
                else if(faceIdx == 1) iHorFPShift = (nWidth>>1) + 1 + (nWidth + 2);
                else if(faceIdx == 7) iHorFPShift = (nWidth>>1) + 1 + 2*(nWidth + 2); 
              }
              pcBufSrc = pSrcYuv->getAddr((ComponentID)ch) + iHorFPShift;
            }
            triangleFaceCopy(nWidth, nHeight, pcBufSrc, 0, nWidth - 1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
          }
          Pel *pSrc = m_pFaceRotBuf;
          Pel *pDst = m_pFacesOrig[faceIdx][ch];
          rotFlipFaceChannelGeneral(pSrc, nWidth, nHeight, m_sVideoInfo.iFaceWidth, pDst, getStride((ComponentID)ch), iRot, true, FACE_NO_FLIP);

          //fill in a rectangular
          pSrc = m_pFacesOrig[faceIdx][ch];
          //left/right
          for(Int row=0; row<nHeight; row++)
          {
            Int x0, x1;
            if(ch == 0)
            {
              if(row <= ((nHeight>>1)-1))
              {
                Double d = 1.0*row/(nHeight-1)*((nWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = (nWidth>>1) - 2 - d_i;
                x1 = nWidth - 1 - x0;
              }
              else
              {
                Int y_m = nHeight - 1 - row;
                Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = d_i;
                x1 = nWidth - 1 - x0;
              }
            }
            else
            {
              Int y_L = row<<getComponentScaleY(chId);
              if(y_L >= (m_sVideoInfo.iFaceHeight>>1))
              {
                y_L ++; 
              } 
              if(y_L <= ((m_sVideoInfo.iFaceHeight>>1)-1))
              {
                Double d = 1.0*y_L/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = (m_sVideoInfo.iFaceWidth>>1) - 2 - d_i;
                x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
              }
              else
              {
                Int y_m = m_sVideoInfo.iFaceHeight - 1 - y_L;
                Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
                Int  d_i = (Int(d + 1)>>1)<<1;
                x0 = d_i;
                x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
              }

              x0 = ((x0 + 1)>>1);
              x1 = x1>>1;
            }

            //left
            Pel* pSrcLine = pSrc + x0;
            pDst = pSrcLine - 1;
            for(Int j=0; j<x0; j++)
              *pDst-- = *pSrcLine;
            //right
            pSrcLine = pSrc + x1;
            pDst = pSrcLine + 1;
            for(Int j=0; j<nWidth-1-x1; j++)
              *pDst++ = *pSrcLine;

            pSrc += getStride((ComponentID)ch);
          }
        }
        continue;
      }

      //memory allocation
      if(!m_pFacesBufTemp)
      {
        assert(!m_pFacesBufTempOrig);
        m_nMarginSizeBufTemp = 2;  //4 tap filter
        m_nStrideBufTemp = nWidth + (m_nMarginSizeBufTemp<<1);
        m_pFacesBufTemp = new Pel*[nFaces];
        memset(m_pFacesBufTemp, 0, sizeof(Pel*)*nFaces);
        m_pFacesBufTempOrig = new Pel*[nFaces];
        memset(m_pFacesBufTempOrig, 0, sizeof(Pel*)*nFaces);
        Int iTotalHeight = (nHeight+(m_nMarginSizeBufTemp<<1));
        for(Int i=0; i<nFaces; i++)
        {
          m_pFacesBufTemp[i] = (Pel *)xMalloc(Pel,  m_nStrideBufTemp*iTotalHeight);
          m_pFacesBufTempOrig[i] = m_pFacesBufTemp[i] +  m_nStrideBufTemp * m_nMarginSizeBufTemp + m_nMarginSizeBufTemp;
        }
      }
      //read content first
      for(Int faceIdx=0; faceIdx<nFaces; faceIdx++)
      {
        assert(faceIdx == m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].id);
        Int iRot = m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].rot;
        Int iStrideSrc = pSrcYuv->getStride((ComponentID)(ch));

        if(faceIdx == 3)
        {
          Int iHorFPShift = (nWidth>>1) + 1 + 3*(nWidth + 2);
          pcBufSrc = pSrcYuv->getAddr((ComponentID)ch) + iHorFPShift;
          pcBufDst = m_pFaceRotBuf;
          triangleFaceCopy(nWidth, nHeight, pcBufSrc, 0, (nWidth>>1), 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);

          pcBufSrc = pSrcYuv->getAddr((ComponentID)ch);
          pcBufDst = m_pFaceRotBuf + (nWidth>>1) + 1;
          triangleFaceCopy(nWidth, nHeight, pcBufSrc, (nWidth>>1)+1, nWidth-1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
        }
        else
        {
          pcBufDst = m_pFaceRotBuf;
          if(faceIdx%2 == 0)
          {
            Int iHorFPShift = 0;
            if(faceIdx == 4)      iHorFPShift = 0;
            else if(faceIdx == 0) iHorFPShift = (nWidth + 2);
            else if(faceIdx == 2) iHorFPShift = 2*(nWidth + 2);
            else if(faceIdx == 6) iHorFPShift = 3*(nWidth + 2);

            pcBufSrc = pSrcYuv->getAddr((ComponentID)ch) + iHorFPShift;
          }
          else
          {
            Int iHorFPShift = 0;
            if(faceIdx == 5)      iHorFPShift = (nWidth>>1) + 1;
            else if(faceIdx == 1) iHorFPShift = (nWidth>>1) + 1 + (nWidth + 2);
            else if(faceIdx == 7) iHorFPShift = (nWidth>>1) + 1 + 2*(nWidth + 2);

            pcBufSrc = pSrcYuv->getAddr((ComponentID)ch) + iHorFPShift;
          }
          triangleFaceCopy(nWidth, nHeight, pcBufSrc, 0, nWidth - 1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
        }

        Pel *pSrc = m_pFaceRotBuf;
        Pel *pDst = m_pFacesBufTempOrig[faceIdx];
        rotFlipFaceChannelGeneral(pSrc, nWidth, nHeight, m_sVideoInfo.iFaceWidth, pDst, m_nStrideBufTemp, iRot, true, FACE_NO_FLIP);
      }

      //padding
      {
        for(Int f=0; f<nFaces; f++)
        {
          Pel *pDst;
          Pel *pSrc;

          pSrc = m_pFacesBufTempOrig[f];
          //left/right
          for(Int row=0; row<nHeight; row++)
          {
            Int x0, x1;
            Int y_L = row<<getComponentScaleY(chId);
            if(y_L >= (m_sVideoInfo.iFaceHeight>>1))
            {
              y_L ++; 
            } 
            if(y_L <= ((m_sVideoInfo.iFaceHeight>>1)-1))
            {
              Double d = 1.0*y_L/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
              Int  d_i = (Int(d + 1)>>1)<<1;
              x0 = (m_sVideoInfo.iFaceWidth>>1) - 2 - d_i;
              x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
            }
            else
            {
              Int y_m = m_sVideoInfo.iFaceHeight - 1 - y_L;
              Double d = 1.0*y_m/(m_sVideoInfo.iFaceHeight-1)*((m_sVideoInfo.iFaceWidth>>1)-2);
              Int  d_i = (Int(d + 1)>>1)<<1;
              x0 = d_i;
              x1 = m_sVideoInfo.iFaceWidth - 1 - x0;
            }

            x0 = ((x0 + 1)>>1);
            x1 = x1>>1;

            //left
            Pel* pSrcLine = pSrc + x0;
            pDst = pSrcLine - 1;
            for(Int j=0; j<m_nMarginSizeBufTemp + x0; j++)
              *pDst-- = *pSrcLine;
            //right
            pSrcLine = pSrc + x1;
            pDst = pSrcLine + 1;
            for(Int j=0; j<nWidth-1-x1 + m_nMarginSizeBufTemp; j++)
              *pDst++ = *pSrcLine;

            pSrc += m_nStrideBufTemp;
          }
          //top
          pSrc = m_pFacesBufTempOrig[f] - m_nMarginSizeBufTemp;
          pDst = pSrc - m_nStrideBufTemp;
          for(Int j=0; j<m_nMarginSizeBufTemp; j++)
          {
            memcpy(pDst, pSrc, m_nStrideBufTemp*sizeof(Pel));
            pDst -= m_nStrideBufTemp;
          }
          //bottom
          pSrc = m_pFacesBufTempOrig[f] + (nHeight - 1)*m_nStrideBufTemp -m_nMarginSizeBufTemp;
          pDst = pSrc + m_nStrideBufTemp;
          for(Int j=0; j<m_nMarginSizeBufTemp; j++)
          {
            memcpy(pDst, pSrc, m_nStrideBufTemp*sizeof(Pel));
            pDst += m_nStrideBufTemp;
          }
        }
      }
      if(m_chromaFormatIDC == CHROMA_420)
      {
        //convert chroma_sample_loc from 0 to 2
        for(Int f=0; f<nFaces; f++)
          chromaResampleType0toType2(m_pFacesBufTempOrig[f], nWidth, nHeight, m_nStrideBufTemp, m_pFacesOrig[f][ch], getStride(chId));
      }
      else
      {
        //420->444
        for(Int f=0; f<nFaces; f++)
          chromaUpsample(m_pFacesBufTempOrig[f], nWidth, nHeight, m_nStrideBufTemp, f, chId);
      }
    }
  }
  else if(pSrcYuv->getChromaFormat()==CHROMA_400 || pSrcYuv->getChromaFormat()==CHROMA_444)
  {  
    if(m_chromaFormatIDC == pSrcYuv->getChromaFormat())
    {
      for(Int faceIdx=0; faceIdx<m_sVideoInfo.iNumFaces; faceIdx++)
      {
        assert(faceIdx == m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].id);
        Int iRot = m_sVideoInfo.framePackStruct.faces[m_facePos[faceIdx][0]][m_facePos[faceIdx][1]].rot;

        for(Int ch=0; ch<getNumChannels(); ch++)
        {
          ComponentID chId = (ComponentID)ch;
          Int iStrideSrc = pSrcYuv->getStride((ComponentID)(ch));
          if(faceIdx == 3)
          {
            Int iHorFPShift = (nWidth>>1) + 2 + 3*(nWidth + 4);
            pcBufSrc = pSrcYuv->getAddr((ComponentID)ch) + iHorFPShift;
            pcBufDst = m_pFaceRotBuf;
            triangleFaceCopy(nWidth, nHeight, pcBufSrc, 0, (nWidth>>1)+1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);

            pcBufSrc = pSrcYuv->getAddr((ComponentID)ch);
            pcBufDst = m_pFaceRotBuf + (nWidth>>1) + 2;
            triangleFaceCopy(nWidth, nHeight, pcBufSrc, (nWidth>>1)+2, nWidth-1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
          }
          else
          {
            pcBufDst = m_pFaceRotBuf;
            if(faceIdx%2 == 0)
            {
              Int iHorFPShift = 0;
              if(faceIdx == 4)      iHorFPShift = 0;
              else if(faceIdx == 0) iHorFPShift = (nWidth + 4);
              else if(faceIdx == 2) iHorFPShift = 2*(nWidth + 4);
              else if(faceIdx == 6) iHorFPShift = 3*(nWidth + 4);

              pcBufSrc = pSrcYuv->getAddr((ComponentID)ch) + iHorFPShift;
            }
            else
            {
              Int iHorFPShift = 0;
              if(faceIdx == 5)      iHorFPShift = (nWidth>>1) + 2;
              else if(faceIdx == 1) iHorFPShift = (nWidth>>1) + 2 + (nWidth + 4);
              else if(faceIdx == 7) iHorFPShift = (nWidth>>1) + 2 + 2*(nWidth + 4);

              pcBufSrc = pSrcYuv->getAddr((ComponentID)ch) + iHorFPShift;
            }
            triangleFaceCopy(nWidth, nHeight, pcBufSrc, 0, nWidth - 1, 0, nHeight - 1, iStrideSrc, pcBufDst, m_sVideoInfo.iFaceWidth, chId, iRot, FACE_NO_FLIP, faceIdx, 0, m_nBitDepth);
          }

          Pel *pSrc = m_pFaceRotBuf;
          Pel *pDst = m_pFacesOrig[faceIdx][ch];
          rotFlipFaceChannelGeneral(pSrc, nWidth, nHeight, m_sVideoInfo.iFaceWidth, pDst, getStride((ComponentID)ch), iRot, true, FACE_NO_FLIP);
        }
      }
    }
    else
      assert(!"Not supported yet!");
  }
  else
    assert(!"Not supported yet");

  //set padding flag
  setPaddingFlag(false);
}

Void TOctahedron::compactFramePack(TComPicYuv *pDstYuv)
{
  if(m_sVideoInfo.iCompactFPStructure == 1)
    compactFramePackType1(pDstYuv);
  else if(m_sVideoInfo.iCompactFPStructure == 2)
    compactFramePackType2(pDstYuv);
  else
    assert(!"Not supported yet!");
}

Void TOctahedron::copyFaceChromaType1(Int face, TComPicYuv *pDstYuv, ComponentID chId, Int rot, Int nWidthC, Int nHeightC)
{
  Pel *pcBufSrc, *pcBufDst;

  if(face == 2)
  {
    pcBufSrc = m_pFaceRotBuf;
    Int iHorFPShift = (nWidthC>>1) + 1 + 1 * (nWidthC + 2);
    pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift;
    triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, 0, (nWidthC>>1), 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
  
    pcBufSrc = m_pFaceRotBuf + (nWidthC>>1) + 1;
    pcBufDst = pDstYuv->getAddr(chId);
    triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, (nWidthC>>1)+1, nWidthC-1, 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
  }
  else if (face == 3)
  {
    pcBufSrc = m_pFaceRotBuf;
    Int iHorFPShift = (nWidthC>>1) + 1 + 1 * (nWidthC + 2);
    pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift + pDstYuv->getStride(chId) * nHeightC;
    triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, 0, (nWidthC>>1), 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
  
    pcBufSrc = m_pFaceRotBuf + (nWidthC>>1) + 1;
    pcBufDst = pDstYuv->getAddr(chId) + pDstYuv->getStride(chId) * nHeightC;
    triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, (nWidthC>>1)+1, nWidthC-1, 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
  }
  else
  {
    pcBufSrc = m_pFaceRotBuf; 

    Int iHorFPShift = 0;
    Int iVerFPShift = 0;

    switch (face)
    {
    case 0:
      iHorFPShift = (nWidthC>>1) + 1;
      iVerFPShift = 0;
      break;
    case 1:
      iHorFPShift = (nWidthC>>1) + 1;
      iVerFPShift = nHeightC;
      break;
    case 4:
      iHorFPShift = 0;
      iVerFPShift = 0;
      break;
    case 5:
      iHorFPShift = 0;
      iVerFPShift = nHeightC;
      break;
    case 6:
      iHorFPShift = (nWidthC + 2);
      iVerFPShift = 0;
      break;
    case 7:
    default:
      iHorFPShift = (nWidthC + 2);
      iVerFPShift = nHeightC;
      break;

    }

    pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift + pDstYuv->getStride(chId) * iVerFPShift; 

    triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, 0, nWidthC-1, 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
  }
}

Void TOctahedron::copyFaceType1(Int face, TComPicYuv *pDstYuv, ComponentID chId, Int rot)
{    
  Pel *pcBufSrc, *pcBufDst;

  if(face == 2)
  {
    pcBufSrc = m_pFaceRotBuf;
    Int iHorFPShift = (m_sVideoInfo.iFaceWidth>>1) + 2 + (m_sVideoInfo.iFaceWidth + 4);
    pcBufDst = pDstYuv->getAddr(COMPONENT_Y) + iHorFPShift;
    triangleFaceCopy(m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, pcBufSrc, 0, (m_sVideoInfo.iFaceWidth>>1) + 1, 0, m_sVideoInfo.iFaceHeight-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(COMPONENT_Y), COMPONENT_Y, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
  
    pcBufSrc = m_pFaceRotBuf + (m_sVideoInfo.iFaceWidth>>1) + 2;
    pcBufDst = pDstYuv->getAddr(COMPONENT_Y);
    triangleFaceCopy(m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, pcBufSrc, (m_sVideoInfo.iFaceWidth>>1)+2, m_sVideoInfo.iFaceWidth-1, 0, m_sVideoInfo.iFaceHeight-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(COMPONENT_Y), COMPONENT_Y, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
  }
  else if (face == 3)
  {
    pcBufSrc = m_pFaceRotBuf;
    Int iHorFPShift = (m_sVideoInfo.iFaceWidth>>1) + 2 + (m_sVideoInfo.iFaceWidth + 4);
    pcBufDst = pDstYuv->getAddr(COMPONENT_Y) + iHorFPShift + pDstYuv->getStride(COMPONENT_Y) * m_sVideoInfo.iFaceHeight;
    triangleFaceCopy(m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, pcBufSrc, 0, (m_sVideoInfo.iFaceWidth>>1) + 1, 0, m_sVideoInfo.iFaceHeight-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(COMPONENT_Y), COMPONENT_Y, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
  
    pcBufSrc = m_pFaceRotBuf + (m_sVideoInfo.iFaceWidth>>1) + 2;
    pcBufDst = pDstYuv->getAddr(COMPONENT_Y) + pDstYuv->getStride(COMPONENT_Y) * m_sVideoInfo.iFaceHeight;
    triangleFaceCopy(m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, pcBufSrc, (m_sVideoInfo.iFaceWidth>>1)+2, m_sVideoInfo.iFaceWidth-1, 0, m_sVideoInfo.iFaceHeight-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(COMPONENT_Y), COMPONENT_Y, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
  }
  else
  {
    pcBufSrc = m_pFaceRotBuf;
  
    Int iHorFPShift = 0;
    Int iVerFPShift = 0;
  
    switch (face)
    {
    case 0:
      iHorFPShift = (m_sVideoInfo.iFaceWidth>>1) + 2;
      iVerFPShift = 0;
      break;
    case 1:
      iHorFPShift = (m_sVideoInfo.iFaceWidth>>1) + 2;
      iVerFPShift = (m_sVideoInfo.iFaceHeight);
      break;
    case 4:
      iHorFPShift = 0;
      iVerFPShift = 0;
      break;
    case 5:
      iHorFPShift = 0;
      iVerFPShift = (m_sVideoInfo.iFaceHeight);
      break;
    case 6:
      iHorFPShift = (m_sVideoInfo.iFaceWidth + 4);
      iVerFPShift = 0;
      break;
    case 7:
    default:
      iHorFPShift = (m_sVideoInfo.iFaceWidth + 4);
      iVerFPShift = (m_sVideoInfo.iFaceHeight);
      break;

    }

    pcBufDst = pDstYuv->getAddr(COMPONENT_Y) + iHorFPShift + pDstYuv->getStride(COMPONENT_Y) * iVerFPShift; 
  
    triangleFaceCopy(m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, pcBufSrc, 0, m_sVideoInfo.iFaceWidth-1, 0, m_sVideoInfo.iFaceHeight-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(COMPONENT_Y), COMPONENT_Y, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
  }
}

Void TOctahedron::compactFramePackType1(TComPicYuv *pDstYuv)
{
  Int iTotalNumOfFaces = m_sVideoInfo.framePackStruct.rows * m_sVideoInfo.framePackStruct.cols;
  if( !m_pFaceRotBuf )
  {
    m_pFaceRotBuf = (Pel*)xMalloc(Pel, m_sVideoInfo.iFaceWidth*m_sVideoInfo.iFaceHeight);
  }

  if(pDstYuv->getChromaFormat()==CHROMA_420)
  {
    if( (m_chromaFormatIDC == CHROMA_444) || (m_chromaFormatIDC == CHROMA_420 && m_bResampleChroma) )
      spherePadding();

    assert(m_sVideoInfo.framePackStruct.chromaFormatIDC == CHROMA_420);

    //1: 444->420;  444->422, H:[1, 6, 1]; 422->420, V[1,1]
    //(Wc*2Hc) and (Wc*Hc) temporal buffer; the resulting buffer is for rotation
    Int nWidthC = m_sVideoInfo.iFaceWidth>>pDstYuv->getComponentScaleX((ComponentID)1);
    Int nHeightC = m_sVideoInfo.iFaceHeight>>pDstYuv->getComponentScaleY((ComponentID)1);  
    Int nMarginSize = (m_filterDs[1].nTaps-1)>>1; //0, depending on V filter and considering south pole
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
      Int rot = m_sVideoInfo.framePackStruct.faces[m_facePos[face][0]][m_facePos[face][1]].rot;
      if(face < m_sVideoInfo.iNumFaces)
      {
        if(m_chromaFormatIDC == CHROMA_444)
        {
          //chroma 444->420
          for(Int ch=1; ch<getNumChannels(); ch++)
          {
            ComponentID chId = (ComponentID)ch;
            chromaDonwsampleH(m_pFacesOrig[face][ch]-nMarginSize*getStride((ComponentID)ch), m_sVideoInfo.iFaceWidth, nHeightC422, getStride(chId), 1, m_pDS422Buf, iStride422);
            chromaDonwsampleV(m_pDS422Buf + nMarginSize*iStride422, nWidthC, m_sVideoInfo.iFaceHeight, iStride422, 1, m_pDS420Buf, iStride420);
            rotFlipFaceChannelGeneral(m_pDS420Buf, nWidthC, nHeightC, iStride420, m_pFaceRotBuf, m_sVideoInfo.iFaceWidth, rot, false, FACE_NO_FLIP);

            copyFaceChromaType1(face, pDstYuv, chId, rot, nWidthC, nHeightC);
          }
        }
        else
        {
          //m_chromaFormatIDC is CHROMA_420
          for(Int ch=1; ch<getNumChannels(); ch++)
          {
            ComponentID chId = (ComponentID)ch;

            if(m_bResampleChroma)
            {
              //convert chroma_sample_loc from 2 to 0
              chromaResampleType2toType0(m_pFacesOrig[face][ch], m_pDS420Buf, nWidthC, nHeightC, getStride(chId), nWidthC);
              rotFlipFaceChannelGeneral(m_pDS420Buf, nWidthC, nHeightC, iStride420, m_pFaceRotBuf, m_sVideoInfo.iFaceWidth, rot, false, FACE_NO_FLIP);
            }
            else
            {
              rotFlipFaceChannelGeneral(m_pFacesOrig[face][ch], nWidthC, nHeightC, getStride(chId), m_pFaceRotBuf, m_sVideoInfo.iFaceWidth, rot, false, FACE_NO_FLIP);

              copyFaceChromaType1(face, pDstYuv, chId, rot, nWidthC, nHeightC);
            }
          }
        }
        //luma
        rotFlipFaceChannelGeneral(m_pFacesOrig[face][0], m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, getStride((ComponentID)0), m_pFaceRotBuf, m_sVideoInfo.iFaceWidth, rot, false, FACE_NO_FLIP);

        copyFaceType1(face, pDstYuv, COMPONENT_Y, rot);
      }
    }
  }
  else if(pDstYuv->getChromaFormat()==CHROMA_444 || pDstYuv->getChromaFormat()==CHROMA_400)
  {    
    if(m_chromaFormatIDC == pDstYuv->getChromaFormat())
    {
      for(Int face=0; face<iTotalNumOfFaces; face++)
      {
        Int rot = m_sVideoInfo.framePackStruct.faces[m_facePos[face][0]][m_facePos[face][1]].rot;
        if(face < m_sVideoInfo.iNumFaces)
        {
          for(Int ch=0; ch<getNumChannels(); ch++)
          {
            ComponentID chId = (ComponentID)ch;
            rotFlipFaceChannelGeneral(m_pFacesOrig[face][ch], m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, getStride(chId), m_pFaceRotBuf, m_sVideoInfo.iFaceWidth, rot, false, FACE_NO_FLIP);

            copyFaceType1(face, pDstYuv, chId, rot);
          }
        }
      }
    }
    else
      assert(!"Not supported!");
  }
}

Void TOctahedron::compactFramePackType2(TComPicYuv *pDstYuv)
{
  Int iTotalNumOfFaces = m_sVideoInfo.framePackStruct.rows * m_sVideoInfo.framePackStruct.cols;
  if( !m_pFaceRotBuf )
  {
    m_pFaceRotBuf = (Pel*)xMalloc(Pel, m_sVideoInfo.iFaceWidth*m_sVideoInfo.iFaceHeight);
  }
  Pel *pcBufSrc, *pcBufDst;

  if(pDstYuv->getChromaFormat()==CHROMA_420)
  {
    if( (m_chromaFormatIDC == CHROMA_444) || (m_chromaFormatIDC == CHROMA_420 && m_bResampleChroma) )
      spherePadding();

    assert(m_sVideoInfo.framePackStruct.chromaFormatIDC == CHROMA_420);

    //1: 444->420;  444->422, H:[1, 6, 1]; 422->420, V[1,1]
    //(Wc*2Hc) and (Wc*Hc) temporal buffer; the resulting buffer is for rotation
    Int nWidthC = m_sVideoInfo.iFaceWidth>>pDstYuv->getComponentScaleX((ComponentID)1);
    Int nHeightC = m_sVideoInfo.iFaceHeight>>pDstYuv->getComponentScaleY((ComponentID)1);  
    Int nMarginSize = (m_filterDs[1].nTaps-1)>>1; //0, depending on V filter and considering south pole
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
      Int rot = m_sVideoInfo.framePackStruct.faces[m_facePos[face][0]][m_facePos[face][1]].rot;
      if(face < m_sVideoInfo.iNumFaces)
      {
        if(m_chromaFormatIDC == CHROMA_444)
        {
          //chroma 444->420
          for(Int ch=1; ch<getNumChannels(); ch++)
          {
            ComponentID chId = (ComponentID)ch;
            chromaDonwsampleH(m_pFacesOrig[face][ch]-nMarginSize*getStride((ComponentID)ch), m_sVideoInfo.iFaceWidth, nHeightC422, getStride(chId), 1, m_pDS422Buf, iStride422);
            chromaDonwsampleV(m_pDS422Buf + nMarginSize*iStride422, nWidthC, m_sVideoInfo.iFaceHeight, iStride422, 1, m_pDS420Buf, iStride420);
            rotFlipFaceChannelGeneral(m_pDS420Buf, nWidthC, nHeightC, iStride420, m_pFaceRotBuf, m_sVideoInfo.iFaceWidth, rot, false, FACE_NO_FLIP);

            if(face == 3)
            {
              pcBufSrc = m_pFaceRotBuf;
              Int iHorFPShift = (nWidthC>>1) + 1 + 3*(nWidthC + 2);
              pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift;
              triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, 0, (nWidthC>>1), 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);

              pcBufSrc = m_pFaceRotBuf + (nWidthC>>1) + 1;
              pcBufDst = pDstYuv->getAddr(chId);
              triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, (nWidthC>>1)+1, nWidthC-1, 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
            }
            else
            {
              pcBufSrc = m_pFaceRotBuf; 
              if(face%2 == 0)
              {
                Int iHorFPShift = 0;
                if(face == 4)      iHorFPShift = 0;
                else if(face == 0) iHorFPShift = (nWidthC + 2);
                else if(face == 6) iHorFPShift = 2*(nWidthC + 2);
                else if(face == 2) iHorFPShift = 3*(nWidthC + 2);

                pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift;
              }
              else
              {
                Int iHorFPShift = 0;
                if(face == 5)      iHorFPShift = (nWidthC>>1) + 1;
                else if(face == 1) iHorFPShift = (nWidthC>>1) + 1 + (nWidthC + 2);
                else if(face == 7) iHorFPShift = (nWidthC>>1) + 1 + 2*(nWidthC + 2);

                pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift;
              }
              triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, 0, nWidthC-1, 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
            }
          }
        }
        else
        {
          //m_chromaFormatIDC is CHROMA_420
          for(Int ch=1; ch<getNumChannels(); ch++)
          {
            ComponentID chId = (ComponentID)ch;

            if(m_bResampleChroma)
            {
              //convert chroma_sample_loc from 2 to 0
              chromaResampleType2toType0(m_pFacesOrig[face][ch], m_pDS420Buf, nWidthC, nHeightC, getStride(chId), nWidthC);
              rotFlipFaceChannelGeneral(m_pDS420Buf, nWidthC, nHeightC, iStride420, m_pFaceRotBuf, m_sVideoInfo.iFaceWidth, rot, false, FACE_NO_FLIP);

              if(face == 3)
              {
                pcBufSrc = m_pFaceRotBuf;
                Int iHorFPShift = (nWidthC>>1) + 1 + 3*(nWidthC + 2);
                pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift;
                triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, 0, (nWidthC>>1), 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);

                pcBufSrc = m_pFaceRotBuf + (nWidthC>>1) + 1;
                pcBufDst = pDstYuv->getAddr(chId);
                triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, (nWidthC>>1)+1, nWidthC-1, 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
              }
              else
              {
                pcBufSrc = m_pFaceRotBuf; 
                if(face%2 == 0)
                {
                  Int iHorFPShift = 0;
                  if(face == 4)      iHorFPShift = 0;
                  else if(face == 0) iHorFPShift = (nWidthC + 2);
                  else if(face == 6) iHorFPShift = 2*(nWidthC + 2);
                  else if(face == 2) iHorFPShift = 3*(nWidthC + 2);

                  pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift;
                }
                else
                {
                  Int iHorFPShift = 0;
                  if(face == 5)      iHorFPShift = (nWidthC>>1) + 1;
                  else if(face == 1) iHorFPShift = (nWidthC>>1) + 1 + (nWidthC + 2);
                  else if(face == 7) iHorFPShift = (nWidthC>>1) + 1 + 2*(nWidthC + 2);

                  pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift;
                }
                triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, 0, nWidthC-1, 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
              }
            }
            else
            {
              rotFlipFaceChannelGeneral(m_pFacesOrig[face][ch], nWidthC, nHeightC, getStride(chId), m_pFaceRotBuf, m_sVideoInfo.iFaceWidth, rot, false, FACE_NO_FLIP);

              if(face == 3)
              {
                pcBufSrc = m_pFaceRotBuf;
                Int iHorFPShift = (nWidthC>>1) + 1 + 3*(nWidthC + 2);
                pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift;
                triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, 0, (nWidthC>>1), 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);

                pcBufSrc = m_pFaceRotBuf + (nWidthC>>1) + 1;
                pcBufDst = pDstYuv->getAddr(chId);
                triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, (nWidthC>>1) + 1, nWidthC-1, 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
              }
              else
              {
                pcBufSrc = m_pFaceRotBuf; 
                if(face%2 == 0)
                {
                  Int iHorFPShift = 0;
                  if(face == 4)      iHorFPShift = 0;
                  else if(face == 0) iHorFPShift = (nWidthC + 2);
                  else if(face == 6) iHorFPShift = 2*(nWidthC + 2);
                  else if(face == 2) iHorFPShift = 3*(nWidthC + 2);
  
                  pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift;
                }
                else
                {
                  Int iHorFPShift = 0;
                  if(face == 5)      iHorFPShift = (nWidthC>>1) + 1;
                  else if(face == 1) iHorFPShift = (nWidthC>>1) + 1 + (nWidthC + 2);
                  else if(face == 7) iHorFPShift = (nWidthC>>1) + 1 + 2*(nWidthC + 2);

                  pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift;
                }
                triangleFaceCopy( nWidthC, nHeightC, pcBufSrc, 0, nWidthC-1, 0, nHeightC-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
              }
            }
          }
        }
        //luma
        rotFlipFaceChannelGeneral(m_pFacesOrig[face][0], m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, getStride((ComponentID)0), m_pFaceRotBuf, m_sVideoInfo.iFaceWidth, rot, false, FACE_NO_FLIP);

        if(face == 3)
        {
          pcBufSrc = m_pFaceRotBuf;
          Int iHorFPShift = (m_sVideoInfo.iFaceWidth>>1) + 2 + 3*(m_sVideoInfo.iFaceWidth + 4);
          pcBufDst = pDstYuv->getAddr(COMPONENT_Y) + iHorFPShift;
          triangleFaceCopy(m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, pcBufSrc, 0, (m_sVideoInfo.iFaceWidth>>1) + 1, 0, m_sVideoInfo.iFaceHeight-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(COMPONENT_Y), COMPONENT_Y, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);

          pcBufSrc = m_pFaceRotBuf + (m_sVideoInfo.iFaceWidth>>1) + 2;
          pcBufDst = pDstYuv->getAddr(COMPONENT_Y);
          triangleFaceCopy(m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, pcBufSrc, (m_sVideoInfo.iFaceWidth>>1)+2, m_sVideoInfo.iFaceWidth-1, 0, m_sVideoInfo.iFaceHeight-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(COMPONENT_Y), COMPONENT_Y, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
        }
        else
        {
          pcBufSrc = m_pFaceRotBuf;
          if(face%2 == 0)
          {
            Int iHorFPShift = 0;
            if(face == 4)      iHorFPShift = 0;
            else if(face == 0) iHorFPShift = (m_sVideoInfo.iFaceWidth + 4);
            else if(face == 6) iHorFPShift = 2*(m_sVideoInfo.iFaceWidth + 4);
            else if(face == 2) iHorFPShift = 3*(m_sVideoInfo.iFaceWidth + 4);

            pcBufDst = pDstYuv->getAddr(COMPONENT_Y) + iHorFPShift; 
          }
          else
          {
            Int iHorFPShift = 0;
            if(face == 5)      iHorFPShift = (m_sVideoInfo.iFaceWidth>>1) + 2;
            else if(face == 1) iHorFPShift = (m_sVideoInfo.iFaceWidth>>1) + 2 + (m_sVideoInfo.iFaceWidth + 4);
            else if(face == 7) iHorFPShift = (m_sVideoInfo.iFaceWidth>>1) + 2 + 2*(m_sVideoInfo.iFaceWidth + 4);

            pcBufDst = pDstYuv->getAddr(COMPONENT_Y) + iHorFPShift;
          }
          triangleFaceCopy(m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, pcBufSrc, 0, m_sVideoInfo.iFaceWidth-1, 0, m_sVideoInfo.iFaceHeight-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(COMPONENT_Y), COMPONENT_Y, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
        }
      }
    }
  }
  else if(pDstYuv->getChromaFormat()==CHROMA_444 || pDstYuv->getChromaFormat()==CHROMA_400)
  {    
    if(m_chromaFormatIDC == pDstYuv->getChromaFormat())
    {
      for(Int face=0; face<iTotalNumOfFaces; face++)
      {
        Int rot = m_sVideoInfo.framePackStruct.faces[m_facePos[face][0]][m_facePos[face][1]].rot;
        if(face < m_sVideoInfo.iNumFaces)
        {
          for(Int ch=0; ch<getNumChannels(); ch++)
          {
            ComponentID chId = (ComponentID)ch;
            rotFlipFaceChannelGeneral(m_pFacesOrig[face][ch], m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, getStride(chId), m_pFaceRotBuf, m_sVideoInfo.iFaceWidth, rot, false, FACE_NO_FLIP);

            if(face == 3)
            {
              pcBufSrc = m_pFaceRotBuf;
              Int iHorFPShift = (m_sVideoInfo.iFaceWidth>>1) + 2 + 3*(m_sVideoInfo.iFaceWidth + 4);
              pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift;
              triangleFaceCopy(m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, pcBufSrc, 0, (m_sVideoInfo.iFaceWidth>>1)+1, 0, m_sVideoInfo.iFaceHeight-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);

              pcBufSrc = m_pFaceRotBuf + (m_sVideoInfo.iFaceWidth>>1) + 2;
              pcBufDst = pDstYuv->getAddr(chId);
              triangleFaceCopy(m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, pcBufSrc, (m_sVideoInfo.iFaceWidth>>1)+2, m_sVideoInfo.iFaceWidth-1, 0, m_sVideoInfo.iFaceHeight-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
            }
            else
            {
              pcBufSrc = m_pFaceRotBuf;
              if(face%2 == 0)
              {
                Int iHorFPShift = 0;
                if(face == 4)      iHorFPShift = 0;
                else if(face == 0) iHorFPShift = (m_sVideoInfo.iFaceWidth + 4);
                else if(face == 6) iHorFPShift = 2*(m_sVideoInfo.iFaceWidth + 4);
                else if(face == 2) iHorFPShift = 3*(m_sVideoInfo.iFaceWidth + 4);

                pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift; 
              }
              else
              {
                Int iHorFPShift = 0;
                if(face == 5)      iHorFPShift = (m_sVideoInfo.iFaceWidth>>1) + 2;
                else if(face == 1) iHorFPShift = (m_sVideoInfo.iFaceWidth>>1) + 2 + (m_sVideoInfo.iFaceWidth + 4);
                else if(face == 7) iHorFPShift = (m_sVideoInfo.iFaceWidth>>1) + 2 + 2*(m_sVideoInfo.iFaceWidth + 4);

                pcBufDst = pDstYuv->getAddr(chId) + iHorFPShift;
              }
              triangleFaceCopy(m_sVideoInfo.iFaceWidth, m_sVideoInfo.iFaceHeight, pcBufSrc, 0, m_sVideoInfo.iFaceWidth-1, 0, m_sVideoInfo.iFaceHeight-1, m_sVideoInfo.iFaceWidth, pcBufDst, pDstYuv->getStride(chId), chId, rot, FACE_NO_FLIP, face, (m_nBitDepth-m_nOutputBitDepth), m_nOutputBitDepth);
            }
          }
        }
      }
    }
    else
      assert(!"Not supported!");
  }
}

// rot is defined in the clock-wise rotation manner
Void TOctahedron::triangleFaceCopy(Int iFaceWidth, Int iFaceHeight, Pel *pSrcBuf, Int iStartHorPos, Int iEndHorPos, Int iStartVerPos, Int iEndVerPos, Int iStrideSrc, Pel *pDstBuf, Int iStrideDst, ComponentID chId, Int rot, FaceFlipType eFaceFlipType, Int face, Int iBDAdjust, Int iMaxBD)
{
  assert(iBDAdjust >=0);
  assert(rot == 0 || rot == 180);
  Int iScaleX    = getComponentScaleX(chId);
  Int iScaleY    = getComponentScaleY(chId);
  Int iWidthSrc  = iEndHorPos - iStartHorPos + 1;
  Int iHeightSrc = iEndVerPos - iStartVerPos   + 1; 
  Int iOffset    = iBDAdjust>0? (1<<(iBDAdjust-1)) : 0; 

  if(!rot)
  {
    Pel *pSrcLine = pSrcBuf;
    for(Int j=0; j<iHeightSrc; j++)
    {
      Pel *pSrc = pSrcLine;
      for(Int i = 0; i < iWidthSrc; i++, pSrc++)
      {
        if(eFaceFlipType == FACE_NO_FLIP)
        {
          if(insideFace(face, (iStartHorPos + i)<<iScaleX, (iStartVerPos + j)<<iScaleY, COMPONENT_Y, chId))
          {  
            pDstBuf[i] = ClipBD(((*pSrc) + iOffset) >> iBDAdjust, iMaxBD);
          }
        }
        else if(eFaceFlipType == FACE_HOR_FLIP)
        {
          if(insideFace(face, (iFaceWidth - 1 -(iStartHorPos+i))<<iScaleX, (iStartVerPos + j)<<iScaleY, COMPONENT_Y, chId))
          { 
            pDstBuf[i] = ClipBD(((*pSrc) + iOffset) >> iBDAdjust, iMaxBD);
          }
        }
        else if(eFaceFlipType == FACE_VER_FLIP)
        {
          if(insideFace(face, (iStartHorPos + i)<<iScaleX, (iFaceHeight - 1 - (iStartVerPos+j))<<iScaleY, COMPONENT_Y, chId))
          { 
            pDstBuf[i] = ClipBD(((*pSrc) + iOffset) >> iBDAdjust, iMaxBD);
          }
        } 
      }
      pDstBuf += iStrideDst;
      pSrcLine += iStrideSrc;
    }
  }
  else if(rot == 180)
  {
    Pel *pSrcLine = pSrcBuf;
    for(Int j=0; j<iHeightSrc; j++)
    {
      Pel *pSrc = pSrcLine;
      for(Int i = 0; i < iWidthSrc; i++, pSrc++)
      {
        if(eFaceFlipType == FACE_NO_FLIP)
        {
          if(insideFace(face, (iFaceWidth - 1 - (iStartHorPos+i))<<iScaleX, (iFaceHeight-1-(iStartVerPos+j))<<iScaleY, COMPONENT_Y, chId))
          {
            pDstBuf[i] = ClipBD(((*pSrc) + iOffset) >> iBDAdjust, iMaxBD);
          }
        }
        else if(eFaceFlipType == FACE_HOR_FLIP)
        {
          if(insideFace(face, (iStartHorPos+i)<<iScaleX, (iFaceHeight-1-(iStartVerPos+j))<<iScaleY, COMPONENT_Y, chId))
          {
            pDstBuf[i] = ClipBD(((*pSrc) + iOffset) >> iBDAdjust, iMaxBD);
          }
        }
        else if(eFaceFlipType == FACE_VER_FLIP)
        {
          if(insideFace(face, (iFaceWidth - 1 - (iStartHorPos+i))<<iScaleX, (iStartVerPos+j)<<iScaleY, COMPONENT_Y, chId))
          {
            pDstBuf[i] = ClipBD(((*pSrc) + iOffset) >> iBDAdjust, iMaxBD);
          }
        }
      }
      pDstBuf += iStrideDst;
      pSrcLine += iStrideSrc;
    }  
  }
}

Void TOctahedron::rotFlipFaceChannelGeneral(Pel *pSrcBuf, Int iWidthSrc, Int iHeightSrc, Int iStrideSrc, Pel *pDstBuf, Int iStrideDst, Int rot, Bool bInverse, FaceFlipType eFaceFlipType)
{
  Int iWidthDst = iWidthSrc;
  Int iHeightDst = iHeightSrc;

  if(bInverse)
  {
    rot = (360-rot)%360;
  }
  if(!rot)
  {
    iWidthDst     = iWidthSrc;
    iHeightDst    = iHeightSrc;
    Pel *pSrcLine = pSrcBuf;
    Pel *pDstLine = pDstBuf;

    for(Int j=0; j<iHeightDst; j++)
    {
      for(Int i=0; i<iWidthDst; i++)
      {
        pDstLine[i] = pSrcLine[i];
      }
      pDstLine += iStrideDst;
      pSrcLine += iStrideSrc;
    }
  }
  else if(rot == 90)
  {
    iWidthDst  = iHeightSrc;
    iHeightDst = iWidthSrc;
    Pel *pSrcLine  = pSrcBuf + iWidthSrc - 1;
    Pel *pDstLine = pDstBuf;
    for(Int j=0; j<iHeightDst; j++)
    {
      Pel *pSrc = pSrcLine;
      for(Int i=0; i<iWidthDst; i++, pSrc += iStrideSrc)
      {
        pDstLine[i] = *pSrc;
      }
      pDstLine += iStrideDst;
      pSrcLine --;
    }
  }
  else if(rot == 180)
  {
    iWidthDst     = iWidthSrc;
    iHeightDst    = iHeightSrc;
    Pel *pSrcLine = pSrcBuf + (iHeightSrc-1)*iStrideSrc + iWidthSrc - 1;
    Pel *pDstLine = pDstBuf;
    for(Int j=0; j<iHeightDst; j++)
    {
      Pel *pSrc = pSrcLine;
      for(Int i=0; i<iWidthDst; i++, pSrc-- )
      {
        pDstLine[i] = *pSrc;
      }
      pDstLine += iStrideDst;
      pSrcLine -= iStrideSrc;
    }
  }
  else if(rot == 270)
  {
    iWidthDst  = iHeightSrc;
    iHeightDst = iWidthSrc;
    Pel *pSrcLine = pSrcBuf + (iHeightSrc-1)*iStrideSrc;
    Pel *pDstLine = pDstBuf;
    for(Int j=0; j<iHeightDst; j++)
    {
      Pel *pSrc = pSrcLine;
      for(Int i=0; i<iWidthDst; i++, pSrc -= iStrideSrc)
      {
        pDstLine[i] = *pSrc;
      }
      pDstLine += iStrideDst;
      pSrcLine ++;
    }
  }
  else
    assert(!"Not supported");

  if(eFaceFlipType == FACE_HOR_FLIP)
  {
    Pel *pDstLine = pDstBuf;
    for(Int j = 0; j < iHeightDst; j++)
    {
      for(Int i = 0; i <= (iWidthDst>>1)-1; i++)
      {
        Pel tmp = pDstLine[i];
        pDstLine[i] = pDstLine[iWidthDst-1-i];
        pDstLine[iWidthDst-1-i] = tmp;
      }
      pDstLine += iStrideDst;
    }
  }
  else if(eFaceFlipType == FACE_VER_FLIP)
  {
    Pel *pDstLine = pDstBuf;
    for(Int i = 0; i < iWidthDst; i++)
    {
      for(Int j = 0; j <= (iHeightDst>>1)-1; j++)
      {
        Pel tmp = pDstLine[j*iStrideDst];
        pDstLine[j*iStrideDst] = pDstLine[(iHeightDst-1-j)*iStrideDst];
        pDstLine[(iHeightDst-1-j)*iStrideDst] = tmp;
      }  
      pDstLine ++;
    }
  }
}

Void TOctahedron::geoToFramePack(IPos* posIn, IPos2D* posOut)
{
  if(m_sVideoInfo.iCompactFPStructure == 0)  // native OHP
  {
    TGeometry::geoToFramePack(posIn, posOut);
  }
  else if(m_sVideoInfo.iCompactFPStructure == 1)
  {
    Int face = posIn->faceIdx;
    Int iFaceWidth  = m_sVideoInfo.iFaceWidth;
    Int iFaceHeight = m_sVideoInfo.iFaceHeight;
    Int rot = m_sVideoInfo.framePackStruct.faces[m_facePos[face][0]][m_facePos[face][1]].rot;
    Int xoffset = 0;
    Int yoffset = 0;

    switch (face)
    {
    case 0:
      xoffset = (iFaceWidth >> 1) + 2;
      yoffset = 0;
      break;
    case 1:
      xoffset = (iFaceWidth >> 1) + 2;
      yoffset = iFaceHeight;
      break;
    case 2:
      xoffset = (iFaceWidth >> 1) + 2 + (iFaceWidth + 4);
      yoffset = 0;
      break;
    case 3:
      xoffset = (iFaceWidth >> 1) + 2 + (iFaceWidth + 4);
      yoffset = iFaceHeight;
      break;
    case 4:
      xoffset = 0;
      yoffset = 0;
      break;
    case 5:
      xoffset = 0;
      yoffset = iFaceHeight;
      break;
    case 6:
      xoffset = (iFaceWidth + 4);
      yoffset = 0;
      break;
    case 7:
    default:
      xoffset = (iFaceWidth + 4);
      yoffset = iFaceHeight;
      break;
    }

    Int xc=0, yc=0;

    if(!rot)
    {
      xc = posIn->u;
      yc = posIn->v;
    }
    else if(rot==90)
    {
      xc = posIn->v;
      yc = m_sVideoInfo.iFaceWidth-1-posIn->u;
    }
    else if(rot==180)
    {
      xc = m_sVideoInfo.iFaceWidth-posIn->u-1;
      yc = m_sVideoInfo.iFaceHeight-posIn->v-1;
    }
    else if(rot==270)
    {
      xc = m_sVideoInfo.iFaceHeight-1-posIn->v;
      yc = posIn->u;
    }
    else
      assert(!"rotation degree is not supported!\n");

    posOut->x=xc+xoffset;
    posOut->y=yc+yoffset;

    if (face == 2 || face == 3)
    {
      int d = (iFaceWidth + 4) * 2;
    
      while (posOut->x >= d)
        posOut->x -= d;
    }
  }
  else if(m_sVideoInfo.iCompactFPStructure == 2)
  {
    Int face = posIn->faceIdx;
    Int iFaceWidth  = m_sVideoInfo.iFaceWidth;
    Int rot = m_sVideoInfo.framePackStruct.faces[m_facePos[face][0]][m_facePos[face][1]].rot;
    Int xoffset = 0;
    Int yoffset = 0;

    if(face == 0)      xoffset = iFaceWidth + 4;
    else if(face == 1) xoffset = (iFaceWidth>>1) + 2 + (iFaceWidth + 4);
    else if(face == 2) xoffset = 3*(iFaceWidth + 4);
    else if(face == 3) xoffset = (iFaceWidth>>1) + 2 + 3*(iFaceWidth + 4);
    else if(face == 4) xoffset = 0;
    else if(face == 5) xoffset = (iFaceWidth>>1) + 2;
    else if(face == 6) xoffset = 2*(iFaceWidth + 4);
    else if(face == 7) xoffset = (iFaceWidth>>1) + 2 + 2*(iFaceWidth + 4);

    Int xc=0, yc=0;

    if(!rot)
    {
      xc = posIn->u;
      yc = posIn->v;
    }
    else if(rot==90)
    {
      xc = posIn->v;
      yc = m_sVideoInfo.iFaceWidth-1-posIn->u;
    }
    else if(rot==180)
    {
      xc = m_sVideoInfo.iFaceWidth-posIn->u-1;
      yc = m_sVideoInfo.iFaceHeight-posIn->v-1;
    }
    else if(rot==270)
    {
      xc = m_sVideoInfo.iFaceHeight-1-posIn->v;
      yc = posIn->u;
    }
    else
      assert(!"rotation degree is not supported!\n");

    posOut->x=xc+xoffset;
    posOut->y=yc+yoffset;

    if(face == 3)
    {
      if(posOut->x >= 4*(iFaceWidth + 4))
      {
        posOut->x = posOut->x - 4*(iFaceWidth + 4);
      } 
    }
  }
}

#endif