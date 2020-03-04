#include <iostream>
#include <string>
#include <iomanip>

#include "include/help_center.h"

using namespace std;

void Help()
{
	string input;
	int option;

	cout << setiosflags(ios::left);

	cout << "Choose one of the command you want to know details about:" << endl;
	cout << "Please enter the name of the command, not the number" << endl;
	cout << "1.  fformat" << endl;
	cout << "2.  ls" << endl;
	cout << "3.  mkdir" << endl;
	cout << "4.  fcreat" << endl;
	cout << "5.  fopen" << endl;
	cout << "6.  fclose" << endl;
	cout << "7.  fread" << endl;
	cout << "8.  fwrite" << endl;
	cout << "9.  fseek" << endl;
	cout << "10. fdelete" << endl;

	getline(cin, input);

	if ("fformat" == input) {
		cout << setw(SPACE_NUM) << "Command:" << "fformat" << endl;
		cout << setw(SPACE_NUM) << "Description: " << "Format the whole virtual disk" << endl;
		cout << setw(SPACE_NUM) << "Usage:" << "fformat" << endl;
	}

	else if ("ls" == input) {
		cout << setw(SPACE_NUM) << "Command:" << "ls" << endl;
		cout << setw(SPACE_NUM) << "Description:";
		cout << "List each file and directory in the current directory" << endl;
		cout << setw(SPACE_NUM) << "Usage:" << "ls" << endl;
	}

	else if ("mkdir" == input) {
		cout << setw(SPACE_NUM) << "Command:" << "mkdir" << endl;
		cout << setw(SPACE_NUM) << "Description:";
		cout << "Create a new directory. If the directory exists, ";
		cout << "the system will skip the operation" << endl;
		cout << setw(SPACE_NUM) << "Usage:" << "mkdir <directory_name>" << endl;
		cout << setw(SPACE_NUM) << "Parameter:" << "directory_name: ";
		cout << "The name of directory that you want to create." << endl;
	}

	else if ("fcreat" == input) {
		cout << setw(SPACE_NUM) << "Command:" << "fcreat" << endl;
		cout << setw(SPACE_NUM) << "Description:";
		cout << "Create a new file. If the file exists, ";
		cout << "the system will skip the operation" << endl;
		cout << setw(SPACE_NUM) << "Usage:" << "fcreat <file_name>" << endl;
		cout << setw(SPACE_NUM) << "Parameter:" << "file_name: ";
		cout << "The name of file that you want to create." << endl;
	}

	else if ("fopen" == input) {
		cout << setw(SPACE_NUM) << "Command:" << "fopen" << endl;
		cout << setw(SPACE_NUM) << "Description:";
		cout << "Open a existing file." << endl;
		cout << setw(SPACE_NUM) << "Usage:" << "fopen <file_name> <mode>" << endl;
		cout << setw(SPACE_NUM) << "Parameter:" << "file_name: ";
		cout << "The name of file that you want to open." << endl;
		cout << setw(SPACE_NUM) << "mode: ";
		cout << "Open mode, -r for read-only; -w for write-only; ";
		cout << "-rw for both." << endl;
	}

	else if ("fclose" == input) {
		cout << setw(SPACE_NUM) << "Command:" << "fclose" << endl;
		cout << setw(SPACE_NUM) << "Description:";
		cout << "Close an opened file." << endl;
		cout << setw(SPACE_NUM) << "Usage:" << "fclose <file_descriptor>" << endl;
		cout << setw(SPACE_NUM) << "Parameter:" << "file_descriptor: ";
		cout << "The descriptor of file that you want to close." << endl;
	}

	else if ("fread" == input) {
		cout << setw(SPACE_NUM) << "Command:" << "fread" << endl;
		cout << setw(SPACE_NUM) << "Description:";
		cout << "Read the content of an opened file." << endl;
		cout << setw(SPACE_NUM) << "Usage:";
		cout << "fread <file_descriptor> <size> (<-f> <file_name>)" << endl;
		cout << setw(SPACE_NUM) << "Parameter:" << "file_descriptor: ";
		cout << "The descriptor of file that you want to read." << endl;
		cout << setw(SPACE_NUM) << "size: ";
		cout << "The size of content that you want to read from the file." << endl;
		cout << setw(SPACE_NUM) << "-f: ";
		cout << "Optional, read the content to a file in your PC." << endl;
		cout << setw(SPACE_NUM) << "file_name: ";
		cout << "Optional, the name of file where you want to save the reading result." << endl;
	}
	else if ("fwrite" == input) {
		cout << setw(SPACE_NUM) << "Command:" << "fwrite" << endl;
		cout << setw(SPACE_NUM) << "Description:";
		cout << "Write content to an opened file." << endl;
		cout << setw(SPACE_NUM) << "Usage:";
		cout << "fwrite <file_descriptor> <size> (<-f> <file_name>)" << endl;
		cout << setw(SPACE_NUM) << "Parameter:" << "file_descriptor: ";
		cout << "The descriptor of file that you want to write." << endl;
		cout << setw(SPACE_NUM) << "size: ";
		cout << "The size of content that you want to write to the file." << endl;
		cout << setw(SPACE_NUM) << "-f: ";
		cout << "Optional, write the content that is from a file in your PC." << endl;
		cout << setw(SPACE_NUM) << "file_name: ";
		cout << "Optional, the name of file which you want to write its content." << endl;
	}
	else if ("fseek" == input) {
		cout << setw(SPACE_NUM) << "Command:" << "fseek" << endl;
		cout << setw(SPACE_NUM) << "Description:";
		cout << "Seek in an opened file." << endl;
		cout << setw(SPACE_NUM) << "Usage:";
		cout << "fseek <file_descriptor> <size> <whence>" << endl;
		cout << setw(SPACE_NUM) << "Parameter:" << "file_descriptor: ";
		cout << "The descriptor of file that you want to seek." << endl;
		cout << setw(SPACE_NUM) << "size: ";
		cout << "The size that you want to seek." << endl;
		cout << setw(SPACE_NUM) << "whence: ";
		cout << "From where to start, 0 for the beggning; 1 for current position; ";
		cout << "2 for the end." << endl;
	}
	else if ("fdelete" == input) {
		cout << setw(SPACE_NUM) << "Command:" << "fdelete" << endl;
		cout << setw(SPACE_NUM) << "Description:";
		cout << "Delete a existing file." << endl;
		cout << setw(SPACE_NUM) << "Usage:" << "fdelete <file_name>" << endl;
		cout << setw(SPACE_NUM) << "Parameter:" << "file_name: ";
		cout << "The name of file that you want to delete." << endl;
	}
	else
		cout << "Wrong command!" << endl;
}