/*
**  main.c
**  written by spike <spike666@mac.com>
**  www.sadistech.com
**  gcmtool.sourceforge.net
**  
**  part of the gcmtool project
**  commandline wrappers for some of the standard functions of GCMutils.h and GCMExtras.h
**
*/

//for the autoheader stuff... (grrr, I hate this thing...)
#if HAVE_CONFIG_H
#include <config.h>
#else
//if you're not configuring from the commandline...
#define VERSION ""
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GCMutils.h"
#include "GCMextras.h"

#include "pathfunc.h"

//commandline arguments
#define ARG_EXTRACT							"-e"
#define ARG_EXTRACT_SYN						""
#define ARG_EXTRACT_OPT						"<source> <dest>"
#define ARG_EXTRACT_HELP					"Extracts the file from <source> (in the GCM) to <dest> (local filesystem)"

#define ARG_VERBOSE							"-v"
#define ARG_VERBOSE_SYN						"--verbose"
#define ARG_VERBOSE_OPT						""
#define ARG_VERBOSE_HELP					"Verbose output"

#define ARG_INFO							"-i"
#define ARG_INFO_SYN						"--info"
#define ARG_INFO_OPT						""
#define ARG_INFO_HELP						"Display additional info about the GCM (useful for auditing)"

#define ARG_LIST							"-l"
#define ARG_LIST_SYN						"--list"
#define ARG_LIST_OPT						""
#define ARG_LIST_HELP						"Lists the filesystem of the GCM"

#define ARG_HEX								"-h"
#define ARG_HEX_SYN							"--hex"
#define ARG_HEX_OPT							""
#define ARG_HEX_HELP						"Display file offsets in hexadecimal notation"

#define ARG_HELP							"-?"
#define ARG_HELP_SYN						"--help"
#define ARG_HELP_OPT						""
#define ARG_HELP_HELP						"Displays this help text."

// commands:
// command_appreviation, command_synonym, command_helptext
// extracting sections...
#define ARG_EXTRACT_DISK_HEADER				"-edh"
#define ARG_EXTRACT_DISK_HEADER_SYN			"--extract-disk-header"
#define ARG_EXTRACT_DISK_HEADER_OPT			"[ " OPT_FILE " " OPT_FILE_OPT " ]"
#define ARG_EXTRACT_DISK_HEADER_HELP		"Extract the disk header (" GCM_DISK_HEADER_FILENAME ")"

#define ARG_EXTRACT_DISK_HEADER_INFO		"-edhi"
#define ARG_EXTRACT_DISK_HEADER_INFO_SYN	"--extract-disk-header-info"
#define ARG_EXTRACT_DISK_HEADER_INFO_OPT	"[ " OPT_FILE " " OPT_FILE_OPT " ]"
#define ARG_EXTRACT_DISK_HEADER_INFO_HELP   "Extract the disk header info (" GCM_DISK_HEADER_INFO_FILENAME ")"

#define ARG_EXTRACT_APPLOADER				"-eal"
#define ARG_EXTRACT_APPLOADER_SYN			"--extract-apploader"
#define ARG_EXTRACT_APPLOADER_OPT			"[ " OPT_FILE " " OPT_FILE_OPT " ]"
#define ARG_EXTRACT_APPLOADER_HELP			"Extract the apploader (" GCM_APPLOADER_FILENAME ")"

#define ARG_EXTRACT_BOOT_DOL				"-ed"
#define ARG_EXTRACT_BOOT_DOL_SYN			"--extract-boot-dol"
#define ARG_EXTRACT_BOOT_DOL_OPT			"[ " OPT_FILE " " OPT_FILE_OPT " ]"
#define ARG_EXTRACT_BOOT_DOL_HELP			"Extract the main executable DOL (" GCM_BOOT_DOL_FILENAME ")"

//injecting sections
#define ARG_INJECT_DISK_HEADER				"-idh"
#define ARG_INJECT_DISK_HEADER_SYN			"--inject-disk-header"
#define ARG_INJECT_DISK_HEADER_OPT			"[ " OPT_FILE " " OPT_FILE_OPT " ]"
#define ARG_INJECT_DISK_HEADER_HELP			"Inject the disk header"

#define ARG_INJECT_DISK_HEADER_INFO			"-idhi"
#define ARG_INJECT_DISK_HEADER_INFO_SYN		"--inject-disk-header-info"
#define ARG_INJECT_DISK_HEADER_INFO_OPT		"[ " OPT_FILE " " OPT_FILE_OPT " ]"
#define ARG_INJECT_DISK_HEADER_INFO_HELP	"Inject the disk header info"

#define ARG_INJECT_APPLOADER				"-ial"
#define ARG_INJECT_APPLOADER_SYN			"--inject-apploader"
#define ARG_INJECT_APPLOADER_OPT			"[ " OPT_FILE " " OPT_FILE_OPT " ]"
#define ARG_INJECT_APPLOADER_HELP			"Inject the apploader"

#define ARG_INJECT_BOOT_DOL					"-id"
#define ARG_INJECT_BOOT_DOL_SYN				"--inject-boot-dol"
#define ARG_INJECT_BOOT_DOL_OPT				"[ " OPT_FILE " " OPT_FILE_OPT " ]"
#define ARG_INJECT_BOOT_DOL_HELP			"Inject the main executable DOL"

//commandline options (modifiers to the arguments... hehe)
#define OPT_FILE							"-f"
#define OPT_FILE_OPT						"<filename>"

//macros... although they may be simple...
//these are for getting help and synonyms and stuff
#define ARG_SYN(ARG)		ARG ## _SYN
#define PRINT_HELP(ARG)		printf("\t" ARG "%s" ARG ## _SYN " " ARG ## _OPT "\n\t\t" ARG ## _HELP "\n\n", strcmp("", ARG ## _SYN) == 0 ? "" : ", ");
//#define PRINT_HELP(ARG)		printf("\t" ARG ", " ARG ## _SYN " " ARG ## _OPT "\n\t\t" ARG ## _HELP "\n\n");

// these are for the argument parsing engine...
#define GET_NEXT_ARG		*(++argv)
#define SKIP_NARG(n)		*(argv += n)	
#define CHECK_ARG(ARG)		strcmp(ARG, currentArg) == 0 || strcmp(ARG ## _SYN, currentArg) == 0
#define PEEK_ARG			*(argv + 1)
#define PEEK_NARG(n)		*(argv + n)

//some utility functions...
void printEntry(GCMFileEntryStruct *e);
void printDirectory(GCMFileEntryStruct *e);

//regular function prototypes...
void openFile(void);
void closeFile(void);

void verbosePrint(char *s);

void printGCMInfo(int hexFlag);
void printUsage(void);
void printExtendedUsage();

void extractFiles(char *source, char *dest);
void extractDiskHeader(char *path);
void extractDiskHeaderInfo(char *path);
void extractApploader(char *path);
void extractBootDol(char *path);

void injectDiskHeader(char *sourcePath);
void injectDiskHeaderInfo(char *sourcePath);
void injectApploader(char *sourcePath);

// some file utility functions...
void writeToFile(char *data, u32 length, char *path);
u32 readFromFile(char *buf, char *path);
u32 getFilesize(char *path);

//globals...

int verboseFlag;	//it's gotta be global, so other functions can see it...

char *filepath;		//path to the file we're working with...
char *filename;		//the name of the file we're working with
FILE *gcmFile;		//the file we're working with

//for working with printing directories
int dirDepth;
int recursiveIndex; //for the recursive printing...

int main (int argc, char * const argv[]) {
	// start flags declarations...
	
	int hexFlag = 0;
	
	//for extracting:
	char *extractFileFrom = NULL;
	char *extractFileTo = NULL;
	
	verboseFlag = 0;
	
	int showInfoFlag = 1;

	int extractDiskHeaderFlag = 0;
	char *extractDiskHeaderFile = GCM_DISK_HEADER_FILENAME;
	
	int extractDiskHeaderInfoFlag = 0;
	char *extractDiskHeaderInfoFile = GCM_DISK_HEADER_INFO_FILENAME;

	int extractApploaderFlag = 0;
	char *extractApploaderFile = GCM_APPLOADER_FILENAME;
	
	int extractBootDolFlag = 0;
	char *extractBootDolFile = GCM_BOOT_DOL_FILENAME;
	
	int injectDiskHeaderFlag = 0;
	char *injectDiskHeaderFile = GCM_DISK_HEADER_FILENAME;
	
	int injectDiskHeaderInfoFlag = 0;
	char *injectDiskHeaderInfoFile = GCM_DISK_HEADER_INFO_FILENAME;
	
	int injectApploaderFlag = 0;
	char *injectApploaderFile = GCM_APPLOADER_FILENAME;

	int listFilesFlag = 0;
	// end flag declarations
	
	//start argument parsing...
	char *currentArg = NULL;

	do {
		currentArg = GET_NEXT_ARG;
		if (!currentArg) {
			//there's no args! uh oh!
			
			//printf("No arguments...\n");
			printUsage();
			exit(0);
		} else if (CHECK_ARG(ARG_VERBOSE)) {
			//turn on verbosity!
			
			verboseFlag++;
			
			verbosePrint("Verbose output ON.");
		} else if (CHECK_ARG(ARG_HELP)) {
			// print extended help
			
			printExtendedUsage();
			exit(0);
		} else if (CHECK_ARG(ARG_INFO)) {
			//they want to see info...
			
			showInfoFlag++;
			
			verbosePrint("Show info flag ON.");
		} else if (CHECK_ARG(ARG_HEX)) {
			//they want hex notation...
			
			hexFlag = 1;
			
			verbosePrint("Hex notation ON.");
		} else if (CHECK_ARG(ARG_EXTRACT)) {
			// extract files...
			// usage: -e <path> <destPath>
			
			extractFileFrom		= GET_NEXT_ARG;
			extractFileTo		= GET_NEXT_ARG;
			
			if (!extractFileFrom || !extractFileTo) {
				//argument error... something was omitted...
				printf("Argument error.\n");
				printUsage();
				exit(1);
			}
			
		} else if (CHECK_ARG(ARG_EXTRACT_DISK_HEADER)) {
			// extract disk header... (to a file called "boot.bin")
			
			extractDiskHeaderFlag++;
			
			if (PEEK_ARG && strcmp(PEEK_ARG, OPT_FILE) == 0) {
				// if they're specifying a file...
				
				SKIP_NARG(1); //skip that -f we just looked at...
				extractDiskHeaderFile = GET_NEXT_ARG;
			}
		
		} else if (CHECK_ARG(ARG_EXTRACT_DISK_HEADER_INFO)) {
			// extract disk header info... (to a file called "bi2.bin")
			
			extractDiskHeaderInfoFlag++;
			
			if (PEEK_ARG && strcmp(PEEK_ARG, OPT_FILE) == 0) {
				// if they're specifying a file...
				
				SKIP_NARG(1); //skip that -f we just looked at...
				extractDiskHeaderInfoFile = GET_NEXT_ARG;
			}
			
		} else if (CHECK_ARG(ARG_EXTRACT_APPLOADER)) {
			//extract apploader... (to a file called "appldr.bin")
			
			extractApploaderFlag++;
			
			if (PEEK_ARG && strcmp(PEEK_ARG, OPT_FILE) == 0) {
				// if they're specifying a file...
				
				SKIP_NARG(1); //skip that -f we just looked at...
				extractApploaderFile = GET_NEXT_ARG;
			}
		} else if (CHECK_ARG(ARG_EXTRACT_BOOT_DOL)) {
			//extract the boot dol...
			
			extractBootDolFlag++;
			
			if (PEEK_ARG && strcmp(PEEK_ARG, OPT_FILE) == 0) {
				//if they specify a file...
				
				SKIP_NARG(1); //skip that -f
				extractBootDolFile = GET_NEXT_ARG;
			}
		} else if (CHECK_ARG(ARG_INJECT_DISK_HEADER)) {
			//inject the diskheader
			
			injectDiskHeaderFlag++;
			
			if (PEEK_ARG && strcmp(PEEK_ARG, OPT_FILE) == 0) {
				//if they're specifying a file... (otherwise use the default);
				
				SKIP_NARG(1); //skip the -f we just looked at...
				injectDiskHeaderFile = GET_NEXT_ARG;
			}
			
		} else if (CHECK_ARG(ARG_INJECT_DISK_HEADER_INFO)) {
			//inject the diskheaderinfo...
			
			injectDiskHeaderInfoFlag++;
			
			if (PEEK_ARG && strcmp(PEEK_ARG, OPT_FILE) == 0) {
				// if they're specifying a file...
				
				SKIP_NARG(1);
				injectDiskHeaderInfoFile = GET_NEXT_ARG;
			}
		} else if (CHECK_ARG(ARG_INJECT_APPLOADER)) {
			//inject the apploader...
			
			injectApploaderFlag++;
			
			if (PEEK_ARG && strcmp(PEEK_ARG, OPT_FILE) == 0) {
				//if they're specifying a file...
				
				SKIP_NARG(1);
				injectApploaderFile = GET_NEXT_ARG;
			}
		} else if (CHECK_ARG(ARG_LIST)) {
			// list filesystem
			
			listFilesFlag++; //turn the listFiles flag on.
		} else {
			// do it to this file... this is the last argument... just ignore the rest...
			
			filepath = currentArg;
			filename = lastPathComponent(filepath);
			
			break;
		}
	} while(*argv);
	//end parsing arguments...
	
	//open the file... and start processing!
	openFile();

	// print the info...
	if (showInfoFlag) {
		printGCMInfo(hexFlag);
	}
	
	// extract files...
	if (extractFileFrom && extractFileTo) {
		extractFiles(extractFileFrom, extractFileTo);
	}
	
	//extract diskheader
	if (extractDiskHeaderFlag) {
		extractDiskHeader(extractDiskHeaderFile);
	}
	
	//extract diskheaderinfo
	if (extractDiskHeaderInfoFlag) {
		extractDiskHeaderInfo(extractDiskHeaderInfoFile);
	}
	
	//extract apploader
	if (extractApploaderFlag) {
		extractApploader(extractApploaderFile);
	}
	
	if (extractBootDolFlag) {
		extractBootDol(extractBootDolFile);
	}
	
	//inject the diskheader
	if (injectDiskHeaderFlag) {
		injectDiskHeader(injectDiskHeaderFile);
	}
	
	//inject the diskheaderinfo
	if (injectDiskHeaderInfoFlag) {
		injectDiskHeaderInfo(injectDiskHeaderInfoFile);
	}
	
	//inject the apploader
	if (injectApploaderFlag) {
		injectApploader(injectApploaderFile);
	}

	// list the files, if necesary...
	if (listFilesFlag) {
		dirDepth = 0;
		recursiveIndex = 0;
		GCMFileEntryStruct *r = GCMGetRootFileEntry(gcmFile);
		printDirectory(r);
		GCMFreeFileEntryStruct(r);
	}
	
	closeFile();
	
	return 0;
}

void openFile(void) {
	/*
	**  opens the GCM file for reading and writing...
	*/
	
	verbosePrint("Opening GCM...");
	
	if (!filepath) {
		printUsage();
		exit(1);
	}
	
	if (!(gcmFile = fopen(filepath, "r+"))) { //open as r+ so we can inject data, too...
		printf("error opening file... (%s)\n", filepath);
		exit(1);
	}
	
}

void closeFile(void) {
	verbosePrint("Closing GCM...");
	fclose(gcmFile);
}

void verbosePrint(char *s) {
	if (verboseFlag) {
		printf("%s\n", s);
	}
}

void printGCMInfo(int hexFlag) {
	/*
	**  This just prints all of the relevant info for the ROM
	**  fun fun fun
	*/

	char systemID = GCMGetSystemID(gcmFile);
	
	char *gameID = (char*)malloc(GCM_GAME_ID_LENGTH + 1);
	GCMGetGameID(gcmFile, gameID);
	
	char regionCode = GCMGetRegionCode(gcmFile);
	
	char *makerCode = (char*)malloc(GCM_MAKER_CODE_LENGTH + 1);
	GCMGetMakerCode(gcmFile, makerCode);
	
	char *gameName = (char*)malloc(256);
	GCMGetGameName(gcmFile, gameName);
	
	printf("Filename:\t%s\n", filename);
	printf("System ID:\t%c (%s)\n", systemID, GCMSystemIDToStr(systemID));
	printf("Game ID:\t%s\n", gameID);
	printf("Region:  \t%c (%s)\n", regionCode, GCMRegionCodeToStr(regionCode));
	printf("Maker Code:\t%s (%s)\n", makerCode, GCMMakerCodeToStr(makerCode));
	printf("Game Name:\t%s\n", gameName);
	
	char format[256]; //this is for the hexFlag stuff...
	
	strcpy(format, "DOL offset:\t");
	strcat(format, (hexFlag) ? "%08X" : "%ld");
	strcat(format, "\n");
	printf(format, GCMGetDolOffset(gcmFile));
	
	strcpy(format, "Apploader:\t");
	strcat(format, (hexFlag) ? "%08X" : "%ld");
	strcat(format, "\n");
	printf(format, GCM_APPLOADER_OFFSET);
	
	strcpy(format,"ApploaderSize:\t");
	strcat(format, (hexFlag) ? "%08X" : "%ld");
	strcat(format, "\n");
	printf(format, GCMGetApploaderSize(gcmFile));
	
	strcpy(format, "FST:      \t");
	strcat(format, (hexFlag) ? "%08X" : "%ld");
	strcat(format, "\n");
	printf(format, GCMGetFSTOffset(gcmFile));
	
	GCMFileEntryStruct *r = GCMGetRootFileEntry(gcmFile);
	u32 entryCount = r->length;
	printf("File count:\t%ld\n", entryCount);
	GCMFreeFileEntryStruct(r);
}

void extractFiles(char *source, char *dest) {
	/*
	**  extract files from source (in GCM) to dest (in the local filesystem)
	*/
	
	char vstring[1024] = "Extracting ";
	strcat(vstring, source);
	strcat(vstring, " from GCM to ");
	strcat(vstring, dest);
	verbosePrint(vstring);
	
	GCMFileEntryStruct *e = GCMGetFileEntryAtPath(gcmFile, source);
	
	//check to see if the file was actually found...
	if (!e) {
		printf("File not found (%s)\n", source);
		return;
	}
	
	//fetch the data
	GCMFetchDataForFileEntry(gcmFile, e);
	
	writeToFile(e->data, e->length, dest);
	
	free(e->data);
	free(e);
}

void extractDiskHeader(char *path) {
	/*
	**  extracts the disk header to boot.bin
	*/
	
	verbosePrint("Extracting the disk header...");
	
	//get the data...
	char *buf = (char*)malloc(GCM_DISK_HEADER_LENGTH);
	GCMGetDiskHeader(gcmFile, buf);
		
	writeToFile(buf, GCM_DISK_HEADER_LENGTH, path);
		
	free(buf);
}

void extractDiskHeaderInfo(char *path) {
	/*
	**  extracts the diskheaderinfo to bi2.bin
	*/
	
	verbosePrint("Extracting the disk header info...");
	
	//get the data...
	char *buf = (char*)malloc(GCM_DISK_HEADER_INFO_LENGTH);
	GCMGetDiskHeaderInfo(gcmFile, buf);
	
	writeToFile(buf, GCM_DISK_HEADER_INFO_LENGTH, path);
	
	free(buf);
}

void extractApploader(char *path) {
	/*
	**  extracts the apploader to appldr.bin
	*/
	
	verbosePrint("Extracting the apploader...");
	
	//get the data...
	u32 apploaderLength = GCMGetApploaderSize(gcmFile) + GCM_APPLOADER_CODE_OFFSET;
	char *buf = (char*)malloc(apploaderLength);
	GCMGetApploader(gcmFile, buf);
	
	writeToFile(buf, apploaderLength, path);
	
	free(buf);
}

void extractBootDol(char *path) {
	/*
	**  extracts the boot DOL from the GCM and saves it to path...
	*/
	
	verbosePrint("Extracting the boot DOL...");
	
	if (!path) return;
	
	u32 length = GCMGetBootDolLength(gcmFile);
	char *buf = (char*)malloc(length);
	
	if (GCMGetBootDol(gcmFile, buf) != length) {
		printf("An error occurred when getting the DOL.\n");
		free(buf);
		return;
	}
	
	writeToFile(buf, length, path);
}

void injectDiskHeader(char *sourcePath) {
	/*
	**  take a diskHeader (boot.bin) from sourcePath and inject it into gcmFile.
	*/
	
	verbosePrint("Injecting the disk header...");
	
	char *buf = (char*)malloc(getFilesize(sourcePath));
	
	if (readFromFile(buf, sourcePath) != GCM_DISK_HEADER_LENGTH) {
		printf("This does not appear to be a diskheader (%s)\n", sourcePath);
		free(buf);
		return;
	}
	
	if (GCMPutDiskHeader(gcmFile, buf) != GCM_SUCCESS) {
		printf("An error occurred when writing the disk header! (%d)\n", ferror(gcmFile));
	}
	
	free(buf);
	return;
}

void injectDiskHeaderInfo(char *sourcePath) {
	/*
	**  take a diskHeaderInfo (bi2.bin) from sourcePath and inject it into gcmFile.
	*/
	
	verbosePrint("Injecting the disk header info...");
	
	char *buf = (char*)malloc(getFilesize(sourcePath));
	
	if (readFromFile(buf, sourcePath) != GCM_DISK_HEADER_INFO_LENGTH) {
		printf("This does not appear to be a diskheaderinfo (%s)\n", sourcePath);
		free(buf);
		return;
	}
	
	if (GCMPutDiskHeaderInfo(gcmFile, buf) != GCM_SUCCESS) {
		printf("An error occurred when writing the disk header info!\n");
	}
	
	free(buf);
	return;
}

void injectApploader(char *sourcePath) {
	/*
	**  this doesn't work, yet... requires some serious shifting of other data...
	*/
	
	verbosePrint("Injecting the apploader...");
	
	//first, load the file into memory...
	
	u32 length = getFilesize(sourcePath);
	char *data = (char*)malloc(length);
	
	if (readFromFile(sourcePath, data) != length) {
		printf("An error occurred reading the file (%s)", sourcePath);
		free(data);
		return;
	}
	
	
}

void printEntry(GCMFileEntryStruct *e) {
	printf("\t%s\n", e->filename);
	printf("size:\t%ld\n\n", e->length);
}

void printDirectory(GCMFileEntryStruct *e) {
	if (!e) {
		return;
	}
	int j = 0;
	
	GCMFetchFilenameForFileEntry(gcmFile, e);
	
	for (j = 0; j < dirDepth; j++) {
		printf(" ");
	}
	
	if (dirDepth == 0) {
		printf("/\n");
	} else {
		if (!e->isDir)
			printf("%s\n", e->filename);
		else
			printf("%s\n", e->filename);
	}
	if (e->isDir) {
		dirDepth++;
		GCMFileEntryStruct *next;
		
		//printf("for(%d++; %d < %ld; i++)\n", i, i, e->length);
		
		for(recursiveIndex++; (unsigned long)recursiveIndex < e->length; recursiveIndex++) {
			next = GCMGetNthFileEntry(gcmFile, recursiveIndex);
			if (next) {
				printDirectory(next);
				free(next);
			}
		}
		recursiveIndex--;
		dirDepth--;
	}
}

void writeToFile(char *data, u32 length, char *path) {
	/*
	**  Takes data of length and writes it to a file path. Displays errors when they happen...
	*/
	
	char msg[1024] = "Writing to file ";
	strcat(msg, path);
	verbosePrint(msg);
	
	if (!data || !length || !path) return;
	
	FILE *ofile = NULL;
	
	if (!(ofile = fopen(path, "w"))) {
		printf("An error occurred trying to open %s\n", path);
		return;
	}
	
	if (fwrite(data, 1, length, ofile) != length) {
		printf("An error occurred trying to write to %s\n", path);
	}
}

u32 readFromFile(char *buf, char *path) {
	/*
	**  reads from the file at path, 
	**  sets buf to the contents of said file...
	**  and returns the length of data.
	**
	**  use getFilesize() when you allocate buf
	**
	**  returns 0 and displays messages on error;
	*/
	
	char msg[1024] = "Reading file ";
	strcat(msg, path);
	verbosePrint(msg);
	
	if (!path || !buf) return 0;
	
	FILE *ifile = NULL;
	
	u32 length = getFilesize(path); //get how much to read...
	
	if (!length || !(ifile = fopen(path, "r"))) {
		printf("An error occurred trying to open %s\n", path);
		return 0;
	}
	
	if (fread(buf, 1, length, ifile) != length) {
		printf("An error occurred when trying to read %s\n", path);
		fclose(ifile);
		return 0;
	}
	
	fclose(ifile);
	return length;
}

u32 getFilesize(char *path) {
	/*
	**  returns the filesize of the file at *path
	**  useful for when you read a file... gotta make sure you allocate enough memory for the file...
	*/
	
	FILE *ifile = NULL;
	
	if (!(ifile = fopen(path, "r"))) {
		return 0;
	}
	
	fseek(ifile, 0, SEEK_END);
	u32 len = ftell(ifile);
	fclose(ifile);
	
	return len;
}

void printUsage() {
	printf("GCMTool %s- A utility for working with Nintendo GameCube DVD images.\n\tgcmtool.sourceforge.net\n\n", VERSION);
	printf("Usage:");
	printf("  gcmtool [ options ] <filename>\n\n");
	printf("Use -? for extended usage.\n\n");
}

void printExtendedUsage() {
	printUsage();
	
	printf("    Options:\n");
	PRINT_HELP(ARG_EXTRACT);
	PRINT_HELP(ARG_VERBOSE);
	PRINT_HELP(ARG_INFO);
	PRINT_HELP(ARG_LIST);
	PRINT_HELP(ARG_HELP);
	printf("\n");
	printf("  You can add -f <filename> to specify a filename for the following options...\n");
	
	PRINT_HELP(ARG_EXTRACT_DISK_HEADER);
	PRINT_HELP(ARG_EXTRACT_DISK_HEADER_INFO);
	PRINT_HELP(ARG_EXTRACT_APPLOADER);
	PRINT_HELP(ARG_EXTRACT_BOOT_DOL);
	PRINT_HELP(ARG_INJECT_DISK_HEADER);
	PRINT_HELP(ARG_INJECT_DISK_HEADER_INFO);
	PRINT_HELP(ARG_INJECT_APPLOADER);
	PRINT_HELP(ARG_INJECT_BOOT_DOL);
}