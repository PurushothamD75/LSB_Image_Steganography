/*
NAME: PURUSHOTHAM D

DATE: 11-11-2024

Project Name:LSB Image Steganography

DESCRIPTION: LSB (Least Significant Bit) steganography is a technique for hiding secret information in digital media,
 such as images or audio, by altering the least significant bits of the data, which causes minimal perceptible changes to the file.
*/

#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include <string.h>


int main( int argc, char *argv[] )
{

	// Validate argument count
    if (argc < 2)
    {
        printf("INFO: Please pass valid arguments.");
		printf("\nINFO:Encodeing - Minimum 4 arguments.\n Usage:- ./a.out -e source_image_file secret_data_file [Destination_image_file]\n");
		printf("\nINFO:Decodeing - Minimum 3 arguments.\n Usage:- ./a.out -d source_image_file  [Destination_image_file]\n");
        return e_failure;
    }
    //Decalring encoding structure variable
    EncodeInfo encInfo;

    //Declaring decoding structure variable
    DecodeInfo decInfo;


OperationType check_operation_type(char *argv[])
{
    //step1: compare with argv with -e
    //step2: if yes -> return e_encode,no goto step3
    //step3: compare argv with -d
    //step4: if yes -> return e_encode,no goto step5
    //step5: return e_unsupported
    if(!strcmp(argv[1],"-e"))
	{
		if(argc < 4)
		{
			printf("INFO: for Encodeing - Minimum 4 arguments need to pass like ./a.out -e source_image_file secret_data_file [Destination_image_file]\n");
			return e_unsupported;
		}
        return e_encode;
	}
    else if(!strcmp(argv[1],"-d"))
	{
		if(argc < 3)
		{
		printf("INFO: for Decodeing - Minimum 3 arguments need to pass like ./a.out -d source_image_file [Destination_image_file]\n");
		return e_unsupported;
	    }
    return e_decode;
}
    else
	{
    return e_unsupported;
	}
}



    // To check any other arguments passed along with ./a.out file or not
    if (argc > 1 )
    {
	// To check whether which operation we want to perform
	switch ( check_operation_type(argv) )
	{
	    case e_encode :

		// To read and validate the arguments we passed
		if ( read_and_validate_encode_args(argv, &encInfo) == e_success )
		{
		    // Encoding process begin
		    if ( do_encoding(&encInfo) == e_success )
		    {
			printf("<---- Encoding successfully done ---->\n");
		    }
		    else
		    {
			printf("ERROR : Failed to encode.\n");
		    }
		}
		else
		{
		    printf("ERROR : Read and validation failed.\n");
		}
		break;

	    case e_decode :

		// To read and validate the arguments we passed
		if ( read_and_validate_decode_args(argv, &decInfo) == e_success )
		{
		    // Decoding process begin
		    if ( do_decoding(&decInfo) == e_success )
		    {
			printf("<---- Decoding successfully done ---->\n");
		    }
		    else
		    {
			printf("ERROR : Failed to decode.\n");
		    }
		}
		else
		{
		    printf("ERROR : Read and validation failed.\n");
		}
		break;

	    case e_unsupported :

		// Error handling
		printf("ERROR : Invalid option.\n");
		break;
	}
    }
    else
    {
	// Error handling
	printf("ERROR : Please pass sufficient number of arguments.\n");
    }
    return 0;
}
