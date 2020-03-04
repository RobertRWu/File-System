// 1752149 吴子睿

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "include/main.h"
#include "include/help_center.h"

using namespace std;


DeviceManager gDeviceManager;
BufferManager gBufferManager;
OpenFileTable gOpenFileTable;
SuperBlock gSuperBlock;
FileSystem gFileSystem;
INodeTable gINodeTable;
FileManager gFileManager;
User gUser;

int main()
{
    string input;
    string temp_str;
    vector<string> args(MAX_ARG_NUM);
    string cmd;
    int i;

    cout << "Unix二级文件系统模拟程序" << endl;
    cout << "If you have any question please input \"help\"." << endl;
    cout << "Please input a command:" << endl;

    while (1) {
        cout << gUser.cur_dir_path_ << ">";
        input.clear();
        for (i = 0; i < MAX_ARG_NUM; i++)
            args[i].clear();
        getline(cin, input);

        istringstream iss(input);

        i = 0;
        /* 通过空格将几个参数分开 */
        while (iss >> temp_str)
            args[i++] = temp_str;

        cmd = args[0];  // 命令
        if (cmd.empty())
            continue;

        else if (cmd == "help")
            Help();

        /* Format the system */
        else if (cmd == "fformat") {
            gOpenFileTable.Format();
            gINodeTable.Format();
            gBufferManager.FormatBuffer();
            gFileSystem.FormatDevice();
            cout << "Format the system succesfully! Please reopen the program" << endl;
            system("pause");
            return 0;
        }

        /* Change directory */
        else if (cmd == "cd")
            gUser.ChangeDir(args[1]);

        /* List all subdirectories and files of current directory */
        else if (cmd == "ls")
            gUser.List();

        /* Make a new directory */
        else if (cmd == "mkdir")
            gUser.MakeDir(args[1]);

        /* Create a new file */
        else if (cmd == "fcreat")
            gUser.CreateFile(args[1]);

        /* Open a file */
        else if (cmd == "fopen")
            gUser.OpenFile(args[1], args[2]);

        /* Close a file */
        else if (cmd == "fclose")
            gUser.CloseFile(args[1]);

        /* Read a file */
        else if (cmd == "fread")
            gUser.ReadFile(args[1], args[2], args[3].c_str(), args[4].c_str());

        /* Write into a file */
        else if (cmd == "fwrite")
            gUser.WriteFile(args[1], args[2], args[3].c_str(), args[4].c_str());

        /* Seek in a file */
        else if (cmd == "fseek")
            gUser.Seek(args[1], args[2], args[3]);

        /* Delete a file */
        else if (cmd == "fdelete")
            gUser.DeleteFile(args[1]);

        /* Exit the program */
        else if (cmd == "exit")
            exit(0);

        /* Undefined command */
        else
            cout << cmd << " : don't find the commond \n";
    }

	return 0;
}