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


    
void encodeJpeg(CvMat **encodedImage, cv::Vec<unsigned char, 3> *rgb_data)
{
/*
    char fileRGB[] = "/home/jackyliu/code/png_to_klg/livingroom_kt0_rs/rgb/scene_00_0000_rs.png";
    cv::Mat3b rgb(480, 640, rgb_data, 1920);

    IplImage * img = new IplImage(rgb);
    IplImage *img =  = cvLoadImage(fileRGB, CV_LOAD_IMAGE_UNCHANGED);

    int jpeg_params[] = {CV_IMWRITE_JPEG_QUALITY, 90, 0};
    if(*encodedImage != 0)
    {
        cvReleaseMat(encodedImage);
    }
    *encodedImage = cvEncodeImage(".jpg", img, jpeg_params);

    printf("hi3");
    fflush(stdout); 
    delete img;*/
}


// rgb
// /home/jackyliu/code/png_to_klg/livingroom_kt0_rs/rgb/scene_00_0000_rs.png
//
// depth
// /home/jackyliu/code/png_to_klg/livingroom_kt0_rs/depth/scene_00_0005_rs.png
//


int ReadFile(const char *name, uint8_t* buffer) 
{ 
    FILE *pFile = NULL;
    unsigned long fileLen = 0;

    pFile = fopen(name, "rb");
    if(!pFile) {
        return -1;
    }

    //Get file length
    fseek(pFile, 0, SEEK_END);
    fileLen = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    //Allocate memory
    buffer = (uint8_t*)malloc(fileLen+1);
    printf("fileSize = %d\n", fileLen);
    if(!buffer)
    {
        fprintf(stderr, "Memory error!");
        return -1;
    }
    
    //Read file contents into buffer
    fread(buffer, fileLen, 1, pFile);
    fclose(pFile);

//    free(buffer); DEBUG

    return 0;
}



void convertToKlg(
    VEC_INFO &vec_info)
{
/*
    FILE* fp = fopen("./rgb/scene_00_0000_rs.png", "rb");
    if(!fp) {
        printf("fail");
    }
    else
    {
        printf("success");
    }
return;*/
    int depth_compress_buf_size = 
        640 * 480 * sizeof(int16_t) * 4;
    uint8_t * depth_compress_buf = 
        (uint8_t*)malloc(depth_compress_buf_size);




    std::string filename = "test.klg";
    FILE * logFile = fopen(filename.c_str(), "wb+");

    int32_t numFrames = (int32_t)vec_info.size() - 1;

    fwrite(&numFrames, sizeof(int32_t), 1, logFile);

    printf("numFrames:%d\n", sizeof(int32_t));

    CvMat *encodedImage = 0;

    VEC_INFO::iterator it = vec_info.begin();
    int count = 0;
    for(it; it != vec_info.end()-1; it++) 
    {
        //count++;
        //if(count > 200)
        //{
        //    break;
        //}

        //640x480
        uint8_t* ptrDepth = NULL;
        //if(0 != ReadFile(it->second.first.c_str(), ptrDepth))
        //{
        //    continue;
        //}
        
        std::string strAbsPathDepth = 
            std::string(
                        getcwd(NULL, 0)) + 
                        it->second.first.substr(1, it->second.first.length());
        IplImage *imgDepth = 
            cvLoadImage(strAbsPathDepth.c_str(), 
                        CV_LOAD_IMAGE_UNCHANGED);
        if(imgDepth == NULL)
        {
            printf("Fail to read depth img\n");
            fclose(logFile);
            return;
        }

        unsigned long compressed_size = depth_compress_buf_size;
        // compress2(
        //   Bytef * dest, 
        //   uLongf * destLen, 
        //   const Bytef * source, 
        //   uLong sourceLen, 
        //   int level);
        printf("sizeBeforeCompressed:%d\n", compressed_size);
        compress2(depth_compress_buf,
                    &compressed_size,
                    (const Bytef*)imgDepth->imageData,//ptrDepth,
                    (uLong)640 * 480 * sizeof(short),
                    Z_BEST_SPEED);
        if(0 != imgDepth)
        {
            cvReleaseImage(&imgDepth);
        }
        printf("sizeAfterCompressed:%d\n", compressed_size);

        free(ptrDepth);


        std::string strAbsPath = std::string(getcwd(NULL, 0)) + it->second.second.substr(1, it->second.second.length());


        //printf("current working directory: %s\n", getcwd(NULL, NULL));
        printf("ABSpath = %s\n", strAbsPath.c_str());
        //printf("ABSpath = %s\n", "/home/rvlrobot/code/png_to_klg/livingroom_kt0_rs/rgb/scene_00_0000_rs.png");
        //return;
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
        int32_t depthSize = compressed_size;
        int32_t imageSize = encodedImage->width;

        /**
         * Format is:
         * int64_t: timestamp
         * int32_t: depthSize
         * int32_t: imageSize
         * depthSize * unsigned char: depth_compress_buf
         * imageSize * unsigned char: encodedImage->data.ptr
         */
        
        unsigned char * depthData = 0;
        unsigned char * rgbData = 0;
        depthData = (unsigned char *)depth_compress_buf;
        rgbData = (unsigned char *)encodedImage->data.ptr;

    printf("timestamp:%d\n", sizeof(int64_t));
    printf("depthSize:%d\n", sizeof(int32_t));
    printf("imageSize:%d\n", sizeof(int32_t));
    printf("depthData:%d\n", depthSize);
    printf("rgbData:%d\n", imageSize);
    printf("short:%d\n", sizeof(short));
        fwrite(&it->first, sizeof(int64_t), 1, logFile);
        fwrite(&depthSize, sizeof(int32_t), 1, logFile);
        fwrite(&imageSize, sizeof(int32_t), 1, logFile);
        fwrite(depthData, depthSize, 1, logFile);
        fwrite(rgbData, imageSize, 1, logFile);
        //numFrames++;

        if(encodedImage != 0)
        {
            cvReleaseMat(&encodedImage);
        }
    }

    free(depth_compress_buf);
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

    int c = 0;
    while((c = getopt(argc, argv, "drw")) != -1)
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
            default:
                break;
        }
    }
    if(option_count < 3)
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
    if(ret != 0) 
    {
        fprintf(stderr, "dataset path not exist");
    }
    printf("current working directory: %s\n", getcwd(NULL, 0));
    

    convertToKlg(vec_info);


}

