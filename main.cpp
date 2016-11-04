/*
 * main.cpp
 *
 * Created on: 3 Nov 2016
 *     Author: Jacky Liu
 */



#include "main.h"


void encodeJpeg(CvMat *encodedImage, cv::Vec<unsigned char, 3> * rgb_data)
{
    cv::Mat3b rgb(480, 640, rgb_data, 1920);

    IplImage * img = new IplImage(rgb);

    int jpeg_params[] = {CV_IMWRITE_JPEG_QUALITY, 90, 0};

    if(encodedImage != 0)
    {
        cvReleaseMat(&encodedImage);
        encodedImage = cvEncodeImage(".jpg", img, jpeg_params);

        delete img;
    }
}


// rgb
// /home/jackyliu/code/png_to_klg/livingroom_kt0_rs/rgb/scene_00_0000_rs.png
//
// depth
// /home/jackyliu/code/png_to_klg/livingroom_kt0_rs/depth/scene_00_0005_rs.png
//
// 
int main(int argc, char* argv[])
{
    CvMat * encodedImage = 0;

    FILE * fileRGB = fopen(
    "/home/jackyliu/code/png_to_klg/livingroom_kt0_rs/rgb/scene_00_0000_rs.png", "rb");
    
    FILE * fileDep = fopen(
    "/home/jackyliu/code/png_to_klg/livingroom_kt0_rs/depth/scene_00_0005_rs.png", "rb");

    // <<depth ptr, rgb ptr>, timestamp>
    std::pair<std::pair<uint8_t *, uint8_t *>, int64_t> frameBuffers[10];

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
                (const Bytef*)frameBuffers[bufferIndex].first.first,//source
                (uLong)640 * 480 * sizeof(short),
                Z_BEST_SPEED);

    encodeJpeg(encodedImage,
                (cv::Vec<unsigned char, 3> *)frameBuffers[bufferIndex].first.second);

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
    
    fwrite(&frameBuffers[bufferIndex].second, sizeof(int64_t), 1, logFile);
    fwrite(&depthSize, sizeof(int32_t), 1, logFile);
    fwrite(&imageSize, sizeof(int32_t), 1, logFile);
    fwrite(depth_compress_buf, depthSize, 1, logFile);
    fwrite(encodedImage->data.ptr, imageSize, 1, logFile);

    numFrames++;




}

