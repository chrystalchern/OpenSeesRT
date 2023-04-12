#include <assert.h>

#include <tcl.h>
#include <BasicModelBuilder.h>
#include <element/Shell/ShellANDeS.h>

Element*
TclDispatch_newShellANDeS(ClientData clientData, Tcl_Interp* interp, int argc, TCL_Char** const argv)
{
  // assert(clientData != nullptr);
  // BasicModelBuilder* builder = (BasicModelBuilder*)clientData;

  Element *theElement = 0;


  if (argc < 6) {
    opserr << "Want: element ShellANDeS $tag $iNode $jNode $kNode $thick $E "
              "$nu $rho";
    return 0;
  }

  int numArgs = OPS_GetNumRemainingInputArgs();

  int iData[4];
  int numData = 4;
  if (OPS_GetIntInput(&numData, iData) != 0) {
    opserr << "WARNING invalid integer tag: element ShellANDeS \n";
    return 0;
  }

  double dData[11];
  numArgs = OPS_GetNumRemainingInputArgs();
  if (OPS_GetDoubleInput(&numArgs, dData) != 0) {
    opserr << "WARNING invalid double thickness: element ShellANDeS \n";
    return 0;
  }
  if (numArgs == 4) {
    theElement = new ShellANDeS(iData[0], iData[1], iData[2], iData[3],
                                dData[0], dData[1], dData[2], dData[3]);
  } else if (numArgs == 11) {
    theElement =
        new ShellANDeS(iData[0], iData[1], iData[2], iData[3], dData[0],
                       dData[1], dData[2], dData[3], dData[4], dData[5],
                       dData[6], dData[7], dData[8], dData[9], dData[10]);
  }

  return theElement;
}