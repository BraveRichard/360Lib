// InterDigital Communications: geometry projection conversion
//
#include <time.h>
#include <iostream>
#include "TApp360ConvertCfg.h"
#include "TAppCommon/program_options_lite.h"
#define SW_360VIDCONV_VERSION             "TApp360_v1.0"

int main(int argc, char* argv[])
{
  TApp360ConvertCfg  cTAppConvCfg;

  // print information
  fprintf( stdout, "\n" );
  fprintf( stdout, "Projection format conversion tool for 360 video: Version [%s]", SW_360VIDCONV_VERSION );
  fprintf( stdout, NVM_ONOS );
  fprintf( stdout, NVM_COMPILEDBY );
  fprintf( stdout, NVM_BITS );
  fprintf( stdout, "\n\n" );

  // create application encoder class
  cTAppConvCfg.create();

  // parse configuration
  try
  {
    if(!cTAppConvCfg.parseCfg( argc, argv ))
    {
      cTAppConvCfg.destroy();
      return 1;
    }
  }
  catch (df::program_options_lite::ParseFailure &e)
  {
    std::cerr << "Error parsing option \""<< e.arg <<"\" with argument \""<< e.val <<"\"." << std::endl;
    return 1;
  }

  // call encoding function
  cTAppConvCfg.convert();

  // destroy application encoder class
  cTAppConvCfg.destroy();


  return 0;
}

