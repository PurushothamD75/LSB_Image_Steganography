#include <stdio.h>
#include "decode.h"
#include "types.h"
#include <string.h>
#include "common.h"
#include <stdlib.h>
#include <unistd.h> // For sleep()

// Function definition for read and validate decode args
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Ensure the source image file is a .bmp file
    if (strstr(argv[2], ".bmp") == NULL) {
        printf("Decoding validation failed: Invalid image file (must be .bmp).\n");
        return e_failure;
    }
    decInfo->d_src_image_fname = argv[2];  // Assign source image filename

    // Validate secret file: If not provided, default to "decode.txt"
    if (argv[3] != NULL) {
        decInfo->d_secret_fname = argv[3];
    } else {
        decInfo->d_secret_fname = "decode.txt";  // Default filename
    }

    return e_success;  // Return success if all validation passed
}

// Function definition for opening files for decoding
Status open_files_dec(DecodeInfo *decInfo)
{
    // Open the source image file (stego image) in read mode
    decInfo->fptr_d_src_image = fopen(decInfo->d_src_image_fname, "r");
    if (decInfo->fptr_d_src_image == NULL) {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->d_src_image_fname);
        return e_failure;
    }

    // Open the secret file in write mode to store the decoded data
    decInfo->fptr_d_secret = fopen(decInfo->d_secret_fname, "w");
    if (decInfo->fptr_d_secret == NULL) {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->d_secret_fname);
        return e_failure;
    }

    return e_success;  // Return success if both files are opened
}

// Function definition for decoding magic string from image
Status decode_magic_string(DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_d_src_image, 54, SEEK_SET);  // Skip the first 54 bytes (BMP header)
    int i = strlen(MAGIC_STRING);
    decInfo->magic_data = malloc(strlen(MAGIC_STRING) + 1);  // Allocate memory for magic string

    // Decode the magic string from the image
    decode_data_from_image(strlen(MAGIC_STRING), decInfo->fptr_d_src_image, decInfo);
    decInfo->magic_data[i] = '\0';  // Null-terminate the decoded magic string

    // Verify if the decoded string matches the expected magic string
    if (strcmp(decInfo->magic_data, MAGIC_STRING) == 0) {
        return e_success;  // Return success if magic string is correct
    } else {
        return e_failure;  // Return failure if magic string doesn't match
    }
}

// Function definition for decoding data (characters) from image
Status decode_data_from_image(int size, FILE *fptr_d_src_image, DecodeInfo *decInfo)
{
    int i;
    char str[8];

    // Read 8 bits (1 byte) at a time and decode it from LSB
    for (i = 0; i < size; i++) {
        fread(str, 8, sizeof(char), fptr_d_src_image);  // Read 8 bits from the image
        decode_byte_from_lsb(&decInfo->magic_data[i], str);  // Decode the byte from LSB
    }

    return e_success;  // Return success after decoding all data
}

// Function definition for decoding a single byte from LSB (Least Significant Bit)
Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    int bit = 7;  // Start from the most significant bit (MSB)
    unsigned char ch = 0x00;

    // Extract the LSB from each byte in the image buffer
    for (int i = 0; i < 8; i++) {
        ch = ((image_buffer[i] & 0x01) << bit--) | ch;  // Shift the LSB into place
    }

    *data = ch;  // Store the decoded byte
    return e_success;  // Return success
}

// Function definition for decoding file extension size from the image
Status decode_file_extn_size(int size, FILE *fptr_d_src_image)
{
    char str[32];
    int length;

    // Read the 32 bits representing the extension size from the image
    fread(str, 32, sizeof(char), fptr_d_src_image);
    decode_size_from_lsb(str, &length);  // Decode the size value from LSB

    // Verify if the decoded size matches the expected size
    if (length == size) {
        return e_success;  // Return success if size matches
    } else {
        return e_failure;  // Return failure if size doesn't match
    }
}

// Function definition for decoding size (32-bit integer) from LSB
Status decode_size_from_lsb(char *buffer, int *size)
{
    int j = 31;
    int num = 0x00;

    // Decode a 32-bit integer by extracting each LSB from the buffer
    for (int i = 0; i < 32; i++) {
        num = ((buffer[i] & 0x01) << j--) | num;
    }

    *size = num;  // Store the decoded size value
    return e_success;  // Return success
}

// Function definition for decoding secret file extension from the image
Status decode_secret_file_extn(char *file_ext, DecodeInfo *decInfo)
{
    file_ext = ".txt";  // Assume the secret file extension is ".txt"
    int i = strlen(file_ext);
    decInfo->d_extn_secret_file = malloc(i + 1);  // Allocate memory for the extension

    // Decode the file extension from the image
    decode_extension_data_from_image(strlen(file_ext), decInfo->fptr_d_src_image, decInfo);
    decInfo->d_extn_secret_file[i] = '\0';  // Null-terminate the decoded extension

    // Verify if the decoded extension matches the expected extension
    if (strcmp(decInfo->d_extn_secret_file, file_ext) == 0) {
        return e_success;  // Return success if extension matches
    } else {
        return e_failure;  // Return failure if extension doesn't match
    }
}

// Function definition for decoding extension data (string) from the image
Status decode_extension_data_from_image(int size, FILE *fptr_d_src_image, DecodeInfo *decInfo)
{
    for (int i = 0; i < size; i++) {
        fread(decInfo->d_src_image_fname, 8, 1, fptr_d_src_image);  // Read 8 bits from image
        decode_byte_from_lsb(&decInfo->d_extn_secret_file[i], decInfo->d_src_image_fname);  // Decode byte from LSB
    }
    return e_success;  // Return success after decoding all extension data
}

// Function definition for decoding secret file size from the image
Status decode_secret_file_size(int file_size, DecodeInfo *decInfo)
{
    char str[32];

    // Read the 32 bits representing the secret file size from the image
    fread(str, 32, sizeof(char), decInfo->fptr_d_src_image);
    decode_size_from_lsb(str, &file_size);  // Decode the size from LSB

    // Store the decoded size in the DecodeInfo structure
    decInfo->size_secret_file = file_size;
    return e_success;  // Return success
}

// Function definition for decoding secret file data from the image
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char ch;

    // Read and decode the secret file data from the image, then write to the secret file
    for (int i = 0; i < decInfo->size_secret_file; i++) {
        fread(decInfo->d_src_image_fname, 8, sizeof(char), decInfo->fptr_d_src_image);  // Read 8 bits
        decode_byte_from_lsb(&ch, decInfo->d_src_image_fname);  // Decode the byte
        fputc(ch, decInfo->fptr_d_secret);  // Write the decoded byte to the secret file
    }

    return e_success;  // Return success after decoding all secret file data
}

// Function definition for performing the entire decoding process
Status do_decoding(DecodeInfo *decInfo)
{
    // Open the necessary files (stego image and secret file) for decoding
    if (open_files_dec(decInfo) == e_success) {
        printf("Open files successfully.\n");
        sleep(1); // Delay for better visibility

        // Decode the magic string from the image
        if (decode_magic_string(decInfo) == e_success) {
            printf("Decoded magic string successfully.\n");
             sleep(1); // Delay for better visibility

            // Decode the file extension size from the image
            if (decode_file_extn_size(strlen(".txt"), decInfo->fptr_d_src_image) == e_success) {
                printf("Decoded file extension size successfully.\n");
                 sleep(1); // Delay for better visibility

                // Decode the secret file extension from the image
                if (decode_secret_file_extn(decInfo->d_extn_secret_file, decInfo) == e_success) {
                    printf("Decoded secret file extension successfully.\n");
                     sleep(1); // Delay for better visibility

                    // Decode the secret file size from the image
                    if (decode_secret_file_size(decInfo->size_secret_file, decInfo) == e_success) {
                        printf("Decoded secret file size successfully.\n");
                        sleep(1); // Delay for better visibility

                        // Decode the secret file data from the image and write it to the secret file
                        if (decode_secret_file_data(decInfo) == e_success) 
                        {
                            printf("Decoded secret file data successfully.\n");
                            sleep(1); // Delay for better visibility
                        } else {
                            printf("Decoding of secret file data failed.\n");
                        }
                    } else {
                        printf("Decoding of secret file size failed.\n");
                        return e_failure;
                    }
                } else {
                    printf("Decoding of secret file extension failed.\n");
                    return e_failure;
                }
            } else {
                printf("Decoding of file extension size failed.\n");
                return e_failure;
            }
        } else {
            printf("Decoding of magic string failed.\n");
            return e_failure;
        }
    } else {
        printf("Opening files failed.\n");
        return e_failure;
    }

    return e_success;  // Return success if everything decoded successfully
}
