/* ------------------------------------------------------------------ **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
** ------------------------------------------------------------------ */
//
// Description: This file contains the class definition for CubicFrame3d.
//
#include <CubicFrame3d.h>
#include <Node.h>
#include <FrameSection.h>
#include <FrameTransform.h>
#include <Matrix.h>
#include <MatrixND.h>
#include <Vector.h>
#include <ID.h>
#include <Domain.h>
#include <Information.h>
#include <Channel.h>
#include <FEM_ObjectBroker.h>
#include <ElementResponse.h>
#include <CompositeResponse.h>
#include <ElementalLoad.h>
#include <BeamIntegration.h>
#include <Parameter.h>

#include <array>
#include <math.h>
#include <string.h>

#define ELE_TAG_CubicFrame3d 0 // TODO

using OpenSees::MatrixND;
using OpenSees::VectorND;

double CubicFrame3d::workArea[200];

CubicFrame3d::CubicFrame3d(int tag, std::array<int,2>& nodes,
                                   int numSec, FrameSection **s,
                                   BeamIntegration &bi,
                                   FrameTransform3d &coordTransf, double r, int cm, int rz, int ry)
 : BasicFrame3d(tag, ELE_TAG_CubicFrame3d, nodes, coordTransf, cMass, rz, ry),
  numSections(numSec), theSections(nullptr), beamInt(nullptr),
  rho(r)
{
  q.zero();

  // Allocate arrays of pointers to FrameSections
  theSections = new FrameSection *[numSections];

  for (int i = 0; i < numSections; i++) {    
    // Get copies of the material model for each integration point
    theSections[i] = s[i]->getFrameCopy();
  
  }
  
  beamInt = bi.getCopy();
  
  if (beamInt == nullptr) {
    opserr << "CubicFrame3d::CubicFrame3d - failed to copy beam integration\n";
    exit(-1);
  }

}

CubicFrame3d::CubicFrame3d()
: BasicFrame3d(0, ELE_TAG_CubicFrame3d),
  numSections(0),
  theSections(nullptr), beamInt(nullptr),
  rho(0.0)
{
  q.zero();
}

CubicFrame3d::~CubicFrame3d()
{    
  for (int i = 0; i < numSections; i++) {
    if (theSections[i])
      delete theSections[i];
  }
  
  // Delete the array of pointers to FrameSection pointer arrays
  if (theSections != nullptr)
    delete [] theSections;
  
  if (beamInt != nullptr)
    delete beamInt;
}

int
CubicFrame3d::commitState()
{
    int retVal = 0;

    // call element commitState to do any base class stuff
    if ((retVal = this->Element::commitState()) != 0) {
      opserr << "CubicFrame3d::commitState () - failed in base class";
    }    

    // Loop over the integration points and commit the material states
    for (int i = 0; i < numSections; i++)
        retVal += theSections[i]->commitState();

    retVal += theCoordTransf->commitState();

    return retVal;
}

int
CubicFrame3d::revertToLastCommit()
{
    int retVal = 0;

    // Loop over the integration points and revert to last committed state
    for (int i = 0; i < numSections; i++)
        retVal += theSections[i]->revertToLastCommit();

    retVal += theCoordTransf->revertToLastCommit();

    return retVal;
}

int
CubicFrame3d::revertToStart()
{
  int retVal = 0;

  // Loop over the integration points and revert states to start
  for (int i = 0; i < numSections; i++)
      retVal += theSections[i]->revertToStart();

  retVal += theCoordTransf->revertToStart();

  return retVal;
}

int
CubicFrame3d::setNodes()
{
  int status = this->BasicFrame3d::setNodes();
  double L = this->getLength(State::Init);

  if (L == 0.0)
    return -1;

  beamInt->getSectionLocations(numSections, L, xi);
  beamInt->getSectionWeights(numSections, L, wt);

// NOTE(cmp) uncomment to match upstream behavior
//status += this->update();
//status += this->setState(State::Pres);
  return status;
}

int
CubicFrame3d::update()
{
  // Note that setNodes must be called prior to calling this
  // so that theCoordTransf, xi and wt are initialized
  int err = 0;

  // Update the transformation
  theCoordTransf->update();

  double oneOverL = 1.0/this->getLength(State::Init);
  
  // Get basic deformations
  const Vector &v = theCoordTransf->getBasicTrialDisp();
  
  // Loop over the integration points
  for (int i = 0; i < numSections; i++) {
      double xi6 = 6.0*xi[i];

      const VectorND<4> e {
           oneOverL*v(0),                               // N
           oneOverL*((xi6-4.0)*v(1) + (xi6-2.0)*v(2)),  // Mz
           oneOverL*((xi6-4.0)*v(3) + (xi6-2.0)*v(4)),  // My
           oneOverL*v(5),                               // T
      };
      
      // Set the section deformations
      err += theSections[i]->setTrialState<4, scheme>(e);
  }

  if (err != 0)
    opserr << "CubicFrame3d::update() - failed setTrialSectionDeformations()\n";

  return err;
}

VectorND<6>&
CubicFrame3d::getBasicForce()
{
  return q;
}

MatrixND<6,6>&
CubicFrame3d::getBasicTangent(State state, int rate)
{
  // Matrix kb(6,6);
  
  // Zero for integral
  kb.zero();
  q.zero();
  
  double L = theCoordTransf->getInitialLength();
  double oneOverL = 1.0/L;

  // Loop over the integration points
  for (int i = 0; i < numSections; i++) {

    MatrixND<4,6> ka;
    ka.zero();

    double xi6 = 6.0*xi[i];

    // Get the section tangent stiffness and stress resultant

    const MatrixND<4,4,double> ks = theSections[i]->getTangent<4,scheme>(state);
               
    // Perform numerical integration
    // kb.addMatrixTripleProduct(1.0, *B, ks, wts(i)/L);
    double wti = wt[i]*oneOverL;

    // Truss terms
    for (int k = 0; k < 4; k++) {
      // N
      ka(k,0) += ks(k,0)*wti;
      // Mz
      double tmp = ks(k,1)*wti;
      ka(k,1) += (xi6-4.0)*tmp;
      ka(k,2) += (xi6-2.0)*tmp;
      // My
      tmp = ks(k,2)*wti;
      ka(k,3) += (xi6-4.0)*tmp;
      ka(k,4) += (xi6-2.0)*tmp;
      // T
      ka(k,5) += ks(k,3)*wti;
    }

    // Beam terms
    for (int k = 0; k < 6; k++) {
      // N
      kb(0,k) += ka(0,k);
      // Mz
      double tmp = ka(1,k);
      kb(1,k) += (xi6-4.0)*tmp;
      kb(2,k) += (xi6-2.0)*tmp;
      // My
      tmp = ka(2,k);
      kb(3,k) += (xi6-4.0)*tmp;
      kb(4,k) += (xi6-2.0)*tmp;
      // T
      kb(5,k) += ka(3,k);
    }

    if (state != State::Init) {
//    const Vector &s = theSections[i]->getStressResultant();
      const VectorND<4,double> s = theSections[i]->getResultant<4,scheme>();
      // q.addMatrixTransposeVector(1.0, *B, s, wts(i));
      q[0] += s[0]*wt[i];
      q[1] += (xi6-4.0)*s[1]*wt[i];
      q[2] += (xi6-2.0)*s[1]*wt[i];
      q[3] += (xi6-4.0)*s[2]*wt[i];
      q[4] += (xi6-2.0)*s[2]*wt[i];
      q[5] += s[3]*wt[i];
    }  
  }
  
  return kb;
}


int
CubicFrame3d::sendSelf(int commitTag, Channel &theChannel)
{
  // place the integer data into an ID

  int dbTag = this->getDbTag();
  int loc = 0;
  
  static Vector data(14);
  data(0) = this->getTag();
  data(1) = connectedExternalNodes(0);
  data(2) = connectedExternalNodes(1);
  data(3) = numSections;
  data(4) = theCoordTransf->getClassTag();
  int crdTransfDbTag  = theCoordTransf->getDbTag();
  if (crdTransfDbTag  == 0) {
    crdTransfDbTag = theChannel.getDbTag();
    if (crdTransfDbTag  != 0) 
      theCoordTransf->setDbTag(crdTransfDbTag);
  }
  data(5) = crdTransfDbTag;
  data(6) = beamInt->getClassTag();
  int beamIntDbTag  = beamInt->getDbTag();
  if (beamIntDbTag  == 0) {
    beamIntDbTag = theChannel.getDbTag();
    if (beamIntDbTag  != 0) 
      beamInt->setDbTag(beamIntDbTag);
  }
  data( 7) = beamIntDbTag;
  data( 8) = rho;
  data( 9) = cMass;
  data(10) = alphaM;
  data(11) = betaK;
  data(12) = betaK0;
  data(13) = betaKc;
  
  if (theChannel.sendVector(dbTag, commitTag, data) < 0) {
    opserr << "CubicFrame3d::sendSelf() - failed to send data Vector\n";
     return -1;
  }    
  
  // send the coordinate transformation
  if (theCoordTransf->sendSelf(commitTag, theChannel) < 0) {
     opserr << "CubicFrame3d::sendSelf() - failed to send crdTranf\n";
     return -1;
  }      

  // send the beam integration
  if (beamInt->sendSelf(commitTag, theChannel) < 0) {
    opserr << "CubicFrame3d::sendSelf() - failed to send beamInt\n";
    return -1;
  }      
  
  //
  // send an ID for the sections containing each sections dbTag and classTag
  // if section ha no dbTag get one and assign it
  //

  ID idSections(2*numSections);
  loc = 0;
  for (int i = 0; i<numSections; i++) {
    int sectClassTag = theSections[i]->getClassTag();
    int sectDbTag = theSections[i]->getDbTag();
    if (sectDbTag == 0) {
      sectDbTag = theChannel.getDbTag();
      theSections[i]->setDbTag(sectDbTag);
    }

    idSections(loc) = sectClassTag;
    idSections(loc+1) = sectDbTag;
    loc += 2;
  }

  if (theChannel.sendID(dbTag, commitTag, idSections) < 0)  {
    opserr << "CubicFrame3d::sendSelf() - failed to send ID data\n";
    return -1;
  }    

  //
  // send the sections
  //
  
  for (int j = 0; j<numSections; j++) {
    if (theSections[j]->sendSelf(commitTag, theChannel) < 0) {
      opserr << "CubicFrame3d::sendSelf() - section " << j << "failed to send itself\n";
      return -1;
    }
  }

  return 0;
}

int
CubicFrame3d::recvSelf(int commitTag, Channel &theChannel,
                                                FEM_ObjectBroker &theBroker)
{
  //
  // receive the integer data containing tag, numSections and coord transformation info
  //
  int dbTag = this->getDbTag();

  static Vector data(14);

  if (theChannel.recvVector(dbTag, commitTag, data) < 0)  {
    opserr << "CubicFrame3d::recvSelf() - failed to recv data Vector\n";
    return -1;
  }
  
  this->setTag((int)data(0));
  connectedExternalNodes(0) = (int)data(1);
  connectedExternalNodes(1) = (int)data(2);
  int nSect = (int)data(3);
  int crdTransfClassTag = (int)data(4);
  int crdTransfDbTag = (int)data(5);
  
  int beamIntClassTag = (int)data(6);
  int beamIntDbTag = (int)data(7);
  
  rho = data(8);
  cMass = (int)data(9);
  
  alphaM = data(10);
  betaK = data(11);
  betaK0 = data(12);
  betaKc = data(13);
  
  // create a new crdTransf object if one needed
  if (theCoordTransf == 0 || theCoordTransf->getClassTag() != crdTransfClassTag) {
      if (theCoordTransf != 0)
          delete theCoordTransf;

    // TODO(cmp) - add FrameTransform to ObjBroker
      theCoordTransf = nullptr; // theBroker.getNewFrameTransform3d(crdTransfClassTag);

      if (theCoordTransf == nullptr) {
        opserr << "CubicFrame3d::recvSelf() - " <<
          "failed to obtain a CrdTrans object with classTag" <<
          crdTransfClassTag << "\n";
        return -2;          
      }
  }

  theCoordTransf->setDbTag(crdTransfDbTag);

  // invoke recvSelf on the crdTransf object
  if (theCoordTransf->recvSelf(commitTag, theChannel, theBroker) < 0) {
    opserr << "CubicFrame3d::sendSelf() - failed to recv crdTranf\n";
    return -3;
  }      

  // create a new beamInt object if one needed
  if (beamInt == 0 || beamInt->getClassTag() != beamIntClassTag) {
      if (beamInt != 0)
          delete beamInt;

      beamInt = theBroker.getNewBeamIntegration(beamIntClassTag);

      if (beamInt == 0) {
        opserr << "CubicFrame3d::recvSelf() - failed to obtain the beam integration object with classTag" <<
          beamIntClassTag << "\n";
        return -1;
      }
  }

  beamInt->setDbTag(beamIntDbTag);

  // invoke recvSelf on the beamInt object
  if (beamInt->recvSelf(commitTag, theChannel, theBroker) < 0)  
  {
     opserr << "CubicFrame3d::sendSelf() - failed to recv beam integration\n";
     return -3;
  }      
  
  //
  // recv an ID for the sections containing each sections dbTag and classTag
  //

  ID idSections(2*nSect);
  int loc = 0;

  if (theChannel.recvID(dbTag, commitTag, idSections) < 0)  {
    opserr << "CubicFrame3d::recvSelf() - failed to recv ID data\n";
    return -1;
  }    

  //
  // now receive the sections
  //
  
  if (numSections != nSect) {

    //
    // we do not have correct number of sections, must delete the old and create
    // new ones before can recvSelf on the sections
    //

    // delete the old
    if (numSections != 0) {
      for (int i=0; i<numSections; i++)
        delete theSections[i];
      delete [] theSections;
    }

    // create a new array to hold pointers
    theSections = new FrameSection *[nSect];

    // create a section and recvSelf on it
    numSections = nSect;
    loc = 0;
    
    for (int i=0; i<numSections; i++) {
      int sectClassTag = idSections(loc);
      int sectDbTag = idSections(loc+1);
      loc += 2;
      // TODO(cmp) add FrameSection to broker
//    theSections[i] = theBroker.getNewSection(sectClassTag);
      if (theSections[i] == 0) {
        opserr << "CubicFrame3d::recvSelf() - Broker could not create Section of class type" <<
          sectClassTag << "\n";
        return -1;
      }
      theSections[i]->setDbTag(sectDbTag);
      if (theSections[i]->recvSelf(commitTag, theChannel, theBroker) < 0) {
        opserr << "CubicFrame3d::recvSelf() - section " <<
          i << "failed to recv itself\n";
        return -1;
      }     
    }

  } else {

    // 
    // for each existing section, check it is of correct type
    // (if not delete old & create a new one) then recvSelf on it
    //
    
    loc = 0;
    for (int i=0; i<numSections; i++) {
      int sectClassTag = idSections(loc);
      int sectDbTag = idSections(loc+1);
      loc += 2;

      // check of correct type
      if (theSections[i]->getClassTag() !=  sectClassTag) {
        // delete the old section[i] and create a new one
        delete theSections[i];
      // TODO(cmp) add FrameSection to broker
//      theSections[i] = theBroker.getNewSection(sectClassTag);
        if (theSections[i] == 0) {
          opserr << "CubicFrame3d::recvSelf() - Broker could not create Section of class type" <<
            sectClassTag << "\n";
          return -1;
        }
      }

      // recvSelf on it
      theSections[i]->setDbTag(sectDbTag);
      if (theSections[i]->recvSelf(commitTag, theChannel, theBroker) < 0) {
        opserr << "CubicFrame3d::recvSelf() - section " << 
          i << "failed to recv itself\n";
        return -1;
      }     
    }
  }

  return 0;
}

void
CubicFrame3d::Print(OPS_Stream &s, int flag)
{

    if (flag == OPS_PRINT_PRINTMODEL_JSON) {
            s << "\t\t\t{";
            s << "\"name\": " << this->getTag() << ", ";
            s << "\"type\": \"CubicFrame3d\"" << ", ";
            s << "\"nodes\": [" << connectedExternalNodes(0) << ", " << connectedExternalNodes(1) << "]";
            s << ", ";
            s << "\"massperlength\": " << rho ;
            s << ", ";
            s << "\"crdTransformation\": \"" << theCoordTransf->getTag();
            s << ", ";

            //
            s << "\"sections\": [";
            for (int i = 0; i < numSections - 1; i++)
                    s << "\"" << theSections[i]->getTag() << "\", ";
            s << "\"" << theSections[numSections - 1]->getTag() << "\"], ";
            s << "\"integration\": ";
            beamInt->Print(s, flag);
            //
            s << "\"}";
    }

    if (flag == OPS_PRINT_CURRENTSTATE) {
          s << "\nCubicFrame3d, element id:  " << this->getTag() << "\n";
          s << "\tConnected external nodes:  " << connectedExternalNodes;
          s << "\tCoordTransf: " << theCoordTransf->getTag() << "\n";
          s << "\tmass density:  " << rho << ", cMass: " << cMass << "\n";

          double N, Mz1, Mz2, Vy, My1, My2, Vz, T;
          double L = theCoordTransf->getInitialLength();
          double oneOverL = 1.0 / L;

          N = q[0];
          Mz1 = q[1];
          Mz2 = q[2];
          Vy = (Mz1 + Mz2)*oneOverL;
          My1 = q[3];
          My2 = q[4];
          Vz = -(My1 + My2)*oneOverL;
          T = q[5];

          s << "\tEnd 1 Forces (P Mz Vy My Vz T): "
                  << -N + p0[0] << ' ' << Mz1 << ' ' << Vy + p0[1] << ' ' << My1 << ' ' << Vz + p0[3] << ' ' << -T << "\n";
          s << "\tEnd 2 Forces (P Mz Vy My Vz T): "
                  << N << ' ' << Mz2 << ' ' << -Vy + p0[2] << ' ' << My2 << ' ' << -Vz + p0[4] << ' ' << T << "\n";
          s << "Number of sections: " << numSections << "\n";
          beamInt->Print(s, flag);

          for (int i = 0; i < numSections; i++) {
            //opserr << "Section Type: " << theSections[i]->getClassTag() << "\n";
            theSections[i]->Print(s,flag);
          }
          //  if (rho != 0)
          //    opserr << "Mass: \n" << this->getMass();
    }
}



Response*
CubicFrame3d::setResponse(const char **argv, int argc, OPS_Stream &output)
{

    Response *theResponse = 0;

    output.tag("ElementOutput");
    output.attr("eleType","CubicFrame3d");
    output.attr("eleTag",this->getTag());
    output.attr("node1",connectedExternalNodes[0]);
    output.attr("node2",connectedExternalNodes[1]);

    //
    // we compare argv[0] for known response types 
    //

    // global force - 
    if (strcmp(argv[0],"forces") == 0 || 
        strcmp(argv[0],"force") == 0  ||
        strcmp(argv[0],"globalForce") == 0 ||
        strcmp(argv[0],"globalForces") == 0) {

      output.tag("ResponseType","Px_1");
      output.tag("ResponseType","Py_1");
      output.tag("ResponseType","Pz_1");
      output.tag("ResponseType","Mx_1");
      output.tag("ResponseType","My_1");
      output.tag("ResponseType","Mz_1");
      output.tag("ResponseType","Px_2");
      output.tag("ResponseType","Py_2");
      output.tag("ResponseType","Pz_2");
      output.tag("ResponseType","Mx_2");
      output.tag("ResponseType","My_2");
      output.tag("ResponseType","Mz_2");


      theResponse = new ElementResponse(this, 1, P);

    // local force -
    }  else if (strcmp(argv[0],"localForce") == 0 || strcmp(argv[0],"localForces") == 0) {

      output.tag("ResponseType","N_1");
      output.tag("ResponseType","Vy_1");
      output.tag("ResponseType","Vz_1");
      output.tag("ResponseType","T_1");
      output.tag("ResponseType","My_1");
      output.tag("ResponseType","Mz_1");
      output.tag("ResponseType","N_2");
      output.tag("ResponseType","Vy_2");
      output.tag("ResponseType","Vz_2");
      output.tag("ResponseType","T_2");
      output.tag("ResponseType","My_2");
      output.tag("ResponseType","Mz_2");

      theResponse = new ElementResponse(this, 2, P);

    // chord rotation -
    }  else if (strcmp(argv[0],"chordRotation") == 0 || strcmp(argv[0],"chordDeformation") == 0 
              || strcmp(argv[0],"basicDeformation") == 0) {

      output.tag("ResponseType","eps");
      output.tag("ResponseType","thetaZ_1");
      output.tag("ResponseType","thetaZ_2");
      output.tag("ResponseType","thetaY_1");
      output.tag("ResponseType","thetaY_2");
      output.tag("ResponseType","thetaX");

      theResponse = new ElementResponse(this, 3, Vector(6));

    // plastic rotation -
    } else if (strcmp(argv[0],"plasticRotation") == 0 || strcmp(argv[0],"plasticDeformation") == 0) {

      output.tag("ResponseType","epsP");
      output.tag("ResponseType","thetaZP_1");
      output.tag("ResponseType","thetaZP_2");
      output.tag("ResponseType","thetaYP_1");
      output.tag("ResponseType","thetaYP_2");
      output.tag("ResponseType","thetaXP");

      theResponse = new ElementResponse(this, 4, Vector(6));
  

    } else if (strcmp(argv[0],"RayleighForces") == 0 ||
               strcmp(argv[0],"rayleighForces") == 0) {
  
      theResponse =  new ElementResponse(this, 12, P);
  
    }   
    else if (strcmp(argv[0],"integrationPoints") == 0)
      theResponse = new ElementResponse(this, 10, Vector(numSections));

    else if (strcmp(argv[0],"integrationWeights") == 0)
      theResponse = new ElementResponse(this, 11, Vector(numSections));

    else if (strcmp(argv[0],"sectionTags") == 0)
      theResponse = new ElementResponse(this, 110, ID(numSections));
    
    // section response -
    else if (strstr(argv[0],"sectionX") != 0) {
      if (argc > 2 && (this->setNodes() == 0)) {

        float sectionLoc = atof(argv[1]);
        double L = this->getLength(State::Init);

        sectionLoc /= L;
        
        float minDistance = fabs(xi[0]-sectionLoc);
        int sectionNum = 0;
        for (int i = 1; i < numSections; i++) {
          if (fabs(xi[i]-sectionLoc) < minDistance) {
            minDistance = fabs(xi[i]-sectionLoc);
            sectionNum = i;
          }
        }
        
        output.tag("GaussPointOutput");
        output.attr("number",sectionNum+1);
        output.attr("eta",xi[sectionNum]*L);
        
        theResponse = theSections[sectionNum]->setResponse(&argv[2], argc-2, output);
      }
    }
    
    else if (strcmp(argv[0],"section") ==0) { 
      // Make sure setNodes() succeeds to ensure
      // xi is initialized and L can be determined
      if (argc > 1 && (this->setNodes() == 0)) {
        
        int sectionNum = atoi(argv[1]);
        double L = this->getLength(State::Init);

        if (sectionNum > 0 && sectionNum <= numSections && argc > 2) {

          output.tag("GaussPointOutput");
          output.attr("number",sectionNum);
          output.attr("eta",xi[sectionNum-1]*L);
          
          theResponse =  theSections[sectionNum-1]->setResponse(&argv[2], argc-2, output);
          
          output.endTag();
        } else if (sectionNum == 0) { // argv[1] was not an int, we want all sections, 
        
          CompositeResponse *theCResponse = new CompositeResponse();
          int numResponse = 0;
          
          for (int i=0; i<numSections; i++) {
            
            output.tag("GaussPointOutput");
            output.attr("number",i+1);
            output.attr("eta",xi[i]*L);
            
            Response *theSectionResponse = theSections[i]->setResponse(&argv[1], argc-1, output);
            
            output.endTag();          
            
            if (theSectionResponse != 0) {
              numResponse = theCResponse->addResponse(theSectionResponse);
            }
          }
          
          if (numResponse == 0) // no valid responses found
            delete theCResponse;
          else
            theResponse = theCResponse;
        }
      }
    }
        // by SAJalali
    else if (strcmp(argv[0], "energy") == 0) {
      return new ElementResponse(this, 13, 0.0);
    }

    if (theResponse == nullptr)
      theResponse = theCoordTransf->setResponse(argv, argc, output);

  output.endTag();
  return theResponse;
}

int 
CubicFrame3d::getResponse(int responseID, Information &eleInfo)
{

  if (responseID == 1)
    return eleInfo.setVector(this->getResistingForce());

  else if (responseID == 12)
    return eleInfo.setVector(this->getRayleighDampingForces());
    
  else if (responseID == 2) {
    double N, V, M1, M2, T;
    double L = theCoordTransf->getInitialLength();
    double oneOverL = 1.0/L;

    // Axial
    N = q[0];
    P(6) =  N;
    P(0) = -N+p0[0];
    
    // Torsion
    T = q[5];
    P(9) =  T;
    P(3) = -T;
    
    // Moments about z and shears along y
    M1 = q[1];
    M2 = q[2];
    P(5)  = M1;
    P(11) = M2;
    V = (M1+M2)*oneOverL;
    P(1) =  V+p0[1];
    P(7) = -V+p0[2];
    
    // Moments about y and shears along z
    M1 = q[3];
    M2 = q[4];
    P(4)  = M1;
    P(10) = M2;
    V = (M1+M2)*oneOverL;
    P(2) = -V+p0[3];
    P(8) =  V+p0[4];

    return eleInfo.setVector(P);
  }

  // Chord rotation
  else if (responseID == 3)
    return eleInfo.setVector(theCoordTransf->getBasicTrialDisp());

  // Plastic rotation
  else if (responseID == 4) {
    static Vector vp(6);
    static Vector ve(6);
    auto kb = this->getBasicTangent(State::Init, 0);
    kb.solve(q, ve);
    vp = theCoordTransf->getBasicTrialDisp();
    vp -= ve;
    return eleInfo.setVector(vp);
  }

  else if (responseID == 10) {
    if (this->setState(State::Init) == 0) {
      double L = this->getLength(State::Init);
      Vector locs(numSections);
      for (int i = 0; i < numSections; i++)
        locs(i) = wt[i]*L;
      return eleInfo.setVector(locs);
    }
  }

  else if (responseID == 11) {
    if (this->setState(State::Init) == 0) {
      double L = theCoordTransf->getInitialLength();
      Vector weights(numSections);
      for (int i = 0; i < numSections; i++)
        weights(i) = wt[i]*L;
      return eleInfo.setVector(weights);
    }
  }

  else if (responseID == 110) {
    ID tags(numSections);
    for (int i = 0; i < numSections; i++)
      tags(i) = theSections[i]->getTag();
    return eleInfo.setID(tags);
  }

  //by SAJalali
  else if (responseID == 13) {
    if (this->setState(State::Init) == 0) {
      double L = this->getLength(State::Init);
      double energy = 0;
      for (int i = 0; i < numSections; i++)
          energy += theSections[i]->getEnergy()*xi[i] * L;

      return eleInfo.setDouble(energy);
    }
  }

  else
    return -1;

  return -1;
}

// AddingSensitivity:BEGIN ///////////////////////////////////
int
CubicFrame3d::setParameter(const char **argv, int argc, Parameter &param)
{

  int status = this->BasicFrame3d::setParameter(argv, argc, param);
  if (status != -1)
    return status;

  if (argc < 1)
    return -1;

  if (strstr(argv[0],"sectionX") != 0) {
      if (argc < 3 || (this->setState(State::Init) != 0))
        return -1;
      
      float sectionLoc = atof(argv[1]);
      double L = this->getLength(State::Init);
      
      sectionLoc /= L;

      float minDistance = fabs(xi[0]-sectionLoc);
      int sectionNum = 0;
      for (int i = 1; i < numSections; i++) {
        if (fabs(xi[i] - sectionLoc) < minDistance) {
          minDistance = fabs(xi[i]-sectionLoc);
          sectionNum = i;
        }
      }  
      return theSections[sectionNum]->setParameter(&argv[2], argc-2, param);
  }

  // If the parameter belongs to a section or lower
  if (strstr(argv[0],"section") != 0) {
    
    if (argc < 3)
      return -1;
    
    // Get section number
    int sectionNum = atoi(argv[1]);
    
    if (sectionNum > 0 && sectionNum <= numSections)
      return theSections[sectionNum-1]->setParameter(&argv[2], argc-2, param);
    else
      return -1;
  }
  
  if (strstr(argv[0],"integration") != 0) {
    
    if (argc < 2)
      return -1;

    return beamInt->setParameter(&argv[1], argc-1, param);
  }

  // Default, send to every object
  int ok = 0;
  int result = -1;

  for (int i = 0; i < numSections; i++) {
    ok = theSections[i]->setParameter(argv, argc, param);
    if (ok != -1)
      result = ok;
  }
  
  ok = beamInt->setParameter(argv, argc, param);
  if (ok != -1)
    result = ok;

  return result;
}



const Matrix &
CubicFrame3d::getInitialStiffSensitivity(int gradNumber)
{
  K.Zero();
  return K;
}

const Vector &
CubicFrame3d::getResistingForceSensitivity(int gradNumber)
{
  double L = theCoordTransf->getInitialLength();
  double oneOverL = 1.0/L;

  int ok = this->setState(State::Init);
  assert(ok == 0);

  // Zero for integration
  static Vector dqdh(6);
  dqdh.Zero();
  
  // Loop over the integration points
  for (int i = 0; i < numSections; i++) {
    
    int order = theSections[i]->getOrder();
    const ID &code = theSections[i]->getType();
    
    //double xi6 = 6.0*pts(i,0);
    double xi6 = 6.0*xi[i];
    //double wti = wts(i);
    double wti = wt[i];
    
    // Get section stress resultant gradient
    const Vector &dsdh = theSections[i]->getStressResultantSensitivity(gradNumber,true);
    
    // Perform numerical integration on internal force gradient
    double sensi;
    for (int j = 0; j < order; j++) {
      sensi = dsdh(j)*wti;
      switch(code(j)) {
      case SECTION_RESPONSE_P:
        dqdh(0) += sensi; 
        break;
      case SECTION_RESPONSE_MZ:
        dqdh(1) += (xi6-4.0)*sensi; 
        dqdh(2) += (xi6-2.0)*sensi; 
        break;
      case SECTION_RESPONSE_MY:
        dqdh(3) += (xi6-4.0)*sensi; 
        dqdh(4) += (xi6-2.0)*sensi; 
        break;
      case SECTION_RESPONSE_T:
        dqdh(5) += sensi; 
        break;
      default:
        break;
      }
    }
  }
  
  // Transform forces
  static Vector dp0dh(6);                // No distributed loads

  P.Zero();

  //////////////////////////////////////////////////////////////

  if (theCoordTransf->isShapeSensitivity()) {
    
    // Perform numerical integration to obtain basic stiffness matrix
    // Some extra declarations
    static Matrix kbmine(6,6);
    kbmine.Zero();
    q.zero();
    
    double tmp;
    
    int j, k;
    
    for (int i = 0; i < numSections; i++) {
      
      int order = theSections[i]->getOrder();
      const ID &code = theSections[i]->getType();
      
      //double xi6 = 6.0*pts(i,0);
      double xi6 = 6.0*xi[i];
      //double wti = wts(i);
      double wti = wt[i];
      
      const Vector &s = theSections[i]->getStressResultant();
      const Matrix &ks = theSections[i]->getSectionTangent();
      
      Matrix ka(workArea, order, 6);
      ka.Zero();
      
      double si;
      for (j = 0; j < order; j++) {
        si = s(j)*wti;
        switch(code(j)) {
        case SECTION_RESPONSE_P:
          q[0] += si;
          for (k = 0; k < order; k++) {
            ka(k,0) += ks(k,j)*wti;
          }
          break;
        case SECTION_RESPONSE_MZ:
          q[1] += (xi6-4.0)*si; 
          q[2] += (xi6-2.0)*si;
          for (k = 0; k < order; k++) {
            tmp = ks(k,j)*wti;
            ka(k,1) += (xi6-4.0)*tmp;
            ka(k,2) += (xi6-2.0)*tmp;
          }
          break;
        case SECTION_RESPONSE_MY:
          q[3] += (xi6-4.0)*si; 
          q[4] += (xi6-2.0)*si;
          for (k = 0; k < order; k++) {
            tmp = ks(k,j)*wti;
            ka(k,3) += (xi6-4.0)*tmp;
            ka(k,4) += (xi6-2.0)*tmp;
          }
          break;
        case SECTION_RESPONSE_T:
          q[5] += si;
          for (k = 0; k < order; k++) {
            ka(k,5) += ks(k,j)*wti;
          }
          break;
        default:
          break;
        }
      }
      for (j = 0; j < order; j++) {
        switch (code(j)) {
        case SECTION_RESPONSE_P:
          for (k = 0; k < 6; k++) {
            kbmine(0,k) += ka(j,k);
          }
          break;
        case SECTION_RESPONSE_MZ:
          for (k = 0; k < 6; k++) {
            tmp = ka(j,k);
            kbmine(1,k) += (xi6-4.0)*tmp;
            kbmine(2,k) += (xi6-2.0)*tmp;
          }
          break;
        case SECTION_RESPONSE_MY:
          for (k = 0; k < 6; k++) {
            tmp = ka(j,k);
            kbmine(3,k) += (xi6-4.0)*tmp;
            kbmine(4,k) += (xi6-2.0)*tmp;
          }
          break;
        case SECTION_RESPONSE_T:
          for (k = 0; k < 6; k++) {
            kbmine(5,k) += ka(j,k);
          }
          break;
        default:
          break;
        }
      }
    }      
    
    const Vector &A_u = theCoordTransf->getBasicTrialDisp();
    double dLdh = theCoordTransf->getdLdh();
    double d1overLdh = -dLdh/(L*L);
    // a^T k_s dadh v
    dqdh.addMatrixVector(1.0, kbmine, A_u, d1overLdh);

    // k dAdh u
    const Vector &dAdh_u = theCoordTransf->getBasicTrialDispShapeSensitivity();
    dqdh.addMatrixVector(1.0, kbmine, dAdh_u, oneOverL);

    // dAdh^T q
    P += theCoordTransf->getGlobalResistingForceShapeSensitivity(q, dp0dh, gradNumber);
  }
  
  // A^T (dqdh + k dAdh u)
  P += theCoordTransf->getGlobalResistingForce(dqdh, dp0dh);
  
  return P;
}


int
CubicFrame3d::commitSensitivity(int gradNumber, int numGrads)
{
  // Get basic deformation and sensitivities
  const Vector &v = theCoordTransf->getBasicTrialDisp();
  
  static Vector dvdh(6);
  dvdh = theCoordTransf->getBasicDisplSensitivity(gradNumber);
  
  double L = theCoordTransf->getInitialLength();
  double oneOverL = 1.0/L;

  // Some extra declarations
  double d1oLdh = theCoordTransf->getd1overLdh();
  
  // Loop over the integration points
  for (int i = 0; i < numSections; i++) {
    
    int order = theSections[i]->getOrder();
    const ID &code = theSections[i]->getType();
    
    Vector e(workArea, order);
    
    //double xi6 = 6.0*pts(i,0);
    double xi6 = 6.0*xi[i];
    
    for (int j = 0; j < order; j++) {
      switch(code(j)) {
      case SECTION_RESPONSE_P:
        e(j) = oneOverL*dvdh(0)
          + d1oLdh*v(0); 
        break;
      case SECTION_RESPONSE_MZ:
        e(j) = oneOverL*((xi6-4.0)*dvdh(1) + (xi6-2.0)*dvdh(2))
          + d1oLdh*((xi6-4.0)*v(1) + (xi6-2.0)*v(2)); 
        break;
      case SECTION_RESPONSE_MY:
        e(j) = oneOverL*((xi6-4.0)*dvdh(3) + (xi6-2.0)*dvdh(4))
          + d1oLdh*((xi6-4.0)*v(3) + (xi6-2.0)*v(4)); 
        break;
      case SECTION_RESPONSE_T:
        e(j) = oneOverL*dvdh(5)
          + d1oLdh*v(5); 
        break;
      default:
        e(j) = 0.0; 
        break;
      }
    }
    
    // Set the section deformations
    theSections[i]->commitSensitivity(e,gradNumber,numGrads);
  }
  
  return 0;
}
