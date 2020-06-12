/*
 * main.cpp
 *
 * Created on: 3 Nov 2016
 *     Author: Jacky Liu
 *
 *
 * Convert .png sequence in TUM compatible dataset to .klg format
 * (.klg is the log file format for Kintinuous and ElasticFusion)
 */



#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <string>
#include <utility>

#include <sstream>

#define NONE "\033[m"
#define RED "\033[0;32;31m"


extern char *optarg;
extern int optind;

/// PATH_PAIR = pair<depth image path, rgb image path>
typedef std::pair<std::string,std::string> PATH_PAIR;

/// SEQ = pair<timestamp, PATH_PAIR>
typedef std::pair<int64_t,PATH_PAIR> SEQ;
typedef std::vector<SEQ> VEC_INFO;

/// Working directory path
std::string strWorkingDir;

bool g_bFlag_reverse = false;
bool g_bFlag_TUM = false;
double g_dScale = 5000;//depth scale


/** @brief Convert png image files to .klg format
 *  
 *  @param vec_info vector of path <timestamp, <depth path, rgb path>>
 *  @param strKlgFileName output file name
 *  @return void
 */
void convertToKlg(
    VEC_INFO &vec_info,
    std::string &strKlgFileName)
{
    std::cout << "klg_name:\n\t" << strKlgFileName << std::endl;


    std::string filename = strKlgFileName;//"test2.klg";
    FILE * logFile = fopen(filename.c_str(), "wb+");

    int32_t numFrames = (int32_t)vec_info.size();

    fwrite(&numFrames, sizeof(int32_t), 1, logFile);

    //CvMat *encodedImage = 0;

    VEC_INFO::iterator it = vec_info.begin();
    int count = 1;
    std::cout << "Progress:\n";
    for(it; it != vec_info.end(); it++) 
    {
        std::string strAbsPathDepth = 
            std::string(
                        getcwd(NULL, 0)) + "/" +
                        it->second.first;


        cv::Mat depth = imread(strAbsPathDepth.c_str(), cv::IMREAD_UNCHANGED);

        double depthScale = g_dScale;
        depth.convertTo(depth, CV_16UC1, 1000 * 1.0 / depthScale);

        int32_t depthSize = depth.total() * depth.elemSize();

        std::string strAbsPath = std::string(
                    getcwd(NULL, 0)) + "/" +
                    it->second.second;

        IplImage *img = 
            cvLoadImage(strAbsPath.c_str(), 
                        CV_LOAD_IMAGE_UNCHANGED);
        if(img == NULL)
        {
            fclose(logFile);
            return;
        }

        int32_t imageSize = img->height * img->width * sizeof(unsigned char) * 3;

        unsigned char * rgbData = 0;
        rgbData = (unsigned char *)img->imageData;

        std::cout << '\r'
                  << std::setw(4) << std::setfill('0') << count << " / "
                  << std::setw(4) << std::setfill('0') << vec_info.size() 
                  << std::flush;
        count++;

        /// Timestamp
        fwrite(&it->first, sizeof(int64_t), 1, logFile);

        /// DepthSize
        fwrite(&depthSize, sizeof(int32_t), 1, logFile);

        /// imageSize
        fwrite(&imageSize, sizeof(int32_t), 1, logFile);

        /// Depth buffer
        fwrite((char*)depth.data, depthSize, 1, logFile);

        /// RGB buffer
        fwrite(rgbData, imageSize, 1, logFile);

        cvReleaseImage(&img);
        depth.release();
    }
    std::cout << std::endl;

    fclose(logFile);
}




/** @brief Parse associations.txt files to vector
 *
 *  @param strAssociation_Path path of associations.txt
 *  @param vec_info vector of path <timestamp, <depth path, rgb path>>
 *  @return void
 */
int parseInfoFile(
            std::string &strAssociation_Path,
            VEC_INFO &vec_info)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    
    FILE *pFile = fopen(strAssociation_Path.c_str(), "r");
    if(!pFile) {
        return -1;
    }
    
    int iFrameCnt = 0;
    while ((read = getline(&line, &len, pFile)) != -1) {

        std::istringstream is(line);
        std::string part;
        int iIdxToken = 0;
        while (getline(is, part))
        {
            if('#' == part[0])/// Skip file comment '#" 
            {
                continue;
            }
        
            int64_t timeSeq = 0;
            std::string strDepthPath;
            std::string strRgbPath;

            std::istringstream iss(part);
            std::string token;

            while (getline(iss, token, ' '))
            {
                if(2 == iIdxToken) //Time rgb
                {//first token which is time


                } 
                else if(3 == iIdxToken)//rgb path
                {
                    strRgbPath = token;
                }
                else if(0 == iIdxToken)//Time depth
                {
                    /// Do nothing
                    //std::cout << token << std::endl;
                    token.erase(
                        std::remove(token.begin(), 
                            token.end(), '.'), token.end());
                    //std::cout << token << std::endl;
                    long long unsigned int numb;
                    std::istringstream ( token ) >> numb;
                    
                    if(true == g_bFlag_TUM) 
                    {
                        timeSeq = (int64_t)numb;
                        //timeSeq = iFrameCnt;
                        iFrameCnt++;
                    }
                    else
                    {
                        timeSeq = numb * 1000000;
                    }
                }
                else if(1 == iIdxToken)//depth path
                {
                    strDepthPath = token;
                }
                iIdxToken++;
            }
            PATH_PAIR path_pair;
            if(g_bFlag_reverse)
                path_pair = PATH_PAIR(strRgbPath, strDepthPath);
            else 
                path_pair = PATH_PAIR(strDepthPath, strRgbPath);
            SEQ seq(timeSeq, path_pair);
            vec_info.push_back(seq);
        }
    }
    fclose(pFile);
    return 0;
}


int main(int argc, char* argv[])
{
    int option_count = 0;
    std::string strAssociation_Path;
    std::string strKlgFileName;

    int c = 0;
    while((c = getopt(argc, argv, "worts")) != -1)
    {
        switch(c)
        {
            case 'w'://Dataset directory root path
                option_count++;
                strWorkingDir = std::string(argv[optind]);
                break;
            case 'o'://klg filename
                option_count++;
                strKlgFileName = std::string(argv[optind]);
                break;
            case 'r'://associations.txt is in reverse order (rgb)(depth)
                option_count++;
                g_bFlag_reverse = true;

                break;
            case 't'://TUM format
                option_count++;
                g_bFlag_TUM = true;
                break;
            case 's'://TUM format
                option_count++;
                sscanf(argv[optind], "%lf", &g_dScale);
                break;
            default:
                break;
        }
    }
    if(option_count < 2)
    {
        fprintf(stderr, 
            "Usage: ./pngtoklg -w (working directory) -o (klg file name)\n"    
            "\n"
            "-w working directory\n"
            "-o output klg filename\n"
            "-t TUM format\n"
            "-s Scale factor in floating point ex.  '5000'\n"
            "For more scale factor detail, please reference: \nhttp://vision.in.tum.de/data/datasets/rgbd-dataset/file_formats#intrinsic_camera_calibration_of_the_kinect\n"
            "Example: ./pngtoklg -w ../livingroom_kt0_rs -o liv.klg\n"
            "associations.txt should be in (depth_time)(depth_file)(rgb_time)(rgb_file) order\n");
        return -1;
    }



    /// Change working directory
    int ret = chdir(strWorkingDir.c_str());
    if(ret != 0) 
    {
        fprintf(stderr, RED "dataset path not exist" NONE);
    }
    printf("\nCurrent working directory:\n\t%s\n", getcwd(NULL, 0));


    strAssociation_Path = strWorkingDir;
    if(strWorkingDir[strWorkingDir.length() - 1] != '/')
    {
        strAssociation_Path += '/';
    }
    strAssociation_Path += "associations.txt";
    
    /// Parse files
    // (timestamp, (depth path, rgb path) )
    VEC_INFO vec_info;


    int err = parseInfoFile(
                strAssociation_Path, 
                vec_info);
    if(err != 0)
    {
        fprintf(stderr, 
            RED "Fail to find associations.txt under working directory!\n" NONE); 
        return -1;
    }

    std::cout << "Depth: " << vec_info.back().second.first << std::endl;
    std::cout << "RGB: " << vec_info.back().second.second << std::endl;
    std::cout << "scale: " << g_dScale << std::endl;

    convertToKlg(vec_info, strKlgFileName);

    std::cout << "Conversion complete!\n";
    return 0;
}

