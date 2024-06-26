/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.2 $
// $Date: 2008-08-26 16:46:09 $
// $Source: /usr/local/cvs/OpenSees/SRC/material/section/ElasticTubeSection3d.h,v $

#ifndef ElasticTubeSection3d_h
#define ElasticTubeSection3d_h

#include <SectionForceDeformation.h>
#include <Matrix.h>
#include <Vector.h>

class Channel;
class FEM_ObjectBroker;
class Information;

class ElasticTubeSection3d: public SectionForceDeformation
{
 public:
  ElasticTubeSection3d(int tag, double E, double d, double tw, double G);
  ElasticTubeSection3d(void);    
  ~ElasticTubeSection3d(void);
  
  int commitState(void);
  int revertToLastCommit(void);
  int revertToStart(void);
  
  const char *getClassType(void) const {return "ElasticTubeSection3d";};
  
  int setTrialSectionDeformation(const Vector&);
  const Vector &getSectionDeformation(void);
  
  const Vector &getStressResultant(void);
  const Matrix &getSectionTangent(void);
  const Matrix &getInitialTangent(void);
  const Matrix &getSectionFlexibility(void);
  const Matrix &getInitialFlexibility(void);
  
  SectionForceDeformation *getCopy(void);
  const ID &getType();
  int getOrder(void) const;
  
  int sendSelf(int commitTag, Channel &theChannel);
  int recvSelf(int commitTag, Channel &theChannel,
	       FEM_ObjectBroker &theBroker);
  
  void Print(OPS_Stream &s, int flag =0);
  
  int setParameter(const char **argv, int argc, Parameter &param);
  int updateParameter(int parameterID, Information &info);
  int activateParameter(int parameterID);
  const Vector& getStressResultantSensitivity(int gradIndex,
					      bool conditional);
  const Matrix& getInitialTangentSensitivity(int gradIndex);
  
 protected:
  
 private:
  
  double E, d, tw, G;
  
  Vector e;			// section trial deformations
  
  static Vector s;
  static Matrix ks;
  static ID code;
  
  int parameterID;
};

#endif
