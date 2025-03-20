#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "common.h"
#include "types.h"
#include <unistd.h> // For sleep()

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    // Seek to the 18th byte where width is stored in BMP header
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity: width * height * bytes per pixel (3 for RGB)
    return width * height * 3;
}

/*
 * Open the source image, secret file, and destination (stego) image files
 * Inputs: Src Image file, Secret file, and Stego Image file
 * Output: FILE pointers for the above files
 * Return Value: e_success or e_failure, based on file open errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Open the source image file in read mode
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    // Open the secret file (text file to hide in the image)
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }

    // Open the stego image file for writing (destination image)
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }

    // No failure, return e_success
    return e_success;
}



// Validate the command-line arguments for encoding
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Step 1: Check if the source image file is a BMP file
    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("Error: source image file must be .bmp file\n");
        return e_failure;
    }
    encInfo->src_image_fname = argv[2];  // Store the source file name

    // Step 2: Check if the secret file is a text file
    if (strstr(argv[3], ".txt") == NULL)
    {
        printf("Error: secret file must be .txt file\n");
        return e_failure;
    }
    encInfo->secret_fname = argv[3];  // Store the secret file name

    // Step 3: Check if the stego output image file is a BMP file
    if (strstr(argv[4], ".bmp") == NULL)
    {
        printf("Error: stego image file must be .bmp file\n");
        return e_failure;
    }
    encInfo->stego_image_fname = argv[4];  // Store the output file name

    return e_success;
}

// Get the size of the secret file
uint get_file_size(FILE *fptr)
{
    // Seek to the end of the file to get the size
    fseek(fptr, 0, SEEK_END);
    return ftell(fptr);  // Return the size of the file
}

// Check the capacity of the source image to hold the secret file
Status check_capacity(EncodeInfo *encInfo)
{
    // Get the image capacity (width * height * 3 for RGB)
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    // Get the size of the secret file
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // Calculate the total number of bytes required to store the image header, magic string, secret file metadata (extension, size), and the secret file data itself
    int total_bytes = 54 + strlen(MAGIC_STRING) * 8 + 32 + 32 + 32 + encInfo->size_secret_file * 8;

    // Check if the image capacity is enough to store the secret file and metadata
    if (total_bytes <= encInfo->image_capacity)
    {
        return e_success;  // Enough space in the image
    }
    else
    {
        return e_failure;  // Not enough space in the image
    }
}

/* Copy the first 54 bytes (BMP header) from the source image to the destination (stego) image */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char str[54];

    // Seek to the beginning of the image file (0th byte)
    fseek(fptr_src_image, 0, SEEK_SET);

    // Read the 54-byte BMP header from the source image
    fread(str, 54, 1, fptr_src_image);

    // Write the 54-byte BMP header to the destination (stego image)
    fwrite(str, 54, 1, fptr_dest_image);
    return e_success;
}

/* Encode the secret file data into the image */
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char str[8];
    for (int i = 0; i < size; i++)
    {
        // Read 8 bytes from the source image (or the image buffer)
        fread(str, 8, 1, fptr_src_image);

        // Encode each byte of data into the least significant bits (LSB) of the image buffer
        encode_byte_to_lsb(data[i], str);

        // Write the modified image buffer to the stego image
        fwrite(str, 8, 1, fptr_stego_image);
    }
    return e_success;
}

/* Encode the magic string into the stego image */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image((char*) magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

/* Encode the secret file extension size into the stego image */
Status encode_secret_file_extn_size(int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char str[32];
    fread(str, 32, 1, fptr_src_image);
    encode_size_to_lsb(size, str);  // Encode the size into the LSB
    fwrite(str, 32, 1, fptr_stego_image);
    return e_success;
}

/* Encode the secret file extension into the stego image */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image((char *)file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

/* Encode the secret file size into the stego image */
Status encode_secret_file_size(long int size, EncodeInfo *encInfo)
{
    char str[32];
    fread(str, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(size, str);  // Encode the size into the LSB
    fwrite(str, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

/* Encode the secret file data into the stego image */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    char str[encInfo->size_secret_file];
    fread(str, encInfo->size_secret_file, 1, encInfo->fptr_secret);
    encode_data_to_image(str, strlen(str), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

/* Encode a single byte of data into the LSB of the image buffer */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    int count = 0;
    for (int i = 7; i >= 0; i--)
    {
        if ((data >> i) & 1)
        {
            image_buffer[count] |= 1;  // Set LSB to 1
        }
        else
        {
            image_buffer[count] &= ~1;  // Set LSB to 0
        }
        count++;
    }
    return e_success;
}

/* Encode the size value into the LSB of the image buffer */
Status encode_size_to_lsb(int size, char *image_buffer)
{
    int count = 0;
    for (int i = 31; i >= 0; i--)
    {
        if ((size >> i) & 1)
        {
            image_buffer[count] |= 1;  // Set LSB to 1
        }
        else
        {
            image_buffer[count] &= ~1;  // Set LSB to 0
        }
        count++;
    }
    return e_success;
}

/* Copy the remaining data from the source image to the destination image */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while ((fread(&ch, 1, 1, fptr_src)) > 0)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}

/* Perform the entire encoding process: embedding the secret file into the image */
Status do_encoding(EncodeInfo *encInfo)
{
    // Open necessary files (source image, secret file, stego image)
    if (open_files(encInfo) == e_success)
    {
        printf("Open files is Success\n");
        sleep(1); // Delay for better visibility

        // Check if the image has enough capacity for the secret file
        if (check_capacity(encInfo) == e_success)
        {
            printf("Check Capacity is Success\n");
            sleep(1); // Delay for better visibility

            // Copy the BMP header (54 bytes) from source to stego image
            if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("Copying bmp header is Success\n");
                  sleep(1); // Delay for better visibility

                // Encode the magic string into the stego image
                if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("Encoded Magic string is Successful\n");
                     sleep(1); // Delay for better visibility

                    // Get and encode the secret file extension
                    strcpy(encInfo->extn_secret_file, strstr(encInfo->secret_fname, "."));
                    printf("Got secret file extension\n");
                     sleep(1); // Delay for better visibility

                    // Encode the secret file extension size and extension into the image
                    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                    {
                        printf("Encoding Secret file extension size is successful\n");
                         sleep(1); // Delay for better visibility

                        // Encode the secret file data size and data itself into the image
                        if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            printf("Secret file extension is encoded succesfully\n");
                             sleep(1); // Delay for better visibility

                            if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("Secret file size is encoded successfully\n");
                                sleep(1); // Delay for better visibility

                                if (encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("Secret file data is encoded successfully\n");
                                     sleep(1); // Delay for better visibility

                                    // Copy the remaining image data from source to destination (stego image)
                                    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                    {
                                        printf("Remaining image data is copied successfully\n");
                                        return e_success;
                                    }
                                    else
                                    {
                                        printf("ERROR : Remaining image data is not copied\n");
                                    }
                                }
                                else
                                {
                                    printf("ERROR : Encoding Secret file data failed\n");
                                }
                            }
                            else
                            {
                                printf("ERROR : Encoding Secret file size failed\n");
                            }
                        }
                        else
                        {
                            printf("ERROR : Encoding Secret file extension failed\n");
                        }
                    }
                    else
                    {
                        printf("ERROR : Encoding Secret file extension size failed\n");
                    }
                }
                else
                {
                    printf("ERROR : Magic string encoding failed\n");
                }
            }
            else
            {
                printf("ERROR : Copying BMP header failed\n");
            }
        }
        else
        {
            printf("ERROR : Capacity check failed\n");
        }
    }
    else
    {
        printf("ERROR : File opening failed\n");
        return e_failure;
    }
}
