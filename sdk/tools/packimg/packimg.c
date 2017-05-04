/*
  Pack kernel and compressed file system into
  bootloader recognizable format.
*/
#include <stdio.h>


int main(int argc, char **argv)
{
  FILE *kernel_img, *fs_image, *out_img;
  char *defaultname="vm.img";
  char *ptr1, *ptr2, *ptr3;
  struct stat         fileStat;
  
    if ((argc!=3)&&(argc!=4)) {
    	printf("usage: packimg kernel_img fs_image [output_name]\n");
    	goto error_return;
    };
	kernel_img = fopen(argv[1], "rb");
	if (!kernel_img) {
		printf("Kernel image file %s open error!\n", argv[1]);
    	goto error_return;
	};
	fs_image   = fopen(argv[2], "rb");
	if (!fs_image) {
		printf("File system image file %s open error!\n", argv[2]);
    	goto error_return;
	};
	if (argc==4)
	    out_img   = fopen(argv[3], "wb");
	else
	    out_img   = fopen(defaultname, "wb");
	if (!out_img) {
		printf("Output file %s create error!\n", (argc==4)?argv[2]:defaultname);
    	goto error_return;
	};
    
    //read input file
    /* Get file size */
    if (stat(argv[1], &fileStat) != 0)
    {
        printf("Cannot get file statistics !!\n");
        fclose(fp1);
        exit(-1);
    }
    imgSize = fileStat.st_size;
    ptr1 = 
    
    
    return 0;
    
error_return:
    return -1;	
}