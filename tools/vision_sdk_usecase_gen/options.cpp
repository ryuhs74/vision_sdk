#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#include "options.h"

Options cmd_options;
extern FILE * yyin;

Options::Options() {
    show_tokens = false;
    write_file = false;
    write_image = false;
    verbose_mode = false;
    log_mode = false;
    output_path = false;

    os_tokens = NULL;
    os_file = NULL;
    os_header = NULL;
    os_image = NULL;
    os_log = NULL;
}

Options::~Options() {
    if (show_tokens)
        ofs_tokens.close();
    if (write_file) {
        ofs_file.close();
        ofs_header.close();
    }
    if (write_image)
        ofs_image.close();
    if(log_mode)
        ofs_log.close();
}

void Options::close_Image() {
    if (write_image)
        ofs_image.close();
}

bool Options::show_Tokens() {
    return show_tokens;
}
bool Options::write_toFile() {
    return write_file;
}

bool Options::write_toImage() {
    return write_image;
}

bool Options::verboseMode() {
    return verbose_mode;
}

bool Options::write_tologFile()
{
    return log_mode;
}

ostream * Options::tokens_File() {
    return &ofs_tokens;
}
ostream * Options::write_File() {
    return &ofs_file;
}
ostream * Options::write_Header() {
    return &ofs_header;
}
ostream * Options::write_Image() {
    return &ofs_image;
}
ostream* Options::log_File()
{
    return &ofs_log;
}

int Options::process_Options(int argc, char * argv[]) {
    bool input_file_given = false;

    string tokens_file_name;

    char * input_file_c_string = NULL;

    string usage =
            " \n"
            " Vision SDK usecase generation tool - (c) Texas Instruments 2014\n"
            " \n"
            " Version : v1.01 ( " + string(__DATE__) + " - " + string(__TIME__) + " )\n"
            " \n"
            " Usage: " + string(argv[0]) + " [options] [input file]\n"
            " \n"
            " Options:\n"
            "   -help            :  Show this help\n"
            "   -file            :  Create usecase .c and .h file \n"
            "   -img             :  Create .jpg image (Needs 'dot' tool from www.graphviz.org to be installed and visible in system path)\n"
            "   -log             :  Creates .txt log file with debugging info\n"
            "   -debug           :  Prints file name and line no of tool source code in error statements (To be used by tool developers only)\n"
            "   -path [pathname] :  Output path where generated files are written. If not specified current directory is used\n"
            "   -v               :  Verbose output on console\n"
            " \n"
            " Supported CPUs: \n"
            "   IPU1_0, IPU1_1, A15, DSP1, DSP2, EVE1, EVE2, EVE3, EVE4 \n"
            " \n"
            " Supported Links:  \n"
            "   AvbRx\n"
            "   Capture\n"
            "   UltrasonicCapture\n"
            "   VPE\n"
            "   Display\n"
            "   Encode\n"
            "   Decode\n"
            "   DrmDisplay\n"
            "   SgxDisplay\n"
            "   Sgx3Dsrv\n"
            "   GrpxSrc\n"
            "   IssCapture\n"
            "   IssM2mIsp\n"
            "   IssM2mSimcop\n"
            "   Hcf\n"
            "\n"
            "   Dup\n"
            "   Split\n"
            "   Gate\n"
            "   Merge\n"
            "   Select\n"
            "   Sync\n"
            "\n"
            "   IPCIn\n"
            "   IPCOut\n"
            "\n"
            "   Null\n"
            "   NullSource\n"
            "\n"
            "   Alg_FrameCopy\n"
            "   Alg_SubframeCopy\n"
            "   Alg_ColorToGray\n"
            "   Alg_EdgeDetect\n"
            "   Alg_DmaSwMs\n"
            "   Alg_DenseOptFlow\n"
            "   Alg_VectorToImage\n"
            "   Alg_FeaturePlaneComputation\n"
            "   Alg_ObjectDetection\n"
            "   Alg_ObjectDraw\n"
            "   Alg_GeoAlign\n"
            "   Alg_PhotoAlign\n"
            "   Alg_Synthesis\n"
            "   Alg_SparseOpticalFlow\n"
            "   Alg_SparseOpticalFlowDraw\n"
            "   Alg_SoftIsp\n"
            "   Alg_IssAewb\n"
            "   Alg_Crc\n"
            "   Alg_UltrasonicFusion\n"
            "   Alg_Census\n"
            "   Alg_DisparityHamDist\n"
            "   Alg_RemapMerge\n"
            "   Alg_StereoPostProcess\n"
            "   Alg_LaneDetect\n"
            "   Alg_LaneDetectDraw\n"
            "\n"
            "   Alg_[Custom alg name]\n"
            "   DefLink_[Custom link name]\n"
            "\n"
            ;

    for (int i = 1; i < argc; i++) {
        char * option = strdup(argv[i]);
        if (strncmp(option, "-", 1) == 0) //check if 1st letter is -, then it is an option
        {
            if (strcmp(option, "-v") == 0)
                verbose_mode = true;
            else if (strcmp(option, "-tokens") == 0) {
                show_tokens = true;
            } else if (strcmp(option, "-file") == 0) {
                write_file = true;
            } else if (strcmp(option, "-img") == 0) {
                write_image = true;
            }else if (strcmp(option, "-log") == 0) {
                log_mode = true;
            }else if (strcmp(option, "-debug") == 0) {
                debug = true;
            }else if (strcmp(option, "-path") == 0) {
                output_path = true;
                if(i+1 < argc)
                {
                    output_path_name = string(argv[i+1]);
                    i++;
                    continue;
                }
                else
                    CHECK_ERROR_ABORT(SHOULD_NOT_REACH, "Error: Output path not provided !!!");
            }else if ((strcmp(option, "--help") == 0)
                    || (strcmp(option, "-help") == 0)
                    || (strcmp(option, "-h") == 0)){
                cerr << usage;
                return -1;
            } else {
                string mesg = "Error: Unknown option [" + string(option) + "] !!!\n";

                cerr << usage;

                CHECK_ERROR_ABORT(SHOULD_NOT_REACH, mesg);
            }
        } else if (input_file_given) {
            string mesg = "Error: Only one input file name can be provided !!!\n" ;

            cerr << usage;

            CHECK_ERROR(SHOULD_NOT_REACH, mesg);

        } else{
            input_file_given = true;
            input_file_c_string = strdup(option);
        }
    }

    if (input_file_given == true) {
        input_file_name = string(input_file_c_string);
        yyin = fopen(input_file_c_string, "r");
        CHECK_ERROR_ABORT(yyin!=NULL, "Error: Input file [" + input_file_name + "] could not be opened !!!");

    } else {
        string mesg = "Error: Input file name not provided !!!";

        cerr << usage;

        CHECK_ERROR_ABORT(SHOULD_NOT_REACH, mesg);


    }

    //need to give initial name to tokens_file_name because filename is set while parsing
    tokens_file_name = input_file_name + "-debug.txt";
    remove(tokens_file_name.c_str());
    if (show_tokens)
        ofs_tokens.open(tokens_file_name.c_str());
    if(output_path)
    {
        if(!(output_path_name.at(output_path_name.length()-1) == '/'))
            output_path_name = output_path_name + "/";
    }
    else
        output_path_name = "";
    return 0;

}

void Options::setFileNames(string filestr) {
    string write_file_name, header_file_name,
            write_image_name, log_file_name;
    write_file_name = output_path_name + filestr + "_priv.c";
    header_file_name = output_path_name + filestr + "_priv.h";
    write_image_name = output_path_name + filestr + "_img.txt";
    log_file_name = output_path_name + filestr + "_log.txt";
    remove(write_file_name.c_str());
    remove(header_file_name.c_str());
    remove(log_file_name.c_str());
    remove(write_image_name.c_str());

    if (write_file) {
        ofs_file.open(write_file_name.c_str());
        ofs_header.open(header_file_name.c_str());
    }
    if (write_image)
        ofs_image.open(write_image_name.c_str());
    if(log_mode)
        ofs_log.open(log_file_name.c_str());
}

string Options::get_output_path_name()
{
    return output_path_name;
}
