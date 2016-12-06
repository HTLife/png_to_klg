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
    printf("%d\n", vec_info.size());
    CvMat *encodedImage = 0;

    VEC_INFO::iterator it = vec_info.begin();
    int count = 0;
    for(it; it != vec_info.end()-1; it++) 
    {
        
        std::string strAbsPathDepth = 
            std::string(
                        getcwd(NULL, 0)) + 
                        it->second.first.substr(1, it->second.first.length());
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

        std::string strAbsPath = std::string(getcwd(NULL, 0)) + it->second.second.substr(1, it->second.second.length());

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

void parseInfoFile(
            std::string &strPath, 
            std::vector<int64_t> &vec_time,
            std::vector<std::string> &vec_path)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    FILE *pFile = fopen(strPath.c_str(), "r");
    if(!pFile) {
        return;
    }

    while ((read = getline(&line, &len, pFile)) != -1) {
        //printf("Retrieved line of length %zu :\n", read);
        //printf("%s", line);

        std::istringstream is(line);
        std::string part;
        int iIdxToken = 0;
        while (getline(is, part, ' '))
        {
            if(0 == iIdxToken)
            {//first token which is time
                part.erase(
                    std::remove(part.begin(), 
                        part.end(), '.'), part.end());
                int numb;
                std::istringstream ( part ) >> numb;
                //printf("%d\n", numb);
                vec_time.push_back(numb);
            } else if(1 == iIdxToken) 
            {
                if(part[part.length()-1] == '\n' && part.length() > 1)
                {

                    part.erase(
                        std::remove(part.begin(), 
                            part.end(), '\n'), part.end());
                }
                vec_path.push_back(part);
            }
            //std::cout << part << std::endl;
            iIdxToken++;
        }
    }
    fclose(pFile);
    //printf("size1: %d, size2:%d\n", vec_time.size(), vec_path.size());
    //printf("'%s'\n", vec_path.back().c_str());
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
    std::string strDepth_Path;
    std::string strRGB_Path;
    std::string strKlgFileName;

    int c = 0;
    while((c = getopt(argc, argv, "drwo")) != -1)
    {
        switch(c)
        {
            case 'd'://depth
                option_count++;
                strDepth_Path = std::string(argv[optind]);
                break;
            case 'r'://rgb
                option_count++;
                strRGB_Path = std::string(argv[optind]);
                break;
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
    if(option_count < 4)
    {
        fprintf(stderr, 
            "Error: Please provide parameters -d and -r") ;
        return -1;
    }

    /// Parse files
    // (timestamp, (depth path, rgb path) )
    VEC_INFO vec_info;

    std::vector<int64_t> vec_time_depth;
    std::vector<std::string> vec_path_depth;
    parseInfoFile(
                strDepth_Path, 
                vec_time_depth, 
                vec_path_depth);
    std::vector<int64_t> vec_time_rgb;
    std::vector<std::string> vec_path_rgb;
    parseInfoFile(
                strRGB_Path, 
                vec_time_rgb, 
                vec_path_rgb);
    
    
    intersectionSequence(vec_info,
                vec_time_depth,
                vec_path_depth,
                vec_time_rgb,
                vec_path_rgb);

    int ret = chdir(strDatasetDir.c_str());
    printf("provided:%s\n", strDatasetDir.c_str());
    if(ret != 0) 
    {
        fprintf(stderr, "dataset path not exist");
    }
    printf("current working directory: %s\n", getcwd(NULL, 0));
    

    convertToKlg(vec_info, strKlgFileName);


}

