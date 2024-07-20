/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
** ****************************************************************** */
//
// Purpose: This file contains the class definition for PrismFrame2d.
// PrismFrame2d is a plane frame member.
//
// Written: cmp 05/2024
// Revised:
//
#ifndef PrismFrame2d_h
#define PrismFrame2d_h

#include <Element.h>
#include <Matrix.h>
#include <Vector.h>
#include <MatrixND.h>
#include <VectorND.h>

class Node;
class Channel;
class Information;
class CrdTransf;
class SectionForceDeformation;
class Response;

class PrismFrame2d : public Element
{
  public:
    PrismFrame2d();        
    PrismFrame2d(int tag, double A, double E, double I, 
		  int Nd1, int Nd2, CrdTransf &theTransf,
		  double alpha = 0.0, double d = 0.0,
		  double rho = 0.0, int cMass = 0,
		  int release = 0, int geom_flag=0);

    PrismFrame2d(int tag, int Nd1, int Nd2, 
		  SectionForceDeformation& theSection, CrdTransf &theTransf,
		  double alpha = 0.0, double d = 0.0,
		  double rho = 0.0, int cMass = 0,
		  int release = 0);

    ~PrismFrame2d();

    const char *getClassType() const {return "PrismFrame2d";};
    static constexpr const char* class_name = "PrismFrame2d";

    int getNumExternalNodes() const;
    const ID &getExternalNodes();
    Node **getNodePtrs();

    int getNumDOF();
    void setDomain(Domain *theDomain);

    int commitState();
    int revertToLastCommit();        
    int revertToStart();

    int update();
    const Matrix &getTangentStiff();
    const Matrix &getInitialStiff();
    const Matrix &getMass();    

    void zeroLoad();	
    int addLoad(ElementalLoad *theLoad, double loadFactor);
    int addInertiaLoadToUnbalance(const Vector &accel);

    const Vector &getResistingForce();
    const Vector &getResistingForceIncInertia();            

    int sendSelf(int commitTag, Channel &theChannel);
    int recvSelf(int commitTag, Channel &theChannel, FEM_ObjectBroker &theBroker);

    void Print(OPS_Stream &s, int flag = 0);

    Response *setResponse (const char **argv, int argc, OPS_Stream &s);
    int getResponse (int responseID, Information &info);

    int setParameter (const char **argv, int argc, Parameter &param);
    int updateParameter (int parameterID, Information &info);

  private:
    void formBasicStiffness(OpenSees::MatrixND<3,3> &) const;

    // Model parameters
    double A,E,I;     // area, elastic modulus, moment of inertia
    double alpha,     // coeff. of thermal expansion,
           depth;     // depth
    double rho;       // mass per unit length

    int cMass;        // consistent mass flag
    int release;      // moment release 0=none, 1=I, 2=J, 3=I,J
    int geom_flag;

    // State variables; not passed in send/recvSelf
    double L;                    // Initial length

    Vector Q;
    OpenSees::MatrixND<3,3> ke;  // stiffness in the basic system
    OpenSees::MatrixND<3,3> km;  // material stiffness in the basic system
    OpenSees::MatrixND<3,3> kg;  // geometric stiffness in the basic system
    OpenSees::MatrixND<6,6> M;   // mass matrix

    OpenSees::VectorND<3>   q;
    OpenSees::VectorND<3>   q0;  // Fixed end forces in basic system
    OpenSees::VectorND<3>   p0;  // Reactions in basic system
    
    Node *theNodes[2];
    
    ID  connectedExternalNodes;    

    CrdTransf *theCoordTransf;

    static Matrix K;
    static Vector P;
};

#endif