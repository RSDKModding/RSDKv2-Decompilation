#ifndef READER_H
#define READER_H

#ifdef FORCE_CASE_INSENSITIVE

#include "fcaseopen.h"
#define FileIO                                          FILE
#define fOpen(path, mode)                               fcaseopen(path, mode)
#define fRead(buffer, elementSize, elementCount, file)  fread(buffer, elementSize, elementCount, file)
#define fSeek(file, offset, whence)                     fseek(file, offset, whence)
#define fTell(file)                                     ftell(file)
#define fClose(file)                                    fclose(file)
#define fWrite(buffer, elementSize, elementCount, file) fwrite(buffer, elementSize, elementCount, file)

#else

#if RETRO_USING_SDL2
#define FileIO                                          SDL_RWops
#define fOpen(path, mode)                               SDL_RWFromFile(path, mode)
#define fRead(buffer, elementSize, elementCount, file)  SDL_RWread(file, buffer, elementSize, elementCount)
#define fSeek(file, offset, whence)                     SDL_RWseek(file, offset, whence)
#define fTell(file)                                     SDL_RWtell(file)
#define fClose(file)                                    SDL_RWclose(file)
#define fWrite(buffer, elementSize, elementCount, file) SDL_RWwrite(file, buffer, elementSize, elementCount)
#else
#define FileIO                                            FILE
#define fOpen(path, mode)                               fopen(path, mode)
#define fRead(buffer, elementSize, elementCount, file)  fread(buffer, elementSize, elementCount, file)
#define fSeek(file, offset, whence)                     fseek(file, offset, whence)
#define fTell(file)                                     ftell(file)
#define fClose(file)                                    fclose(file)
#define fWrite(buffer, elementSize, elementCount, file) fwrite(buffer, elementSize, elementCount, file)
#endif

#endif

struct FileInfo {
    char fileName[0x100];
    int fileSize;
    int vFileSize;
    int readPos;
    int bufferPosition;
    int virtualFileOffset;
#if !RETRO_USE_ORIGINAL_CODE
    bool encrypted;
    byte *fileBuffer;
    FileIO *cFileHandle;
    byte isMod;
#endif
};

extern char binFileName[0x400];

extern char fileName[0x100];
extern byte FileBuffer[0x2000];
extern int FileSize;
extern int VFileSize;
extern int ReadPos;
extern int ReadSize;
extern int BufferPosition;
extern int VirtualFileOffset;
extern byte isModdedFile;

extern FileIO *CFileHandle;

inline void CopyFilePath(char *dest, const char *src)
{
    strcpy(dest, src);
    for (int i = 0;; ++i) {
        if (i >= strlen(dest)) {
            break;
        }

        if (dest[i] == '/')
            dest[i] = '\\';
    }
}
bool CheckBinFile(const char *filePath);

bool LoadFile(const char *filePath, FileInfo *fileInfo);
inline bool CloseFile()
{
    int result = 0;
    if (CFileHandle)
        result = fClose(CFileHandle);

    CFileHandle = NULL;
    return result;
}

void FileRead(void *dest, int size);

bool ParseVirtualFileSystem(FileInfo *fileInfo);

inline size_t FillFileBuffer()
{
    if (ReadPos + 0x2000 <= FileSize)
        ReadSize = 0x2000;
    else 
        ReadSize = FileSize - ReadPos;

    size_t result = fRead(FileBuffer, 1u, ReadSize, CFileHandle);
    ReadPos += ReadSize;
    BufferPosition = 0;
    return result;
}

inline void GetFileInfo(FileInfo *fileInfo)
{
    StrCopy(fileInfo->fileName, fileName);
    fileInfo->bufferPosition    = BufferPosition;
    fileInfo->readPos           = ReadPos - ReadSize;
    fileInfo->fileSize          = FileSize;
    fileInfo->vFileSize         = VFileSize;
    fileInfo->virtualFileOffset = VirtualFileOffset;
    fileInfo->isMod             = isModdedFile;
}
void SetFileInfo(FileInfo *fileInfo);
size_t GetFilePosition();
void SetFilePosition(int newPos);
bool ReachedEndOfFile();

 // For Music Streaming
bool LoadFile2(const char *filePath, FileInfo *fileInfo);
bool ParseVirtualFileSystem2(FileInfo *fileInfo);
size_t FileRead2(FileInfo *info, void *dest, int size, bool fromBuffer);
inline bool CloseFile2(FileInfo *info)
{
    if (info->fileBuffer)
        free(info->fileBuffer);

    info->cFileHandle = NULL;
    info->fileBuffer  = NULL;
    return true;
}
size_t GetFilePosition2(FileInfo *info);
void SetFilePosition2(FileInfo *info, int newPos);

#endif // !READER_H
