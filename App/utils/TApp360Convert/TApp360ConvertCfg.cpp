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

/** \file     TApp360ConvertCfg.cpp
\brief    Handle encoder configuration parameters
*/

#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <string>
#include <limits>
#include <math.h>
#include <iomanip>
#include "TApp360Def.h"
#include "TApp360ChromaFormat.h"
#include "TApp360PicYuv.h"
#include "TLib360/TGeometry.h"
#include "TLib360/TViewPort.h"
#include "TApp360ConvertCfg.h"
#include "TAppCommon/program_options_lite.h"
#include "TLibVideoIO/TVideoIOYuv.h"
#if SVIDEO_SPSNR_NN
#include "TLib360/TPSNRMetricCalc.h"
#include "TLib360/TSPSNRMetricCalc.h"
#endif
#if SVIDEO_WSPSNR
#include "TLib360/TWSPSNRMetricCalc.h"
#endif
#if SVIDEO_SPSNR_I
#include "TLib360/TSPSNRIMetricCalc.h"
#endif
#if SVIDEO_CPPPSNR
#include "TLib360/TCPPPSNRMetricCalc.h"
#endif

#ifdef WIN32
#define strdup _strdup
#endif

#define MACRO_TO_STRING_HELPER(val) #val
#define MACRO_TO_STRING(val) MACRO_TO_STRING_HELPER(val)

using namespace std;
namespace po = df::program_options_lite;

//! \ingroup TAppEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================
Char TApp360ConvertCfg::m_sPSNRName[METRIC_NUM][256] = {  {"    PSNR"}
                                                         ,{"SPSNR_NN"}
#if SVIDEO_WSPSNR
                                                         ,{"  WSPSNR"}
#endif
#if SVIDEO_SPSNR_I
                                                         ,{" SPSNR_I"}
#endif
#if SVIDEO_CPPPSNR
                                                         ,{"CPP-PSNR"}
#endif
                                                       };

TApp360ConvertCfg::TApp360ConvertCfg()
  : m_pchInputFile(NULL)
  , m_pchOutputFile(NULL)
  , m_pchRefFile(NULL)
  , m_pchSphData(NULL)
  , m_pchVPortFile(NULL)
  , m_pchSpherePointsFile(NULL)
  , m_inputColourSpaceConvert(IPCOLOURSPACE_UNCHANGED)
  //, m_snrInternalColourSpace(false)
  , m_outputInternalColourSpace(false)
  , m_temporalSubsampleRatio(1)
  , m_faceSizeAlignment(8)
{
}

TApp360ConvertCfg::~TApp360ConvertCfg()
{
  if(m_pchInputFile) free(m_pchInputFile); m_pchInputFile=NULL;
  if(m_pchOutputFile) free(m_pchOutputFile); m_pchOutputFile=NULL;
  if(m_pchRefFile) free(m_pchRefFile); m_pchRefFile=NULL;
  if(m_pchSphData) free(m_pchSphData); m_pchSphData=NULL;
  if(m_pchVPortFile) free(m_pchVPortFile); m_pchVPortFile=NULL;
  if(m_pchSpherePointsFile) free(m_pchSpherePointsFile); m_pchSpherePointsFile=NULL;
}

Void TApp360ConvertCfg::create()
{
}

Void TApp360ConvertCfg::destroy()
{
}

Bool confirmPara(Bool bflag, const Char* message);

static InputColourSpaceConversion stringToInputColourSpaceConvert(const std::string &value, const Bool bIsForward)
{
  if (value.empty() || value=="UNCHANGED")
  {
    return IPCOLOURSPACE_UNCHANGED;
  }
  if (bIsForward)
  {
    if (value=="YCbCrtoYYY")
    {
      return IPCOLOURSPACE_YCbCrtoYYY;
    }
    if (value=="YCbCrtoYCrCb")
    {
      return IPCOLOURSPACE_YCbCrtoYCrCb;
    }
    if (value=="RGBtoGBR")
    {
      return IPCOLOURSPACE_RGBtoGBR;
    }
  }
  else
  {
    if (value=="YCrCbtoYCbCr")
    {
      return IPCOLOURSPACE_YCbCrtoYCrCb;
    }
    if (value=="GBRtoRGB")
    {
      return IPCOLOURSPACE_RGBtoGBR;
    }
  }
  return NUMBER_INPUT_COLOUR_SPACE_CONVERSIONS;
}

static std::string getListOfColourSpaceConverts(const Bool bIsForward)
{
  if (bIsForward)
  {
    return "UNCHANGED, YCbCrtoYCrCb, YCbCrtoYYY or RGBtoGBR";
  }
  else
  {
    return "UNCHANGED, YCrCbtoYCbCr or GBRtoRGB";
  }
}

static inline ChromaFormat numberToChromaFormat(const Int val)
{
  switch (val)
  {
  case 400: return CHROMA_400; break;
  case 420: return CHROMA_420; break;
  case 422: return CHROMA_422; break;
  case 444: return CHROMA_444; break;
  default:  return NUM_CHROMA_FORMAT;
  }
}

static inline std::istringstream &operator>>(std::istringstream &in, GeometryRotation &rot)     //input
{
  in>>rot.degree[0];
  in>>rot.degree[1];
  in>>rot.degree[2];
  return in;
}
static inline std::istringstream &operator>>(std::istringstream &in, SVideoFPStruct &sFPStruct)     //input
{
  in>>sFPStruct.rows;
  in>>sFPStruct.cols;
  for ( Int i = 0; i < sFPStruct.rows; i++ )
    for(Int j=0; j<sFPStruct.cols; j++)
    {
      in>>sFPStruct.faces[i][j].id;
      in>>sFPStruct.faces[i][j].rot;
    }
    return in;
}
static inline std::istringstream &operator>>(std::istringstream &in, ViewPortSettings &vp)     //input
{
  in>>vp.hFOV;
  in>>vp.vFOV;
  in>>vp.fYaw;
  in>>vp.fPitch;
  return in;
}

Void TApp360ConvertCfg::setDefaultFramePackingParam(SVideoInfo& sVideoInfo)
{
#if SVIDEO_CPPPSNR
  if(sVideoInfo.geoType == SVIDEO_EQUIRECT || sVideoInfo.geoType == SVIDEO_EQUALAREA || sVideoInfo.geoType == SVIDEO_VIEWPORT || sVideoInfo.geoType == SVIDEO_CRASTERSPARABOLIC)
#else
  if(sVideoInfo.geoType == SVIDEO_EQUIRECT || sVideoInfo.geoType == SVIDEO_EQUALAREA || sVideoInfo.geoType == SVIDEO_VIEWPORT)
#endif
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
// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param  argc        number of arguments
\param  argv        array of arguments
\retval             true when success
*/
Bool TApp360ConvertCfg::parseCfg( Int argc, Char* argv[] )
{
  Bool do_help = false;

  string cfg_InputFile;
  string cfg_OutputFile;
  string cfg_RefFile;
  string cfg_SphFile;
  string cfg_ViewFile;
  string cfg_SpherePointsFile;

  Int tmpInternalChromaFormat, tmpOutputChromaFormat;
  Int tmpInputChromaFormat;
#if SVIDEO_CPPPSNR
  Int tmpReferenceChromaFormat;
#endif
  string inputColourSpaceConvert;

  Int warnUnknowParameter = 0;

#if SVIDEO_CPPPSNR
  memset(&m_referenceSVideoInfo, 0, sizeof(m_referenceSVideoInfo));
#endif
  memset(&m_sourceSVideoInfo, 0, sizeof(m_sourceSVideoInfo));
  memset(&m_codingSVideoInfo, 0, sizeof(m_codingSVideoInfo));
  m_inputGeoParam.chromaFormat = CHROMA_444;
  m_inputGeoParam.bResampleChroma = false;
  m_inputGeoParam.nBitDepth = 8;
  m_inputGeoParam.iInterp[CHANNEL_TYPE_LUMA] = SI_LANCZOS3;
  m_inputGeoParam.iInterp[CHANNEL_TYPE_CHROMA] = SI_LANCZOS2;

  po::Options opts;
  opts.addOptions()
    ("help",                                            do_help,                                          false, "this help text")
    ("c",    po::parseConfigFile, "configuration file name")
    ("WarnUnknowParameter,w",                           warnUnknowParameter,                                  1, "warn for unknown configuration parameters instead of failing")

    // File, I/O and source parameters
    ("InputFile,i",                                     cfg_InputFile,                               string(""), "Original YUV input file name")
    ("OutputFile,o",                                    cfg_OutputFile,                              string(""), "Converted YUV output file name")
    ("RefFile,r",                                       cfg_RefFile,                                 string(""), "Ref YUV file name for PSNR calculation")
    ("SphFile,s",                                       cfg_SphFile,                                 string(""), "Spherical points data file name for S-PSNR-NN/S-PSNR-I calculation")
    ("ViewPortFile,v",                                  cfg_ViewFile,                                string(""), "Viewport paramete file name for dynamic viewport generation")
    ("SpherePointsFile,p",                              cfg_SpherePointsFile,                        string(""), "File name for point coordinates on the sphere of the converted projction")
    ("SourceWidth,-wdt",                                m_iInputWidth,                                        0, "Source picture width")
    ("SourceHeight,-hgt",                               m_iInputHeight,                                       0, "Source picture height")
    ("InputBitDepth",                                   m_inputBitDepth[CHANNEL_TYPE_LUMA],                   8, "Bit-depth of input file")
    ("OutputBitDepth",                                  m_outputBitDepth[CHANNEL_TYPE_LUMA],                  0, "Bit-depth of output file (default:InternalBitDepth)")
    ("ReferenceBitDepth",                               m_referenceBitDepth[CHANNEL_TYPE_LUMA],               0, "Bit-depth of output file (default:OutputBitDepth)")
    ("MSBExtendedBitDepth",                             m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA],             0, "bit depth of luma component after addition of MSBs of value 0 (used for synthesising High Dynamic Range source material). (default:InputBitDepth)")
    ("InternalBitDepth",                                m_internalBitDepth[CHANNEL_TYPE_LUMA],                0, "Bit-depth the codec operates at. (default:MSBExtendedBitDepth). If different to MSBExtendedBitDepth, source data will be converted")
    ("InputBitDepthC",                                  m_inputBitDepth[CHANNEL_TYPE_CHROMA],                 0, "As per InputBitDepth but for chroma component. (default:InputBitDepth)")
    ("OutputBitDepthC",                                 m_outputBitDepth[CHANNEL_TYPE_CHROMA],                0, "As per OutputBitDepth but for chroma component. (default:InternalBitDepthC)")
    ("ReferenceBitDepthC",                              m_referenceBitDepth[CHANNEL_TYPE_CHROMA],             0, "As per OutputBitDepth but for chroma component. (default:OutputBitDepthC)")
    ("MSBExtendedBitDepthC",                            m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA],           0, "As per MSBExtendedBitDepth but for chroma component. (default:MSBExtendedBitDepth)")
    ("InternalBitDepthC",                               m_internalBitDepth[CHANNEL_TYPE_CHROMA],              0, "As per InternalBitDepth but for chroma component. (default:InternalBitDepth)")
    ("InternalChromaFormat,-intercf",                   tmpInternalChromaFormat,                              0, "InternalChromaFormatIDC (400|420|422|444 or set 0 (default) for same as OutputChromaFormat)")
    ("ResampleChroma,-rc",                              m_inputGeoParam.bResampleChroma,                  false, "ResampleChroma indiates to do conversion with aligned phase with luma")
    ("OutputChromaFormat,-ocf",                         tmpOutputChromaFormat,                                0, "OutputChromaFormatIDC (400|420|422|444 or set 0 (default) for same as InputChromaFormat)")

    ("InputColourSpaceConvert",                         inputColourSpaceConvert,                     string(""), "Colour space conversion to apply to input video. Permitted values are (empty string=UNCHANGED) " + getListOfColourSpaceConverts(true))
    ("OutputInternalColourSpace",                       m_outputInternalColourSpace,                      false, "If true, then no colour space conversion is applied for reconstructed video, otherwise inverse of input is applied.")
    ("InputChromaFormat,-icf",                          tmpInputChromaFormat,                               420, "InputChromaFormatIDC")  
    ("ConformanceWindowMode",                           m_conformanceWindowMode,                              0, "Window conformance mode (0: no window, 1:automatic padding, 2:padding, 3:conformance")
    ("HorizontalPadding,-pdx",                          m_aiPad[0],                                           0, "Horizontal source padding for conformance window mode 2")
    ("VerticalPadding,-pdy",                            m_aiPad[1],                                           0, "Vertical source padding for conformance window mode 2")
    ("ConfWinLeft",                                     m_confWinLeft,                                        0, "Left offset for window conformance mode 3")
    ("ConfWinRight",                                    m_confWinRight,                                       0, "Right offset for window conformance mode 3")
    ("ConfWinTop",                                      m_confWinTop,                                         0, "Top offset for window conformance mode 3")
    ("ConfWinBottom",                                   m_confWinBottom,                                      0, "Bottom offset for window conformance mode 3")
    ("FaceSizeAlignment",                               m_faceSizeAlignment,                                  4, "Unit size for alignment")
    ("FrameRate,-fr",                                   m_iFrameRate,                                         0, "Frame rate")
    ("FrameSkip,-fs",                                   m_FrameSkip,                                         0u, "Number of frames to skip at start of input YUV")
    ("TemporalSubsampleRatio,-ts",                      m_temporalSubsampleRatio,                            1u, "Temporal sub-sample ratio when reading input YUV")
    ("FramesToBeEncoded,f",                             m_framesToBeConverted,                                0, "Number of frames to be converted (default=all)")
    ("ClipInputVideoToRec709Range",                     m_bClipInputVideoToRec709Range,                   false, "If true then clip input video to the Rec. 709 Range on loading when InternalBitDepth is less than MSBExtendedBitDepth")
    ("ClipOutputVideoToRec709Range",                    m_bClipOutputVideoToRec709Range,                  false, "If true then clip output video to the Rec. 709 Range on saving when OutputBitDepth is less than InternalBitDepth")
    ("SummaryOutFilename",                              m_summaryOutFilename,                          string(), "Filename to use for producing summary output file. If empty, do not produce a file.")
    ("SummaryPicFilenameBase",                          m_summaryPicFilenameBase,                      string(), "Base filename to use for producing summary picture output files. The actual filenames used will have I.txt, P.txt and B.txt appended. If empty, do not produce a file.")
    ("SummaryVerboseness",                              m_summaryVerboseness,                                0u, "Specifies the level of the verboseness of the text output")
#if SVIDEO_SPSNR_NN
    ("PSNR,-psnr",                                      m_psnrEnabled[METRIC_PSNR],                        true, "Flag to enable psnr calculation")
    ("SPSNR_NN,-spsnr_nn",                              m_psnrEnabled[METRIC_SPSNR_NN],                    true, "Flag to enable spsnr-nn calculation")
#endif
#if SVIDEO_WSPSNR
    ("WSPSNR,-wspsnr",                                  m_psnrEnabled[METRIC_WSPSNR],                      true, "Flag to enable ws-psnr calculation")
#endif
#if SVIDEO_SPSNR_I
    ("SPSNR_I,-spsnr_i",                                m_psnrEnabled[METRIC_SPSNR_I],                     true,  "Flag to enable spsnr-i calculation")
#endif
#if SVIDEO_CPPPSNR
    ("CPP_PSNR,-cpsnr",                                 m_psnrEnabled[METRIC_CPPPSNR],                    true, "Flag to enable cpp-psnr calculation")
    ("CPP_Width,-cwidth",                               m_cppPsnrWidth,                                       0, "Width Of CPP For CPP PSNR Calculation")
    ("CPP_Height,-cheight",                             m_cppPsnrHeight,                                      0, "Height Of CPP For CPP PSNR Calculation")
    ("ReferenceGeometryType",                           m_referenceSVideoInfo.geoType,                        0, "The Reference geometry for quality estimation")
    ("ReferenceFPStructure",                            m_referenceSVideoInfo.framePackStruct,                m_referenceSVideoInfo.framePackStruct,"Reference framepacking structure")
    ("ReferenceCompactFPStructure",                     m_referenceSVideoInfo.iCompactFPStructure,            1, "Compact coding framepacking structure; only valid for octahedron and icosahedron projection format")
    ("ReferenceChromaFormat,-rcf",                      tmpReferenceChromaFormat,                           420, "ReferenceChromaFormatIDC") 
    ("ReferenceFaceHeight",                             m_iReferenceFaceHeight,                               0, "Reference Face height for")
    ("ReferenceFaceWidth",                              m_iReferenceFaceWidth,                                0, "Reference Face width for")
#endif
    ("InputGeometryType",                               m_sourceSVideoInfo.geoType,          0,                                    "The geometry of input 360 video")
    ("SourceFPStructure",                               m_sourceSVideoInfo.framePackStruct,  m_sourceSVideoInfo.framePackStruct,   "Source framepacking structure")
    ("SourceCompactFPStructure",                        m_sourceSVideoInfo.iCompactFPStructure, 1,                             "Compact source framepacking structure; only valid for octahedron and icosahedron projection format")
    ("CodingGeometryType",                              m_codingSVideoInfo.geoType,          0,                                    "The output geometry for conversion")
    ("CodingFPStructure",                               m_codingSVideoInfo.framePackStruct,  m_codingSVideoInfo.framePackStruct,   "Coding framepacking structure")
    ("CodingCompactFPStructure",                        m_codingSVideoInfo.iCompactFPStructure, 1,                             "Compact coding framepacking structure; only valid for octahedron and icosahedron projection format") 
    ("SVideoRotation",                                  m_codingSVideoInfo.sVideoRotation,                    m_codingSVideoInfo.sVideoRotation,    "Rotation along X, Y, Z") 
    ("ViewPortSettings",                                m_codingSVideoInfo.viewPort,                          m_codingSVideoInfo.viewPort,           "Viewport settings for static viewport generation") 
    ("CodingFaceWidth",                                 m_iCodingFaceWidth,                                   0,                                    "Face width for coding")
    ("CodingFaceHeight",                                m_iCodingFaceHeight,                                  0,                                    "Face height for coding")
    ("InterpolationMethodY,-interpY",                   m_inputGeoParam.iInterp[CHANNEL_TYPE_LUMA],   (Int)SI_LANCZOS3,            "Interpolation method for luma, 0: default setting(lanczos3); 1:NN, 2: bilinear, 3: bicubic, 4: lanczos2, 5: lanczos3")
    ("InterpolationMethodC,-interpC",                   m_inputGeoParam.iInterp[CHANNEL_TYPE_CHROMA], (Int)SI_LANCZOS2,            "Interpolation method for chroma, 0: default setting(lanczos2); 1:NN, 2: bilinear, 3: bicubic, 4: lanczos2, 5: lanczos3")
    ("ChromaSampleLocType,-csl",                        m_inputGeoParam.iChromaSampleLocType,                 2,                                   "Chroma sample location type relative to luma, 0: 0.5 shift in vertical direction; 1: 0.5 shift in both directions, 2: aligned with luma (default setting), 3: 0.5 shift in horizontal direction")
    ;

  po::setDefaults(opts);
  po::ErrorReporter err;
  const list<const Char*>& argv_unhandled = po::scanArgv(opts, argc, (const Char**) argv, err);

  for (list<const Char*>::const_iterator it = argv_unhandled.begin(); it != argv_unhandled.end(); it++)
  {
    fprintf(stderr, "Unhandled argument ignored: `%s'\n", *it);
  }

  if (argc == 1 || do_help)
  {
    /* argc == 1: no options have been specified */
    po::doHelp(cout, opts);
    return false;
  }

  if (err.is_errored)
  {
    if (!warnUnknowParameter)
    {
      /* error report has already been printed on stderr */
      return false;
    }
  }

  /*
  * Set default parameters
  */
  setDefaultFramePackingParam(m_sourceSVideoInfo);
  setDefaultFramePackingParam(m_codingSVideoInfo);
# if SVIDEO_CPPPSNR
  setDefaultFramePackingParam(m_referenceSVideoInfo);
#endif

  /*
  * Set any derived parameters
  */
  m_iSourceWidth = m_iInputWidth;
  m_iSourceHeight = m_iInputHeight;
#if SVIDEO_CPPPSNR
  m_iReferenceSourceWidth  = m_iReferenceFaceWidth ;
  m_iReferenceSourceHeight = m_iReferenceFaceHeight;
#endif
  //fill the information;
  if (m_sourceSVideoInfo.geoType >= SVIDEO_TYPE_NUM)
  {
    printf( "InputGeometryType is invalid.\n" );
    exit( EXIT_FAILURE );
  }
  fillSourceSVideoInfo(m_sourceSVideoInfo, m_iInputWidth, m_iInputHeight);
  if((m_sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON || m_sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON) && ((m_sourceSVideoInfo.iFaceWidth%4) != 0 || (m_sourceSVideoInfo.iFaceHeight%4) != 0))
  {
    printf("For OHP and ISP, face width and height (%d, %d) are not multiple of 4.\n", m_sourceSVideoInfo.iFaceWidth, m_sourceSVideoInfo.iFaceHeight);
    exit( EXIT_FAILURE );
  }

#if SVIDEO_CPPPSNR
  if (m_referenceSVideoInfo.geoType >= SVIDEO_TYPE_NUM)
  {
      printf( "ReferenceGeometryType is invalid.\n" );
      exit( EXIT_FAILURE);
  }
  fillSourceSVideoInfo(m_referenceSVideoInfo, m_iReferenceFaceWidth, m_iReferenceFaceHeight);
#endif

  if (m_codingSVideoInfo.geoType >= SVIDEO_TYPE_NUM)
  {
    printf( "CodingGeometryType is invalid.\n" );
    exit( EXIT_FAILURE );
  }
  //calculate the width/height for encoding based on frame packing information;
  if(!m_faceSizeAlignment)
  {
    printf("FaceSizeAlignment must be greater than 0, it is reset to 8 (default value).\n");
    m_faceSizeAlignment = 8;
  }
  if((m_faceSizeAlignment &1) && numberToChromaFormat(tmpInputChromaFormat)==CHROMA_420)
  { 
    //to fix the chroma resolution and luma resolution issue for 4:2:0;
    printf("FaceSizeAlignment must be even for chroma 4:2:0 format, it is reset to %d.\n", m_faceSizeAlignment+1);
    m_faceSizeAlignment = m_faceSizeAlignment+1;
  }
  calcOutputResolution(m_sourceSVideoInfo, m_codingSVideoInfo, m_iSourceWidth, m_iSourceHeight, m_faceSizeAlignment);

  /* convert std::string to c string for compatability */
  m_pchInputFile = cfg_InputFile.empty() ? NULL : strdup(cfg_InputFile.c_str());

  /* convert std::string to c string for compatability */
  m_pchRefFile = cfg_RefFile.empty() ? NULL : strdup(cfg_RefFile.c_str());
  m_pchSphData = cfg_SphFile.empty() ? NULL : strdup(cfg_SphFile.c_str());
  m_pchVPortFile = cfg_ViewFile.empty() ? NULL : strdup(cfg_ViewFile.c_str());
  m_pchSpherePointsFile = cfg_SpherePointsFile.empty() ? NULL : strdup(cfg_SpherePointsFile.c_str());
  m_framesToBeConverted = ( m_framesToBeConverted + m_temporalSubsampleRatio - 1 ) / m_temporalSubsampleRatio;
  m_iSourceHeightOrg = m_iSourceHeight;

  /* rules for input, output and internal bitdepths as per help text */
  if (m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ] == 0)
  {
    m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ] = m_inputBitDepth      [CHANNEL_TYPE_LUMA  ];
  }
  if (m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] == 0)
  {
    m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] = m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ];
  }
  if (m_internalBitDepth   [CHANNEL_TYPE_LUMA  ] == 0)
  {
    m_internalBitDepth   [CHANNEL_TYPE_LUMA  ] = m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ];
  }
  if (m_internalBitDepth   [CHANNEL_TYPE_CHROMA] == 0)
  {
    m_internalBitDepth   [CHANNEL_TYPE_CHROMA] = m_internalBitDepth   [CHANNEL_TYPE_LUMA  ];
  }
  if (m_inputBitDepth      [CHANNEL_TYPE_CHROMA] == 0)
  {
    m_inputBitDepth      [CHANNEL_TYPE_CHROMA] = m_inputBitDepth      [CHANNEL_TYPE_LUMA  ];
  }
  if (m_outputBitDepth     [CHANNEL_TYPE_LUMA  ] == 0)
  {
    m_outputBitDepth     [CHANNEL_TYPE_LUMA  ] = m_internalBitDepth   [CHANNEL_TYPE_LUMA  ];
  }
  if (m_outputBitDepth     [CHANNEL_TYPE_CHROMA] == 0)
  {
    m_outputBitDepth     [CHANNEL_TYPE_CHROMA] = m_outputBitDepth   [CHANNEL_TYPE_LUMA];
  }
  if (m_referenceBitDepth     [CHANNEL_TYPE_LUMA  ] == 0)
  {
    m_referenceBitDepth     [CHANNEL_TYPE_LUMA  ] = m_outputBitDepth   [CHANNEL_TYPE_LUMA  ];
  }
  if (m_referenceBitDepth     [CHANNEL_TYPE_CHROMA] == 0)
  {
    m_referenceBitDepth     [CHANNEL_TYPE_CHROMA] = m_referenceBitDepth     [CHANNEL_TYPE_LUMA  ];
  }

  m_InputChromaFormatIDC = numberToChromaFormat(tmpInputChromaFormat);  
  m_OutputChromaFormatIDC      = ((tmpOutputChromaFormat == 0) ? (m_InputChromaFormatIDC) : (numberToChromaFormat(tmpOutputChromaFormat)));
  m_inputGeoParam.chromaFormat    = ((tmpInternalChromaFormat == 0) ? (m_OutputChromaFormatIDC) : (numberToChromaFormat(tmpInternalChromaFormat)));
  m_inputGeoParam.nBitDepth =  m_internalBitDepth[0];
  m_inputGeoParam.nOutputBitDepth =  m_outputBitDepth[0];
  m_sourceSVideoInfo.framePackStruct.chromaFormatIDC = m_InputChromaFormatIDC;
  m_codingSVideoInfo.framePackStruct.chromaFormatIDC = m_OutputChromaFormatIDC;
#if SVIDEO_CPPPSNR
  m_ReferenceChromaFormatIDC   = numberToChromaFormat(tmpReferenceChromaFormat);
  m_referenceSVideoInfo.framePackStruct.chromaFormatIDC = m_ReferenceChromaFormatIDC;
#endif

  if(cfg_OutputFile.empty())
    m_pchOutputFile = NULL;
  else
  {
    m_pchOutputFile = (Char*)malloc(512);
    Char *pStr = strdup(cfg_OutputFile.c_str());
    Char *pCh = strrchr(pStr, '.');
    if(pCh)
      *pCh = '\0';
    sprintf(m_pchOutputFile, "%s_%dx%dx%d_cf%d.yuv", pStr, m_iSourceWidth, m_iSourceHeight, m_outputBitDepth[0], m_OutputChromaFormatIDC);//Brave:outputfile name set
    free(pStr);
  }

  m_inputColourSpaceConvert = stringToInputColourSpaceConvert(inputColourSpaceConvert, true);
  switch (m_conformanceWindowMode)//Brave :?????
  {
  case 0:
    {
      // no conformance or padding
      m_confWinLeft = m_confWinRight = m_confWinTop = m_confWinBottom = 0;
      m_aiPad[1] = m_aiPad[0] = 0;
      break;
    }
  case 1:
    {
      // automatic padding to minimum CU size
      Int minCuSize = m_faceSizeAlignment;
      if (m_iSourceWidth % minCuSize)
      {
        m_aiPad[0] = m_confWinRight  = ((m_iSourceWidth / minCuSize) + 1) * minCuSize - m_iSourceWidth;
        m_iSourceWidth  += m_confWinRight;
      }
      if (m_iSourceHeight % minCuSize)
      {
        m_aiPad[1] = m_confWinBottom = ((m_iSourceHeight / minCuSize) + 1) * minCuSize - m_iSourceHeight;
        m_iSourceHeight += m_confWinBottom;
      }
      /*
      if (m_aiPad[0] % TComSPS::getWinUnitX(m_OutputChromaFormatIDC) != 0)
      {
        fprintf(stderr, "Error: picture width is not an integer multiple of the specified chroma subsampling\n");
        exit(EXIT_FAILURE);
      }
      if (m_aiPad[1] % TComSPS::getWinUnitY(m_OutputChromaFormatIDC) != 0)
      {
        fprintf(stderr, "Error: picture height is not an integer multiple of the specified chroma subsampling\n");
        exit(EXIT_FAILURE);
      }
      */
      break;
    }
  case 2:
    {
      //padding
      m_iSourceWidth  += m_aiPad[0];
      m_iSourceHeight += m_aiPad[1];
      m_confWinRight  = m_aiPad[0];
      m_confWinBottom = m_aiPad[1];
      break;
    }
  case 3:
    {
      // conformance
      if ((m_confWinLeft == 0) && (m_confWinRight == 0) && (m_confWinTop == 0) && (m_confWinBottom == 0))
      {
        fprintf(stderr, "Warning: Conformance window enabled, but all conformance window parameters set to zero\n");
      }
      if ((m_aiPad[1] != 0) || (m_aiPad[0]!=0))
      {
        fprintf(stderr, "Warning: Conformance window enabled, padding parameters will be ignored\n");
      }
      m_aiPad[1] = m_aiPad[0] = 0;
      break;
    }
  }

  // check validity of input parameters
  xCheckParameter();

  // print-out parameters
  xPrintParameter();

  return true;
}

Void TApp360ConvertCfg::fillSourceSVideoInfo(SVideoInfo& sVidInfo, Int inputWidth, Int inputHeight)
{
#if SVIDEO_CPPPSNR
  if(sVidInfo.geoType == SVIDEO_EQUIRECT || sVidInfo.geoType == SVIDEO_EQUALAREA || sVidInfo.geoType == SVIDEO_VIEWPORT ||  sVidInfo.geoType == SVIDEO_CRASTERSPARABOLIC)
#else
  if(sVidInfo.geoType == SVIDEO_EQUIRECT || sVidInfo.geoType == SVIDEO_EQUALAREA || sVidInfo.geoType == SVIDEO_VIEWPORT)
#endif
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
    //sVidInfo.framePackStruct.faces[0][0].width = sVidInfo.iFaceWidth;
    //sVidInfo.framePackStruct.faces[0][0].height = sVidInfo.iFaceHeight;
  }
  else if(sVidInfo.geoType == SVIDEO_CUBEMAP)
  {
    //assert(sVidInfo.framePackStruct.cols*sVidInfo.framePackStruct.rows == 6);
    sVidInfo.iNumFaces = 6;
    //maybe there are some virtual faces;
    sVidInfo.iFaceWidth = inputWidth/sVidInfo.framePackStruct.cols;
    sVidInfo.iFaceHeight = inputHeight/sVidInfo.framePackStruct.rows;
    assert(sVidInfo.iFaceWidth == sVidInfo.iFaceHeight);
  }
  else if(sVidInfo.geoType == SVIDEO_OCTAHEDRON
         || (sVidInfo.geoType == SVIDEO_ICOSAHEDRON)
    )
  {
    //assert(sVidInfo.framePackStruct.cols*sVidInfo.framePackStruct.rows == 8);
    sVidInfo.iNumFaces = (sVidInfo.geoType == SVIDEO_OCTAHEDRON)? 8 : 20;
    //maybe there are some virtual faces
    if(sVidInfo.geoType == SVIDEO_OCTAHEDRON && sVidInfo.iCompactFPStructure)
    {
      sVidInfo.iFaceWidth = (inputWidth/sVidInfo.framePackStruct.cols) - 4;
      sVidInfo.iFaceHeight = (inputHeight<<1)/sVidInfo.framePackStruct.rows;
    }
    else if(sVidInfo.geoType == SVIDEO_ICOSAHEDRON && sVidInfo.iCompactFPStructure)
    {
      Int halfCol = (sVidInfo.framePackStruct.cols>>1);
#if SVIDEO_SEC_ISP
      sVidInfo.iFaceWidth = (2*inputWidth - 16*halfCol - 8 )/(2*halfCol + 1);
      sVidInfo.iFaceHeight = (inputHeight - 96)/sVidInfo.framePackStruct.rows;
#else
      sVidInfo.iFaceWidth = (2*inputWidth - 8*halfCol - 4 )/(2*halfCol + 1);
      sVidInfo.iFaceHeight = inputHeight/sVidInfo.framePackStruct.rows;
#endif
    }
    else
    {
      sVidInfo.iFaceWidth = inputWidth/sVidInfo.framePackStruct.cols;
      sVidInfo.iFaceHeight = inputHeight/sVidInfo.framePackStruct.rows;
    }
    //assert(sVidInfo.iFaceWidth == sVidInfo.iFaceHeight);
  }
  else
    assert(!"Not supported yet");
}

Void TApp360ConvertCfg::calcOutputResolution(SVideoInfo& sourceSVideoInfo, SVideoInfo& codingSVideoInfo, Int& iOutputWidth, Int& iOutputHeight, Int minCuSize)
{
  //calulate the coding resolution;
  if(sourceSVideoInfo.geoType == SVIDEO_EQUIRECT || sourceSVideoInfo.geoType == SVIDEO_EQUALAREA || sourceSVideoInfo.geoType == SVIDEO_VIEWPORT)
  {
    if(  codingSVideoInfo.geoType == SVIDEO_CUBEMAP 
      || codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON
      || codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON
      || codingSVideoInfo.geoType == SVIDEO_VIEWPORT
      )
    {
      if(codingSVideoInfo.geoType == SVIDEO_CUBEMAP) 
        codingSVideoInfo.iNumFaces = 6;
      else if(codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON)
        codingSVideoInfo.iNumFaces = 8;
      else if(codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
        codingSVideoInfo.iNumFaces = 20;
      else if(codingSVideoInfo.geoType == SVIDEO_VIEWPORT)
        codingSVideoInfo.iNumFaces = 1;

      if(m_iCodingFaceWidth==0 || m_iCodingFaceHeight==0)
      {
        Int tmp;
        if(codingSVideoInfo.geoType == SVIDEO_CUBEMAP) 
        {
          tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight/codingSVideoInfo.iNumFaces));
          codingSVideoInfo.iFaceWidth = codingSVideoInfo.iFaceHeight = (tmp + (minCuSize-1))/minCuSize*minCuSize;
        }
        else if(codingSVideoInfo.geoType ==SVIDEO_OCTAHEDRON
                || codingSVideoInfo.geoType ==SVIDEO_ICOSAHEDRON
          )
        {
          tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight*4/(sqrt(3.0)*codingSVideoInfo.iNumFaces)));
          codingSVideoInfo.iFaceHeight = ((Int)(tmp*sqrt(3.0)/2.0 +0.5) + (minCuSize-1))/minCuSize*minCuSize;
          codingSVideoInfo.iFaceWidth = (tmp + (minCuSize-1))/minCuSize*minCuSize;
        }
        if((codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON || codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON) && ((codingSVideoInfo.iFaceWidth%4) != 0 || (codingSVideoInfo.iFaceHeight%4) != 0))
        {
          codingSVideoInfo.iFaceWidth = (codingSVideoInfo.iFaceWidth>>2)<<2; 
          codingSVideoInfo.iFaceHeight = (codingSVideoInfo.iFaceHeight>>2)<<2; 
        }
        iOutputWidth = codingSVideoInfo.iFaceWidth* codingSVideoInfo.framePackStruct.cols;
        iOutputHeight = codingSVideoInfo.iFaceHeight * codingSVideoInfo.framePackStruct.rows;
        if(codingSVideoInfo.iCompactFPStructure)
        {
          if(codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON)
          {
            iOutputWidth  = (codingSVideoInfo.iFaceWidth + 4) * codingSVideoInfo.framePackStruct.cols;
            iOutputHeight = (codingSVideoInfo.iFaceHeight>>1) * codingSVideoInfo.framePackStruct.rows;
          }
          else if(codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
          {
            Int halfCol = (codingSVideoInfo.framePackStruct.cols>>1);
#if SVIDEO_SEC_ISP
            iOutputWidth  = halfCol*(codingSVideoInfo.iFaceWidth + 8) + (codingSVideoInfo.iFaceWidth>>1) + 4;
            iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight + 96;
#else
      iOutputWidth  = halfCol*(codingSVideoInfo.iFaceWidth + 4) + (codingSVideoInfo.iFaceWidth>>1) + 2;
            iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight;
#endif
          }
        }
      }
      else
      {
        codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
        if((codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON || codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON) && ((codingSVideoInfo.iFaceWidth%4) != 0 || (codingSVideoInfo.iFaceHeight%4) != 0))
        {
          codingSVideoInfo.iFaceWidth = (codingSVideoInfo.iFaceWidth>>2)<<2; 
          codingSVideoInfo.iFaceHeight = (codingSVideoInfo.iFaceHeight>>2)<<2; 
        }
        iOutputWidth = codingSVideoInfo.iFaceWidth* codingSVideoInfo.framePackStruct.cols;
        iOutputHeight = codingSVideoInfo.iFaceHeight * codingSVideoInfo.framePackStruct.rows;
        if(codingSVideoInfo.iCompactFPStructure)
        {
          if(codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON)
          {
            iOutputWidth  = (codingSVideoInfo.iFaceWidth + 4) * codingSVideoInfo.framePackStruct.cols;
            iOutputHeight = (codingSVideoInfo.iFaceHeight>>1) * codingSVideoInfo.framePackStruct.rows;
          }
          else if(codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
          {
            Int halfCol = (codingSVideoInfo.framePackStruct.cols>>1);
#if SVIDEO_SEC_ISP
            iOutputWidth  = halfCol*(codingSVideoInfo.iFaceWidth + 8) + (codingSVideoInfo.iFaceWidth>>1) + 4;
            iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight + 96;
#else
      iOutputWidth  = halfCol*(codingSVideoInfo.iFaceWidth + 4) + (codingSVideoInfo.iFaceWidth>>1) + 2;
            iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight;
#endif
          }
        }
      }
    }
#if SVIDEO_CPPPSNR
    else if(codingSVideoInfo.geoType ==SVIDEO_EQUIRECT || codingSVideoInfo.geoType == SVIDEO_EQUALAREA || codingSVideoInfo.geoType == SVIDEO_CRASTERSPARABOLIC)
#else
    else if(codingSVideoInfo.geoType ==SVIDEO_EQUIRECT || codingSVideoInfo.geoType == SVIDEO_EQUALAREA)
#endif
    {

      codingSVideoInfo.iNumFaces = 1;
      if(m_iCodingFaceWidth==0 || m_iCodingFaceHeight==0)
      {
        codingSVideoInfo.iFaceWidth = (sourceSVideoInfo.iFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceHeight = (sourceSVideoInfo.iFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
      }
      else
      {
        codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
      }

      Int degree = codingSVideoInfo.framePackStruct.faces[0][0].rot;
      if(degree ==90 || degree ==270)
      {
        iOutputWidth = codingSVideoInfo.iFaceHeight;
        iOutputHeight = codingSVideoInfo.iFaceWidth;
      }
      else
      {
        iOutputWidth = codingSVideoInfo.iFaceWidth;
        iOutputHeight = codingSVideoInfo.iFaceHeight; 
      }
    }
    else
      assert(!"Not supported yet");
  }
  else if(    (sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) 
           || (sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON) 
           || (sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
           )
  { 
    if(codingSVideoInfo.geoType == SVIDEO_EQUIRECT || codingSVideoInfo.geoType == SVIDEO_EQUALAREA || codingSVideoInfo.geoType == SVIDEO_VIEWPORT)
    {
      codingSVideoInfo.iNumFaces = 1;
      if(m_iCodingFaceWidth==0 || m_iCodingFaceHeight==0)
      {
        Int tmp;
        if(sourceSVideoInfo.geoType == SVIDEO_CUBEMAP) 
          tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight*sourceSVideoInfo.iNumFaces*2.0));
        else if(sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON 
                || (sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
          )
          tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight*sourceSVideoInfo.iNumFaces));
        else
          assert(!"To be extended\n");
        codingSVideoInfo.iFaceHeight = (tmp/2 + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceWidth = codingSVideoInfo.iFaceHeight*2;
      }
      else
      {
        codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
      }
      Int degree = codingSVideoInfo.framePackStruct.faces[0][0].rot;
      if(degree ==90 || degree ==270)
      {
        iOutputWidth = codingSVideoInfo.iFaceHeight;
        iOutputHeight = codingSVideoInfo.iFaceWidth;
      }
      else
      {
        iOutputWidth = codingSVideoInfo.iFaceWidth;
        iOutputHeight = codingSVideoInfo.iFaceHeight;
      }
    }
    else if(codingSVideoInfo.geoType == SVIDEO_CUBEMAP )
    {
      codingSVideoInfo.iNumFaces = 6;
      if(m_iCodingFaceWidth==0 || m_iCodingFaceHeight==0)
      {
        if(sourceSVideoInfo.geoType == SVIDEO_CUBEMAP)
        {
          codingSVideoInfo.iFaceWidth = (sourceSVideoInfo.iFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
          codingSVideoInfo.iFaceHeight = (sourceSVideoInfo.iFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
        }
        else if(sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON
                || (sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
          )
        {
          Int tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight*sourceSVideoInfo.iNumFaces*0.5/codingSVideoInfo.iNumFaces));
          codingSVideoInfo.iFaceHeight = codingSVideoInfo.iFaceWidth = (tmp + (minCuSize-1))/minCuSize*minCuSize;
        }
        else
          assert(!"To be extended\n");
      }
      else
      {
        codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
      }
      iOutputWidth = codingSVideoInfo.iFaceWidth*codingSVideoInfo.framePackStruct.cols;
      iOutputHeight = codingSVideoInfo.iFaceHeight*codingSVideoInfo.framePackStruct.rows;
    }
    else if( codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON
           || (codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
      )
    {
      codingSVideoInfo.iNumFaces = (codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON)? 8 : 20;
      if(m_iCodingFaceWidth==0 || m_iCodingFaceHeight==0)
      {
        if(sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON
          || (sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
          )
        {
          codingSVideoInfo.iFaceWidth = (sourceSVideoInfo.iFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
          codingSVideoInfo.iFaceHeight = (sourceSVideoInfo.iFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
        }
        else if(sourceSVideoInfo.geoType == SVIDEO_CUBEMAP)
        {
          Int tmp = Int(sqrt((Double)sourceSVideoInfo.iFaceWidth * sourceSVideoInfo.iFaceHeight*sourceSVideoInfo.iNumFaces*4/(sqrt(3.0)*codingSVideoInfo.iNumFaces)));
          codingSVideoInfo.iFaceHeight = ((Int)(tmp*sqrt(3.0)/2.0 +0.5) + (minCuSize-1))/minCuSize*minCuSize;
          codingSVideoInfo.iFaceWidth = (tmp + (minCuSize-1))/minCuSize*minCuSize;
        }
        else
          assert(!"To be extended\n");
      }
      else
      {
        codingSVideoInfo.iFaceWidth = (m_iCodingFaceWidth + (minCuSize-1))/minCuSize*minCuSize;
        codingSVideoInfo.iFaceHeight = (m_iCodingFaceHeight + (minCuSize-1))/minCuSize*minCuSize;
      }
      if((codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON || codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON) && ((codingSVideoInfo.iFaceWidth%4) != 0 || (codingSVideoInfo.iFaceHeight%4) != 0))
      {
        codingSVideoInfo.iFaceWidth = (codingSVideoInfo.iFaceWidth>>2)<<2; 
        codingSVideoInfo.iFaceHeight = (codingSVideoInfo.iFaceHeight>>2)<<2; 
      }
      iOutputWidth = codingSVideoInfo.iFaceWidth*codingSVideoInfo.framePackStruct.cols;
      iOutputHeight = codingSVideoInfo.iFaceHeight*codingSVideoInfo.framePackStruct.rows;
      if(codingSVideoInfo.iCompactFPStructure)
      {
        if(codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON)
        {
          iOutputWidth  = (codingSVideoInfo.iFaceWidth + 4)*codingSVideoInfo.framePackStruct.cols;
          iOutputHeight = (codingSVideoInfo.iFaceHeight>>1)*codingSVideoInfo.framePackStruct.rows;
        }
        else if(codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON)
        {
          Int halfCol = (codingSVideoInfo.framePackStruct.cols>>1);
          iOutputWidth  = halfCol*(codingSVideoInfo.iFaceWidth + 4) + (codingSVideoInfo.iFaceWidth>>1) + 2;
          iOutputHeight = codingSVideoInfo.framePackStruct.rows*codingSVideoInfo.iFaceHeight;
        }
      } 
    }
    else
      assert(!"Not supported yet");
  }
}

// ====================================================================================================================
// Private member functions
// ====================================================================================================================

Void TApp360ConvertCfg::xCheckParameter()
{

  Bool check_failed = false; /* abort if there is a fatal configuration problem */
#define xConfirmPara(a,b) check_failed |= confirmPara(a,b)
  //const UInt maxBitDepth=(m_OutputChromaFormatIDC==CHROMA_400) ? m_internalBitDepth[CHANNEL_TYPE_LUMA] : std::max(m_internalBitDepth[CHANNEL_TYPE_LUMA], m_internalBitDepth[CHANNEL_TYPE_CHROMA]);
  // check range of parameters
  xConfirmPara( m_inputBitDepth[CHANNEL_TYPE_LUMA  ] < 8,                                   "InputBitDepth must be at least 8" );
  xConfirmPara( m_inputBitDepth[CHANNEL_TYPE_CHROMA] < 8,                                   "InputBitDepthC must be at least 8" );

  xConfirmPara( (m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA  ] < m_inputBitDepth[CHANNEL_TYPE_LUMA  ]), "MSB-extended bit depth for luma channel (--MSBExtendedBitDepth) must be greater than or equal to input bit depth for luma channel (--InputBitDepth)" );
  xConfirmPara( (m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] < m_inputBitDepth[CHANNEL_TYPE_CHROMA]), "MSB-extended bit depth for chroma channel (--MSBExtendedBitDepthC) must be greater than or equal to input bit depth for chroma channel (--InputBitDepthC)" );
  xConfirmPara( (m_outputBitDepth[CHANNEL_TYPE_LUMA  ] > m_internalBitDepth[CHANNEL_TYPE_LUMA  ]), "Output bitdepth (--OutputBitDepth) must be less than or equal to internal bit depth (--InternalBitDepth)" );
  std::string sTempIPCSC="InputColourSpaceConvert must be empty, "+getListOfColourSpaceConverts(true);
  xConfirmPara( m_inputColourSpaceConvert >= NUMBER_INPUT_COLOUR_SPACE_CONVERSIONS,         sTempIPCSC.c_str() );
  xConfirmPara( m_InputChromaFormatIDC >= NUM_CHROMA_FORMAT,                                "InputChromaFormatIDC must be either 400, 420, 422 or 444" );
  xConfirmPara( m_inputGeoParam.chromaFormat >= NUM_CHROMA_FORMAT,                          "InternalChromaFormatIDC must be either 400, 420, 422 or 444" );
  xConfirmPara( m_OutputChromaFormatIDC >= NUM_CHROMA_FORMAT,                               "OutputChromaFormatIDC must be either 400, 420, 422 or 444" );
  if(m_OutputChromaFormatIDC == CHROMA_444 && m_inputGeoParam.chromaFormat != CHROMA_444)
  {
    printf("InternalChromaFormat is changed to 444 for better conversion quality because the output is 444!\n");
    m_inputGeoParam.chromaFormat = CHROMA_444;
  }
  if(m_InputChromaFormatIDC == CHROMA_444 && m_inputGeoParam.chromaFormat != CHROMA_444)
  {
    printf("InternalChromaFormat is changed to 444 because the input is 444!\n");
    m_inputGeoParam.chromaFormat = CHROMA_444;
  }
  if(m_OutputChromaFormatIDC == CHROMA_400 && m_inputGeoParam.chromaFormat != CHROMA_400)
  {
    printf("InternalChromaFormat is changed to 400 because the output is 400!\n");
    m_inputGeoParam.chromaFormat = CHROMA_400;
  }
  if(m_InputChromaFormatIDC == CHROMA_400 && m_inputGeoParam.chromaFormat != CHROMA_400)
  {
    printf("InternalChromaFormat is changed to 400 because the input is 400!\n");
    m_inputGeoParam.chromaFormat = CHROMA_400;
  }
  if(m_inputGeoParam.chromaFormat != CHROMA_420 && m_inputGeoParam.bResampleChroma)
  {
    printf("ResampleChroma is reset to false because the internal chroma format is not 4:2:0!\n");
    m_inputGeoParam.bResampleChroma = false;
  }
  if(m_inputGeoParam.iChromaSampleLocType < 0 || m_inputGeoParam.iChromaSampleLocType > 3)
  {
    printf("ChromaSampleLocType must be in the range of [0, 3], and it is reset to 2!\n");
    m_inputGeoParam.iChromaSampleLocType = 2;
  }
  xConfirmPara( m_iFrameRate <= 0,                                                          "Frame rate must be more than 1" );
  xConfirmPara( m_framesToBeConverted <= 0,                                                   "Total Number Of Frames encoded must be more than 0" );
  xConfirmPara( m_temporalSubsampleRatio < 1,                                               "Temporal subsample rate must be no less than 1" );
  xConfirmPara( m_faceSizeAlignment <= 0,                                                   "m_faceSizeAlignment must be greater than zero");
  /*
  xConfirmPara( m_iSourceWidth  % TComSPS::getWinUnitX(m_OutputChromaFormatIDC) != 0, "Picture width must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_iSourceHeight % TComSPS::getWinUnitY(m_OutputChromaFormatIDC) != 0, "Picture height must be an integer multiple of the specified chroma subsampling");

  xConfirmPara( m_aiPad[0] % TComSPS::getWinUnitX(m_OutputChromaFormatIDC) != 0, "Horizontal padding must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_aiPad[1] % TComSPS::getWinUnitY(m_OutputChromaFormatIDC) != 0, "Vertical padding must be an integer multiple of the specified chroma subsampling");
  
  xConfirmPara( m_confWinLeft   % TComSPS::getWinUnitX(m_OutputChromaFormatIDC) != 0, "Left conformance window offset must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_confWinRight  % TComSPS::getWinUnitX(m_OutputChromaFormatIDC) != 0, "Right conformance window offset must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_confWinTop    % TComSPS::getWinUnitY(m_OutputChromaFormatIDC) != 0, "Top conformance window offset must be an integer multiple of the specified chroma subsampling");
  xConfirmPara( m_confWinBottom % TComSPS::getWinUnitY(m_OutputChromaFormatIDC) != 0, "Bottom conformance window offset must be an integer multiple of the specified chroma subsampling");
  */
  xConfirmPara(m_faceSizeAlignment<=0, "FaceSizeAlignment must be greater than 0");
  //check source;
  if(m_sourceSVideoInfo.geoType == SVIDEO_EQUIRECT || m_sourceSVideoInfo.geoType == SVIDEO_EQUALAREA)
  {
    xConfirmPara(m_sourceSVideoInfo.framePackStruct.cols != 1 || m_sourceSVideoInfo.framePackStruct.rows != 1, "source: cols and rows of frame packing must be 1 for equirectangular and equalarea");
    for(Int j=0; j<m_sourceSVideoInfo.framePackStruct.rows; j++)
      for(Int i=0; i<m_sourceSVideoInfo.framePackStruct.cols; i++)
      {
        xConfirmPara(m_sourceSVideoInfo.framePackStruct.faces[j][i].id != 0, "source: face id of frame packing must be 0 for equirectangular and equalarea");
        xConfirmPara((m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 0) &&(m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 90)&&(m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 180)&& (m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 270)
          , "source: face rotation of frame packing must be {0, 90, 180, 270}");
      }
  }
  if(m_sourceSVideoInfo.geoType == SVIDEO_CUBEMAP)
  {
    xConfirmPara(m_sourceSVideoInfo.framePackStruct.cols*m_sourceSVideoInfo.framePackStruct.rows < 6, "source: cols x rows of frame packing must be no smaller than 6 for cubemap");
    Int iActualFaces = 0;
    for(Int j=0; j<m_sourceSVideoInfo.framePackStruct.rows; j++)
      for(Int i=0; i<m_sourceSVideoInfo.framePackStruct.cols; i++)
      {
        //xConfirmPara(m_sourceSVideoInfo.framePackStruct.faces[j][i].id < 0 || m_sourceSVideoInfo.framePackStruct.faces[j][i].id > 5, "source: face id of frame packing must be in [0,5] for cubemap");
        iActualFaces += (m_sourceSVideoInfo.framePackStruct.faces[j][i].id <6)? 1: 0;
        xConfirmPara((m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 0) &&(m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 90)&&(m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 180)&& (m_sourceSVideoInfo.framePackStruct.faces[j][i].rot != 270)
          , "source: face rotation of frame packing must be {0, 90, 180, 270}");
      }
      xConfirmPara(iActualFaces <6, "number of actual faces source must be no smaller than 6");
  }
  if(m_sourceSVideoInfo.iCompactFPStructure && !(m_sourceSVideoInfo.geoType == SVIDEO_OCTAHEDRON || m_sourceSVideoInfo.geoType == SVIDEO_ICOSAHEDRON))
  {
    printf("CompactCodingFPFormat is automatically disabled for source video because it is only supported for OHP and ISP\n");
    m_sourceSVideoInfo.iCompactFPStructure = 0;
  }
  //check coding;
  if(m_codingSVideoInfo.geoType == SVIDEO_EQUIRECT || m_codingSVideoInfo.geoType == SVIDEO_EQUALAREA)
  {
    xConfirmPara(m_codingSVideoInfo.framePackStruct.cols != 1 || m_codingSVideoInfo.framePackStruct.rows != 1, "coding: cols and rows of frame packing must be 1 for equirectangular and equalarea");
    for(Int j=0; j<m_codingSVideoInfo.framePackStruct.rows; j++)
      for(Int i=0; i<m_codingSVideoInfo.framePackStruct.cols; i++)
      {
        xConfirmPara(m_codingSVideoInfo.framePackStruct.faces[j][i].id != 0, "coding: face id of frame packing must be 0 for equirectangular and equalarea");
        xConfirmPara((m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 0) &&(m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 90) &&(m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 180) && (m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 270)
          , "coding: face rotation of frame packing must be {0, 90, 180, 270}");
      }
  }
  if(m_codingSVideoInfo.geoType == SVIDEO_CUBEMAP)
  {
    //xConfirmPara(m_codingSVideoInfo.framePackStruct.cols*m_codingSVideoInfo.framePackStruct.rows != 6, "coding: cols x rows of frame packing must be 6 for cubemap");
    Int iTotalNumFaces = m_codingSVideoInfo.framePackStruct.cols*m_codingSVideoInfo.framePackStruct.rows; //including virtual faces; 
    for(Int j=0; j<m_codingSVideoInfo.framePackStruct.rows; j++)
      for(Int i=0; i<m_codingSVideoInfo.framePackStruct.cols; i++)
      {
        Char msg[256];
        sprintf(msg, "coding: face id of frame packing must be in [0, %d] for cubemap", iTotalNumFaces);
        xConfirmPara(m_codingSVideoInfo.framePackStruct.faces[j][i].id < 0 || m_codingSVideoInfo.framePackStruct.faces[j][i].id > iTotalNumFaces, msg);
        xConfirmPara((m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 0) &&(m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 90)&&(m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 180)&& (m_codingSVideoInfo.framePackStruct.faces[j][i].rot != 270)
          , "coding: face rotation of frame packing must be {0, 90, 180, 270}");
      }
  }
  if(m_codingSVideoInfo.iCompactFPStructure && !(m_codingSVideoInfo.geoType == SVIDEO_OCTAHEDRON || m_codingSVideoInfo.geoType == SVIDEO_ICOSAHEDRON))
  {
    printf("CompactCodingFPFormat is automatically disabled for output video because it is only supported for OHP and ISP\n");
    m_codingSVideoInfo.iCompactFPStructure = 0;
  }

  if(m_codingSVideoInfo.geoType != SVIDEO_VIEWPORT)
  {
    if(m_pchVPortFile) free(m_pchVPortFile);
    m_pchVPortFile = NULL;
  }
  else if(m_pchVPortFile)
  {
    FILE *fViewPort = fopen(m_pchVPortFile,"r");
    if(!fViewPort)
    {
      printf("Open ViewPortFile:%s failed, reset ViewPortFile to NULL\n", m_pchVPortFile);
      free(m_pchVPortFile);
      m_pchVPortFile = NULL;
    }
    else
      fclose(fViewPort);
  }
  for(Int i=0; i<3; i++)
  {
    Char msgBuf[256];
    sprintf(msgBuf, "coding: SVideoRotation for dim(%d) must be in [0, 360)", i);
    xConfirmPara(m_codingSVideoInfo.sVideoRotation.degree[i] <0 || m_codingSVideoInfo.sVideoRotation.degree[i] >=360, msgBuf);
  }

  if(m_inputGeoParam.iInterp[CHANNEL_TYPE_LUMA] <=0 || m_inputGeoParam.iInterp[CHANNEL_TYPE_LUMA] >=SI_TYPE_NUM)
  {
    printf("The interpolation method is not valid for luma, and it is set to default value: Lanczos3\n");
    m_inputGeoParam.iInterp[CHANNEL_TYPE_LUMA] = SI_LANCZOS3;
  }
  if(m_inputGeoParam.iInterp[CHANNEL_TYPE_CHROMA] <=0 || m_inputGeoParam.iInterp[CHANNEL_TYPE_CHROMA] >=SI_TYPE_NUM)
  {
    printf("The interpolation method is not valid for chroma, and it is set to default value: Lanczos2\n");
    m_inputGeoParam.iInterp[CHANNEL_TYPE_CHROMA] = SI_LANCZOS2;
  }
  if(!m_pchRefFile)
  {
    for(Int i=0; i<METRIC_NUM; i++)
      m_psnrEnabled[i]=false;
  }
  if( m_codingSVideoInfo.geoType==SVIDEO_VIEWPORT )
  {
#if SVIDEO_SPSNR_NN
    if(m_psnrEnabled[METRIC_SPSNR_NN])
    {
      m_psnrEnabled[METRIC_SPSNR_NN]=false;
      printf("S-PSNR-NN is not implemented for this geometry(%d)\n", m_codingSVideoInfo.geoType);
    }
#endif
#if SVIDEO_WSPSNR
    if(m_psnrEnabled[METRIC_WSPSNR])
    {
      m_psnrEnabled[METRIC_WSPSNR]=false;
      printf("WS-PSNR is not implemented for this geometry(%d)\n", m_codingSVideoInfo.geoType);
    }
#endif
#if SVIDEO_SPSNR_I
    if(m_psnrEnabled[METRIC_SPSNR_I])
    {
      m_psnrEnabled[METRIC_SPSNR_I]=false;
      printf("S-PSNR-I is not implemented for this geometry(%d)\n", m_codingSVideoInfo.geoType);
    }
#endif
#if SVIDEO_CPPPSNR
    if(m_psnrEnabled[METRIC_CPPPSNR])
    {
      m_psnrEnabled[METRIC_CPPPSNR]=false;
      printf("CPP PSNR is not implemented for this geometry(%d)\n", m_codingSVideoInfo.geoType);
    }
#endif
  }
#if SVIDEO_SPSNR_NN || SVIDEO_SPSNR_I
#if SVIDEO_SPSNR_NN && SVIDEO_SPSNR_I
  if( m_psnrEnabled[METRIC_SPSNR_NN] || m_psnrEnabled[METRIC_SPSNR_I])
#elif SVIDEO_SPSNR_NN && !SVIDEO_SPSNR_I 
  if( m_psnrEnabled[METRIC_SPSNR_NN])
#else
  if( m_psnrEnabled[METRIC_SPSNR_I])
#endif
  {
    xConfirmPara(!(m_pchSphData), "SphFile has to be specified\n");
    FILE *fp = fopen(m_pchSphData, "r");
    if(!fp)
      m_psnrEnabled[METRIC_SPSNR_NN] = false;
    else
      fclose(fp);
  }
#endif
  if(isGeoConvertSkipped() && m_pchOutputFile)
  {
    printf("output is same as input \n");
    free(m_pchOutputFile);
    m_pchOutputFile=NULL;
  }

#undef xConfirmPara
  if (check_failed)
  {
    exit(EXIT_FAILURE);
  }
}

Void TApp360ConvertCfg::xPrintParameter()
{
  printf("\n");
  printf("Input          File                    : %s\n", m_pchInputFile          );
  printf("Output         File                    : %s\n", m_pchOutputFile         );
  printf("Reference      File                    : %s\n", m_pchRefFile? m_pchRefFile : "NULL");
  printf("SphFile        File                    : %s\n", m_pchSphData? m_pchSphData : "NULL");
  printf("ViewPortFile   File                    : %s\n", m_pchVPortFile? m_pchVPortFile : "NULL");
  printf("SpherePointsFile File                  : %s\n", m_pchSpherePointsFile? m_pchSpherePointsFile : "NULL");
  printf("Real     Format                        : %dx%d %gHz\n", m_iSourceWidth - m_confWinLeft - m_confWinRight, m_iSourceHeight - m_confWinTop - m_confWinBottom, (Double)m_iFrameRate/m_temporalSubsampleRatio );
  printf("Internal Format                        : %dx%d %gHz\n", m_iSourceWidth, m_iSourceHeight, (Double)m_iFrameRate/m_temporalSubsampleRatio );
  printf("Frame index                            : %u - %d (%d frames)\n", m_FrameSkip, m_FrameSkip+m_framesToBeConverted-1, m_framesToBeConverted );

  printf("Input bit depth                        : (Y:%d, C:%d)\n", m_inputBitDepth[CHANNEL_TYPE_LUMA], m_inputBitDepth[CHANNEL_TYPE_CHROMA] );
  //printf("MSB-extended bit depth                 : (Y:%d, C:%d)\n", m_MSBExtendedBitDepth[CHANNEL_TYPE_LUMA], m_MSBExtendedBitDepth[CHANNEL_TYPE_CHROMA] );
  printf("Internal bit depth                     : (Y:%d, C:%d)\n", m_internalBitDepth[CHANNEL_TYPE_LUMA], m_internalBitDepth[CHANNEL_TYPE_CHROMA] );
  printf("Output bit depth                       : (Y:%d, C:%d)\n", m_outputBitDepth[CHANNEL_TYPE_LUMA], m_outputBitDepth[CHANNEL_TYPE_CHROMA] );
  printf("Reference bit depth                    : (Y:%d, C:%d)\n", m_referenceBitDepth[CHANNEL_TYPE_LUMA], m_referenceBitDepth[CHANNEL_TYPE_CHROMA] );
  printf("\n");

  printf("-----360 Video Parameters----\n");
  printf("InputGeometryType: ");
  printGeoTypeName(m_sourceSVideoInfo.geoType, m_sourceSVideoInfo.iCompactFPStructure);
  printf("\nChromaFormat:%d Resolution:%dx%dxF%d FPStructure:%dx%d | ", m_sourceSVideoInfo.framePackStruct.chromaFormatIDC, m_sourceSVideoInfo.iFaceWidth, m_sourceSVideoInfo.iFaceHeight, m_sourceSVideoInfo.iNumFaces,
    m_sourceSVideoInfo.framePackStruct.cols, m_sourceSVideoInfo.framePackStruct.rows);
  for(Int i=0; i<m_sourceSVideoInfo.framePackStruct.rows; i++)
  {
    for(Int j=0; j<m_sourceSVideoInfo.framePackStruct.cols; j++)
      printf("Id_%d(R_%d) ", m_sourceSVideoInfo.framePackStruct.faces[i][j].id,m_sourceSVideoInfo.framePackStruct.faces[i][j].rot);
    printf(" | ");
  }
  printf("\n\nCodingGeometryType: ");
  printGeoTypeName(m_codingSVideoInfo.geoType, m_codingSVideoInfo.iCompactFPStructure);
  printf("\nChromaFormat:%d Resolution:%dx%dxF%d FPStructure:%dx%d | ", m_codingSVideoInfo.framePackStruct.chromaFormatIDC, m_codingSVideoInfo.iFaceWidth, m_codingSVideoInfo.iFaceHeight, m_codingSVideoInfo.iNumFaces,
    m_codingSVideoInfo.framePackStruct.cols, m_codingSVideoInfo.framePackStruct.rows);
  for(Int i=0; i<m_codingSVideoInfo.framePackStruct.rows; i++)
  {
    for(Int j=0; j<m_codingSVideoInfo.framePackStruct.cols; j++)
      printf("Id_%d(R_%d) ", m_codingSVideoInfo.framePackStruct.faces[i][j].id, m_codingSVideoInfo.framePackStruct.faces[i][j].rot);
    printf(" | ");
  }
  if(m_codingSVideoInfo.geoType == SVIDEO_VIEWPORT)
    printf("\nGlobal viewport setting: %.2f %.2f %.2f %.2f", m_codingSVideoInfo.viewPort.hFOV, m_codingSVideoInfo.viewPort.vFOV, m_codingSVideoInfo.viewPort.fYaw, m_codingSVideoInfo.viewPort.fPitch);
  printf("\n\nPacked frame resolution: %dx%d (Input face resolution:%dx%d)", m_iSourceWidth, m_iSourceHeight, m_iCodingFaceWidth, m_iCodingFaceHeight);
  printf("\nInterpolation method for luma: %d, interpolation method for chroma: %d", m_inputGeoParam.iInterp[CHANNEL_TYPE_LUMA], m_inputGeoParam.iInterp[CHANNEL_TYPE_CHROMA]);
  printf("\nChromaSampleLocType: %d", m_inputGeoParam.iChromaSampleLocType);
  if(isGeoConvertSkipped())
    printf("\nGeometry conversion is skipped!");
  printf("\n\n");

  fflush(stdout);
}

Void TApp360ConvertCfg::printChromaFormat()
{
  std::cout << std::setw(43) << "Input ChromaFormatIDC = ";
  switch (m_InputChromaFormatIDC)
  {
  case CHROMA_400:  std::cout << "  4:0:0"; break;
  case CHROMA_420:  std::cout << "  4:2:0"; break;
  case CHROMA_422:  std::cout << "  4:2:2"; break;
  case CHROMA_444:  std::cout << "  4:4:4"; break;
  default:
    std::cerr << "Invalid";
    exit(EXIT_FAILURE);
  }
  std::cout << std::endl;

  std::cout << std::setw(43) << "Internal ChromaFormatIDC = ";
  switch (m_inputGeoParam.chromaFormat)
  {
  case CHROMA_400:  std::cout << "  4:0:0"; break;
  case CHROMA_420:  std::cout << "  4:2:0"; break;
  case CHROMA_422:  std::cout << "  4:2:2"; break;
  case CHROMA_444:  std::cout << "  4:4:4"; break;
  default:
    std::cerr << "Invalid";
    exit(EXIT_FAILURE);
  }
  if(m_inputGeoParam.chromaFormat == CHROMA_420)
    std::cout << " ChromaResample: " << m_inputGeoParam.bResampleChroma;
  std::cout << std::endl;

  std::cout << std::setw(43) << "Output ChromaFormatIDC = ";
  switch (m_OutputChromaFormatIDC)
  {
  case CHROMA_400:  std::cout << "  4:0:0"; break;
  case CHROMA_420:  std::cout << "  4:2:0"; break;
  case CHROMA_422:  std::cout << "  4:2:2"; break;
  case CHROMA_444:  std::cout << "  4:4:4"; break;
  default:
    std::cerr << "Invalid";
    exit(EXIT_FAILURE);
  }
  std::cout << "\n" << std::endl;
}

Void TApp360ConvertCfg::printGeoTypeName(Int nType, Bool bCompactFPFormat)
{
  switch(nType) 
  {
  case SVIDEO_EQUIRECT:
    printf("Equirectangular ");
    break;
  case SVIDEO_CUBEMAP:
    printf("Cubemap ");
    break;
  case SVIDEO_EQUALAREA:
    printf("Equal-area ");
    break;
  case SVIDEO_OCTAHEDRON:
    if(!bCompactFPFormat)
    {
      printf("Octahedron ");
    }
    else
    {
      printf("Compact octahedron");
    }
    break;
  case SVIDEO_VIEWPORT:
    printf("Viewport ");
    break;
  case SVIDEO_ICOSAHEDRON:
    if(!bCompactFPFormat)
    {
      printf("Icosahedron ");
    }
    else
    {
      printf("Compact icosahedron ");
    }
    break;
  default:
    printf("Unknown ");
    break;
  }
}

Void  TApp360ConvertCfg::convert()
{
  TComPicYuv*       pcPicYuvOrg = NULL;
  Int iNextFrame=0;
  FILE *fViewPort=NULL;
  Bool bGeoConvertSkip = isGeoConvertSkipped();
  Bool bDirectFPConvert = isDirectFPConvert();
  if(bDirectFPConvert)   assert(!bGeoConvertSkip); 
  // Video I/O
  TVideoIOYuv cTVideoIOYuvInputFile, cTVideoIOYuvOutputFile, cTVideoIOYuvRefFile;

  Double  dPSNRSum[METRIC_NUM][MAX_NUM_COMPONENT];
  cTVideoIOYuvInputFile.open( m_pchInputFile,     false, m_inputBitDepth, m_MSBExtendedBitDepth, m_internalBitDepth );  // read  mode
  cTVideoIOYuvInputFile.skipFrames(m_FrameSkip, m_iInputWidth, m_iInputHeight, m_InputChromaFormatIDC);

  if(m_pchRefFile)
  {
    cTVideoIOYuvRefFile.open( m_pchRefFile,     false, m_referenceBitDepth, m_referenceBitDepth, m_referenceBitDepth );  // read  mode
    cTVideoIOYuvRefFile.skipFrames(m_FrameSkip, m_iSourceWidth, m_iSourceHeight, m_OutputChromaFormatIDC);               
  }

  if (m_pchOutputFile)
  {
    cTVideoIOYuvOutputFile.open(m_pchOutputFile, true, m_outputBitDepth, m_outputBitDepth, m_outputBitDepth);  // write mode
  }  
  printChromaFormat();

  Int   iNumConverted = 0;
  Bool  bEos = false;

  const InputColourSpaceConversion ipCSC  =  m_inputColourSpaceConvert;
  const InputColourSpaceConversion ipCSCOutput = (!m_outputInternalColourSpace) ? m_inputColourSpaceConvert : IPCOLOURSPACE_UNCHANGED;

  TComPicYuv cPicYuvTrueOrg;
  TComPicYuv *pcPicYuvReadFromFile = NULL;  ///< for file reading;
  TComPicYuv *pcPicYuvRot = NULL;  ///< adjust to frame packed video to normal sphere video;
  TGeometry  *pcInputGeomtry = NULL, *pcCodingGeomtry = NULL;
  TComPicYuv *pcPicYuvReadFromRefFile = NULL;
#if SVIDEO_SPSNR_NN
  TPSNRMetric  cPSNRCalc;
  TSPSNRMetric cSPSNRCalc;
#endif
#if SVIDEO_WSPSNR
  TWSPSNRMetric cWSPSNRCalc;
#endif
#if SVIDEO_SPSNR_I
  TSPSNRIMetric cSPSNRICalc;
#endif
#if SVIDEO_CPPPSNR
  TCPPPSNRMetric cCPPPSNRCalc;
#endif
  pcPicYuvReadFromFile = new TComPicYuv;
  pcPicYuvReadFromFile->createWithoutCUInfo ( m_iInputWidth, m_iInputHeight, m_InputChromaFormatIDC, true );
  if(m_pchRefFile)
  {
    pcPicYuvReadFromRefFile = new TComPicYuv;
#if SVIDEO_CPPPSNR
    pcPicYuvReadFromRefFile->createWithoutCUInfo ( m_iReferenceSourceWidth, m_iReferenceSourceHeight, m_ReferenceChromaFormatIDC, true );
#else
    pcPicYuvReadFromRefFile->createWithoutCUInfo ( m_iSourceWidth, m_iSourceHeight, m_OutputChromaFormatIDC, true );
#endif
#if SVIDEO_SPSNR_NN
    cSPSNRCalc.setSPSNREnabledFlag(m_psnrEnabled[METRIC_SPSNR_NN]);
    if(m_psnrEnabled[METRIC_SPSNR_NN])
    {
      cPSNRCalc.setOutputBitDepth(m_outputBitDepth);
      cPSNRCalc.setReferenceBitDepth(m_referenceBitDepth);
      cSPSNRCalc.setOutputBitDepth(m_outputBitDepth);
      cSPSNRCalc.setReferenceBitDepth(m_referenceBitDepth);
    }
#endif
#if SVIDEO_WSPSNR
    cWSPSNRCalc.setWSPSNREnabledFlag(m_psnrEnabled[METRIC_WSPSNR]);
    if(m_psnrEnabled[METRIC_WSPSNR])
    {
      cWSPSNRCalc.setOutputBitDepth(m_outputBitDepth);
      cWSPSNRCalc.setReferenceBitDepth(m_referenceBitDepth);
    }
#endif
#if SVIDEO_SPSNR_I
    cSPSNRICalc.setSPSNRIEnabledFlag(m_psnrEnabled[METRIC_SPSNR_I]);
    if(m_psnrEnabled[METRIC_SPSNR_I])
    {
      cSPSNRICalc.setOutputBitDepth(m_outputBitDepth);
      cSPSNRICalc.setReferenceBitDepth(m_referenceBitDepth);
      cSPSNRICalc.setOutputBitDepth(m_outputBitDepth);//Brave:repeat    meaningless
      cSPSNRICalc.setReferenceBitDepth(m_referenceBitDepth);//Brave:repeat    meaningless
    }
#endif
#if SVIDEO_CPPPSNR
    cCPPPSNRCalc.setCPPPSNREnabledFlag(m_psnrEnabled[METRIC_CPPPSNR]);
    if( m_psnrEnabled[METRIC_CPPPSNR])
    {
      cCPPPSNRCalc.setOutputBitDepth(m_outputBitDepth);
      cCPPPSNRCalc.setReferenceBitDepth(m_referenceBitDepth);
    }
#endif
  }

  Int iAdjustWidth = m_iInputWidth;
  Int iAdjustHeight = m_iInputHeight;
  if(m_sourceSVideoInfo.geoType == SVIDEO_EQUIRECT || m_sourceSVideoInfo.geoType == SVIDEO_EQUALAREA)
  {
    if(m_sourceSVideoInfo.framePackStruct.faces[0][0].rot == 90 || m_sourceSVideoInfo.framePackStruct.faces[0][0].rot == 270)
    {
      iAdjustWidth = m_iInputHeight;
      iAdjustHeight = m_iInputWidth;
    }
    if(m_sourceSVideoInfo.framePackStruct.faces[0][0].rot)
    {
      pcPicYuvRot = new TComPicYuv;
      pcPicYuvRot->createWithoutCUInfo( iAdjustWidth, iAdjustHeight, m_InputChromaFormatIDC, true );
    }
  }

  pcInputGeomtry = TGeometry::create(m_sourceSVideoInfo, &m_inputGeoParam); 
  pcCodingGeomtry = TGeometry::create(m_codingSVideoInfo, &m_inputGeoParam);
#if SVIDEO_CPPPSNR
  //pcReferenceGeometry = TGeometry::create(m_referenceSVideoInfo, &m_inputGeoParam);
#endif

  // allocate original YUV buffer
  if(!bGeoConvertSkip)
  {
    pcPicYuvOrg = new TComPicYuv;
    pcPicYuvOrg->createWithoutCUInfo  ( m_iSourceWidth, m_iSourceHeight, m_OutputChromaFormatIDC, true );
    cPicYuvTrueOrg.createWithoutCUInfo(m_iSourceWidth, m_iSourceHeight, m_OutputChromaFormatIDC, true );
  }

  //init metric;
  memset(dPSNRSum[0], 0, sizeof(dPSNRSum));
#if SVIDEO_SPSNR_NN
  if( m_psnrEnabled[METRIC_SPSNR_NN])
  {
    cSPSNRCalc.sphSampoints(m_pchSphData);
    cSPSNRCalc.createTable(pcPicYuvReadFromRefFile, pcCodingGeomtry);
  }
#endif
#if SVIDEO_WSPSNR
  if( m_psnrEnabled[METRIC_WSPSNR])
  {
    cWSPSNRCalc.setCodingGeoInfo(*pcCodingGeomtry->getSVideoInfo(), m_inputGeoParam.iChromaSampleLocType);
    cWSPSNRCalc.createTable(pcPicYuvReadFromRefFile, pcCodingGeomtry);
  }
#endif
#if SVIDEO_SPSNR_I
  if( m_psnrEnabled[METRIC_SPSNR_I])
  {
    cSPSNRICalc.init(m_inputGeoParam, m_codingSVideoInfo, m_referenceSVideoInfo, m_iSourceWidth, m_iSourceHeight, m_iReferenceSourceWidth, m_iReferenceSourceHeight);
    cSPSNRICalc.sphSampoints(m_pchSphData);//Brave:read data from sphere_655362.txt
    cSPSNRICalc.createTable(pcPicYuvReadFromRefFile, pcCodingGeomtry);
  }
#endif
#if SVIDEO_CPPPSNR
  if( m_psnrEnabled[METRIC_CPPPSNR])
  {
    cCPPPSNRCalc.initCPPPSNR(m_inputGeoParam, m_cppPsnrWidth, m_cppPsnrHeight, m_codingSVideoInfo, m_referenceSVideoInfo);
  }
#endif
  //dump all points on the sphere;
  if(m_pchSpherePointsFile)
  {
    printf("Generate the sphere sampling points for the projection format(%d) and store it in file %s.\n", m_codingSVideoInfo.geoType, m_pchSpherePointsFile);
    pcCodingGeomtry->dumpSpherePoints(m_pchSpherePointsFile);
  }
  printf("Frame#  ");
  for(Int i=0; i<METRIC_NUM; i++)
  {
    if (m_psnrEnabled[i])
      printf("%s_Y       %s_U       %s_V     | ", m_sPSNRName[i], m_sPSNRName[i], m_sPSNRName[i]);
  }
  printf("\n");

  if(m_pchVPortFile)
  {
    fViewPort = fopen(m_pchVPortFile,"r");
    assert(fViewPort);
    if( fscanf(fViewPort, "%d ", &iNextFrame) != 1 )
    {
      iNextFrame = m_framesToBeConverted+1;
      fclose(fViewPort);
      fViewPort = NULL;
    }
  }

  // starting time
  Double dResult;
  clock_t lBefore = clock();

  while ( !bEos && m_framesToBeConverted)
  {
    // read input YUV file
    Int aiPad[2]={0,0};
    cTVideoIOYuvInputFile.read(NULL, pcPicYuvReadFromFile, IPCOLOURSPACE_UNCHANGED, aiPad, m_InputChromaFormatIDC, m_bClipInputVideoToRec709Range );
    if (cTVideoIOYuvInputFile.isEof())
      break;

    if(!bGeoConvertSkip)
    {
      if(pcPicYuvRot)
      {
        pcPicYuvReadFromFile->rot(pcPicYuvRot, (360-m_sourceSVideoInfo.framePackStruct.faces[0][0].rot)%360);
        pcInputGeomtry->convertYuv(pcPicYuvRot);
      }
      else
      {
        if((pcInputGeomtry->getSVideoInfo()->geoType == SVIDEO_OCTAHEDRON || pcInputGeomtry->getSVideoInfo()->geoType == SVIDEO_ICOSAHEDRON) && pcInputGeomtry->getSVideoInfo()->iCompactFPStructure) 
        {
          pcInputGeomtry->compactFramePackConvertYuv(pcPicYuvReadFromFile);
        }
        else
        {
          pcInputGeomtry->convertYuv(pcPicYuvReadFromFile);//***m_pFacesOrig//[face][component][raster scan position]
        }
      }  

      if(fViewPort)
      {
        if (iNextFrame==iNumConverted)
        { 
          Float fovx,fovy,yaw,pitch;
          if(fscanf(fViewPort, "%f %f %f %f ", &fovx,&fovy,&yaw,&pitch) == 4)
          {
            ((TViewPort*)pcCodingGeomtry)->setViewPort(fovx,fovy,yaw,pitch);
            if(fscanf(fViewPort, "%d ", &iNextFrame) != 1)
              iNextFrame = m_framesToBeConverted+1;
          }
          else
          {
            printf("Frame:%d, format error for viewport settings. The viewport will not be changed any more!\n", iNumConverted);
            iNextFrame = m_framesToBeConverted+1;
          }
        }
      }

      if(!bDirectFPConvert)
      {
        pcInputGeomtry->geoConvert(pcCodingGeomtry);
      }
      else
      {
        pcInputGeomtry->setPaddingFlag(true);
      }
      if((pcCodingGeomtry->getSVideoInfo()->geoType == SVIDEO_OCTAHEDRON || pcCodingGeomtry->getSVideoInfo()->geoType == SVIDEO_ICOSAHEDRON) && pcCodingGeomtry->getSVideoInfo()->iCompactFPStructure)
      {
        if(!bDirectFPConvert)
        {
          pcCodingGeomtry->compactFramePack(&cPicYuvTrueOrg);
        }
        else
        {
          pcInputGeomtry->compactFramePack(&cPicYuvTrueOrg);
        }
      }
      else
      {
        if(!bDirectFPConvert)
        {
          pcCodingGeomtry->framePack(&cPicYuvTrueOrg);
        }
        else
        {
          pcInputGeomtry->framePack(&cPicYuvTrueOrg);
        }
      }
      cTVideoIOYuvInputFile.ColourSpaceConvert(cPicYuvTrueOrg, *pcPicYuvOrg, ipCSC, true);
    }
    else
      pcPicYuvOrg = pcPicYuvReadFromFile;

    // increase number of received frames
    printf("\nFrame:%d ", iNumConverted);
    iNumConverted++;
    bEos = ( (iNumConverted == m_framesToBeConverted) );


    // write bistream to file if necessary
    if ( iNumConverted > 0 && m_pchOutputFile)
    {
      cTVideoIOYuvOutputFile.write( pcPicYuvOrg, ipCSCOutput, m_confWinLeft, m_confWinRight, m_confWinTop, m_confWinBottom, NUM_CHROMA_FORMAT, m_bClipOutputVideoToRec709Range  );
    }

    // temporally skip frames
    if( m_temporalSubsampleRatio > 1 )
    {
      cTVideoIOYuvInputFile.skipFrames(m_temporalSubsampleRatio-1, m_iInputWidth, m_iInputHeight, m_InputChromaFormatIDC);
    }
    if(m_pchRefFile) 
    {
      cTVideoIOYuvRefFile.read(NULL, pcPicYuvReadFromRefFile, IPCOLOURSPACE_UNCHANGED, aiPad, m_OutputChromaFormatIDC, m_bClipInputVideoToRec709Range );
      if (!cTVideoIOYuvRefFile.isEof())
      {
#if SVIDEO_SPSNR_NN
        if(m_psnrEnabled[METRIC_PSNR])
        {
          cPSNRCalc.xCalculatePSNR(pcPicYuvReadFromRefFile, pcPicYuvOrg);
          printf(" %6.4lf dB    %6.4lf dB    %6.4lf dB |", cPSNRCalc.getPSNR()[COMPONENT_Y], cPSNRCalc.getPSNR()[COMPONENT_Cb], cPSNRCalc.getPSNR()[COMPONENT_Cr] );
        }
        if(m_psnrEnabled[METRIC_SPSNR_NN])
        {
          cSPSNRCalc.xCalculateSPSNR(pcPicYuvReadFromRefFile, pcPicYuvOrg);
          printf(" %6.4lf dB    %6.4lf dB    %6.4lf dB |", cSPSNRCalc.getSPSNR()[COMPONENT_Y], cSPSNRCalc.getSPSNR()[COMPONENT_Cb], cSPSNRCalc.getSPSNR()[COMPONENT_Cr] );
        }
#endif
#if SVIDEO_WSPSNR
        if(m_psnrEnabled[METRIC_WSPSNR])
        {
          cWSPSNRCalc.xCalculateWSPSNR(pcPicYuvReadFromRefFile, pcPicYuvOrg);
          printf(" %6.4lf dB    %6.4lf dB    %6.4lf dB |", cWSPSNRCalc.getWSPSNR()[COMPONENT_Y], cWSPSNRCalc.getWSPSNR()[COMPONENT_Cb], cWSPSNRCalc.getWSPSNR()[COMPONENT_Cr] );
        }
#endif
#if SVIDEO_SPSNR_I
        if(m_psnrEnabled[METRIC_SPSNR_I])
        {
          cSPSNRICalc.xCalculateSPSNRI(pcPicYuvReadFromRefFile, pcPicYuvOrg);
          printf(" %6.4lf dB    %6.4lf dB    %6.4lf dB |", cSPSNRICalc.getSPSNRI()[COMPONENT_Y], cSPSNRICalc.getSPSNRI()[COMPONENT_Cb], cSPSNRICalc.getSPSNRI()[COMPONENT_Cr] );
        }
#endif
#if SVIDEO_CPPPSNR
        if(m_psnrEnabled[METRIC_CPPPSNR])
        {
          cCPPPSNRCalc.xCalculateCPPPSNR(pcPicYuvReadFromRefFile, pcPicYuvOrg);
          printf(" %6.4lf dB    %6.4lf dB    %6.4lf dB |", cCPPPSNRCalc.getCPPPSNR()[COMPONENT_Y], cCPPPSNRCalc.getCPPPSNR()[COMPONENT_Cb], cCPPPSNRCalc.getCPPPSNR()[COMPONENT_Cr] );
        }
#endif
        for( Int i = 0; i < MAX_NUM_COMPONENT; i++)
        {
#if SVIDEO_SPSNR_NN
          if(m_psnrEnabled[METRIC_PSNR])
          {
            dPSNRSum[METRIC_PSNR][i] += cPSNRCalc.getPSNR()[i];
          }
          if(m_psnrEnabled[METRIC_SPSNR_NN])
          {
            dPSNRSum[METRIC_SPSNR_NN][i] += cSPSNRCalc.getSPSNR()[i];
          }
#endif
#if SVIDEO_WSPSNR
          if(m_psnrEnabled[METRIC_WSPSNR])
          {
            dPSNRSum[METRIC_WSPSNR][i] += cWSPSNRCalc.getWSPSNR()[i];
          }
#endif
#if SVIDEO_SPSNR_I
          if(m_psnrEnabled[METRIC_SPSNR_I])
          {
            dPSNRSum[METRIC_SPSNR_I][i] += cSPSNRICalc.getSPSNRI()[i];
          }
#endif
#if SVIDEO_CPPPSNR
          if(m_psnrEnabled[METRIC_CPPPSNR])
          {
            dPSNRSum[METRIC_CPPPSNR][i] += cCPPPSNRCalc.getCPPPSNR()[i];
          }
#endif
        }
      }
    }
  }

  if(m_pchRefFile)
  {
    printf("\n\nAverage PSNRS \n\n");
    for(Int j=0; j<METRIC_NUM; j++)
    {
      if( m_psnrEnabled[j])
        printf(" %6.4lf     %6.4lf     %6.4lf  |", dPSNRSum[j][COMPONENT_Y]/iNumConverted, dPSNRSum[j][COMPONENT_Cb]/iNumConverted, dPSNRSum[j][COMPONENT_Cr]/iNumConverted );
    }
    printf("\n");
  }

  // ending time
  dResult = (Double)(clock()-lBefore) / CLOCKS_PER_SEC;
  printf("\n Total Time: %12.3f sec.\n", dResult);

  if(fViewPort)
    fclose(fViewPort);

  // Video I/O
  cTVideoIOYuvInputFile.close();
  cTVideoIOYuvOutputFile.close();
  if(m_pchRefFile)
    cTVideoIOYuvRefFile.close();

  // delete original YUV buffer
  if(!bGeoConvertSkip)
  {
    if(pcPicYuvOrg)
    {
      pcPicYuvOrg->destroy();
      delete pcPicYuvOrg;
      pcPicYuvOrg = NULL;
    }
  }
  else
    pcPicYuvOrg = NULL;

  // delete used buffers in encoder class
  cPicYuvTrueOrg.destroy();

  if(pcPicYuvReadFromFile)
  {
    pcPicYuvReadFromFile->destroy();
    delete pcPicYuvReadFromFile;
    pcPicYuvReadFromFile = NULL;
  }
  if(pcPicYuvRot)
  {
    pcPicYuvRot->destroy();
    delete pcPicYuvRot;
    pcPicYuvRot = NULL;
  }
  if(pcPicYuvReadFromRefFile)
  {
    pcPicYuvReadFromRefFile->destroy();
    delete pcPicYuvReadFromRefFile;
    pcPicYuvReadFromRefFile = NULL;
  }
  if(pcInputGeomtry)
  {
    delete pcInputGeomtry;
    pcInputGeomtry=NULL;
  }
  if(pcCodingGeomtry)
  {
    delete pcCodingGeomtry;
    pcCodingGeomtry=NULL;
  }
}

Bool confirmPara(Bool bflag, const Char* message)
{
  if (!bflag)
  {
    return false;
  }

  printf("Error: %s\n",message);
  return true;
}
//! \}
