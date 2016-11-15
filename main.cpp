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

#include <vector>
#include <string>
#include <utility>

extern char *optarg;
extern int optind;

typedef std::pair<std::string,std::string> PATH_PAIR;
typedef std::pair<int64_t,PATH_PAIR> SEQ;
typedef std::vector<SEQ> VEC_INFO;

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


void ReadFile(char *name, uint8_t* buffer) 
{ 
    FILE *pFile = NULL;
    unsigned long fileLen = 0;

    pFile = fopen(name, "rb");
    if(!pFile) {
        return;
    }

    //Get file length
    fseek(pFile, 0, SEEK_END);
    fileLen = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    //Allocate memory
    buffer = (uint8_t*)malloc(fileLen+1);
    if(!buffer)
    {
        fprintf(stderr, "Memory error!");
        return;
    }
    
    //Read file contents into buffer
    fread(buffer, fileLen, 1, pFile);
    fclose(pFile);

//    free(buffer); DEBUG


}


void writeOneFrame()
{

    CvMat * encodedImage = 0;
    //640x480
    //
    char fileRGB[] = "/home/jackyliu/code/png_to_klg/livingroom_kt0_rs/rgb/scene_00_0000_rs.png";
    char fileDepth[] = "/home/jackyliu/code/png_to_klg/livingroom_kt0_rs/depth/scene_00_0005_rs.png";
    uint8_t* ptrDepth = NULL;
    ReadFile(fileRGB, ptrDepth);
    int64_t timestamp = 0;
    


    boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration duration(time.time_of_day());
    int64_t imageTime = duration.total_microseconds();


    std::string filename = "test.klg";
    FILE * logFile = fopen(filename.c_str(), "wb+");
    


    int32_t numFrames = 0;
    
    fwrite(&numFrames, sizeof(int32_t), 1, logFile);
    
    int depth_compress_buf_size = 640 * 480 * sizeof(int16_t) * 4;
    uint8_t * depth_compress_buf = (uint8_t*)malloc(depth_compress_buf_size);

    int bufferIndex = 0;
    unsigned long compressed_size = depth_compress_buf_size;
    // compress2(
    //   Bytef * dest, 
    //   uLongf * destLen, 
    //   const Bytef * source, 
    //   uLong sourceLen, 
    //   int level);
    compress2(depth_compress_buf,
                &compressed_size,
                //(const Bytef*)frameBuffers[bufferIndex].first.first,//source
                (const Bytef*)ptrDepth,
                (uLong)640 * 480 * sizeof(short),
                Z_BEST_SPEED);

    //encodeJpeg(&encodedImage,
                //(cv::Vec<unsigned char, 3> *)frameBuffers[bufferIndex].first.second);
      //          (cv::Vec<unsigned char, 3> *)&ptrRGB);

    IplImage *img = cvLoadImage(fileRGB, CV_LOAD_IMAGE_UNCHANGED);

    int jpeg_params[] = {CV_IMWRITE_JPEG_QUALITY, 90, 0};
    if(encodedImage != 0)
    {
        cvReleaseMat(&encodedImage);
    }
    encodedImage = cvEncodeImage(".jpg", img, jpeg_params);

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
    
    fwrite(&timestamp, sizeof(int64_t), 1, logFile);
    fwrite(&depthSize, sizeof(int32_t), 1, logFile);
    fwrite(&imageSize, sizeof(int32_t), 1, logFile);
    fwrite(depth_compress_buf, depthSize, 1, logFile);
    fwrite(encodedImage->data.ptr, imageSize, 1, logFile);

    numFrames++;
}



void parseInfoFile(
            std::string &strRGB_Path, 
            std::string &strDepth_Path,
            VEC_INFO &vec_info)
{
    FILE *pFile = fopen(strRGB_Path.c_str(), "r");
    if(!pFile) {
        return;
    }
    fclose(pFile);
}
            

int main(int argc, char* argv[])
{

    int iRGB = -1;//rgb.txt argv index
    int iDepth = -1;//depth.txt argv index

    int c = 0;
    while((c = getopt(argc, argv, "dr")) != -1)
    {
        switch(c)
        {
            case 'd'://depth
                iDepth = optind;
                break;
            case 'r'://rgb
                iRGB = optind;
                break;
            default:
                break;
        }
    }

    /// Parse files
    // (timestamp, (depth path, rgb path) )
    VEC_INFO vec_info;
    std::string strDepth_Path(argv[iDepth]);
    std::string strRGB_Path(argv[iRGB]);
    
    parseInfoFile(strRGB_Path, strDepth_Path, vec_info);
}

