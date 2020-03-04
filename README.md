## File System (Unix)

### Description

This project is a Unix file system similar to the secondary file system, that is, using a common large file to simulate a file volume of Unix file system. A file volume is actually a logical disk, the data stored in the disk are actually stored in blocks, which is 512 bytes each. This file system provides the user with the command line interface, see the basic function modules below.

### Modules 

* Buffer:

    The cache control block class and cache are defined in this file.

* BufferManager:

    Cache management module, using LRU algorithm, provides function interfaces such as read, write (including delayed write), release, clear, and flush the buffer to the disk. It is responsible for managing all the cache blocks in the system.

* DeviceManager:

    Since this project has only one file system device, the device manager is only responsible for direct read and write of disk files.

* File:

    Open file control block, open file control table and file I/O parameter classes are defined. Open file control table stores 100 open file control blocks, and provides with function interfaces such as allocating free table items and obtaining open file control block according to file descriptor.

* FileManager:

    Encapsulates file processing operation process, including changing the current directory, printing current directory file list, creating folders, opening files, creating files, closing files, seeking file pointer, reading files, writing files, deleting files system functions.

* FileSystem:

    Responsible for file system disk management, including storage (SuperBlock, DiskINode, data disk) space management and SuperBlock management. Provides with interfaces for allocating and retrieving DiskINode nodes, data block nodes, and formatting disk blocks.

* INode:

    INode and DiskINode as well as related function interface such as read, write, load are defined.

* OpenFileManager:

    Open the file management module, responsible for the management of open files, provides users with direct operation of the file descriptor interface.

* User:

    Provide users with operation interface. Changing the user's command into the corresponding function call. Also includes checking the correctness and validity of user input.

* Utility:

    Some basic functions.

### How to Use?

Import into a VS 2019 Project with the current code structure.