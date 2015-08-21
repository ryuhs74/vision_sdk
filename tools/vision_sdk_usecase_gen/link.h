#ifndef LINK_H
#define LINK_H

#include <vector>
#include <string>
#include "processor.h"

#define BLOCK_SPACE "       "

extern Processor proc;

using namespace std;

class Link;
class Link {
protected:

    string name;
    string linkIDName;
    string linkIDAsgnName;
    string prmName;

    ClassType cType;
    ProcType pType;

    int matrixPos; //Position in matrix of connections (Usecase class)
    int execPos; //Sequence num assigned to link
    int procID; //ID given by processor to link obj

    bool mulInQue; //True if multiple input
    bool mulOutQue; //True if multiple output

    vector<pair<Link*, int> > inLink; //Link and queueid
    vector<pair<Link*, int> > outLink;

public:
    Link();
    virtual ~Link();

    string getName();
    void setName(string nm);

    string getLinkName();
    void setLinkName(string nm);

    string getLinkIDName();
    void setLinkIDName();

    string getPrmName();
    void setPrmName(string nm);

    ProcType getProcType();
    void setProcType(ProcType processor);

    ClassType getClassType();
    void setProcType(string strproc);

    void setProcID();
    int getProcID();

    int getMatrixPos();
    void setMatrixPos(int pos);
    int getExecPos();
    void setExecPos(int pos);

    bool getMulInQue();
    bool getMulOutQue();

    void setInQueueID(int objNum, int qid);
    void setOutQueueID(int objNum, int qid);

    int getInQueueID(Link* obj);
    int getOutQueueID(Link* obj);

    void printInLink(ostream &out);
    void printOutLink(ostream &out);

    int getInLinkSize();
    int getOutLinkSize();

    vector<pair<Link*, int> > * getOutLinkPtr();

    virtual int setInLink(Link* obj)=0; //returns QueueID
    virtual int setOutLink(Link* obj)=0; //return QueueID

    virtual void genIncludes(ostream &fp) = 0;
    virtual void genLinkID(ostream &fp) = 0;
    virtual void genCreatePrms(ostream &fp) = 0;
    virtual void genResetLinkPrms(ostream &fp, string obj) = 0;
    virtual void genSetLinkPrms(ostream &fp, string obj) = 0;

    void genSetLinkID(ostream &fp, string obj);
    void genCreate(ostream &fp, string obj);
    void genStart(ostream &fp, string obj);
    void genStop(ostream &fp, string obj);
    void genDelete(ostream &fp, string obj);
    void genBufferStatistics(ostream &fp, string obj);
    void genStatistics(ostream &fp, string obj);
};
/*******************************************************/
/*******************************************************/
class AVBReceive: public Link {
    ~AVBReceive();
public:
    AVBReceive(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg: public Link {
    ~Alg();
public:
    Alg(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);

};
/*******************************************************/
class Alg_ColorToGray: public Link {
    ~Alg_ColorToGray();
public:
    Alg_ColorToGray(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};

/*******************************************************/
class Alg_DMASwMs: public Link {
    ~Alg_DMASwMs();
public:
    Alg_DMASwMs(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_DenseOptFlow: public Link {
    ~Alg_DenseOptFlow();
public:
    Alg_DenseOptFlow(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_EdgeDetect: public Link {
    ~Alg_EdgeDetect();
public:
    Alg_EdgeDetect(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_SoftIsp: public Link {
    ~Alg_SoftIsp();
public:
    Alg_SoftIsp(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_IssAewb: public Link {
    ~Alg_IssAewb();
public:
    Alg_IssAewb(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};

/*******************************************************/
class Alg_FeaturePlaneComputation: public Link {
    ~Alg_FeaturePlaneComputation();
public:
    Alg_FeaturePlaneComputation(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_ObjectDetection: public Link {
    ~Alg_ObjectDetection();
public:
    Alg_ObjectDetection(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_FrameCopy: public Link {
    ~Alg_FrameCopy();
public:
    Alg_FrameCopy(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};

/*******************************************************/
class Alg_MyAlgFinish: public Link {
    ~Alg_MyAlgFinish();
public:
    Alg_MyAlgFinish(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};

/*******************************************************/
class Alg_MyAlg1: public Link {
    ~Alg_MyAlg1();
public:
    Alg_MyAlg1(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};

/*******************************************************/
class Alg_MyAlg2: public Link {
    ~Alg_MyAlg2();
public:
    Alg_MyAlg2(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};

/*******************************************************/
class Alg_MyAlg3: public Link {
    ~Alg_MyAlg3();
public:
    Alg_MyAlg3(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};

/*******************************************************/
class Alg_Census: public Link {
    ~Alg_Census();
public:
    Alg_Census(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_DisparityHamDist: public Link {
    ~Alg_DisparityHamDist();
public:
    Alg_DisparityHamDist(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_UltrasonicFusion: public Link {
    ~Alg_UltrasonicFusion();
public:
    Alg_UltrasonicFusion(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_GeoAlign: public Link {
    ~Alg_GeoAlign();
public:
    Alg_GeoAlign(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_ObjectDraw: public Link {
    ~Alg_ObjectDraw();
public:
    Alg_ObjectDraw(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_PhotoAlign: public Link {
    ~Alg_PhotoAlign();
public:
    Alg_PhotoAlign(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_Synthesis: public Link {
    ~Alg_Synthesis();
public:
    Alg_Synthesis(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_SparseOpticalFlow: public Link {
    ~Alg_SparseOpticalFlow();
public:
    Alg_SparseOpticalFlow(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_SparseOpticalFlowDraw: public Link {
    ~Alg_SparseOpticalFlowDraw();
public:
    Alg_SparseOpticalFlowDraw(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_LaneDetect: public Link {
    ~Alg_LaneDetect();
public:
    Alg_LaneDetect(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_LaneDetectDraw: public Link {
    ~Alg_LaneDetectDraw();
public:
    Alg_LaneDetectDraw(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_VectoImg: public Link {
    ~Alg_VectoImg();
public:
    Alg_VectoImg(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Capture: public Link {
    ~Capture();
public:
    Capture(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};
/*******************************************************/
class IssCapture: public Link {
    ~IssCapture();
public:
    IssCapture(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};
/*******************************************************/
class IssM2mIsp: public Link {
    ~IssM2mIsp();
public:
    IssM2mIsp(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};
/*******************************************************/
class IssM2mSimcop: public Link {
    ~IssM2mSimcop();
public:
    IssM2mSimcop(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};

/*******************************************************/
class UltrasonicCapture: public Link {
    ~UltrasonicCapture();
public:
    UltrasonicCapture(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};

/*******************************************************/
class Decode: public Link {
    ~Decode();
public:
    Decode(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID
};
/*******************************************************/
class DisplayCtrl: public Link {
    ~DisplayCtrl();
public:
    DisplayCtrl(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID
};
/*******************************************************/
class Display: public Link {
    ~Display();
public:
    Display(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};

/*******************************************************/
class Dup: public Link {
    ~Dup();
public:
    Dup(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};

/*******************************************************/
class Split: public Link {
    ~Split();
public:
    Split(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};

/*******************************************************/
class Gate: public Link {
    ~Gate();
public:
    Gate(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};

/*******************************************************/
class Encode: public Link {
    ~Encode();
public:
    Encode(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};

/*******************************************************/
class GrpxSrc: public Link {
    ~GrpxSrc();
public:
    GrpxSrc(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};
/*******************************************************/
class IPCIn: public Link {
    string ipcName;
    ~IPCIn();

public:
    IPCIn(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};
/*******************************************************/
class IPCOut: public Link {
    string ipcName;
    ~IPCOut();

public:
    IPCOut(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};
/*******************************************************/
class Merge: public Link {
    ~Merge();
public:
    Merge(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID
};

/*******************************************************/
class NullSource: public Link {
    ~NullSource();
public:
    NullSource(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID
};
/*******************************************************/
class Null: public Link {
    ~Null();
public:
    Null(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID
};
/*******************************************************/
class Select: public Link {
    ~Select();
public:
    Select(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};
/*******************************************************/
class DrmDisplay: public Link {
    ~DrmDisplay();
public:
    DrmDisplay(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};
/*******************************************************/
class SgxDisplay: public Link {
    ~SgxDisplay();
public:
    SgxDisplay(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};

/*******************************************************/
class Sgx3Dsrv: public Link {
    ~Sgx3Dsrv();
public:
    Sgx3Dsrv(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID

};

/*******************************************************/
class Sync: public Link {
    ~Sync();
public:
    Sync(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj); //return QueueID
};
/*******************************************************/
class VPE: public Link {
    ~VPE();
public:
    VPE(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj);
};
/*******************************************************/
class Hcf: public Link {
    ~Hcf();
public:
    Hcf(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj);
};
/***********************************************************/
class DefLink: public Link {
    ~DefLink();
public:
    DefLink(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj); //returns QueueID
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_SubframeCopy: public Link {
    ~Alg_SubframeCopy();
public:
    Alg_SubframeCopy(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_RemapMerge: public Link {
    ~Alg_RemapMerge();
public:
    Alg_RemapMerge(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_StereoPostProcess: public Link {
    ~Alg_StereoPostProcess();
public:
    Alg_StereoPostProcess(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};
/*******************************************************/
class Alg_Crc: public Link {
    ~Alg_Crc();
public:
    Alg_Crc(string nm);
    void genIncludes(ostream &fp);
    void genLinkID(ostream &fp);
    void genCreatePrms(ostream &fp);
    void genResetLinkPrms(ostream &fp, string obj);
    void genSetLinkPrms(ostream &fp, string obj);

    int setInLink(Link* obj);
    int setOutLink(Link* obj);
};

#endif
