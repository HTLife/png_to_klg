/*
 * main.cpp
 *
 * Created on: 3 Nov 2016
 *     Author: Jacky Liu
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

//(depth, rgb)
typedef std::pair<std::string,std::string> PATH_PAIR;
typedef std::pair<int64_t,PATH_PAIR> SEQ;
typedef std::vector<SEQ> VEC_INFO;

std::string strDatasetDir;



void convertToKlg(
    VEC_INFO &vec_info,
    std::string &strKlgFileName)
{
    std::cout << "klg_name=" << strKlgFileName << std::endl;
    std::string filename = strKlgFileName;//"test2.klg";
    FILE * logFile = fopen(filename.c_str(), "wb+");

    int32_t numFrames = (int32_t)vec_info.size() - 1;

    fwrite(&numFrames, sizeof(int32_t), 1, logFile);

    //printf("klg_1\n");
    //printf("%d\n", vec_info.size());
    CvMat *encodedImage = 0;

    VEC_INFO::iterator it = vec_info.begin();
    for(it; it != vec_info.end()-1; it++) 
    {
        std::string strAbsPathDepth = 
            std::string(
                        getcwd(NULL, 0)) + "/" +
                        it->second.first;
                        //it->second.first.substr(1, it->second.first.length());
        std::cout << strAbsPathDepth << std::endl;
        //IplImage *imgDepth = 
        //    cvLoadImage(strAbsPathDepth.c_str(), 
       //                 CV_LOAD_IMAGE_UNCHANGED);


        cv::Mat depth = imread(strAbsPathDepth.c_str(), cv::IMREAD_UNCHANGED);

        //if(imgDepth == NULL)
        //{
        //    printf("Fail to read depth img\n");
        //    fclose(logFile);
        //    return;
        //}
        double depthScale = 0.0001;
        depth.convertTo(depth, CV_16UC1, 1000 * depthScale);

        int32_t depthSize = depth.total() * depth.elemSize();

        std::string strAbsPath = std::string(
                    getcwd(NULL, 0)) + "/" +
                    it->second.second;
                    //it->second.second.substr(1, it->second.second.length());

        std::cout << strAbsPath << std::endl;
        IplImage *img = 
            cvLoadImage(strAbsPath.c_str(), 
                        CV_LOAD_IMAGE_UNCHANGED);
        if(img == NULL)
        {
            fclose(logFile);
            return;
        }
        int jpeg_params[] = {CV_IMWRITE_JPEG_QUALITY, 90, 0};
        if(encodedImage != 0)
        {
            cvReleaseMat(&encodedImage);
        }
        encodedImage = cvEncodeImage(".jpg", img, jpeg_params);
        cvReleaseImage(&img);
        int32_t imageSize = encodedImage->width;

        unsigned char * rgbData = 0;
        rgbData = (unsigned char *)encodedImage->data.ptr;
        /**
         * Format is:
         * int64_t: timestamp
         * int32_t: depthSize
         * int32_t: imageSize
         * depthSize * unsigned char: depth_compress_buf
         * imageSize * unsigned char: encodedImage->data.ptr
         */

        /// Timestamp
        fwrite(&it->first, sizeof(int64_t), 1, logFile);

        /// DepthSize
        fwrite(&depthSize, sizeof(int32_t), 1, logFile);

        /// imageSize
        fwrite(&imageSize, sizeof(int32_t), 1, logFile);

        /// Depth buffer
        //outStream.write((char*)depth.data, depthSize);
        fwrite((char*)depth.data, depthSize, 1, logFile);
        //fwrite(imgDepth->imageData, imgDepth->imageSize, 1, logFile);

        /// RGB buffer
        fwrite(rgbData, imageSize, 1, logFile);

        //if(0 != imgDepth)
        //{
        //    cvReleaseImage(&imgDepth);
        //}
        depth.release();
        if(encodedImage != 0)
        {
            cvReleaseMat(&encodedImage);
        }
    }

    if(encodedImage != 0)
    {
        cvReleaseMat(&encodedImage);
    }
    fclose(logFile);
}




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
                if(0 == iIdxToken) //Time rgb
                {//first token which is time
                    token.erase(
                        std::remove(token.begin(), 
                            token.end(), '.'), token.end());
                    int numb;
                    std::istringstream ( token ) >> numb;
                    timeSeq = numb;
                } 
                else if(1 == iIdxToken)//rgb path
                {
                    strRgbPath = token;
                }
                else if(2 == iIdxToken)//Time depth
                {
                    /// Do nothing
                }
                else if(3 == iIdxToken)//depth path
                {
                    strDepthPath = token;
                }

                iIdxToken++;
            }
            PATH_PAIR path_pair(strDepthPath, strRgbPath);
            SEQ seq(timeSeq, path_pair);
            vec_info.push_back(seq);
        }
    }
    fclose(pFile);
    return 0;
}

void intersectionSequence(
    VEC_INFO &vec_info,
    std::vector<int64_t> &vec_time_1,
    std::vector<std::string> &vec_path_1,
    std::vector<int64_t> &vec_time_2,
    std::vector<std::string> &vec_path_2)
{
    int i = 0, j = 0;
    while (i < vec_time_1.size() && 
           j < vec_time_2.size())
    {
        if (vec_time_1[i] < vec_time_2[j])
            i++;
        else if (vec_time_2[j] < vec_time_1[i])
            j++;
        else /* if arr1[i] == arr2[j] */
        {
            //printf(" %d ", arr2[j++]);
            PATH_PAIR path_pair(vec_path_1[i], vec_path_2[j]);
            SEQ seq(vec_time_1[i], path_pair);
            vec_info.push_back(seq);
            i++;
        }
    }
}

int main(int argc, char* argv[])
{
    int option_count = 0;
    std::string strAssociation_Path;
    std::string strKlgFileName;

    int c = 0;
    while((c = getopt(argc, argv, "wo")) != -1)
    {
        switch(c)
        {
            case 'w'://Dataset directory root path
                option_count++;
                strDatasetDir = std::string(argv[optind]);
                break;
            case 'o'://klg filename
                option_count++;
                strKlgFileName = std::string(argv[optind]);
                break;
            default:
                break;
        }
    }
    if(option_count < 2)
    {
        fprintf(stderr, 
            "Usage: ./pngtoklg -w (working directory) -o (klg file name)"    
            "Example: ./pngtoklg -w ../livingroom_kt0_rs -o liv.klg");        
        return -1;
    }



    /// Change working directory
    int ret = chdir(strDatasetDir.c_str());
    if(ret != 0) 
    {
        fprintf(stderr, RED "dataset path not exist" NONE);
    }
    printf("\nCurrent working directory:\n\t%s\n", getcwd(NULL, 0));


    strAssociation_Path = strDatasetDir;
    if(strDatasetDir[strDatasetDir.length() - 1] != '/')
    {
        strAssociation_Path += '/';
    }
    strAssociation_Path += "associations.txt";
    
    /// Parse files
    // (timestamp, (depth path, rgb path) )
    VEC_INFO vec_info;

    //std::vector<int64_t> vec_time_depth;
    //std::vector<std::string> vec_path_depth;

    int err = parseInfoFile(
                strAssociation_Path, 
                vec_info);
    if(err != 0)
    {
        fprintf(stderr, 
            RED "Fail to find associations.txt under working directory!\n" NONE); 
        return -1;
    }
    /*
    std::vector<int64_t> vec_time_rgb;
    std::vector<std::string> vec_path_rgb;
    parseInfoFile(
                strRGB_Path, 
                vec_time_rgb, 
                vec_path_rgb);*/
    
    //std::cout << "depth time vec size\t= " << vec_time_depth.size() << std::endl;
    //std::cout << "depth img vec size\t= " << vec_path_depth.size() << std::endl;
    //std::cout << "rgb time vec size\t= " << vec_time_rgb.size() << std::endl;
    //std::cout << "rgb img vec size\t= " << vec_path_rgb.size() << std::endl;

    /*intersectionSequence(vec_info,
                vec_time_depth,
                vec_path_depth,
                vec_time_rgb,
                vec_path_rgb);*/
    //printf("strDepth_Path=%s\n", strDepth_Path.c_str());
    //printf("vec_info size=%d\n", vec_info.size());


    

    convertToKlg(vec_info, strKlgFileName);


}

