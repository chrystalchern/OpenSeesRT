//
// Description: This file contains the implementation of
//    TclBasicBuilder_addBrickUP() ,
//    TclBasicBuilder_addTwentyEightNodeBrickUP(),
//    TclBasicBuilder_addBBarBrickUP()
//
//
// Jinchi Lu and Zhaohui Yang (May 2004)
//
#include <stdlib.h>
#include <string.h>
#include <Domain.h>
#include <BrickUP.h>
#include <BBarBrickUP.h>
#include <Twenty_Node_Brick.h>
#include <Twenty_Eight_Node_BrickUP.h>

#include <runtime/BasicModelBuilder.h>

int
TclBasicBuilder_addTwentyNodeBrick(ClientData clientData, Tcl_Interp *interp,
                                   int argc,
                                   TCL_Char ** const argv)
{
  // ensure the destructor has not been called -
  BasicModelBuilder *builder = (BasicModelBuilder*)clientData;
  Domain* theTclDomain = builder->getDomain();

  if (builder == 0 || clientData == 0) {
    opserr << "WARNING builder has been destroyed\n";
    return TCL_ERROR;
  }
  if (builder->getNDM() != 3) {
    opserr << "WARNING -- model dimensions and/or nodal DOF not compatible "
              "with 20NodeBrick element\n";
    return TCL_ERROR;
  }
  // check the number of arguments is correct
  int argStart = 2;
  if ((argc - argStart) < 22) {
    opserr << "WARNING insufficient arguments\n";
    opserr << "Want: element 20NodeBrick eleTag? N1? N2? N3? N4? N5? N6? N7? "
              "N8? N9? N10? N11? N12? N13? N14? N15? N16? N17? N18? N19? N20? "
              "matTag? <b1? b2? b3?>\n";
    return TCL_ERROR;
  }
  // get the id and end nodes
  int brickId, Nod[20], matID;
  double b1 = 0.0;
  double b2 = 0.0;
  double b3 = 0.0;
  if (Tcl_GetInt(interp, argv[argStart], &brickId) != TCL_OK) {
    opserr << "WARNING invalid 20NodeBrick eleTag" << endln;
    return TCL_ERROR;
  }
  for (int i = 0; i < 20; i++)
    if (Tcl_GetInt(interp, argv[1 + argStart + i], &(Nod[i])) != TCL_OK) {
      opserr << "WARNING invalid Node number\n";
      opserr << "20NodeBrick element: " << brickId << endln;
      return TCL_ERROR;
    }
  if (Tcl_GetInt(interp, argv[21 + argStart], &matID) != TCL_OK) {
    opserr << "WARNING invalid matID\n";
    opserr << "20NodeBrick element: " << brickId << endln;
    return TCL_ERROR;
  }
  if ((argc - argStart) >= 23) {
    if (Tcl_GetDouble(interp, argv[22 + argStart], &b1) != TCL_OK) {
      opserr << "WARNING invalid b1\n";
      opserr << "20NodeBrick element: " << brickId << endln;
      return TCL_ERROR;
    }
  }
  if ((argc - argStart) >= 24) {
    if (Tcl_GetDouble(interp, argv[23 + argStart], &b2) != TCL_OK) {
      opserr << "WARNING invalid b2\n";
      opserr << "20NodeBrick element: " << brickId << endln;
      return TCL_ERROR;
    }
  }
  if ((argc - argStart) >= 25) {
    if (Tcl_GetDouble(interp, argv[24 + argStart], &b3) != TCL_OK) {
      opserr << "WARNING invalid b3\n";
      opserr << "20NodeBrick element: " << brickId << endln;
      return TCL_ERROR;
    }
  }
  NDMaterial *theMaterial = OPS_getNDMaterial(matID);
  if (theMaterial == 0) {
    opserr << "WARNING material not found\n";
    opserr << "Material: " << matID;
    opserr << "\20NodeBrick element: " << brickId << endln;
    return TCL_ERROR;
  }
  // now create the brick and add it to the Domain
  Twenty_Node_Brick *theTwentyNodeBrick =
      new Twenty_Node_Brick(brickId, Nod[0], Nod[1], Nod[2], Nod[3], Nod[4],
                            Nod[5], Nod[6], Nod[7],
                            Nod[8], Nod[9], Nod[10], Nod[11], Nod[12], Nod[13],
                            Nod[14],
                            Nod[15], Nod[16], Nod[17], Nod[18], Nod[19],
                            *theMaterial, b1, b2, b3);
  if (theTwentyNodeBrick == 0) {
    opserr << "WARNING ran out of memory creating element\n";
    opserr << "20NodeBrick element: " << brickId << endln;
    return TCL_ERROR;
  }
  if (theTclDomain->addElement(theTwentyNodeBrick) == false) {
    opserr << "WARNING could not add element to the domain\n";
    opserr << "20NodeBrick element: " << brickId << endln;
    delete theTwentyNodeBrick;
    return TCL_ERROR;
  }
  // if get here we have successfully created the element and added it to the
  // domain
  return TCL_OK;
}

int
TclBasicBuilder_addBrickUP(ClientData clientData, Tcl_Interp *interp, int argc,
                           TCL_Char ** const argv)
{
  // ensure the destructor has not been called -
  BasicModelBuilder *builder = (BasicModelBuilder*)clientData;
  Domain* theTclDomain = builder->getDomain();

  if (builder == 0 || clientData == 0) {
    opserr << "WARNING builder has been destroyed\n";
    return TCL_ERROR;
  }

  if (builder->getNDM() != 3 || builder->getNDF() != 4) {
    opserr << "WARNING -- model dimensions and/or nodal DOF not compatible "
              "with QuadUP element\n";
    return TCL_ERROR;
  }

  // check the number of arguments is correct
  int argStart = 2;

  if ((argc - argStart) < 15) {
    opserr << "WARNING insufficient arguments\n";
    opserr << "Want: element brickUP eleTag? N1? N2? N3? N4? N5? N6? N7? N8? "
              "matTag? bulk? rhof? perm_x? perm_y? perm_z? <b1? b2? b3?>\n";
    return TCL_ERROR;
  }

  // get the id and end nodes
  int brickUPId, Nod[8], matID;
  double bk, r, perm1, perm2, perm3;
  double b1 = 0.0;
  double b2 = 0.0;
  double b3 = 0.0;

  if (Tcl_GetInt(interp, argv[argStart], &brickUPId) != TCL_OK) {
    opserr << "WARNING invalid brickUP eleTag" << endln;
    return TCL_ERROR;
  }

  for (int i = 0; i < 8; i++)
    if (Tcl_GetInt(interp, argv[1 + argStart + i], &(Nod[i])) != TCL_OK) {
      opserr << "WARNING invalid Node number\n";
      opserr << "brickUP element: " << brickUPId << endln;
      return TCL_ERROR;
    }

  if (Tcl_GetInt(interp, argv[9 + argStart], &matID) != TCL_OK) {
    opserr << "WARNING invalid matID\n";
    opserr << "brickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[10 + argStart], &bk) != TCL_OK) {
    opserr << "WARNING invalid fluid bulk modulus\n";
    opserr << "brickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[11 + argStart], &r) != TCL_OK) {
    opserr << "WARNING invalid fluid mass density\n";
    opserr << "brickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[12 + argStart], &perm1) != TCL_OK) {
    opserr << "WARNING invalid permeability_x\n";
    opserr << "brickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[13 + argStart], &perm2) != TCL_OK) {
    opserr << "WARNING invalid permeability_y\n";
    opserr << "brickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[14 + argStart], &perm3) != TCL_OK) {
    opserr << "WARNING invalid permeability_z\n";
    opserr << "brickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if ((argc - argStart) >= 16) {
    if (Tcl_GetDouble(interp, argv[15 + argStart], &b1) != TCL_OK) {
      opserr << "WARNING invalid b1\n";
      opserr << "brickUP element: " << brickUPId << endln;
      return TCL_ERROR;
    }
  }
  if ((argc - argStart) >= 17) {
    if (Tcl_GetDouble(interp, argv[16 + argStart], &b2) != TCL_OK) {
      opserr << "WARNING invalid b2\n";
      opserr << "brickUP element: " << brickUPId << endln;
      return TCL_ERROR;
    }
  }
  if ((argc - argStart) >= 18) {
    if (Tcl_GetDouble(interp, argv[17 + argStart], &b3) != TCL_OK) {
      opserr << "WARNING invalid b3\n";
      opserr << "brickUP element: " << brickUPId << endln;
      return TCL_ERROR;
    }
  }

  NDMaterial *theMaterial = OPS_getNDMaterial(matID);

  if (theMaterial == 0) {
    opserr << "WARNING material not found\n";
    opserr << "Material: " << matID;
    opserr << "\nbrickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  // now create the brickUP and add it to the Domain
  BrickUP *theBrickUP = new BrickUP(
      brickUPId, Nod[0], Nod[1], Nod[2], Nod[3], Nod[4], Nod[5], Nod[6], Nod[7],
      *theMaterial, bk, r, perm1, perm2, perm3, b1, b2, b3);
  if (theBrickUP == 0) {
    opserr << "WARNING ran out of memory creating element\n";
    opserr << "brickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if (theTclDomain->addElement(theBrickUP) == false) {
    opserr << "WARNING could not add element to the domain\n";
    opserr << "brickUP element: " << brickUPId << endln;
    delete theBrickUP;
    return TCL_ERROR;
  }

  // if get here we have successfully created the element and added it to the
  // domain
  return TCL_OK;
}

int
TclBasicBuilder_addTwentyEightNodeBrickUP(ClientData clientData, Tcl_Interp *interp,
                                          int argc, TCL_Char ** const argv)

{
  // ensure the destructor has not been called -
  BasicModelBuilder *builder = (BasicModelBuilder*)clientData;
  Domain* theTclDomain = builder->getDomain();

  if (builder == 0 || clientData == 0) {
    opserr << "WARNING builder has been destroyed\n";
    return TCL_ERROR;
  }

  if (builder->getNDM() != 3) {
    opserr << "WARNING -- model dimensions and/or nodal DOF not compatible "
              "with 20_8_BrickUP element\n";
    return TCL_ERROR;
  }

  // check the number of arguments is correct
  int argStart = 2;

  if ((argc - argStart) < 27) {
    opserr << "WARNING insufficient arguments\n";
    opserr << "Want: element 20_8_BrickUP eleTag? N1? N2? N3? N4? N5? N6? N7? "
              "N8? N9? N10? N11? N12? N13? N14? N15? N16? N17? N18? N19? N20? "
              "matTag? bulk? rhof? perm_x? perm_y? perm_z? <b1? b2? b3?>\n";
    return TCL_ERROR;
  }

  // get the id and end nodes
  int brickUPId, Nod[20], matID;
  double bk, r, perm1, perm2, perm3;
  double b1 = 0.0;
  double b2 = 0.0;
  double b3 = 0.0;

  if (Tcl_GetInt(interp, argv[argStart], &brickUPId) != TCL_OK) {
    opserr << "WARNING invalid 20_8_BrickUP eleTag" << endln;
    return TCL_ERROR;
  }

  for (int i = 0; i < 20; i++)
    if (Tcl_GetInt(interp, argv[1 + argStart + i], &(Nod[i])) != TCL_OK) {
      opserr << "WARNING invalid Node number\n";
      opserr << "20_8_BrickUP element: " << brickUPId << endln;
      return TCL_ERROR;
    }

  if (Tcl_GetInt(interp, argv[21 + argStart], &matID) != TCL_OK) {
    opserr << "WARNING invalid matID\n";
    opserr << "20_8_BrickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[22 + argStart], &bk) != TCL_OK) {
    opserr << "WARNING invalid fluid bulk modulus\n";
    opserr << "20_8_BrickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[23 + argStart], &r) != TCL_OK) {
    opserr << "WARNING invalid fluid mass density\n";
    opserr << "20_8_BrickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[24 + argStart], &perm1) != TCL_OK) {
    opserr << "WARNING invalid permeability_x\n";
    opserr << "20_8_BrickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[25 + argStart], &perm2) != TCL_OK) {
    opserr << "WARNING invalid permeability_y\n";
    opserr << "20_8_BrickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[26 + argStart], &perm3) != TCL_OK) {
    opserr << "WARNING invalid permeability_z\n";
    opserr << "20_8_BrickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if ((argc - argStart) >= 28) {
    if (Tcl_GetDouble(interp, argv[27 + argStart], &b1) != TCL_OK) {
      opserr << "WARNING invalid b1\n";
      opserr << "20_8_BrickUP element: " << brickUPId << endln;
      return TCL_ERROR;
    }
  }
  if ((argc - argStart) >= 29) {
    if (Tcl_GetDouble(interp, argv[28 + argStart], &b2) != TCL_OK) {
      opserr << "WARNING invalid b2\n";
      opserr << "20_8_BrickUP element: " << brickUPId << endln;
      return TCL_ERROR;
    }
  }
  if ((argc - argStart) >= 30) {
    if (Tcl_GetDouble(interp, argv[29 + argStart], &b3) != TCL_OK) {
      opserr << "WARNING invalid b3\n";
      opserr << "20_8_BrickUP element: " << brickUPId << endln;
      return TCL_ERROR;
    }
  }

  NDMaterial *theMaterial = OPS_getNDMaterial(matID);

  if (theMaterial == 0) {
    opserr << "WARNING material not found\n";
    opserr << "Material: " << matID;
    opserr << "\n20_8_BrickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  // now create the brickUP and add it to the Domain
  TwentyEightNodeBrickUP *theTwentyEightNodeBrickUP =
      new TwentyEightNodeBrickUP(
          brickUPId, Nod[0], Nod[1], Nod[2], Nod[3], Nod[4], Nod[5], Nod[6],
          Nod[7], Nod[8], Nod[9], Nod[10], Nod[11], Nod[12], Nod[13], Nod[14],
          Nod[15], Nod[16], Nod[17], Nod[18], Nod[19], *theMaterial, bk, r,
          perm1, perm2, perm3, b1, b2, b3);
  if (theTwentyEightNodeBrickUP == 0) {
    opserr << "WARNING ran out of memory creating element\n";
    opserr << "20_8_BrickUP element: " << brickUPId << endln;
    return TCL_ERROR;
  }

  if (theTclDomain->addElement(theTwentyEightNodeBrickUP) == false) {
    opserr << "WARNING could not add element to the domain\n";
    opserr << "20_8_BrickUP element: " << brickUPId << endln;
    delete theTwentyEightNodeBrickUP;
    return TCL_ERROR;
  }

  // if get here we have successfully created the element and added it to the
  // domain
  return TCL_OK;
}

/*  *****************************************************************************

    BBAR  BRICK  U_P

    *****************************************************************************
 */

int
TclBasicBuilder_addBBarBrickUP(ClientData clientData, Tcl_Interp *interp, int argc,
                               TCL_Char ** const argv)
{
  // ensure the destructor has not been called -
  BasicModelBuilder *builder = (BasicModelBuilder*)clientData;
  Domain* theTclDomain = builder->getDomain();

  if (builder == 0 || clientData == 0) {
    opserr << "WARNING builder has been destroyed\n";
    return TCL_ERROR;
  }

  if (builder->getNDM() != 3 || builder->getNDF() != 4) {
    opserr << "WARNING -- model dimensions and/or nodal DOF not compatible "
              "with QuadUP element\n";
    return TCL_ERROR;
  }

  // check the number of arguments is correct
  int argStart = 2;

  if ((argc - argStart) < 15) {
    opserr << "WARNING insufficient arguments\n";
    opserr << "Want: element BBarBrickUP eleTag? N1? N2? N3? N4? N5? N6? N7? "
              "N8? matTag? bulk? rhof? perm_x? perm_y? perm_z? <b1? b2? b3?>\n";
    return TCL_ERROR;
  }

  // get the id and end nodes
  int BBarBrickUPId, Nod[8], matID;
  double bk, r, perm1, perm2, perm3;
  double b1 = 0.0;
  double b2 = 0.0;
  double b3 = 0.0;

  if (Tcl_GetInt(interp, argv[argStart], &BBarBrickUPId) != TCL_OK) {
    opserr << "WARNING invalid BBarBrickUP eleTag" << endln;
    return TCL_ERROR;
  }

  for (int i = 0; i < 8; i++)
    if (Tcl_GetInt(interp, argv[1 + argStart + i], &(Nod[i])) != TCL_OK) {
      opserr << "WARNING invalid Node number\n";
      opserr << "BBarBrickUP element: " << BBarBrickUPId << endln;
      return TCL_ERROR;
    }

  if (Tcl_GetInt(interp, argv[9 + argStart], &matID) != TCL_OK) {
    opserr << "WARNING invalid matID\n";
    opserr << "BBarBrickUP element: " << BBarBrickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[10 + argStart], &bk) != TCL_OK) {
    opserr << "WARNING invalid fluid bulk modulus\n";
    opserr << "BBarBrickUP element: " << BBarBrickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[11 + argStart], &r) != TCL_OK) {
    opserr << "WARNING invalid fluid mass density\n";
    opserr << "BBarBrickUP element: " << BBarBrickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[12 + argStart], &perm1) != TCL_OK) {
    opserr << "WARNING invalid permeability_x\n";
    opserr << "BBarBrickUP element: " << BBarBrickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[13 + argStart], &perm2) != TCL_OK) {
    opserr << "WARNING invalid permeability_y\n";
    opserr << "BBarBrickUP element: " << BBarBrickUPId << endln;
    return TCL_ERROR;
  }

  if (Tcl_GetDouble(interp, argv[14 + argStart], &perm3) != TCL_OK) {
    opserr << "WARNING invalid permeability_z\n";
    opserr << "BBarBrickUP element: " << BBarBrickUPId << endln;
    return TCL_ERROR;
  }

  if ((argc - argStart) >= 16) {
    if (Tcl_GetDouble(interp, argv[15 + argStart], &b1) != TCL_OK) {
      opserr << "WARNING invalid b1\n";
      opserr << "BBarBrickUP element: " << BBarBrickUPId << endln;
      return TCL_ERROR;
    }
  }
  if ((argc - argStart) >= 17) {
    if (Tcl_GetDouble(interp, argv[16 + argStart], &b2) != TCL_OK) {
      opserr << "WARNING invalid b2\n";
      opserr << "BBarBrickUP element: " << BBarBrickUPId << endln;
      return TCL_ERROR;
    }
  }
  if ((argc - argStart) >= 18) {
    if (Tcl_GetDouble(interp, argv[17 + argStart], &b3) != TCL_OK) {
      opserr << "WARNING invalid b3\n";
      opserr << "BBarBrickUP element: " << BBarBrickUPId << endln;
      return TCL_ERROR;
    }
  }

  NDMaterial *theMaterial = OPS_getNDMaterial(matID);

  if (theMaterial == 0) {
    opserr << "WARNING material not found\n";
    opserr << "Material: " << matID;
    opserr << "\nBBarBrickUP element: " << BBarBrickUPId << endln;
    return TCL_ERROR;
  }

  // now create the BBarBrickUP and add it to the Domain
  BBarBrickUP *theBBarBrickUP = new BBarBrickUP(
      BBarBrickUPId, Nod[0], Nod[1], Nod[2], Nod[3], Nod[4], Nod[5], Nod[6],
      Nod[7], *theMaterial, bk, r, perm1, perm2, perm3, b1, b2, b3);
  if (theBBarBrickUP == 0) {
    opserr << "WARNING ran out of memory creating element\n";
    opserr << "BBarBrickUP element: " << BBarBrickUPId << endln;
    return TCL_ERROR;
  }

  if (theTclDomain->addElement(theBBarBrickUP) == false) {
    opserr << "WARNING could not add element to the domain\n";
    opserr << "BBarBrickUP element: " << BBarBrickUPId << endln;
    delete theBBarBrickUP;
    return TCL_ERROR;
  }

  // if get here we have successfully created the element and added it to the
  // domain
  return TCL_OK;
}