/* list_dynamic_symbol.c
* Usage: list_dynamic_symbol <exec_file_name>
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <elf.h>

/* Little/Big Endian Translation Macro */
#define	RDKEndianSwap32Macro(VALUE)	\
	(((VALUE >> 24) & 0x000000FF) |		\
		((VALUE >>  8) & 0x0000FF00) |	\
		((VALUE <<  8) & 0x00FF0000) |	\
		((VALUE << 24) & 0xFF000000))

#define	RDKEndianSwap16Macro(VALUE)	\
	(((VALUE >>  8) & 0x00FF) |				\
		((VALUE <<  8) & 0xFF00))

/* Main routine */
int main(int argc, char *argv[])
{
	Elf32_Ehdr *e_hdr_ptr;
	Elf32_Shdr *s_hdr_ptr;

	Elf32_Sym *symptr;

	int fd, i;
	unsigned char buf[256];
	char endian_flag;

	unsigned int SecName, SecHdrFileOffset, SecHdrTblEntrSize, SecHdrTblEntrNum, NamStrSecTblIndex;

	unsigned int SecNamStrTblFileOffset = 0, SecNamStrTblSize = 0;
	char *SecNameStrTable;

	unsigned char SymTblEntry[32];

	unsigned int DynStrTblFileOffset = 0;
	int DynStrTblNum = 0, dyn_count = 0;
	unsigned int DynStrNamStrTblFileOffset = 0;
	char *DynStrNamStrTable;
	unsigned int DynStrNamStrTblSize = 0;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <elf_file_name>\n", argv[0]);
		exit(1);
	}

	if ((fd = open(argv[1], O_RDONLY)) == -1) {
		fprintf(stderr, "Can't open file \"%s\".\n", argv[1]);
		exit(1);
	}

	if (read(fd, buf, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
		fprintf(stderr, "read elf header error\n");
		close(fd);
		exit(1);
	}
	e_hdr_ptr = (Elf32_Ehdr *)buf;
	endian_flag = e_hdr_ptr->e_ident[EI_DATA];
	if (endian_flag == ELFDATA2MSB)
	{
		e_hdr_ptr->e_shoff = RDKEndianSwap32Macro(e_hdr_ptr->e_shoff);
		e_hdr_ptr->e_shentsize = RDKEndianSwap16Macro(e_hdr_ptr->e_shentsize);
		e_hdr_ptr->e_shnum = RDKEndianSwap16Macro(e_hdr_ptr->e_shnum);
		e_hdr_ptr->e_shstrndx = RDKEndianSwap16Macro(e_hdr_ptr->e_shstrndx);
	}
	SecHdrFileOffset = (unsigned int)e_hdr_ptr->e_shoff;
	SecHdrTblEntrSize = (unsigned int)e_hdr_ptr->e_shentsize;
	SecHdrTblEntrNum = (unsigned int)e_hdr_ptr->e_shnum;
	NamStrSecTblIndex = (unsigned int)e_hdr_ptr->e_shstrndx;
	
	if (SecHdrTblEntrNum == 0) {
		fprintf(stderr, "There are no sections in file \"%s\".\n", argv[1]);
		exit(1);
	}
	
	// Set the section name string table offset
	SecNamStrTblFileOffset = SecHdrFileOffset + NamStrSecTblIndex * SecHdrTblEntrSize;
	if (lseek(fd, (off_t)SecNamStrTblFileOffset, SEEK_SET) != SecNamStrTblFileOffset
			|| SecNamStrTblFileOffset == 0) {
		fprintf(stderr, "lseek to Section Table Entry for Section Name String Table error.\n");
		close(fd);
		exit(1);
	}
	if (read(fd, buf, (size_t)SecHdrTblEntrSize) != (ssize_t)SecHdrTblEntrSize) {
		fprintf(stderr, "read section table entry for section name string table error\n");
		close(fd);
		exit(1);
	}
	s_hdr_ptr = (Elf32_Shdr *)buf;
	if (endian_flag == ELFDATA2MSB)
	{
		s_hdr_ptr->sh_offset = RDKEndianSwap32Macro(s_hdr_ptr->sh_offset);
		s_hdr_ptr->sh_size = RDKEndianSwap32Macro(s_hdr_ptr->sh_size);
	}
	SecNamStrTblFileOffset = (unsigned int)s_hdr_ptr->sh_offset;
	SecNamStrTblSize = (unsigned int)s_hdr_ptr->sh_size;

	// Read the section name string table and buffer it
	if (lseek(fd, (off_t)SecNamStrTblFileOffset, SEEK_SET) != SecNamStrTblFileOffset
			|| SecNamStrTblFileOffset == 0 ) {
		fprintf(stderr, "lseek to Section Name String Table error.\n");
		close(fd);
		exit(1);
   }
	SecNameStrTable = (char*)malloc((size_t)SecNamStrTblSize);
	if (read(fd, SecNameStrTable, (size_t)SecNamStrTblSize) != (ssize_t)SecNamStrTblSize) {
		fprintf(stderr, "read section name string table error\n");
		free(SecNameStrTable);
		close(fd);
		exit(1);
	}

	// Retrieve section headers
	if (lseek(fd, (off_t)SecHdrFileOffset, SEEK_SET) != SecHdrFileOffset || SecHdrFileOffset == 0) {
		fprintf(stderr, "lseek to section header error.\n");
		free(SecNameStrTable);
		close(fd);
		exit(1);
	}
	for (i = 0; i < (int)SecHdrTblEntrNum; i++) {
		if (read(fd, buf, (size_t)SecHdrTblEntrSize) != (ssize_t)SecHdrTblEntrSize) {
			fprintf(stderr, "read section header table error\n");
			free(SecNameStrTable);
			close(fd);
			exit(1);
		}
		s_hdr_ptr = (Elf32_Shdr *)buf;
		if (endian_flag == ELFDATA2MSB)
		{
			s_hdr_ptr->sh_name = RDKEndianSwap32Macro(s_hdr_ptr->sh_name);
			s_hdr_ptr->sh_offset = RDKEndianSwap32Macro(s_hdr_ptr->sh_offset);
			s_hdr_ptr->sh_size = RDKEndianSwap32Macro(s_hdr_ptr->sh_size);
			s_hdr_ptr->sh_entsize = RDKEndianSwap32Macro(s_hdr_ptr->sh_entsize);
		}
		if (strcmp(SecNameStrTable + s_hdr_ptr->sh_name, ".dynsym") == 0) {
			DynStrTblFileOffset = (unsigned int)s_hdr_ptr->sh_offset;
			DynStrTblNum = (int)((s_hdr_ptr->sh_size)/(s_hdr_ptr->sh_entsize));
		}
		if (strcmp(SecNameStrTable + s_hdr_ptr->sh_name, ".dynstr") == 0) {
			DynStrNamStrTblFileOffset = (unsigned int)s_hdr_ptr->sh_offset;
			DynStrNamStrTblSize = (unsigned int)s_hdr_ptr->sh_size;
		}
	}
	free(SecNameStrTable);

	// Read .dynsym section content and buffer it
	if (lseek(fd, (off_t)DynStrNamStrTblFileOffset, SEEK_SET) != DynStrNamStrTblFileOffset
			|| DynStrNamStrTblFileOffset == 0)
		fprintf(stderr, "lseek to Dynamical symbol name String Table error.\n");
	DynStrNamStrTable = (char*)malloc((size_t)(DynStrNamStrTblSize + 1));
	read(fd, DynStrNamStrTable, (size_t)(DynStrNamStrTblSize + 1));

	// Retrieve symbol table
	if (lseek(fd, (off_t)DynStrTblFileOffset, SEEK_SET) != DynStrTblFileOffset || DynStrTblFileOffset == 0) {
		fprintf(stderr, "lseek to Dynamical Symbol Table error.\n");
		free(DynStrNamStrTable);
		close(fd);
		exit(-1);
	}
	dyn_count = DynStrTblNum;
	for (i = 0; i < DynStrTblNum; i++) {
		read(fd, SymTblEntry, (size_t)(sizeof(Elf32_Sym)));
		symptr = (Elf32_Sym *)SymTblEntry;
		if (endian_flag == ELFDATA2MSB)
			symptr->st_name = RDKEndianSwap32Macro(symptr->st_name);
		if (symptr->st_name != 0 && symptr->st_info != 0x13)
			fprintf(stdout, "%s\n", DynStrNamStrTable + symptr->st_name);
		else
			dyn_count --;
	}
//	fprintf(stdout, "Dynamic symbol table count: %d\n", dyn_count);

	free(DynStrNamStrTable);
	close(fd);
	return 0;
}
