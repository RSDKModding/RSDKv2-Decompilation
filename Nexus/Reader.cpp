#include "RetroEngine.hpp"
#include <string>

char binFileName[0x400];

char fileName[0x100];
byte FileBuffer[0x2000];
int FileSize;
int VFileSize;
int ReadPos;
int ReadSize;
int BufferPosition;
int VirtualFileOffset;
byte isModdedFile = false;

FileIO *CFileHandle = nullptr;

bool CheckBinFile(const char *filePath) {
    FileInfo info;

    Engine.UseBinFile         = false;
    Engine.usingDataFileStore = false;

    CFileHandle = fOpen(filePath, "rb");
    if (CFileHandle) {
        Engine.UseBinFile = true;
        StrCopy(binFileName, filePath);
        fClose(CFileHandle);
        CFileHandle = NULL;
        return true;
    } else {
        Engine.UseBinFile = false;
        CFileHandle       = NULL;
        return false;
    }

    return false;
}

inline bool ends_with(std::string const &value, std::string const &ending) {
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

bool LoadFile(const char *filePath, FileInfo *fileInfo) {
    MEM_ZEROP(fileInfo);

    if (CFileHandle)
        fClose(CFileHandle);

    CFileHandle = NULL;

    char filePathBuf[0x100];
    StrCopy(filePathBuf, filePath);

    if (Engine.forceFolder)
        Engine.UseBinFile = Engine.usingDataFileStore;
    Engine.forceFolder = false;

    Engine.usingDataFileStore = Engine.UseBinFile;

#if RETRO_USE_MOD_LOADER
    fileInfo->isMod = false;
    isModdedFile    = false;
#endif
    bool addPath = true;
    // Fixes any case differences
    char pathLower[0x100];
    memset(pathLower, 0, sizeof(char) * 0x100);
    for (int c = 0; c < strlen(filePathBuf); ++c) {
        pathLower[c] = tolower(filePathBuf[c]);
    }

#if RETRO_USE_MOD_LOADER
    for (int m = 0; m < modList.size(); ++m) {
        if (modList[m].active) {
            std::map<std::string, std::string>::const_iterator iter = modList[m].fileMap.find(pathLower);
            if (iter != modList[m].fileMap.cend()) {
                StrCopy(filePathBuf, iter->second.c_str());
                Engine.forceFolder = true;
                Engine.UseBinFile  = false;
                fileInfo->isMod    = true;
                isModdedFile       = true;
                addPath            = false;
                break;
            }
        }
    }
#endif

#if RETRO_PLATFORM == RETRO_OSX
    if (addPath) {
        char pathBuf[0x100];
        sprintf(pathBuf, "%s/%s", gamePath, filePathBuf);
        sprintf(filePathBuf, "%s", pathBuf);
    }
#endif

    StrCopy(fileInfo->fileName, "");
    StrCopy(fileName, "");

    if (Engine.UseBinFile && !Engine.forceFolder) {
        CFileHandle = fOpen(binFileName, "rb");
        fSeek(CFileHandle, 0, SEEK_END);
        FileSize       = (int)fTell(CFileHandle);
        BufferPosition = 0;
        ReadSize       = 0;
        ReadPos        = 0;

        StrCopy(fileInfo->fileName, filePath);
        StrCopy(fileName, filePath);
        if (!ParseVirtualFileSystem(fileInfo)) {
            fClose(CFileHandle);
            CFileHandle = NULL;
            PrintLog("Couldn't load file '%s'", filePathBuf);
            return false;
        }
        fileInfo->readPos           = ReadPos;
        fileInfo->fileSize          = VFileSize;
        fileInfo->virtualFileOffset = VirtualFileOffset;
        fileInfo->bufferPosition    = BufferPosition;
        fileInfo->encrypted         = true;
    } else {
        StrCopy(fileInfo->fileName, filePathBuf);
        StrCopy(fileName, filePathBuf);
        CFileHandle = fOpen(fileInfo->fileName, "rb");
        if (!CFileHandle) {
            PrintLog("Couldn't load file '%s'", filePathBuf);
            return false;
        }

        VirtualFileOffset = 0;
        fSeek(CFileHandle, 0, SEEK_END);
        fileInfo->fileSize = (int)fTell(CFileHandle);
        FileSize           = fileInfo->fileSize;
        fSeek(CFileHandle, 0, SEEK_SET);
        ReadPos                     = 0;
        fileInfo->readPos           = ReadPos;
        fileInfo->virtualFileOffset = 0;
        fileInfo->bufferPosition    = 0;
        fileInfo->encrypted         = false;
    }
    BufferPosition = 0;
    ReadSize       = 0;

    PrintLog("Loaded File '%s'", filePathBuf);

    return true;
}

bool ParseVirtualFileSystem(FileInfo *fileInfo) {
    char filename[0x50];
    char fullFilename[0x50];
    char stringBuffer[0x50];
    ushort dirCount = 0;
    int fileOffset  = 0;
    int fNamePos    = 0;
    int headerSize  = 0;
    int i           = 0;
    byte fileBuffer = 0;

    int j             = 0;
    VirtualFileOffset = 0;
    for (int i = 0; fileInfo->fileName[i]; i++) {
        if (fileInfo->fileName[i] == '/') {
            fNamePos = i;
            j        = 0;
        } else {
            ++j;
        }
        fullFilename[i] = fileInfo->fileName[i];
    }
    ++fNamePos;
    for (i = 0; i < j; ++i) filename[i] = fileInfo->fileName[i + fNamePos];
    filename[j]            = 0;
    fullFilename[fNamePos] = 0;

    fSeek(CFileHandle, 0, SEEK_SET);
    Engine.UseBinFile = false;
    BufferPosition    = 0;
    ReadSize          = 0;
    ReadPos           = 0;

    FileRead(&fileBuffer, 1);
    headerSize = fileBuffer;
    FileRead(&fileBuffer, 1);
    headerSize += fileBuffer << 8;
    FileRead(&fileBuffer, 1);
    headerSize += fileBuffer << 16;
    FileRead(&fileBuffer, 1);
    headerSize += fileBuffer << 24;

    FileRead(&fileBuffer, 1);
    dirCount = fileBuffer;

    i                  = 0;
    fileOffset         = 0;
    int nextFileOffset = 0;
    while (i < dirCount) {
        FileRead(&fileBuffer, 1);
        for (j = 0; j < fileBuffer; ++j) {
            FileRead(&stringBuffer[j], 1);
        }
        stringBuffer[j] = 0;

        if (StrComp(fullFilename, stringBuffer)) {
            FileRead(&fileBuffer, 1);
            fileOffset = fileBuffer;
            FileRead(&fileBuffer, 1);
            fileOffset += fileBuffer << 8;
            FileRead(&fileBuffer, 1);
            fileOffset += fileBuffer << 16;
            FileRead(&fileBuffer, 1);
            fileOffset += fileBuffer << 24;

            // Grab info for next dir to know when we've found an error
            // Ignore dir name we dont care
            if (i == dirCount - 1) {
                nextFileOffset = FileSize - headerSize; // There is no next dir, so just make this the EOF
            } else {
                FileRead(&fileBuffer, 1);
                for (j = 0; j < fileBuffer; ++j) {
                    FileRead(&stringBuffer[j], 1);
                    stringBuffer[j] ^= -1 - fileBuffer;
                }
                stringBuffer[j] = 0;

                FileRead(&fileBuffer, 1);
                nextFileOffset = fileBuffer;
                FileRead(&fileBuffer, 1);
                nextFileOffset += fileBuffer << 8;
                FileRead(&fileBuffer, 1);
                nextFileOffset += fileBuffer << 16;
                FileRead(&fileBuffer, 1);
                nextFileOffset += fileBuffer << 24;
            }

            i = dirCount;
        } else {
            fileOffset = -1;
            FileRead(&fileBuffer, 1);
            FileRead(&fileBuffer, 1);
            FileRead(&fileBuffer, 1);
            FileRead(&fileBuffer, 1);
            ++i;
        }
    }

    if (fileOffset == -1) {
        Engine.UseBinFile = true;
        return false;
    } else {
        fSeek(CFileHandle, fileOffset + headerSize, SEEK_SET);
        BufferPosition    = 0;
        ReadSize          = 0;
        ReadPos           = 0;
        VirtualFileOffset = fileOffset + headerSize;
        i                 = 0;
        while (i < 1) {
            FileRead(&fileBuffer, 1);
            ++VirtualFileOffset;
            j = 0;
            while (j < fileBuffer) {
                FileRead(&stringBuffer[j], 1);
                ++j;
                ++VirtualFileOffset;
            }
            stringBuffer[j] = 0;

            if (StrComp(filename, stringBuffer)) {
                i = 1;
                FileRead(&fileBuffer, 1);
                j = fileBuffer;
                FileRead(&fileBuffer, 1);
                j += fileBuffer << 8;
                FileRead(&fileBuffer, 1);
                j += fileBuffer << 16;
                FileRead(&fileBuffer, 1);
                j += fileBuffer << 24;
                VirtualFileOffset += 4;
                VFileSize = j;
            } else {
                FileRead(&fileBuffer, 1);
                j = fileBuffer;
                FileRead(&fileBuffer, 1);
                j += fileBuffer << 8;
                FileRead(&fileBuffer, 1);
                j += fileBuffer << 16;
                FileRead(&fileBuffer, 1);
                j += fileBuffer << 24;
                VirtualFileOffset += 4;
                VirtualFileOffset += j;
            }

            // No File has been found (next file would be in a new dir)
            if (VirtualFileOffset >= nextFileOffset + headerSize) {
                Engine.UseBinFile = true;
                return false;
            }
            fSeek(CFileHandle, VirtualFileOffset, SEEK_SET);
            BufferPosition = 0;
            ReadSize       = 0;
            ReadPos        = VirtualFileOffset;
        }
        Engine.UseBinFile = true;
        return true;
    }
    // Engine.UseBinFile = true;
    return false;
}

void FileRead(void *dest, int size) {
    byte *data = (byte *)dest;

    if (ReadPos <= FileSize) {
        if (Engine.UseBinFile && !Engine.forceFolder) {
            while (size > 0) {
                if (BufferPosition == ReadSize)
                    FillFileBuffer();

                *data++ = FileBuffer[BufferPosition++] ^ 0xFF;
                size--;
            }
        } else {
            while (size > 0) {
                if (BufferPosition == ReadSize)
                    FillFileBuffer();

                *data++ = FileBuffer[BufferPosition++];
                size--;
            }
        }
    }
}

void SetFileInfo(FileInfo *fileInfo) {
    Engine.forceFolder = false;
    if (!fileInfo->isMod) {
        Engine.UseBinFile = Engine.usingDataFileStore;
    } else {
        Engine.forceFolder = true;
    }

    isModdedFile = fileInfo->isMod;
    if (Engine.UseBinFile && !Engine.forceFolder) {
        CFileHandle       = fOpen(binFileName, "rb");
        VirtualFileOffset = fileInfo->virtualFileOffset;
        VFileSize         = fileInfo->fileSize;
        fSeek(CFileHandle, 0, SEEK_END);
        FileSize = (int)fTell(CFileHandle);
        ReadPos  = fileInfo->readPos;
        fSeek(CFileHandle, ReadPos, SEEK_SET);
        FillFileBuffer();
        BufferPosition = fileInfo->bufferPosition;
    } else {
        StrCopy(fileName, fileInfo->fileName);
        CFileHandle       = fOpen(fileInfo->fileName, "rb");
        VirtualFileOffset = 0;
        FileSize          = fileInfo->fileSize;
        ReadPos           = fileInfo->readPos;
        fSeek(CFileHandle, ReadPos, SEEK_SET);
        FillFileBuffer();
        BufferPosition = fileInfo->bufferPosition;
    }
}

size_t GetFilePosition() {
    if (Engine.UseBinFile)
        return BufferPosition + ReadPos - ReadSize - VirtualFileOffset;
    else
        return BufferPosition + ReadPos - ReadSize;
}

void SetFilePosition(int newPos) {
    if (Engine.UseBinFile) {
        ReadPos = VirtualFileOffset + newPos;
    } else {
        ReadPos = newPos;
    }
    fSeek(CFileHandle, ReadPos, SEEK_SET);
    FillFileBuffer();
}

bool ReachedEndOfFile() {
    if (Engine.UseBinFile)
        return BufferPosition + ReadPos - ReadSize - VirtualFileOffset >= VFileSize;
    else
        return BufferPosition + ReadPos - ReadSize >= FileSize;
}

bool LoadFile2(const char *filePath, FileInfo *fileInfo) {
    if (fileInfo->cFileHandle)
        fClose(fileInfo->cFileHandle);

    MEM_ZEROP(fileInfo);

    char filePathBuf[0x100];
    StrCopy(filePathBuf, filePath);

    if (Engine.forceFolder)
        Engine.UseBinFile = Engine.usingDataFileStore;
    Engine.forceFolder = false;

    Engine.usingDataFileStore = Engine.UseBinFile;

#if RETRO_USE_MOD_LOADER
    fileInfo->isMod = false;
    isModdedFile    = false;
#endif
    bool addPath = true;
    // Fixes ".ani" ".Ani" bug and any other case differences
    char pathLower[0x100];
    memset(pathLower, 0, sizeof(char) * 0x100);
    for (int c = 0; c < strlen(filePathBuf); ++c) {
        pathLower[c] = tolower(filePathBuf[c]);
    }

#if RETRO_USE_MOD_LOADER
    for (int m = 0; m < modList.size(); ++m) {
        if (modList[m].active) {
            std::map<std::string, std::string>::const_iterator iter = modList[m].fileMap.find(pathLower);
            if (iter != modList[m].fileMap.cend()) {
                StrCopy(filePathBuf, iter->second.c_str());
                Engine.forceFolder = true;
                Engine.UseBinFile  = false;
                fileInfo->isMod    = true;
                isModdedFile       = true;
                addPath            = false;
                break;
            }
        }
    }
#endif

#if RETRO_PLATFORM == RETRO_OSX
    if (addPath) {
        char pathBuf[0x100];
        sprintf(pathBuf, "%s/%s", gamePath, filePathBuf);
        sprintf(filePathBuf, "%s", pathBuf);
    }
#endif

    StrCopy(fileInfo->fileName, "");

    if (Engine.UseBinFile && !Engine.forceFolder) {
        fileInfo->cFileHandle = fOpen(binFileName, "rb");
        fSeek(fileInfo->cFileHandle, 0, SEEK_END);
        fileInfo->fileSize       = (int)fTell(fileInfo->cFileHandle);
        fileInfo->bufferPosition = 0;
        // ReadSize       = 0;
        fileInfo->readPos   = 0;
        fileInfo->encrypted = true;

        StrCopy(fileInfo->fileName, filePath);
        if (!ParseVirtualFileSystem2(fileInfo)) {
            fClose(fileInfo->cFileHandle);
            fileInfo->cFileHandle = NULL;
            PrintLog("Couldn't load file '%s'", filePathBuf);
            return false;
        }
        fileInfo->fileBuffer = (byte *)malloc(fileInfo->vFileSize);
        FileRead2(fileInfo, fileInfo->fileBuffer, fileInfo->vFileSize, false);
        fileInfo->readPos        = 0;
        fileInfo->bufferPosition = 0;
        fClose(fileInfo->cFileHandle);
    } else {
        StrCopy(fileInfo->fileName, filePathBuf);
        fileInfo->cFileHandle = fOpen(fileInfo->fileName, "rb");
        if (!fileInfo->cFileHandle) {
            PrintLog("Couldn't load file '%s'", filePathBuf);
            return false;
        }

        fSeek(fileInfo->cFileHandle, 0, SEEK_END);
        fileInfo->vFileSize = (int)fTell(fileInfo->cFileHandle);
        fileInfo->fileSize  = fileInfo->vFileSize;
        fSeek(fileInfo->cFileHandle, 0, SEEK_SET);
        ReadPos                     = 0;
        fileInfo->readPos           = ReadPos;
        fileInfo->virtualFileOffset = 0;
        fileInfo->bufferPosition    = 0;
        fileInfo->fileBuffer        = (byte *)malloc(fileInfo->vFileSize);
        FileRead2(fileInfo, fileInfo->fileBuffer, fileInfo->vFileSize, false);
        fileInfo->readPos        = 0;
        fileInfo->bufferPosition = 0;
        fClose(fileInfo->cFileHandle);
        fileInfo->encrypted = false;
    }
    fileInfo->bufferPosition = 0;

    PrintLog("Loaded File '%s'", filePathBuf);

    return true;
}

bool ParseVirtualFileSystem2(FileInfo *fileInfo) {
    char filename[0x50];
    char fullFilename[0x50];
    char stringBuffer[0x50];
    ushort dirCount = 0;
    int fileOffset  = 0;
    int fNamePos    = 0;
    int headerSize  = 0;
    int i           = 0;
    byte fileBuffer = 0;

    int j                       = 0;
    fileInfo->virtualFileOffset = 0;
    for (int i = 0; fileInfo->fileName[i]; i++) {
        if (fileInfo->fileName[i] == '/') {
            fNamePos = i;
            j        = 0;
        } else {
            ++j;
        }
        fullFilename[i] = fileInfo->fileName[i];
    }
    ++fNamePos;
    for (i = 0; i < j; ++i) filename[i] = fileInfo->fileName[i + fNamePos];
    filename[j]            = 0;
    fullFilename[fNamePos] = 0;

    fSeek(fileInfo->cFileHandle, 0, SEEK_SET);
    Engine.UseBinFile        = false;
    fileInfo->bufferPosition = 0;
    // ReadSize             = 0;
    fileInfo->readPos = 0;

    FileRead2(fileInfo, &fileBuffer, 1, false);
    headerSize = fileBuffer;
    FileRead2(fileInfo, &fileBuffer, 1, false);
    headerSize += fileBuffer << 8;
    FileRead2(fileInfo, &fileBuffer, 1, false);
    headerSize += fileBuffer << 16;
    FileRead2(fileInfo, &fileBuffer, 1, false);
    headerSize += fileBuffer << 24;

    FileRead2(fileInfo, &fileBuffer, 1, false);
    dirCount = fileBuffer;

    i                  = 0;
    fileOffset         = 0;
    int nextFileOffset = 0;
    while (i < dirCount) {
        FileRead2(fileInfo, &fileBuffer, 1, false);
        for (j = 0; j < fileBuffer; ++j) {
            FileRead2(fileInfo, &stringBuffer[j], 1, false);
        }
        stringBuffer[j] = 0;

        if (StrComp(fullFilename, stringBuffer)) {
            FileRead2(fileInfo, &fileBuffer, 1, false);
            fileOffset = fileBuffer;
            FileRead2(fileInfo, &fileBuffer, 1, false);
            fileOffset += fileBuffer << 8;
            FileRead2(fileInfo, &fileBuffer, 1, false);
            fileOffset += fileBuffer << 16;
            FileRead2(fileInfo, &fileBuffer, 1, false);
            fileOffset += fileBuffer << 24;

            // Grab info for next dir to know when we've found an error
            // Ignore dir name we dont care
            if (i == dirCount - 1) {
                nextFileOffset = FileSize - headerSize; // There is no next dir, so just make this the EOF
            } else {
                FileRead2(fileInfo, &fileBuffer, 1, false);
                for (j = 0; j < fileBuffer; ++j) {
                    FileRead2(fileInfo, &stringBuffer[j], 1, false);
                }

                FileRead2(fileInfo, &fileBuffer, 1, false);
                nextFileOffset = fileBuffer;
                FileRead2(fileInfo, &fileBuffer, 1, false);
                nextFileOffset += fileBuffer << 8;
                FileRead2(fileInfo, &fileBuffer, 1, false);
                nextFileOffset += fileBuffer << 16;
                FileRead2(fileInfo, &fileBuffer, 1, false);
                nextFileOffset += fileBuffer << 24;
            }

            i = dirCount;
        } else {
            fileOffset = -1;
            FileRead2(fileInfo, &fileBuffer, 1, false);
            FileRead2(fileInfo, &fileBuffer, 1, false);
            FileRead2(fileInfo, &fileBuffer, 1, false);
            FileRead2(fileInfo, &fileBuffer, 1, false);
            ++i;
        }
    }

    if (fileOffset == -1) {
        Engine.UseBinFile = true;
        return false;
    } else {
        fSeek(fileInfo->cFileHandle, fileOffset + headerSize, SEEK_SET);
        fileInfo->bufferPosition    = 0;
        fileInfo->readPos           = 0;
        fileInfo->virtualFileOffset = fileOffset + headerSize;
        i                           = 0;
        while (i < 1) {
            FileRead2(fileInfo, &fileBuffer, 1, false);
            ++fileInfo->virtualFileOffset;
            j = 0;
            while (j < fileBuffer) {
                FileRead2(fileInfo, &stringBuffer[j], 1, false);
                ++j;
                ++fileInfo->virtualFileOffset;
            }
            stringBuffer[j] = 0;

            if (StrComp(filename, stringBuffer)) {
                i = 1;
                FileRead2(fileInfo, &fileBuffer, 1, false);
                j = fileBuffer;
                FileRead2(fileInfo, &fileBuffer, 1, false);
                j += fileBuffer << 8;
                FileRead2(fileInfo, &fileBuffer, 1, false);
                j += fileBuffer << 16;
                FileRead2(fileInfo, &fileBuffer, 1, false);
                j += fileBuffer << 24;
                fileInfo->virtualFileOffset += 4;
                fileInfo->vFileSize = j;
            } else {
                FileRead2(fileInfo, &fileBuffer, 1, false);
                j = fileBuffer;
                FileRead2(fileInfo, &fileBuffer, 1, false);
                j += fileBuffer << 8;
                FileRead2(fileInfo, &fileBuffer, 1, false);
                j += fileBuffer << 16;
                FileRead2(fileInfo, &fileBuffer, 1, false);
                j += fileBuffer << 24;
                fileInfo->virtualFileOffset += 4;
                fileInfo->virtualFileOffset += j;
            }

            // No File has been found (next file would be in a new dir)
            if (fileInfo->virtualFileOffset >= nextFileOffset + headerSize) {
                Engine.UseBinFile = true;
                return false;
            }
            fSeek(fileInfo->cFileHandle, fileInfo->virtualFileOffset, SEEK_SET);
            fileInfo->bufferPosition = 0;
            // ReadSize       = 0;
            fileInfo->readPos = fileInfo->virtualFileOffset;
        }
        Engine.UseBinFile = true;
        return true;
    }
    // Engine.UseBinFile = true;
    return false;
}

size_t FileRead2(FileInfo *info, void *dest, int size, bool fromBuffer) {
    byte *data = (byte *)dest;
    int rPos   = (int)GetFilePosition2(info);
    memset(data, 0, size);

    if (fromBuffer) {
        if (info->readPos + size >= info->vFileSize)
            size = info->vFileSize - info->readPos;
        memcpy(dest, &info->fileBuffer[info->readPos], size);
        info->readPos += size;
        info->bufferPosition = 0;
        return size;
    } else {
        if (rPos <= info->fileSize) {
            if (Engine.UseBinFile && !Engine.forceFolder) {
                int rSize = 0;
                if (rPos + size <= info->fileSize)
                    rSize = size;
                else
                    rSize = info->fileSize - rPos;

                size_t result = fRead(data, 1u, rSize, info->cFileHandle);
                info->readPos += rSize;
                info->bufferPosition = 0;

                while (size > 0) {
                    data++;
                    --size;
                }

                return result;
            } else {
                int rSize = 0;
                if (rPos + size <= info->fileSize)
                    rSize = size;
                else
                    rSize = info->fileSize - rPos;

                size_t result = fRead(data, 1u, rSize, info->cFileHandle);
                info->readPos += rSize;
                info->bufferPosition = 0;
                return result;
            }
        }
    }
    return 0;
}

size_t GetFilePosition2(FileInfo *info) { return info->readPos; }
void SetFilePosition2(FileInfo *info, int newPos) { info->readPos = newPos; }