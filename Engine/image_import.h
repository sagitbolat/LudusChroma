#pragma once
#include <stdint.h>
#include <stdio.h>

#include "sky_structs.h"

#define INT8ARRAY_TO_INT16(arr, i) (arr[i]) + (arr[i+1] << 8)
#define INT8ARRAY_TO_INT32(arr, i) (arr[i]) + (arr[i+1] << 8) + (arr[i+2] << 16) + (arr[i+3] << 24)
#define INT8ARRAY_TO_INT64(arr, i) (arr[i]) + (arr[i+1] << 8) + (arr[i+2] << 16) + (arr[i+3] << 24) + ((uint64_t)arr[i+4] << 32) + ((uint64_t)arr[i+5] << 40) + ((uint64_t)arr[i+6] << 48) + ((uint64_t)arr[i+7] << 56)

#define INT8ARRAY_TO_WORD(arr, i) INT8ARRAY_TO_INT16(arr, i)
#define INT8ARRAY_TO_DWORD(arr, i) INT8ARRAY_TO_INT32(arr, i)
#define INT8ARRAY_TO_LONG(arr, i) INT8ARRAY_TO_INT64(arr, i)


// Import BMP file
size_t ImportBMP (
    const char* image_file_name, FILE* image_file,
    uint64_t* image_width, uint64_t* image_height, 
    uint32_t* bytes_per_pixel, uint32_t* byte_offset,
    uint32_t* padding_size 
) {
    const int FILE_HEADER_SIZE = 14;
    const int INFO_HEADER_SIZE = 40;
    uint8_t BYTES_PER_PIXEL = 3;
    
    


    // SECTION: file_header
    uint8_t fh[FILE_HEADER_SIZE];
    size_t fh_size = fread(fh, sizeof(uint8_t), FILE_HEADER_SIZE, image_file);
    
    // NOTE: Error checking
    if (fh_size != FILE_HEADER_SIZE) {
        // TODO: Error
        printf("ImportBMP Error in file '%s'. File header sizes do not match. Possible EOF error or wrong format.%c", image_file_name, '\n');
    }
    if (fh[0] != 'B' && fh[1] != 'M') {
        // TODO: Filetype Error
        printf("ImportBMP Error in file '%s'. Incorrect filetype. Filetype must be of type bmp.%c", image_file_name, '\n');
    }
    // TODO: Might be unnecessary to save file size
    //uint32_t file_size = INT8ARRAY_TO_INT32(fh, 2);
    *byte_offset = INT8ARRAY_TO_INT32(fh, 10);





    // SECTION: info_header
    uint8_t ih[INFO_HEADER_SIZE];
    fread(ih, 1, INFO_HEADER_SIZE, image_file);

    uint32_t ih_size_read = INT8ARRAY_TO_INT32(ih, 0);

    *image_width    = INT8ARRAY_TO_INT64(ih, 4); //bytes 4 -> 11
    *image_height   = INT8ARRAY_TO_INT64(ih, 12);//bytes 12 -> 19 
    uint16_t planes = INT8ARRAY_TO_INT16(ih, 20); 

    if (planes != 1) {
        // TODO: Throw error.
    }

    uint16_t bits_per_pixel = INT8ARRAY_TO_INT16(ih, 22);

    //TODO: Add support for the rest of the header info. See: https://www.fileformat.info/format/bmp/egff.htm
    // For file header. Especially section about BMP Version 3 (windows 3.x)

    // NOTE: Error checking
    if (ih_size_read != INFO_HEADER_SIZE) {
        // TODO: Error
        printf("ImportBMP Error in file '%s'. Info header sizes do not match.\n\tExpected info header size: %d\n\tActual info header size: %d\n\tPossible EOF error or wrong format.%c", image_file_name, INFO_HEADER_SIZE, ih_size_read, '\n');
    }
    if (BYTES_PER_PIXEL * 8 != bits_per_pixel) {
        // TODO: Pixel Resolution Error
        BYTES_PER_PIXEL = bits_per_pixel / 8;
    }
    *bytes_per_pixel = BYTES_PER_PIXEL;




    // SECTION: image data
    uint32_t width_in_bytes = (*image_width) * BYTES_PER_PIXEL;
    *padding_size = (4 - (width_in_bytes) % 4) % 4;

    size_t image_size = (*image_width) * (*image_height) * BYTES_PER_PIXEL;
    return image_size;
}


// NOTE: Do bitmap loading.
// NOTE: Returns the address of the asset on the GameMemory->transient_storage Arena.
size_t LoadBitmap(const char* image_file_name, ImageData* image_data) {
    FILE* image_file = fopen(image_file_name, "rb");
    uint64_t image_width = 0;
    uint64_t image_height = 0;
    uint32_t bytes_per_pixel = 0;
    uint32_t byte_offset = 0; // NOTE: The offset in bytes from the start of the file to the image data.
    uint32_t padding_size = 0; // NOTE: The padding at the end of each line of the bmp data. 
    size_t image_size = ImportBMP(image_file_name, image_file, &image_width, &image_height, &bytes_per_pixel, &byte_offset, &padding_size);


    image_data->width = (uint32_t)image_width;
    image_data->height = (uint32_t)image_height;
    image_data->bytes_per_pixel = bytes_per_pixel;
    image_data->data = (uint8_t*)(malloc(sizeof(uint8_t) * image_size));
     
    uint8_t* temp_data = (uint8_t*)(malloc(sizeof(uint8_t) * image_size));
    //move to start of bitmap data
    fseek(image_file, byte_offset, SEEK_SET);
    //read the data
    //for (int h = 0; h < int(image_height); ++h) {
    //    for (int w = 0; w < (int)image_width; ++w) {
    //        int i = ((h * (int)image_width) + w) * (int)bytes_per_pixel;
    //        //for (int k = (int)bytes_per_pixel - 1; k > 0; --k) {
    //        //    fread(&(image_data->data[i+k]), 1, 1, image_file);
    //        //}
    //        fread(&(image_data->data[i+2]), 1, 1, image_file);
    //        fread(&(image_data->data[i+1]), 1, 1, image_file);
    //        fread(&(image_data->data[i+0]), 1, 1, image_file);
    //        if (bytes_per_pixel > 3) fread(&(image_data->data[i+3]), 1, 1, image_file);
    //    }
    //    if (padding_size > 0) fseek(image_file, padding_size, SEEK_CUR);         
    //}
    fread(temp_data, sizeof(uint8_t), (int)image_height * (int)image_width * (int)bytes_per_pixel, image_file);
    fclose(image_file);

    // NOTE: Convert from BGRA to RGBA and flip vertically (because original bmp is upside down).
    int row_size = ((int)image_width * (int)bytes_per_pixel) + padding_size;
    for (int h = 0; h < (int)image_height; ++h) {
        for (int w = 0; w < (int)image_width; ++w) {
            int index = (h * row_size) + (w * bytes_per_pixel);
            if (bytes_per_pixel == 4) {
                image_data->data[index + 0] = temp_data[index + 2];
                image_data->data[index + 1] = temp_data[index + 1];
                image_data->data[index + 2] = temp_data[index + 0];
                image_data->data[index + 3] = temp_data[index + 3];
            } else if (bytes_per_pixel == 3) {
                image_data->data[index + 0] = temp_data[index + 0];
                image_data->data[index + 1] = temp_data[index + 1];
                image_data->data[index + 2] = temp_data[index + 2];
            }
        }
    }
    free(temp_data);
    return image_size;
}

void FreeBitmap(ImageData* img_data) {
    img_data->width = 0;
    img_data->height = 0;
    img_data->bytes_per_pixel = 0;
    free(img_data->data);
}