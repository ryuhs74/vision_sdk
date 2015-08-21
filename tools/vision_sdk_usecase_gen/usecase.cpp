#include <iomanip>
#include <ostream>

#include "usecase.h"


Usecase::Usecase() {
    for (int i = 0; i < OBJ_MAX; i++) {
        for (int j = 0; j < OBJ_MAX; j++) {
            matrix[i][j] = false;
            userMat[i][j] = false;
        }
    }
    numIpc = 0;
    fileName = "";
    structName = "";
    logFile = &cout;
}

void Usecase::initialize() {
    if (fileName.length() == 0)
        setFileName("out");
    cmd_options.setFileNames(fileName);
    logFile = cmd_options.log_File();
    assignCPU();
    createAllConn();
    setSequence();
    assignLinkID();
}

void Usecase::print() {
    if(cmd_options.verboseMode())
    {
        //printFileName(&cout);
        printExecSeq(&cout);
        //printTable(&cout);
    }
    if(cmd_options.write_tologFile())
    {
        printFileName(logFile);
        printExecSeq(logFile);
        printTable(logFile);
    }
}

void Usecase::genFiles() {
    if (cmd_options.write_toImage())
        genImgFile();
    if (cmd_options.write_toFile())
        genFile();
}

void Usecase::setFileName(string name) {
    fileName = name;
    structName = name + string("Obj");
}

string Usecase::getFileName() {
    return fileName;
}

void Usecase::setStructName(string name) {
    structName = name;
}

string Usecase::getStructName() {
    return structName;
}

Link* Usecase::getObject(string name) {
    map<string, Link*>::iterator it;
    it = inst_object.find(name);
    if (it == inst_object.end())
        return NULL;
    else
        return it->second;
}

Link* Usecase::createObject(string name) {
    Link* obj = getObject(name);
    if (obj != NULL)
        return obj;
    else {
        createNewObj(name, obj);
        temp_seq.push_back(obj);
        inst_object[name] = obj;
        obj->setMatrixPos(temp_seq.size() - 1);
        return obj;
    }
}

void Usecase::createNewObj(string name, Link* &obj) {
    string root = getRoot(name);
    if (root == "AvbRx")
        obj = new AVBReceive(name);
    else if (root == "Alg") {
        string sec = getSecRoot(name);
        if (sec == "ColorToGray")
            obj = new Alg_ColorToGray(name);
        else if (sec == "DenseOptFlow")
            obj = new Alg_DenseOptFlow(name);
        else if (sec == "DmaSwMs")
            obj = new Alg_DMASwMs(name);
        else if (sec == "EdgeDetect")
            obj = new Alg_EdgeDetect(name);
        else if (sec == "SoftIsp")
            obj = new Alg_SoftIsp(name);
        else if (sec == "IssAewb")
            obj = new Alg_IssAewb(name);
        else if (sec == "ObjectDetection")
            obj = new Alg_ObjectDetection(name);
        else if (sec == "FeaturePlaneComputation")
            obj = new Alg_FeaturePlaneComputation(name);
        else if (sec == "FrameCopy")
            obj = new Alg_FrameCopy(name);
        else if (sec == "MyAlgFinish")
            obj = new Alg_MyAlgFinish(name);
        else if (sec == "MyAlg1")
            obj = new Alg_MyAlg1(name);
        else if (sec == "MyAlg2")
            obj = new Alg_MyAlg2(name);
        else if (sec == "MyAlg3")
            obj = new Alg_MyAlg3(name);
        else if (sec == "Census")
            obj = new Alg_Census(name);
        else if (sec == "DisparityHamDist")
            obj = new Alg_DisparityHamDist(name);
        else if (sec == "UltrasonicFusion")
            obj = new Alg_UltrasonicFusion(name);
        else if (sec == "GeoAlign")
            obj = new Alg_GeoAlign(name);
        else if (sec == "ObjectDraw")
            obj = new Alg_ObjectDraw(name);
        else if (sec == "PhotoAlign")
            obj = new Alg_PhotoAlign(name);
        else if (sec == "Synthesis")
            obj = new Alg_Synthesis(name);
        else if (sec == "SparseOpticalFlow")
            obj = new Alg_SparseOpticalFlow(name);
        else if (sec == "SparseOpticalFlowDraw")
            obj = new Alg_SparseOpticalFlowDraw(name);
        else if (sec == "LaneDetect")
            obj = new Alg_LaneDetect(name);
        else if (sec == "LaneDetectDraw")
            obj = new Alg_LaneDetectDraw(name);
        else if (sec == "VectorToImage")
            obj = new Alg_VectoImg(name);
        else if (sec == "SubframeCopy")
            obj = new Alg_SubframeCopy(name);
        else if (sec == "RemapMerge")
            obj = new Alg_RemapMerge(name);
        else if (sec == "StereoPostProcess")
            obj = new Alg_StereoPostProcess(name);
        else if (sec == "Crc")
            obj = new Alg_Crc(name);
        else{
            obj = new Alg(name);
            cout << "Warning: Custom defined Algorithm plugin Link [" << sec << "] found !!!" << endl;
        }
    } else if (root == "Capture")
        obj = new Capture(name);
    else if (root == "IssCapture")
        obj = new IssCapture(name);
    else if (root == "IssM2mIsp")
        obj = new IssM2mIsp(name);
    else if (root == "IssM2mSimcop")
        obj = new IssM2mSimcop(name);
    else if (root == "UltrasonicCapture")
        obj = new UltrasonicCapture(name);
    else if (root == "Decode")
        obj = new Decode(name);
    else if (root == "DisplayCtrl")
        obj = new DisplayCtrl(name);
    else if (root == "Display")
        obj = new Display(name);
    else if (root == "Dup")
        obj = new Dup(name);
    else if (root == "Split")
        obj = new Split(name);
    else if (root == "Gate")
        obj = new Gate(name);
    else if (root == "Encode")
        obj = new Encode(name);
    else if (root == "GrpxSrc")
        obj = new GrpxSrc(name);
    else if (root == "IPCIn")
        obj = new IPCIn(name);
    else if (root == "IPCOut")
        obj = new IPCOut(name);
    else if (root == "Merge")
        obj = new Merge(name);
    else if (root == "Null")
            obj = new Null(name);
    else if (root == "NullSource")
        obj = new NullSource(name);
    else if (root == "Select")
        obj = new Select(name);
    else if (root == "Sync")
        obj = new Sync(name);
    else if (root == "SgxDisplay")
        obj = new SgxDisplay(name);
    else if (root == "DrmDisplay")
        obj = new DrmDisplay(name);
    else if (root == "Sgx3Dsrv")
        obj = new Sgx3Dsrv(name);
    else if (root == "VPE")
        obj = new VPE(name);
    else if (root == "Hcf")
        obj = new Hcf(name);
    else if (root == "DefLink")
        obj = new DefLink(name);
    else
        CHECK_ERROR_ABORT(false, "Error: Link [" + name + "] name does not match supported links. Use '-h' option to see list of supported links !!!");
}

void Usecase::assignCPU() {
    for (int i = 0; i < temp_seq.size(); i++)
        (temp_seq.at(i))->setProcType((temp_seq.at(i))->getProcType());
}

void Usecase::assignLinkID() {
    for (int i = 0; i < exec_seq.size(); i++)
        (exec_seq.at(i))->setLinkIDName();
}



/**For each head:
 * //if no parent || all parents assigned/visited
        //if not assigned,
        1. assign itself -- set all false

        2. find a child unassigned, call func assign on to child - child visited true
        3. if all child assigned, move assign parent

//if all parents not assigned
1. find a parent unassigned, unvisited call assign func on it -- visited mark true
 */

void Usecase::assignSeq(int curr, bool* asgn, bool* visited, bool* done) {

    int N = temp_seq.size(), parv = -1, parAsgn=-1, child = -1, par = -1;
    bool allAsgn = true;
    for (int i = 0; i < N; i++) {
        if(matrix[i][curr] == true && !(asgn[i]||visited[i])) //if parent
        {
            parAsgn = i;
            allAsgn = false;
        }
        else if(matrix[i][curr] == true && visited[i] && !asgn[i])
            parv = i;
    }


    if(allAsgn == true)
    {
        //if not visited yet
        if(visited[curr] == false)
        {
            if(parv == -1) //all parent assigned
            {
                //assign itself
                if(asgn[curr] == false)
                {
                    exec_seq.push_back(temp_seq.at(curr));
                    (temp_seq.at(curr))->setExecPos(exec_seq.size() - 1);
                    asgn[curr] = true;
                    for (int k = 0; k < N; k++)
                        visited[k] = false;
                }
                //find a child unassigned
                for (int i = 0; i < N; i++)
                    if(matrix[curr][i] == true && asgn[i] == false)
                        child = i;

                if(child != -1)
                    assignSeq(child, asgn, visited, done);
                else //if all child assign visit all parents again
                {
                    done[curr] = true;
                    for (int i = 0; i < N; i++) {
                        if(matrix[i][curr] == true && done[i] == false)
                            assignSeq(i, asgn, visited, done);
                    }
                }
            }
            else
            {
                visited[curr] = true;
                assignSeq(parv, asgn, visited, done);
            }
        }
        else
        {
            if(asgn[curr] == false)
            {
                //assign itself
                exec_seq.push_back(temp_seq.at(curr));
                (temp_seq.at(curr))->setExecPos(exec_seq.size() - 1);
                asgn[curr] = true;
                for (int k = 0; k < N; k++)
                    visited[k] = false;
            }

            //find a child unassigned
            for (int i = 0; i < N; i++)
                if(matrix[curr][i] == true && asgn[i] == false)
                    child = i;

            if(child != -1)
                assignSeq(child, asgn, visited, done);
            else //if all child assign visit all parents again
            {
                done[curr] = true;
                for (int i = 0; i < N; i++) {
                    if(matrix[i][curr] == true && done[i] == false)
                        assignSeq(i, asgn, visited, done);
                }
            }
        }
    }
    else
    {
        visited[curr] = true;
        assignSeq(parAsgn, asgn, visited, done);
    }
}

void Usecase::setSequence() {
    //All links with no incoming link is assigned sequence number
    int N = temp_seq.size();
    bool asgn[N], visited[N], done[N];
    bool head = true;

    for (int i = 0; i < N; i++) {
        asgn[i] = false;
        visited[i] = false;
        done[i] = false;
    }

    for (int i = 0; i < N; i++) {
        head = true;
        //checks if any incoming link i.e any link with seq num not assigned
        for (int j = 0; j < N; j++) {
            if (matrix[j][i] == true) {
                head = false;
                break;
            }
        }
        if (head == true) { //For each head
            assignSeq(i, asgn, visited, done);
        }
    }
}

void Usecase::printFileName(ostream* out) {
    (*out) << "UseCase Name: " << fileName << endl;
}

void Usecase::printMatrix(ostream* out) {
    (*out) << "*******" << endl;
    for (int i = 0; i < temp_seq.size(); i++)
        (*out) << (temp_seq.at(i))->getName() << " "
                << (temp_seq.at(i))->getMatrixPos() << endl;

    for (int i = 0; i < temp_seq.size(); i++) {
        for (int j = 0; j < temp_seq.size(); j++)
            (*out) << matrix[i][j] << " ";
        (*out) << endl;
    }
}

void Usecase::connect(Link* obj1, Link* obj2) {
    if (obj2 != NULL) {
        int pos1 = obj1->getMatrixPos();
        int pos2 = obj2->getMatrixPos();

        if (userMat[pos1][pos2] == false) // Not already connected
        {
            //cout<<obj1->getProcType()<<" "<<obj2->getProcType()<<endl;
            if ((obj1->getProcType() == obj2->getProcType()) ||
                    obj2->getClassType() == cAlg_SubframeCopy ||
                    ((obj1->getClassType() == cIPCOut)
                            && (obj2->getClassType()) == cIPCIn)) { //If same processor types, or if both links are IPC's

                if(cmd_options.verboseMode())
                    cout << obj1->getName() << " is connected to "
                        << obj2->getName() << endl;

                if(cmd_options.write_tologFile())
                    *logFile << obj1->getName() << " is connected to "
                        << obj2->getName() << endl;

                int i = 0, pos1 = -1, pos2 = -1;
                while ((pos1 == -1 || pos2 == -1) && i < temp_seq.size()) {
                    if (temp_seq.at(i) == obj1)
                        pos1 = i;
                    else if (temp_seq.at(i) == obj2)
                        pos2 = i;
                    i++;
                }

                int q1 = obj1->setOutLink(obj2);
                int q2 = obj2->setInLink(obj1);
                obj1->setOutQueueID(q1, q2);
                obj2->setInQueueID(q2, q1);

                matrix[pos1][pos2] = true;
                userMat[pos1][pos2] = true;

                if ((obj1->getClassType() == cIPCOut)
                        && (obj2->getClassType() == cIPCIn)) {
                    obj2->setName(
                            "IPCIn_" + procName[obj2->getProcType()] + "_"
                                    + procName[obj1->getProcType()] + "_"
                                    + toString(obj2->getProcID()));
                    obj1->setName(
                            "IPCOut_" + procName[obj1->getProcType()] + "_"
                                    + procName[obj2->getProcType()] + "_"
                                    + toString(obj1->getProcID()));
                }
            } else {
                if(cmd_options.verboseMode())
                    cout << obj1->getName() << ", " << obj2->getName()
                        << ": On different Processor, Intermediate IPCs needed"
                        << endl;
                if(cmd_options.write_tologFile())
                    (*logFile) << obj1->getName() << ", " << obj2->getName()
                        << ": On different Processor, Intermediate IPCs needed"
                        << endl;


                userMat[pos1][pos2] = true; //Mark link as connected(user sees them as connected, even though intermediate IPC's are needed)

                //Create to ipc links
                if (getRoot(obj1->getName()) != "IPCOut"
                        && getRoot(obj2->getName()) != "IPCIn") //not either one is IPC
                {
                    string ipc = "IPCOut_" + toString(numIpc);
                    numIpc++;
                    string ipc2 = "IPCIn_" + toString(numIpc);
                    numIpc++;

                    Link* IPC1 = createObject(ipc);
                    IPC1->setProcType(obj1->getProcType());
                    Link* IPC2 = createObject(ipc2);
                    IPC2->setProcType(obj2->getProcType());

                    connect(obj1, IPC1);
                    connect(IPC1, IPC2);
                    connect(IPC2, obj2);

                } else if (getRoot(obj1->getName()) == "IPCOut") {
                    string ipc2 = "IPCIn_" + toString(numIpc);
                    numIpc++;

                    Link* IPC2 = createObject(ipc2);
                    IPC2->setProcType(obj2->getProcType());

                    connect(obj1, IPC2);
                    connect(IPC2, obj2);
                } else if (getRoot(obj2->getName()) == "IPCIn") {
                    string ipc = "IPCOut_" + toString(numIpc); // Some no
                    numIpc++;

                    Link* IPC1 = createObject(ipc);
                    IPC1->setProcType(obj1->getProcType());

                    connect(obj1, IPC1);
                    connect(IPC1, obj2);
                }
            }
        }
    }
}

void Usecase::genImgFile() {
    ostream& fp = *(cmd_options.write_Image());

    fp << "digraph " << fileName << " {\n" << endl;
    //Nodes
    /*
     * Color Code:
     * IPU1_0 : lightblue
     * IPU1_1 : darkturquoise
     * A15    : lightsalmon
     * DSP1   : palegreen
     * DSP2   : darkolivegreen1
     * EVE1   : yellow
     * EVE2   : gold
     * EVE3   : orange
     * EVE4   : goldenrod4
     */
    //TODO: Mention in documentation change color here
    //Colour Scheme
    fp << BLOCK_SPACE << "ColorScheme [shape=none, margin=0, label=<\n";
    fp << BLOCK_SPACE
            << "<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";

    if (proc.getObjsAssgn(IPU1_0))
        fp << BLOCK_SPACE << "<TR><TD bgcolor=\"lightblue\">IPU1_0</TD></TR>\n";
    if (proc.getObjsAssgn(IPU1_1))
        fp << BLOCK_SPACE
                << "<TR><TD bgcolor=\"darkturquoise\">IPU1_1</TD></TR>\n";
    if (proc.getObjsAssgn(DSP1))
        fp << BLOCK_SPACE << "<TR><TD bgcolor=\"palegreen\">DSP1</TD></TR>\n";
    if (proc.getObjsAssgn(DSP2))
        fp << BLOCK_SPACE
                << "<TR><TD bgcolor=\"darkolivegreen1\">DSP2</TD></TR>\n";
    if (proc.getObjsAssgn(EVE1))
        fp << BLOCK_SPACE << "<TR><TD bgcolor=\"yellow\">EVE1</TD></TR>\n";
    if (proc.getObjsAssgn(EVE2))
        fp << BLOCK_SPACE << "<TR><TD bgcolor=\"gold\">EVE2</TD></TR>\n";
    if (proc.getObjsAssgn(EVE3))
        fp << BLOCK_SPACE << "<TR><TD bgcolor=\"orange\">EVE3</TD></TR>\n";
    if (proc.getObjsAssgn(EVE4))
        fp << BLOCK_SPACE << "<TR><TD bgcolor=\"goldenrod4\">EVE4</TD></TR>\n";
    if (proc.getObjsAssgn(A15))
        fp << BLOCK_SPACE << "<TR><TD bgcolor=\"lightsalmon\">A15</TD></TR>\n";

    fp << BLOCK_SPACE << "</TABLE>>];\n" << endl;

    //Links
    if (exec_seq.size())
        fp << BLOCK_SPACE
                << "/************** LINKS ************************/\n";
    for (int i = 0; i < exec_seq.size(); i++) {
        ProcType pType = (exec_seq.at(i))->getProcType();
        if (pType == IPU1_0)
            fp << BLOCK_SPACE << (exec_seq.at(i))->getName()
                    << " [color=lightblue, style=filled]" << endl; //shape=box
        else if (pType == IPU1_1)
            fp << BLOCK_SPACE << (exec_seq.at(i))->getName()
                    << " [color=darkturquoise, style=filled]" << endl;
        else if (pType == A15)
            fp << BLOCK_SPACE << (exec_seq.at(i))->getName()
                    << " [color=lightsalmon, style=filled]" << endl;
        else if (pType == DSP1)
            fp << BLOCK_SPACE << (exec_seq.at(i))->getName()
                    << " [color=palegreen, style=filled]" << endl;
        else if (pType == DSP2)
            fp << BLOCK_SPACE << (exec_seq.at(i))->getName()
                    << " [color=darkolivegreen1, style=filled]" << endl;
        else if (pType == EVE1)
            fp << BLOCK_SPACE << (exec_seq.at(i))->getName()
                    << " [color=yellow, style=filled]" << endl;
        else if (pType == EVE2)
            fp << BLOCK_SPACE << (exec_seq.at(i))->getName()
                    << " [color=gold, style=filled]" << endl;
        else if (pType == EVE3)
            fp << BLOCK_SPACE << (exec_seq.at(i))->getName()
                    << " [color=orange, style=filled]" << endl;
        else if (pType == EVE4)
            fp << BLOCK_SPACE << (exec_seq.at(i))->getName()
                    << " [color=goldenrod4, style=filled]" << endl;
    }
    fp << endl;

    //Connections
    fp << BLOCK_SPACE
            << "/************** CONNECTIONS ************************/\n";
    for (int i = 0; i < temp_seq.size(); i++) {
        for (int j = 0; j < temp_seq.size(); j++) {
            if (matrix[i][j] == true) {
                //[taillabel=Q0, headlabel=Q0, minlen=2, labeldistance=2]
                if ((temp_seq.at(i))->getOutLinkSize() > 1
                        && (temp_seq.at(j))->getInLinkSize() > 1)
                    fp << BLOCK_SPACE << (temp_seq.at(i))->getName() << " -> "
                            << (temp_seq.at(j))->getName() << "[headlabel=Q"
                            << (temp_seq.at(j))->getInQueueID(temp_seq.at(i))
                            << ", taillabel=Q"
                            << (temp_seq.at(i))->getOutQueueID(temp_seq.at(j))
                            << ", minlen=2, labeldistance=3]" << endl;
                else if ((temp_seq.at(i))->getOutLinkSize() > 1)
                    fp << BLOCK_SPACE << (temp_seq.at(i))->getName() << " -> "
                            << (temp_seq.at(j))->getName() << "[taillabel=Q"
                            << (temp_seq.at(i))->getOutQueueID(temp_seq.at(j))
                            << ", minlen=2, labeldistance=3]" << endl;
                else if ((temp_seq.at(j))->getInLinkSize() > 1)
                    fp << BLOCK_SPACE << (temp_seq.at(i))->getName() << " -> "
                            << (temp_seq.at(j))->getName() << "[headlabel=Q"
                            << (temp_seq.at(j))->getInQueueID(temp_seq.at(i))
                            << " minlen=2, labeldistance=3]" << endl;
                else
                    fp << BLOCK_SPACE << (temp_seq.at(i))->getName() << " -> "
                            << (temp_seq.at(j))->getName() << endl;
            }
        }
    }

    fp << "}" << endl;

    cmd_options.close_Image();
    string outputPath = cmd_options.get_output_path_name();
    //TODO: Mention in documentation requirements
    string cmd = "dot -Tjpg " + outputPath + fileName + "_img.txt -o "+outputPath+ fileName + ".jpg";
    system(cmd.c_str());
}

void Usecase::genFile() {

    ostream& fp = *(cmd_options.write_File());
    ostream& fph = *(cmd_options.write_Header());
    initFiles(fp, fph);
    genIncludes(fp, fph);
    genStruct(fp, fph);
    genSetLinkID(fp, fph);
    genResetLinkPrms(fp, fph);
    genSetLinkPrms(fp, fph);
    genConnectLinks(fp, fph);
    genCreate(fp, fph);
    genStart(fp, fph);
    genStop(fp, fph);
    genDelete(fp, fph);
    genBufferStatistics(fp, fph);
    genStatistics(fp, fph);
    endFiles(fp, fph);

}

void Usecase::initFiles(ostream& fp, ostream& fph)
{
    string headcmmt = "\
/*\n\
*******************************************************************************\n\
*\n\
* Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/\n\
* ALL RIGHTS RESERVED\n\
*\n\
*******************************************************************************\n\
*/\n\
\n\
/*\n\
*******************************************************************************\n\
*\n\
* IMPORTANT NOTE:\n\
*  This file is AUTO-GENERATED by Vision SDK use case generation tool\n\
*\n\
*******************************************************************************\n\
*/";

    fp<<headcmmt<<endl;
    fph<<headcmmt<<endl;

    fph << endl;
    fph<<"#ifndef _"<<fileName<<"_H_"<<endl;
    fph<<"#define _"<<fileName<<"_H_"<<endl;
    fph << endl;
    fp<<"#include \""<<fileName<<"_priv.h\""<<endl;

}

void Usecase::genIncludes(ostream& fp, ostream& fph)
{
    bool unique;
    fph<<"#include <include/link_api/system.h>"<<endl;
    for (int i = 0; i < exec_seq.size(); i++)
    {
        unique = true;
        for (int j = i+1; j < exec_seq.size(); j++)
        {
            if((exec_seq.at(i))->getClassType() == (exec_seq.at(j))->getClassType())
                unique = false;
        }
        if(unique)
            (exec_seq.at(i))->genIncludes(fph);
    }
}

void Usecase::genStruct(ostream& fp, ostream& fph) {
    fph << "\n";
    fph << "typedef struct {" << endl;
    for (int i = 0; i < exec_seq.size(); i++)
        (exec_seq.at(i))->genLinkID(fph);
    fph << "\n";
    for (int i = 0; i < exec_seq.size(); i++)
        (exec_seq.at(i))->genCreatePrms(fph);
    fph << "} " << structName << ";" << endl;
    fph << "\n";
}

void Usecase::genSetLinkID(ostream& fp, ostream& fph) {
    string obj = "pObj";
    fph << "Void " << fileName << "_SetLinkId(" << structName << " *" << obj
            << ");\n" << endl; //header line
    fp << "Void " << fileName << "_SetLinkId(" << structName << " *" << obj
            << "){" << endl;
    for (int i = 0; i < exec_seq.size(); i++)
        (exec_seq.at(i))->genSetLinkID(fp, obj);
    fp << "}\n" << endl;
}

void Usecase::genResetLinkPrms(ostream& fp, ostream& fph) {
    string obj = "pObj";
    fph << "Void " << fileName << "_ResetLinkPrms(" << structName << " *" << obj
            << ");\n" << endl; //header line
    fp << "Void " << fileName << "_ResetLinkPrms(" << structName << " *" << obj
            << "){" << endl;
    for (int i = 0; i < exec_seq.size(); i++)
        (exec_seq.at(i))->genResetLinkPrms(fp, obj);
    fp << "}\n" << endl;
}

void Usecase::genSetLinkPrms(ostream& fp, ostream& fph)
{
    string obj = "pObj";
    fph << "Void " << fileName << "_SetPrms(" << structName << " *" << obj
            << ");\n" << endl; //header line
    fp << "Void " << fileName << "_SetPrms(" << structName << " *" << obj
            << "){" << endl;
    for (int i = 0; i < exec_seq.size(); i++)
        (exec_seq.at(i))->genSetLinkPrms(fp, obj);
    fp << "}\n" << endl;
}

void Usecase::genConnectLinks(ostream& fp, ostream& fph) {
    string obj = "pObj";
    fph << "Void " << fileName << "_ConnectLinks(" << structName << " *" << obj
            << ");\n" << endl; //header line
    fp << "Void " << fileName << "_ConnectLinks(" << structName << " *" << obj
            << "){\n" << endl;
    for (int i = 0; i < exec_seq.size(); i++) {
        Link* A = exec_seq.at(i);
        vector<pair<Link*, int> >* outLinkPtr =
                (exec_seq.at(i))->getOutLinkPtr();
        for (int j = 0; j < (outLinkPtr->size()); j++) {
            Link* B = (outLinkPtr->at(j)).first;
            int num = (outLinkPtr->at(j)).second;

            fp << BLOCK_SPACE << "//" << A->getName() << " -> " << B->getName()
                    << endl;
            if(A->getMulOutQue())
                fp << BLOCK_SPACE << obj << "->" << A->getPrmName()
                        << ".outQueParams["<<j<<"].nextLink = " << obj << "->"
                        << B->getLinkName() << ";\n";
            else
                fp << BLOCK_SPACE << obj << "->" << A->getPrmName()
                                    << ".outQueParams.nextLink = " << obj << "->"
                                    << B->getLinkName() << ";\n";
            if(B->getMulInQue())
            {
                fp << BLOCK_SPACE << obj << "->" << B->getPrmName()
                        << ".inQueParams["<<num<<"].prevLinkId = " << obj << "->"
                        << A->getLinkName() << ";\n";
                fp << BLOCK_SPACE << obj << "->" << B->getPrmName()
                        << ".inQueParams["<<num<<"].prevLinkQueId = " << j << ";\n\n";
            }
            else
            {
                fp << BLOCK_SPACE << obj << "->" << B->getPrmName()
                        << ".inQueParams.prevLinkId = " << obj << "->"
                        << A->getLinkName() << ";\n";
                fp << BLOCK_SPACE << obj << "->" << B->getPrmName()
                        << ".inQueParams.prevLinkQueId = " << j << ";\n\n";
            }
        }
    }
    fp << "}\n" << endl;
}

void Usecase::genCreate(ostream& fp, ostream& fph) {
    string obj = "pObj";
    fph << "Int32 " << fileName << "_Create(" << structName << " *" << obj
            << ", Void *appObj);\n" << endl; //header line
    fp << "Int32 " << fileName << "_Create(" << structName << " *" << obj
            << ", Void *appObj){" << endl;
    fp << "\n"<<BLOCK_SPACE <<"Int32 status;\n" << endl;
    fp << BLOCK_SPACE << fileName << "_SetLinkId(" << obj << ");\n";
    fp << BLOCK_SPACE << fileName << "_ResetLinkPrms(" << obj << ");\n";
    fp<<endl;
    fp << BLOCK_SPACE << fileName << "_SetPrms(" << obj << ");\n";
    fp << BLOCK_SPACE << fileName << "_SetAppPrms(" << obj << ", appObj);\n";
    fp<<endl;
    fp << BLOCK_SPACE << fileName << "_ConnectLinks(" << obj << ");\n";

    for (int i = 0; i < exec_seq.size(); i++) {
        (exec_seq.at(i))->genCreate(fp, obj);
        fp << endl;
    }

    fp << BLOCK_SPACE << "return status;"<<endl;
    fp << "}\n" << endl;
}

void Usecase::genStart(ostream& fp, ostream& fph) {
    //in reverse Order
    string obj = "pObj";
    fph << "Int32 " << fileName << "_Start(" << structName << " *" << obj
            << ");\n" << endl; //header line
    fp << "Int32 " << fileName << "_Start(" << structName << " *" << obj << "){"
            << endl;
    fp << "\n"<<BLOCK_SPACE <<"Int32 status;\n" << endl;
    for (int i = exec_seq.size() - 1; i >= 0; i--) {
        (exec_seq.at(i))->genStart(fp, obj);
        fp << endl;
    }

    fp << BLOCK_SPACE << "return status;"<<endl;
    fp << "}\n" << endl;
}

void Usecase::genStop(ostream& fp, ostream& fph) {
    //in reverse Order
    string obj = "pObj";
    fph << "Int32 " << fileName << "_Stop(" << structName << " *" << obj
            << ");\n" << endl; //header line
    fp << "Int32 " << fileName << "_Stop(" << structName << " *" << obj << "){"
            << endl;
    fp << "\n"<<BLOCK_SPACE <<"Int32 status;\n" << endl;
    for (int i = exec_seq.size() - 1; i >= 0; i--) {
        (exec_seq.at(i))->genStop(fp, obj);
        fp << endl;
    }

    fp << BLOCK_SPACE << "return status;"<<endl;
    fp << "}\n" << endl;
}

void Usecase::genDelete(ostream& fp, ostream& fph) {
    //in reverse Order
    string obj = "pObj";
    fph << "Int32 " << fileName << "_Delete(" << structName << " *" << obj
            << ");\n" << endl; //header line
    fp << "Int32 " << fileName << "_Delete(" << structName << " *" << obj << "){"
            << endl;
    fp << "\n"<<BLOCK_SPACE <<"Int32 status;\n" << endl;
    for (int i = exec_seq.size() - 1; i >= 0; i--) {
        (exec_seq.at(i))->genDelete(fp, obj);
        fp << endl;
    }

    fp <<BLOCK_SPACE << "return status;"<<endl;
    fp << "}\n" << endl;
}

void Usecase::genBufferStatistics(ostream& fp, ostream& fph)
{
    string obj = "pObj";
    fph << "Void " << fileName << "_printBufferStatistics(" << structName << " *" << obj
            << ");\n" << endl; //header line
    fp << "Void " << fileName << "_printBufferStatistics(" << structName << " *" << obj
            << "){" << endl;
    for (int i = 0; i < exec_seq.size(); i++)
    {
        (exec_seq.at(i))->genBufferStatistics(fp, obj);
        if((i==exec_seq.size()-1) || ((exec_seq.at(i)->getProcType() != (exec_seq.at(i+1))->getProcType())))
            fp << BLOCK_SPACE <<"Task_sleep(500);" <<endl;
    }

    fp << "}\n" << endl;
}

void Usecase::genStatistics(ostream& fp, ostream& fph)
{
    string obj = "pObj";
    fph << "Void " << fileName << "_printStatistics(" << structName << " *" << obj
            << ");\n" << endl; //header line
    fp << "Void " << fileName << "_printStatistics(" << structName << " *" << obj
            << "){" << endl;
    for (int i = 0; i < exec_seq.size(); i++)
    {
        (exec_seq.at(i))->genStatistics(fp, obj);
        if((i==exec_seq.size()-1) || ((exec_seq.at(i)->getProcType() != (exec_seq.at(i+1))->getProcType())))
            fp << BLOCK_SPACE <<"Task_sleep(500);" <<endl;
    }
    fp << "}\n" << endl;
}

void Usecase::endFiles(ostream& fp, ostream& fph)
{
    fph<<"Void "<<fileName<<"_SetAppPrms("<<structName<<" *pObj, Void *appObj);\n"<<endl;
    fph<<"#endif /* _"<<fileName<<"_H_ */"<<endl;
}

void Usecase::setNewConn(vector<Link*>* vec) {
    connections.push_back(*vec);
}

void Usecase::createAllConn() {
    int sz = connections.size();
    for (int i = 0; i < sz; i++) {
        int vsz = (connections.at(i)).size();
        for (int j = vsz - 2; j >= 0; j--) {
            connect((connections.at(i)).at(j + 1), (connections.at(i)).at(j));
            //cout<<((connections.at(i)).at(j+1))->getName()<<" "<<((connections.at(i)).at(j))->getName()<<endl;
        }
    }
}

void Usecase::printExecSeq(ostream* out)
{
    (*out) << "\n********" << endl;
    (*out) << "Execution Sequence: " << endl;
    for (int i = 0; i < exec_seq.size(); i++)
        (*out) << (exec_seq.at(i))->getName() << endl;
}

void Usecase::printTable(ostream* out) {

    (*out) << endl;
    (*out) << "***********" << endl;
    (*out) << "Name\t    CPU\t       LinkType\t  INSTNum" << endl;
    for (int i = 0; i < exec_seq.size(); i++) {
        (*out) << setw(10) << left << (exec_seq.at(i))->getName() << "  ";
        (*out) << setw(10) << left << procName[(exec_seq.at(i))->getProcType()]
                << "  ";
        (*out) << setw(10) << left << getRoot((exec_seq.at(i))->getName())
                << "  ";
        (*out) << setw(10) << left << (exec_seq.at(i))->getProcID() << endl;
        //print each object's inLink and Outlink along with Queue ID
        (*out) << "InLink: " << endl;
        (exec_seq.at(i))->printInLink(*out);
        (*out) << "OutLink: " << endl;
        (exec_seq.at(i))->printOutLink(*out);
        (*out) << "--------------------------------------" << endl;
    }
}
