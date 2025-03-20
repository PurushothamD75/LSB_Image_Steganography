***Project Overview: This project implements Least Significant Bit (LSB) Image Steganography to hide a secret message within a .bmp or .png image file. Steganography is a technique of concealing information within files, making it valuable in areas requiring covert data transmission, such as military, intelligence, and secure message passing.

**What is Steganography? Steganography is the practice of hiding data within another medium. This project utilizes digital steganography, where the message is hidden within the least significant bits (LSB) of an image's pixel data. By using LSBs, the quality of the image remains visually unchanged while embedding data discreetly.

**Why Use Steganography? *Hide Secret Credentials *Prevent Data Forgery *Enable Secure Message Passing *Applications of Steganography *Steganography is widely used in:

**Military Communications: *Intelligence Agencies *Medical Imaging *Internet Banking *Secure Online Elections

**BMP Image Format and Pixel Structure:

*BMP File Structure: The BMP file format consists of a 54-byte header, followed by RGB pixel data, making it suitable for LSB embedding.

*Pixel Structure: Each pixel in BMP images is represented by RGB values. Modifying the LSB of these values minimally impacts the image, making BMP a preferred format for LSB steganography.

*How LSB Image Steganography Works: In this project, the secret data is encoded bit-by-bit into the LSB of each pixel in a BMP file. During decoding, these bits are extracted to reconstruct the hidden message.

**Features: Encoding: Hides a secret text file within an image file. Decoding: Retrieves the hidden message from an encoded image. Error Handling: Verifies file formats and provides error messages if encoding fails due to capacity limitations.

**Usage Instructions:

*Command-Line Interface

->Encoding a Message: ./lsb_steg -e <image.bmp> <secret.txt> [output_file]

<image.bmp>: The BMP image in which to hide the secret. <secret.txt>: The text file containing the secret message. [output_file]: Optional output file name. Default is steged_img.bmp.

->Decoding a Message: ./lsb_steg -d <encoded_image.bmp> [output_file]

<encoded_image.bmp>: The BMP image with the hidden message. [output_file]: Optional output file for the decoded message. Default is decoded.txt.

**Example Usage:

Encoding: ./lsb_steg -e original.bmp secret.txt steged_img.bmp Decoding:./lsb_steg -d steged_img.bmp decoded.txt

**Design Overview:

Encoding Process ->Check Image Capacity: Verifies if the image has enough capacity to hold the secret message. ->Embed Magic String: Adds a unique identifier (#) to detect hidden data during decoding. ->Encode Metadata: Stores the file extension and message size. ->Embed Message: Hides the actual message bit-by-bit in the image. ->Save Encoded Image: Outputs a new image file with the embedded message.

*Decoding Process ->Verify Magic String: Confirms the presence of hidden data. ->Extract Metadata: Reads the file extension and size of the secret message. ->Retrieve Message: Decodes the hidden message bit-by-bit. ->Save Decoded Message: Outputs the hidden message as a text file.

**Error Handling:

*The program provides error messages if: ->The image file lacks the required capacity to embed the message. ->Incorrect file formats are provided for encoding or decoding.

**References: Wikipedia - Steganography Wikipedia - Pixel BMP File Structure
