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

/** \file     TGeometry.h
    \brief    Geometry class (header)
*/

#ifndef __TGEOMETRY__
#define __TGEOMETRY__
#include <math.h>
#include "../TLibCommon/CommonDef.h"
#include "../TLibCommon/TComPicYuv.h"


// ====================================================================================================================
// Class definition
// ====================================================================================================================

#if SVIDEO_EXT

//maco for integration;
#define SVIDEO_VIEWPORT_PSNR                             1
#define SVIDEO_VIEWPORT_OUTPUT                           0
#define SVIDEO_VIEWPORT_PSNR_REPORT_PER_FRAME            0
#define SVIDEO_SPSNR_NN                                  1
#define SVIDEO_SPSNR_NN_REPORT_PER_FRAME                 1
#define SVIDEO_SPSNR_I                                   1
#define SVIDEO_SPSNR_I_REPORT_PER_FRAME                  1
#define SVIDEO_WSPSNR                                    1
#define SVIDEO_WSPSNR_REPORT_PER_FRAME                   1
#define SVIDEO_CPPPSNR                                   1
#define SVIDEO_CPPPSNR_REPORT_PER_FRAME                  1
#if SVIDEO_WSPSNR
#define SVIDEO_WSPSNR_E2E                                1          //depends on SVIDEO_WSPSNR;
#define SVIDEO_WSPSNR_E2E_REPORT_PER_FRAME               1
#endif
#define SVIDEO_SEC_ISP                                   1//Brave change it to 0
//~end;


#define SV_MAX_NUM_SAMPLING          64
#define SV_MAX_NUM_FACES             20
#define MISSED_SAMPLE_VALUE          (-1)

#define SVIDEO_DEBUG                 0

#define scos(x)         cos((Double)(x))
#define ssin(x)         sin((Double)(x))
#define satan(x)        atan((Double)(x))
#define satan2(y, x)    atan2((Double)(y), (Double)(x))
#define sacos(x)        acos((Double)(x))
#define sasin(x)        asin((Double)(x))
#define ssqrt(x)        sqrt((Double)(x))
#define sfloor(x)       floor((Double)(x))
#define sfabs(x)        fabs((Double)(x))
#define stan(x)         tan((Double)(x))


typedef Double          POSType;

static const Double S_PI = 3.14159265358979323846;
static const Double S_PI_2 = 1.57079632679489661923;
static const Double S_EPS = 1.0e-6;
static const Int    S_PAD_MAX = 16;
static const POSType S_ICOSA_GOLDEN = ((ssqrt(5.0)+1.0)/2.0);

static const Int   S_INTERPOLATE_PrecisionBD = 14;
static const Int   S_log2NumFaces[SV_MAX_NUM_FACES+1] = { 0, 
                                                          1, 1, 
                                                          2, 2, 
                                                          3, 3, 3, 3,
                                                          4, 4, 4, 4, 4, 4, 4, 4, 
                                                          5, 5, 5, 5 };
static const Int  S_LANCZOS_LUT_SCALE = 100;

enum GeometryType
{
  SVIDEO_EQUIRECT = 0,
  SVIDEO_CUBEMAP,
  SVIDEO_EQUALAREA,
  SVIDEO_OCTAHEDRON,
  SVIDEO_VIEWPORT,
  SVIDEO_ICOSAHEDRON,
#if SVIDEO_CPPPSNR
  SVIDEO_CRASTERSPARABOLIC,
#endif
  SVIDEO_TYPE_NUM,
};

enum SInterpolationType
{
  SI_UNDEFINED = 0,
  SI_NN,
  SI_BILINEAR,
  SI_BICUBIC,
  SI_LANCZOS2,
  SI_LANCZOS3,
  SI_TYPE_NUM
};

enum FaceFlipType
{
  FACE_NO_FLIP = 0,
  FACE_HOR_FLIP,
  FACE_VER_FLIP,
  FACE_FLIP_NUM
};

struct IPos
{
  Int   faceIdx;
  Short u;
  Short v;
  IPos() : faceIdx(0), u(0), v(0) {};
  IPos(Int f, Int xIn, Int yIn) : faceIdx(f), u(xIn), v(yIn) {};
};
struct SPos
{
  Int   faceIdx;
  POSType x;
  POSType y;
  POSType z;
  SPos() : faceIdx(0), x(0), y(0), z(0) {};
  SPos(Int f, POSType xIn, POSType yIn, POSType zIn ) : faceIdx(f), x(xIn), y(yIn), z(zIn) {};
};

struct CPos3D
{
  POSType x;
  POSType y;
  POSType z;  
};
struct CPos2D
{
  POSType x;
  POSType y;
};
struct IPos2D
{
  Short x;
  Short y;
};

struct FaceProperty
{
  Int id;
  Int rot;
  
  Int width;
  Int height;
  
};
struct SVideoFPStruct
{
  ChromaFormat chromaFormatIDC;
  Int rows;
  Int cols;
  FaceProperty faces[12][12];
};
struct GeometryRotation
{
  Int degree[3];  //[x/y/z];
};
struct ViewPortSettings
{
  Float hFOV;
  Float vFOV;
  Float fYaw;              //
  Float fPitch;
};
#if SVIDEO_VIEWPORT_PSNR
struct ViewPortPSNRParam
{
  Bool      bViewPortPSNREnabled;
  std::vector<ViewPortSettings> viewPortSettingsList;
  Int       iViewPortWidth;
  Int       iViewPortHeight;
};
#endif

struct SVideoInfo
{
  Int geoType;
  SVideoFPStruct framePackStruct; 
  GeometryRotation sVideoRotation;  

  Int iFaceWidth;          //native size without framepacking;
  Int iFaceHeight;         //native size without framepacking;
  Int iNumFaces;          //geometry faces; it does not include virtual faces may be in the frame packing;
  ViewPortSettings viewPort;
  Int iCompactFPStructure; //compact type;
};
struct Filter1DInfo
{
  Int nTaps;
  Int nlog2Norm;
  Int iFilterCoeff[16]; 
};

struct TriMesh
{
  POSType vertex[3][3];
  POSType normVec[3];
  POSType origin[3];
  POSType baseVec[2][3]; 
};

class TGeometry;
struct PxlFltLut
{
  Int facePos;          //MSBs for pos; LSBs for faceIdx;
  UShort weightIdx; 
};
typedef Void (TGeometry::*interpolateWeightFP)(ComponentID chId, SPos *pSPosIn, PxlFltLut &wlist);


struct InputGeoParam
{
  ChromaFormat chromaFormat;
  Bool bResampleChroma;
  Int nBitDepth;
  Int nOutputBitDepth;
  Int iInterp[MAX_NUM_CHANNEL_TYPE];
  Int iChromaSampleLocType;
};

struct SpherePoints
{
  Int iNumOfPoints;
  Double (*pPointPos)[2];  //[0:latitude [-90,90]; 1: longitude [-180, 180]]
};

class TGeometry
{
protected:
  SVideoInfo m_sVideoInfo;
//Brave:add
  Pel **braveLocation;//Brave:[component][lacation]
//Brave:add

  Pel ***m_pFacesBuf; 
  Pel ***m_pFacesOrig; //[face][component][raster scan position]
  
  //descriptor for the buffer;
  ChromaFormat m_chromaFormatIDC;   //chroma format of geometry;
  Bool m_bResampleChroma;           //resample chroma location if it is 4:2:0;
  Int  m_iMarginX;
  Int  m_iMarginY;
  Int  m_nBitDepth;                //Internal bitdepth;
  Int  m_nOutputBitDepth;          //Output bitdepth;

  //temp buffer for chroma downsampling used for framepacking;
  Pel* m_pDS422Buf;
  Pel* m_pDS420Buf;
  Filter1DInfo m_filterDs[2];  //[0:Hor][1:Ver]
  Pel* m_pFaceRotBuf;

  //temp buffer for chroma upsampling;
  Int *m_pUpsTempBuf; //extended buffer for chroma upsampling; [widthC+filtersize]*[heightC*2];
  Int m_iUpsTempBufMarginSize;
  Int m_iStrideUpsTempBuf;
  Filter1DInfo m_filterUps[4];  //[0:Hor phase0; 1:Hor phase1; 2:Ver 3/4 phase; 3: Ver 1/4 phase] The norm for different phases in one direciton must be the same;

  Pel **m_pFacesBufTemp;   //[face][raster scan position]; store the chroma data, and used for chroma upsampling;
  Int m_nMarginSizeBufTemp;
  Int m_nStrideBufTemp;
  Pel **m_pFacesBufTempOrig;

  Bool m_bPadded;
  //interpolation;
  SInterpolationType m_InterpolationType[MAX_NUM_CHANNEL_TYPE];
  //frame packing;
  Int m_facePos[SV_MAX_NUM_FACES][2];   //[face][0:row, 1:col];

  static Char m_strGeoName[SVIDEO_TYPE_NUM][256];
  POSType *m_pfLanczosFltCoefLut[MAX_NUM_CHANNEL_TYPE];
  Int m_iLanczosParamA[MAX_NUM_CHANNEL_TYPE];

  Int  m_WeightMap_NumOfBits4Faces;   //5;
  Bool m_bGeometryMapping;
  interpolateWeightFP m_interpolateWeight[MAX_NUM_CHANNEL_TYPE]; 

  Int m_iInterpFilterTaps[MAX_NUM_CHANNEL_TYPE][2];                                        //[channel][hor/ver];
  Int **m_pWeightLut[2];
  PxlFltLut *m_pPixelWeight[SV_MAX_NUM_FACES][2];                   //[SV_MAX_NUM_FACES][2][pxl_idx];

  Int m_iChromaSampleLocType;
  Void setChromaResamplingFilter(Int iChromaSampleLocType);

  Bool m_bGeometryMapping4SpherePadding;
  PxlFltLut *m_pPixelWeight4SherePadding[SV_MAX_NUM_FACES][2];
  Bool m_bConvOutputPaddingNeeded;

  Void geometryMapping4SpherePadding();
  Void getSPLutIdx(Int ch, Int x, Int y, Int& iIdx);

  Void initInterpolation(Int *pInterpolateType);
  Void chromaUpsample(Pel *pSrcBuf, Int nWidthC, Int nHeightC, Int iStrideSrc, Int iFaceId, ComponentID chId);
  Void rotOneFaceChannel(Pel *pSrc, Int iWidthSrc, Int iHeightSrc, Int iStrideSrc, Int iNumSamplesPerPixel, Int ch, Int rot, TComPicYuv *pDstYuv, Int offsetX, Int offsetY, Int faceIdx, Int iBDAdjust);
  Void rotFaceChannelGeneral(Pel *pSrc, Int iWidthSrc, Int iHeightSrc, Int iStrideSrc, Int nSPPSrc, Int rot, Pel *pDst, Int iStrideDst, Int nSPPDst, Bool bInverse=false);
  Void chromaDonwsampleH(Pel *pSrcBuf, Int iWidth, Int iHeight, Int iStrideSrc, Int iNumPels, Pel *pDstBuf, Int iStrideDst); //horizontal 2:1 downsampling;
  Void chromaDonwsampleV(Pel *pSrcBuf, Int iWidth, Int iHeight, Int iStrideSrc, Int iNumPels, Pel *pDstBuf, Int iStrideDst); //vertical 2:1 downsampling;
  inline Int round(POSType t) { return (Int)(t+ (t>=0? 0.5 :-0.5)); }; 
  Void rotate3D(SPos& sPos, Int rx, Int ry, Int rz);

  Int getFilterSize(SInterpolationType filterType);
  Void initFilterWeightLut();
  Void interpolate_nn_weight(ComponentID chId, SPos *pSPosIn, PxlFltLut &wlist);
  Void interpolate_bilinear_weight(ComponentID chId, SPos *pSPosIn, PxlFltLut &wlist);
  Void interpolate_bicubic_weight(ComponentID chId, SPos *pSPosIn, PxlFltLut &wlist);
  Void interpolate_lanczos_weight(ComponentID chId, SPos *pSPosIn, PxlFltLut &wlist);

  Void chromaResampleType0toType2(Pel *pSrcBuf, Int nWidthC, Int nHeightC, Int iStrideSrc, Pel *pDstBuf, Int iStrideDst);
  Void chromaResampleType2toType0(Pel *pSrcBuf, Pel *pDst, Int nWidthC, Int nHeightC, Int iStrideSrc, Int iStrideDst); 
  Void fillRegion(TComPicYuv *pDstYuv, Int x, Int y, Int rot, Int iFaceWidth, Int iFaceHeight);
  
  //frame packing; 
  Void parseFacePos(Int *pFacePos);

  Void vecMul(const POSType *v0, const POSType *v1, POSType *normVec, Int bNormalized);
  Void initTriMesh(TriMesh& meshFaces);

  //debug;
  Void dumpAllFacesToFile(Char *pPrefixFN, Bool bMarginIncluded, Bool bAppended);
  Void dumpBufToFile(Pel *pSrc, Int iWidth, Int iHeight, Int iNumSamples, Int iStride, FILE *fp);  
 
public:

  TGeometry();
  virtual ~TGeometry();
  Void geoInit(SVideoInfo& sVideoInfo, InputGeoParam *pInGeoParam);


  GeometryType getType() { return (GeometryType)m_sVideoInfo.geoType; };
  Int getNumChannels() const { return ::getNumberValidComponents(m_chromaFormatIDC); };
  Int getStride(const ComponentID id) const {  return (((m_sVideoInfo.iFaceWidth) + (m_iMarginX  <<1)) >> ::getComponentScaleX(id, m_chromaFormatIDC));  }; 
  Int getMarginX(const ComponentID id) const { return (m_iMarginX >> ::getComponentScaleX(id, m_chromaFormatIDC));  }
  Int getMarginY(const ComponentID id) const { return (m_iMarginY >> ::getComponentScaleY(id, m_chromaFormatIDC));  }
  Int getComponentScaleX(const ComponentID id) const { return (::getComponentScaleX(id, m_chromaFormatIDC));  }
  Int getComponentScaleY(const ComponentID id) const { return (::getComponentScaleY(id, m_chromaFormatIDC));  }
  Pel *getAddr(Int fId, Int compId) { return m_pFacesOrig[fId][compId]; }
  Int getMarginSize(Int bY) { return (bY? m_iMarginY : m_iMarginX); }
  Void setPaddingFlag(Bool bFlag) { m_bPadded = bFlag; }
  Char* getGeoName() { return m_strGeoName[m_sVideoInfo.geoType]; };
  //analysis;
  Void dumpSpherePoints(Char *pFileName, Bool bAppended=false, SpherePoints *pSphPoints=NULL);

  virtual Void clamp(IPos *pIPos);
  virtual Void map2DTo3D(SPos& IPosIn, SPos *pSPosOut) = 0; 
  virtual Void map3DTo2D(SPos *pSPosIn, SPos *pSPosOut) = 0; 
  virtual Void convertYuv(TComPicYuv *pSrcYuv);
  virtual Void geoConvert(TGeometry *pGeoDst);
  virtual Void framePack(TComPicYuv *pDstYuv);

  virtual Void compactFramePackConvertYuv(TComPicYuv * pSrcYuv);
  virtual Void compactFramePack(TComPicYuv *pDstYuv);
  SVideoInfo*  getSVideoInfo() {return &m_sVideoInfo;}

  virtual Void geoToFramePack(IPos* posIn, IPos2D* posOut);
#if SVIDEO_SPSNR_I
  virtual Pel  getPelValue(ComponentID chId, SPos in);
#endif
  virtual Void spherePadding(Bool bEnforced=false);
  virtual Bool insideFace(Int fId, Int x, Int y, ComponentID chId, ComponentID origchId) { return ( x>=0 && x<(m_sVideoInfo.iFaceWidth>>getComponentScaleX(chId)) && y>=0 && y<(m_sVideoInfo.iFaceHeight>>getComponentScaleY(chId)) ); }
  virtual Bool validPosition4Interp(ComponentID chId, POSType x, POSType y);
  virtual Void geometryMapping(TGeometry *pGeoSrc);

  static TGeometry* create(SVideoInfo& sVideoInfo, InputGeoParam *pInGeoParam);
  
};

#endif
#endif // __TGEOMETRY__

