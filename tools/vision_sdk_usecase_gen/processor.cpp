#include "processor.h"
#include "error.h"
#include "support.h"

Processor::Processor() {
    for (int i = 0; i < NUMPROC; i++)
        for (int j = 0; j < ClassCount; j++)
            objsAsgn[i][j] = 0;
    for (int i = 0; i < NUMPROC; i++)
        objCount[i] = 0;
    for (int i = 0; i < ClassCount; i++)
        linkIDAsgn[i] = 0;
}

int Processor::getProcID(ProcType pType, ClassType cType, string name) {
    int num;
    switch (cType) {
        case cAVBReceive:
            if(!(pType == IPU1_1 || pType == IPU1_0 || pType == A15))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[IPU1_0, IPU1_1, A15])");
            break;

        case cDecode:
        case cDisplayCtrl:
        case cDisplay:
        case cEncode:
        case cGrpxSrc:
        case cVPE:
        case cHcf:
        case cCapture:
            if(!(pType == IPU1_0))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[IPU1_0]) !!!");
            break;
        case cIssCapture:
            if(!(pType == IPU1_0))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[IPU1_0]) !!!");
            break;
        case cIssM2mIsp:
            if(!(pType == IPU1_0))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[IPU1_0]) !!!");
            break;
        case cIssM2mSimcop:
            if(!(pType == IPU1_0))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[IPU1_0]) !!!");
            break;
        case cUltrasonicCapture:
            if(!(pType == IPU1_0))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[IPU1_0]) !!!");
            break;

        case cSgxDisplay:
            if(!(pType == A15))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[A15]) !!!");
            break;
        case cDrmDisplay:
            if(!(pType == A15))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[A15]) !!!");
            break;
        case cSgx3Dsrv:
            if(!(pType == A15))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[A15]) !!!");
            break;

        case cAlg_ColorToGray:
            if(!(pType == DSP1 || pType == DSP2))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[DSP1, DSP2]) !!!");
            break;
        case cAlg_DMASwMs:
            if(!(pType == DSP1 || pType == DSP2 || pType == A15 || pType == IPU1_0 || pType == IPU1_1))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[DSP1, DSP2, A15, IPU1_0, IPU1_1]) !!!");
            break;
        case cAlg_DenseOptFlow:
            if(!(pType == EVE1 || pType == EVE2 || pType == EVE3 || pType == EVE4))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[EVE1, EVE2, EVE3, EVE4]) !!!");
            break;
        case cAlg_EdgeDetect:
            if(!(pType == EVE1 || pType == EVE2 || pType == EVE3 || pType == EVE4))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[EVE1, EVE2, EVE3, EVE4]) !!!");
            break;
        case cAlg_SoftIsp:
            if(!(pType == EVE1 || pType == EVE2 || pType == EVE3 || pType == EVE4))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[EVE1, EVE2, EVE3, EVE4]) !!!");
            break;
        case cAlg_IssAewb:
            if(!(pType == IPU1_0))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[IPU1_0]) !!!");
            break;

        case cAlg_FrameCopy:
            if(!(pType == DSP1 || pType == DSP2 || pType == A15 || pType == EVE1 || pType == EVE2 || pType == EVE3 || pType == EVE4))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[DSP1, DSP2, A15, EVE1, EVE2, EVE3, EVE4]) !!!");
            break;

        case cAlg_MyAlgFinish:
            if(!(pType == IPU1_0))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[IPU1_0]) !!!");
            break;

        case cAlg_MyAlg1:
        case cAlg_MyAlg2:
        case cAlg_MyAlg3:
            if(!(pType == IPU1_0 || pType == IPU1_1 || pType == DSP1 || pType == DSP2 || pType == A15 || pType == EVE1 || pType == EVE2 || pType == EVE3 || pType == EVE4))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[DSP1, DSP2, A15, EVE1, EVE2, EVE3, EVE4, IPU1_0, IPU1_1]) !!!");
            break;

        case cAlg_Census:
            if(!(pType == EVE1 || pType == EVE2 || pType == EVE3 || pType == EVE4))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[EVE1, EVE2, EVE3, EVE4]) !!!");
            break;
        case cAlg_DisparityHamDist:
            if(!(pType == EVE1 || pType == EVE2 || pType == EVE3 || pType == EVE4))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[EVE1, EVE2, EVE3, EVE4]) !!!");
            break;
        case cAlg_UltrasonicFusion:
            if(!(pType == DSP1 || pType == DSP2))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[DSP1, DSP2]) !!!");
            break;
        case cAlg_FeaturePlaneComputation:
            if(!(pType == EVE1 || pType == EVE2 || pType == EVE3 || pType == EVE4))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[EVE1, EVE2, EVE3, EVE4]) !!!");
            break;
        case cAlg_ObjectDetection:
            if(!(pType == DSP1 || pType == DSP2))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[DSP1, DSP2]) !!!");
            break;
        case cAlg_GeoAlign:
            if(!(pType == DSP1 || pType == DSP2))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[DSP1, DSP2]) !!!");
            break;
        case cAlg_ObjectDraw:
            if(!(pType == IPU1_0 || pType == IPU1_1))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[IPU1_0, IPU1_1]) !!!");
            break;
        case cAlg_PhotoAlign:
            if(!(pType == DSP1 || pType == DSP2))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[DSP1, DSP2]) !!!");
            break;
        case cAlg_Synthesis:
            if(!(pType == DSP1 || pType == DSP2))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[DSP1, DSP2]) !!!");
            break;
        case cAlg_VectoImg:
            if(!(pType == DSP1 || pType == DSP2))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[DSP1, DSP2]) !!!");
            break;
        case cAlg_SubframeCopy:
            if(!(pType == EVE1 || pType == EVE2 || pType == EVE3 || pType == EVE4))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[EVE1, EVE2, EVE3, EVE4]) !!!");
            break;
        case cAlg_RemapMerge:
            if(!(pType == EVE1 || pType == EVE2 || pType == EVE3 || pType == EVE4))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[EVE1, EVE2, EVE3, EVE4]) !!!");
            break;
        case cAlg_StereoPostProcess:
            if(!(pType == DSP1 || pType == DSP2))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[DSP1, DSP2]) !!!");
            break;
        case cAlg_Crc:
            if(!(pType == IPU1_0))
                CHECK_ERROR(SHOULD_NOT_REACH, "Error: Link [" + name + "] cannot be assigned to CPU [" + procName[pType] + "] (Valid CPUs:[IPU1_0]) !!!");
            break;

    }
    if (getRoot(name) == "Alg")
        cType = cAlg;
    num = objsAsgn[pType][cType];
    objsAsgn[pType][cType]++;
    objCount[pType]++;
    return num;
}

int Processor::getObjsAssgn(ProcType pType) {
    return objCount[pType];
}

string Processor::getLinkID(ProcType pType, ClassType cType, int procID, string name) //TODO: Confirm this is the way
{
    string linkIDName;
    string pName = procName[pType];
    switch (cType) {
    case cAVBReceive:
        CHECK_ERROR_ABORT(linkIDAsgn[cType] < 1,
                        "Error: Link [AbvRx] cannot have more than one instance !!!")
                ;
        linkIDName = pName + string("_LINK (SYSTEM_LINK_ID_AVB_RX)");
        linkIDAsgn[cType]++;
        break;
    case cAlg:
        linkIDName = pName + string("_LINK (SYSTEM_LINK_ID_ALG_")
                + toString(procID) + string(")"); //linkIDAsgn[cType]
        linkIDAsgn[cType]++;
        break;
    case cCapture:
        CHECK_ERROR_ABORT(linkIDAsgn[cType] < 2,
                "Error: Link [Capture] cannot have more than two instance !!!")
        ;
        linkIDName = string("SYSTEM_LINK_ID_CAPTURE_")
                + toString(linkIDAsgn[cType]);
        linkIDAsgn[cType]++;
        break;
    case cIssCapture:
        linkIDName = string("SYSTEM_LINK_ID_ISSCAPTURE_")
                + toString(procID); //linkIDAsgn[cType]
        linkIDAsgn[cType]++;
        break;
    case cIssM2mIsp:
        linkIDName = string("SYSTEM_LINK_ID_ISSM2MISP_")
                + toString(procID); //linkIDAsgn[cType]
        linkIDAsgn[cType]++;
        break;
    case cIssM2mSimcop:
        linkIDName = string("SYSTEM_LINK_ID_ISSM2MSIMCOP_")
                + toString(procID); //linkIDAsgn[cType]
        linkIDAsgn[cType]++;
        break;
    case cUltrasonicCapture:
        CHECK_ERROR_ABORT(linkIDAsgn[cType] < 1,
                "Error: Link [UltrasonicCapture] cannot have more than one instance !!!")
        ;
        linkIDName = "SYSTEM_LINK_ID_ULTRASONIC_CAPTURE";
        linkIDAsgn[cType]++;
        break;

    case cDecode:
        linkIDName = string("SYSTEM_LINK_ID_VDEC_")
                + toString(linkIDAsgn[cType]); //NO CHECK
        linkIDAsgn[cType]++;
        break;
    case cEncode:
        linkIDName = string("SYSTEM_LINK_ID_VENC_")
                + toString(linkIDAsgn[cType]); //NO CHECK
        linkIDAsgn[cType]++;
        break;
    case cDisplayCtrl:
        CHECK_ERROR_ABORT(linkIDAsgn[cType] < 1,
                "Error: Link [DisplayCtrl] cannot have more than one instance !!!")
        ;
        linkIDName = string("SYSTEM_LINK_ID_DISPLAYCTRL")
                + toString(linkIDAsgn[cType]); //NO CHECK
        linkIDAsgn[cType]++;
        break;
    case cDisplay:
        linkIDName = string("SYSTEM_LINK_ID_DISPLAY_")
                + toString(linkIDAsgn[cType]);
        linkIDAsgn[cType]++;
        break;
    case cDup:
        linkIDName = pName + string("_LINK (SYSTEM_LINK_ID_DUP_")
                + toString(procID) + string(")");
        linkIDAsgn[cType]++;
        break;
    case cSplit:
        linkIDName = pName + string("_LINK (SYSTEM_LINK_ID_SPLIT_")
                + toString(procID) + string(")");
        linkIDAsgn[cType]++;
        break;
    case cGate:
        linkIDName = pName + string("_LINK (SYSTEM_LINK_ID_GATE_")
                + toString(procID) + string(")");
        linkIDAsgn[cType]++;
        break;
    case cGrpxSrc:
        linkIDName = pName + string("_LINK (SYSTEM_LINK_ID_GRPX_SRC_")
                + toString(procID) + string(")");
        linkIDAsgn[cType]++;
        break;
    case cIPCIn:
        linkIDName = pName + string("_LINK (SYSTEM_LINK_ID_IPC_IN_")
                + toString(procID) + string(")");
        linkIDAsgn[cType]++;
        break;
    case cIPCOut:
        linkIDName = pName + string("_LINK (SYSTEM_LINK_ID_IPC_OUT_")
                + toString(procID) + string(")");
        linkIDAsgn[cType]++;
        break;
    case cMerge:
        linkIDName = pName + string("_LINK (SYSTEM_LINK_ID_MERGE_")
                + toString(procID) + string(")");
        linkIDAsgn[cType]++;
        break;
    case cNull:
        linkIDName = pName + string("_LINK (SYSTEM_LINK_ID_NULL_")
                + toString(procID) + string(")");
        linkIDAsgn[cType]++;
        break;
    case cNullSource:
        linkIDName = pName + string("_LINK (SYSTEM_LINK_ID_NULL_SRC_")
                + toString(procID) + string(")");
        linkIDAsgn[cType]++;
        break;
    case cSelect:
        linkIDName = pName + string("_LINK (SYSTEM_LINK_ID_SELECT_")
                + toString(procID) + string(")");
        linkIDAsgn[cType]++;
        break;
    case cSync:
        linkIDName = pName + string("_LINK (SYSTEM_LINK_ID_SYNC_")
                + toString(procID) + string(")");
        linkIDAsgn[cType]++;
        break;
    case cSgxDisplay:
        linkIDName = "SYSTEM_LINK_ID_SGXDISPLAY_" + toString(procID);
        linkIDAsgn[cType]++;
        break;
    case cDrmDisplay:
        linkIDName = "SYSTEM_LINK_ID_DRMDISPLAY_" + toString(procID);
        linkIDAsgn[cType]++;
        break;
    case cSgx3Dsrv:
        linkIDName = "SYSTEM_LINK_ID_SGX3DSRV_" + toString(procID);
        linkIDAsgn[cType]++;
        break;
    case cVPE:
        linkIDName = string("SYSTEM_LINK_ID_VPE_")
                + toString(linkIDAsgn[cType]);
        linkIDAsgn[cType]++;
        break;
    case cHcf:
        linkIDName = string("SYSTEM_LINK_ID_HCF_")
                + toString(linkIDAsgn[cType]);
        linkIDAsgn[cType]++;
        break;
    case cDefLink:
        CHECK_ERROR(false, "Warning: Custom defined Link [" + name + "] found. \n"
                         + "         You will need to manually edit the generated .c file to fill in \n"
                         + "         missing information like Link ID, create parameters !!!");
        break;
    default:
        CHECK_ERROR_ABORT(false, "Error: Link [" + name + "] name does not match supported links. Use '-h' option to see list of supported links !!!");
    };
    return linkIDName;
}
