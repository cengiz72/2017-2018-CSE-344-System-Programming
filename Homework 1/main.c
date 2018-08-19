#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <errno.h>
#include <math.h>
#define IMAGE_WITDH     256
#define IMAGE_LENGTH    257
#define MOTOROLLA 		77
#define INTEL 			73
#define BITSPERSAMPLE   258
#define SAMPLEPERPIXEL  277
#define STRIPOFFSET 	273
#define DWORD      uint32_t
#define WORD       uint16_t
/*Each entry occurs 12 bytes. */
typedef struct tag {
	WORD tagType;
	WORD tagField;
	DWORD numberOfItems;
	DWORD tagValue;

}TAG;
/*
  This function finds tags needed for image like length,witdh,stripoffset,etc.
  @param buffersize, size of image.
  @param tagArray , an array for five needed tags.
*/
void findTags(TAG tagArray[],int neededTags,int M_Or_I,unsigned char *buffer,int bufferSize);
/*
	This function convert one byte into eight bits and store eight bits into an array.
	@param bits ,array for storing bits obtained from one byte.
	@param byte, it is byte that is converted to bits 
*/
void byteToBits(unsigned char *bits,unsigned char byte);
/*
	This function print the pixels as 0 or 1 with respect to pixel value.
	0--> black
	1--> white
	@param M_Or_I ,indicate byte order ,Motorolla or Intel.
*/
void printPixels(TAG *tagArray,int neededTags,int M_Or_I,const unsigned char * buffer);
/*
	return the size of given image.
*/
long int fsize(const char *filename);
/*
	determine the byte order of image.
*/
int IsIntelOrMotorolla(const char *filename);
/*
  create one peace of 4 bytes by using 4 peace of byte.  
*/
uint32_t bytesToDword(int M_Or_I,const unsigned char * buffer);
/*
  create one peace of 2 bytes by using 2 peace of byte.  
*/
uint16_t bytesToWord(int M_Or_I,const unsigned char * buffer);
/*
  operation for motorolla  
*/
void motorolla(long int size,const char *filename);
/*
  operation for intel.
*/
void intel(long int size,const char *filename);
int main(int argc,char *argv []) {

	if (IsIntelOrMotorolla(argv[1]))
		intel(fsize(argv[1]),argv[1]);
	else 
		motorolla(fsize(argv[1]),argv[1]);
	return 0;
}

long int fsize(const char *filename) {
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
}
void byteToBits(unsigned char *bits,unsigned char byte) {
    for (int i = 0; i < 8; ++i)
		bits[7-i]= (byte >> i) & 1;
		
}
void printPixels(TAG *tagArray,int neededTags,int M_Or_I,const unsigned char * buffer) {

	uint32_t length,witdh,bitsPerSample,samplePerPixel,bitsPerPixel,stripOffset;
	int i = 0,j = 0, newWidth;
	unsigned char * totalPixels;
	for (i = 0; i < neededTags; ++i) {

		switch(tagArray[i].tagType) {
			case IMAGE_LENGTH   :
								  length = tagArray[i].tagValue;
							      break;
			case IMAGE_WITDH    : 
								  witdh = tagArray[i].tagValue;
								  break;

			case BITSPERSAMPLE  : 
								  bitsPerSample = tagArray[i].tagValue;
								  break;

			case SAMPLEPERPIXEL : 
								  samplePerPixel = tagArray[i].tagValue; 
								  break;

			case STRIPOFFSET    : 
								  stripOffset = tagArray[i].tagValue; 
								  break;
		}
	}

	printf("Witdh       :  %d pixels\n",witdh);
	printf("Height      :  %d pixels\n",length);
	if (M_Or_I == 0)
		printf("Byte-order  :  Motorolla\n");
	else 
		printf("Byte-order  :  Intel\n");
	bitsPerPixel = samplePerPixel * bitsPerSample;
	if (bitsPerPixel == 1) {
		newWidth = 8 - witdh % 8 + witdh;
		totalPixels  = (unsigned char *) malloc(newWidth);
		for (i = 0; i < length; ++i) {
			
			for (j = 0; j < newWidth/8; ++j) {
				byteToBits(&totalPixels[0 + j*8],*(buffer + stripOffset + i*(newWidth/8) + j));
			}

			for (int k = 0; k < witdh; ++k) {
                 printf("%d",totalPixels[k]);
			}
			printf("\n");
		}
		free(totalPixels);
	}

	else {
		for (i = 0; i < length; ++i) {
			for (j = 0; j < witdh; ++j) {
			
			    if (*(buffer + stripOffset + i * witdh + j)  == 0)
					printf("0");
				else
					printf("1");
			}
			printf("\n");
		}
	}
}
int IsIntelOrMotorolla(const char *filename) {

	unsigned char byteOrder;
	int flag = 0;
	int file = open(filename,O_RDONLY);
	if (file == -1) {
		perror("File not opened!\n");
		close(file);
		exit(-1);	
	}

	while(!flag)

		if (read(file,&byteOrder,sizeof(unsigned char)) == sizeof(unsigned char))
			flag = 1;
	
	close(file);
	if (byteOrder == MOTOROLLA) return 0;
	else if (byteOrder == INTEL) return 1;
}
void findTags(TAG tagArray[],int neededTags,int M_Or_I,unsigned char *buffer,int bufferSize) {
	
    uint32_t offsetToImage,offsetFromTheIFD;
    uint16_t numberOfTags;
    int i;
    int flag = 0;
	offsetToImage = bytesToDword(M_Or_I,buffer + 4);
	numberOfTags = bytesToWord(M_Or_I,buffer + offsetToImage);
	offsetFromTheIFD = offsetToImage + 2;
	for (i = 0; i < numberOfTags; ++i) {
		
		switch(bytesToWord(M_Or_I,buffer + offsetFromTheIFD)) {
			case IMAGE_LENGTH   :
								tagArray[neededTags].tagType = IMAGE_LENGTH;
								offsetFromTheIFD = offsetFromTheIFD + 2; // tag type
								tagArray[neededTags].tagField = bytesToWord(M_Or_I,buffer + offsetFromTheIFD);
								offsetFromTheIFD = offsetFromTheIFD + 2; // tag field
								offsetFromTheIFD = offsetFromTheIFD + 4; // tag number of item

								if (tagArray[neededTags].tagField == 4)
							    	tagArray[neededTags].tagValue = bytesToDword(M_Or_I,buffer + offsetFromTheIFD);
								else if (tagArray[neededTags].tagField == 3)
									tagArray[neededTags].tagValue = bytesToWord(M_Or_I,buffer + offsetFromTheIFD);

								offsetFromTheIFD = offsetFromTheIFD + 4; // tag value
								++neededTags;
								break;
			case IMAGE_WITDH    : 
								tagArray[neededTags].tagType = IMAGE_WITDH;
								offsetFromTheIFD = offsetFromTheIFD + 2; // tag type
								tagArray[neededTags].tagField = bytesToWord(M_Or_I,buffer + offsetFromTheIFD);
								offsetFromTheIFD = offsetFromTheIFD + 2; // tag field
								offsetFromTheIFD = offsetFromTheIFD + 4; // tag number of item

								if (tagArray[neededTags].tagField == 4)
							    	tagArray[neededTags].tagValue = bytesToDword(M_Or_I,buffer + offsetFromTheIFD);
								else if (tagArray[neededTags].tagField == 3)
									tagArray[neededTags].tagValue = bytesToWord(M_Or_I,buffer + offsetFromTheIFD);

								offsetFromTheIFD = offsetFromTheIFD + 4; // tag value
								++neededTags;
								break;
			case BITSPERSAMPLE  :
								tagArray[neededTags].tagType = BITSPERSAMPLE;
								offsetFromTheIFD = offsetFromTheIFD + 2; // tag type
								offsetFromTheIFD = offsetFromTheIFD + 2; // tag field
								offsetFromTheIFD = offsetFromTheIFD + 4; // tag number of item
							    tagArray[neededTags].tagValue = bytesToWord(M_Or_I,buffer + offsetFromTheIFD);
								offsetFromTheIFD = offsetFromTheIFD + 4; // tag value
								++neededTags;
								flag = 1;
								break;
			case SAMPLEPERPIXEL : 
								tagArray[neededTags].tagType = SAMPLEPERPIXEL;
								offsetFromTheIFD = offsetFromTheIFD + 2; // tag type
								offsetFromTheIFD = offsetFromTheIFD + 2; // tag field
								offsetFromTheIFD = offsetFromTheIFD + 4; // tag number of item
							    tagArray[neededTags].tagValue = bytesToWord(M_Or_I,buffer + offsetFromTheIFD);
								offsetFromTheIFD = offsetFromTheIFD + 4; // tag value
								++neededTags;
								break;
			case STRIPOFFSET    :
								tagArray[neededTags].tagType = STRIPOFFSET;
								offsetFromTheIFD = offsetFromTheIFD + 2; // tag type
								offsetFromTheIFD = offsetFromTheIFD + 2; // tag field
								offsetFromTheIFD = offsetFromTheIFD + 4; // tag number of item
							    tagArray[neededTags].tagValue = bytesToDword(M_Or_I,buffer + offsetFromTheIFD);
								offsetFromTheIFD = offsetFromTheIFD + 4; // tag value
								++neededTags;
								break;
			default             : 
								offsetFromTheIFD = offsetFromTheIFD + 12; // no needed tag
								break;
		}
	}

	if (!flag) {
		tagArray[neededTags].tagValue = 1;
		tagArray[neededTags].tagType = BITSPERSAMPLE;
		++neededTags;
	}
}
void motorolla(long int size,const char *filename) {

	int file;
	int readInput;
	uint32_t offsetToImage , offsetFromTheIFD;
	int neededTags = 0;
	int flag = 0;
	unsigned char *buffer;
	TAG tagArray[5];
	buffer = (unsigned char*)malloc(size);
	file = open(filename,O_RDONLY);
	if (file == -1) {
		perror("File not opened!\n");
		close(file);
		exit(-1);	
	}
	readInput = read(file,buffer,size);
	close(file);
	findTags(tagArray,neededTags,0,buffer,size);
	neededTags = 5;
	printPixels(tagArray,neededTags,0,buffer);
	free(buffer);

}

void intel(long int size,const char *filename) {
	int file;
	int readInput;
	uint32_t offsetToImage , offsetFromTheIFD;
	int neededTags = 0;
	int flag = 0;
	unsigned char *buffer;
	TAG tagArray[5];
	buffer = (unsigned char*)malloc(size);
	file = open(filename,O_RDONLY);
	if (file == -1) {
		perror("File not opened!\n");
		close(file);
		exit(-1);	
	}
	readInput = read(file,buffer,size);
	findTags(tagArray,neededTags,1,buffer,size);
	neededTags = 5;
	printPixels(tagArray,neededTags,1,buffer);
	free(buffer);
	close(file);

}
uint32_t bytesToDword(int M_Or_I,const unsigned char * buffer) {

	uint32_t Word;
    if (M_Or_I == 0) {
		Word = (uint32_t )(*buffer);
		Word = Word << 8;
		Word = Word + (uint32_t )(*(buffer+1));
		Word = Word << 8;
		Word = Word + (uint32_t )(*(buffer+2));
		Word = Word << 8;
		Word = Word + (uint32_t )(*(buffer+3));
    }
    else {
		Word = (uint32_t )(*(buffer+3));
		Word = Word << 8;
		Word = Word + (uint32_t )(*(buffer+2));
		Word = Word << 8;
		Word = Word + (uint32_t )(*(buffer+1));
		Word = Word << 8;
		Word = Word + (uint32_t )(*buffer);
    }

	return Word;


}
uint16_t bytesToWord(int M_Or_I,const unsigned char * buffer) {

	uint16_t Word;
    if (M_Or_I == 0) {
		Word = (uint16_t)(*(buffer));
		Word = Word << 8;
		Word = Word + (uint16_t)(*(buffer+1));
    }
    else {
		Word = (uint16_t)(*(buffer + 1));
		Word = Word << 8;
		Word = Word + (uint16_t)(*(buffer));
    }

	return Word;

}
