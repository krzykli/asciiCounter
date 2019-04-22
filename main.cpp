
#include <stdio.h>
#include "windows.h"
#include "stdint.h"
#include "tracy\Tracy.hpp"
#include <time.h>
#include <immintrin.h>

struct ThreadData
{
    char *headBuffer;
    unsigned int *headTable;
    unsigned int batchSize;
};


DWORD WINAPI worker(LPVOID lpParam)
{
    ZoneScoped;
    ThreadData* tData = (ThreadData*)lpParam;

    for (int i=0; i < tData->batchSize; i++)
    {
        char code = tData->headBuffer[i];
        unsigned int *slot = tData->headTable + code;
        *slot += 1;
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPTSTR lpCmdLine,
        int nShowCmd)
{
    AllocConsole();
    HANDLE stdHdl = GetStdHandle(STD_OUTPUT_HANDLE);

    clock_t start, afterRead, end;
    double cpu_time_used;
    double time_read;
    start = clock();

    unsigned int numberOfFiles = 5;
    const char *filePaths[] = {
        "C:\\Users\\krzykli\\kRay.ppm",
        "C:\\Users\\krzykli\\modelExport.py",
        "C:\\Users\\krzykli\\Pictures\\popup.png",
        "C:\\Users\\krzykli\\Downloads\\6e48z1kc7r40-bugatti.zip",
        "C:\\Users\\krzykli\\Downloads\\WacomTablet_6.3.23-1.exe"
    };

    const unsigned int BUFFER_SIZE = 1024 * 1024 * 1024;

    char *buffer = (char *)malloc(BUFFER_SIZE);
    __m128 a1 = _mm_set_ps( 4.0f, 4.1f, 4.2f, 4.3f );
    __m128 a2 = _mm_set_ps( 8.0f, 1.1f, 2.2f, 3.3f );
    __m128 sum4 = _mm_add_ps( a1, a2 );

    char *currentHead = buffer;
    unsigned int bytesReadTotal = 0;

    unsigned int masterAsciiTable[128] = {0};

    for (int i=0; i < numberOfFiles; i++)
    {
        const char* filePath = filePaths[i];
        HANDLE fd;
        DWORD bytesRead;
        BOOL result;
        fd = CreateFile(filePath,
                        GENERIC_READ,           // open for writing
                        0,                      // do not share
                        NULL,                   // default security
                        OPEN_ALWAYS,            // create new file only
                        FILE_ATTRIBUTE_NORMAL,  // normal file
                        NULL);                  // no attr. template
        ReadFile(fd, (void*)currentHead, BUFFER_SIZE - bytesReadTotal, &bytesRead, 0);
        bytesReadTotal += bytesRead;
        currentHead += bytesRead;
    }
    afterRead = clock();
    time_read = ((double) (afterRead - start)) / CLOCKS_PER_SEC;

    // Split data into threaded batches
    const char maxThreads = 8;
    unsigned int batch = bytesReadTotal / maxThreads;

    unsigned int asciiTables[maxThreads][128] = {0};
    DWORD threadIDs[maxThreads];
    HANDLE threadHandles[maxThreads];

    for (uint8_t i=0; i < maxThreads; i++)
    {
        ThreadData *tData= (ThreadData*)malloc(sizeof(ThreadData));
        tData->headBuffer = buffer + batch * i;
        tData->headTable = asciiTables[i];
        tData->batchSize = batch;

        HANDLE myThread = CreateThread(
                NULL,
                0,
                worker,
                tData,
                0,
                &threadIDs[i]);
        threadHandles[i] = myThread;
    }

    for (int i = 0; i < maxThreads; i++)
        WaitForSingleObject(threadHandles[i], INFINITE);

    for (int t=0; t < maxThreads; t++)
    {
        for (int i=0; i < 128; i++)
        {
            masterAsciiTable[i] += asciiTables[t][i];
        }
    }

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    char asdf[128] = {0};

    DWORD writtenChars;
    for (int i=0; i < 128; i++)
    {
        char asdf[32] = {0};
        sprintf(asdf, "\r(%i) %c %i\n", i, i, masterAsciiTable[i]);
        WriteConsole(stdHdl, asdf, 32, &writtenChars, NULL);
    }

    char s0[64] = {0};
    sprintf(s0, "\rCharacters read: %i\n", bytesReadTotal);
    WriteConsole(stdHdl, s0, 64, &writtenChars, NULL);

    char s1[64] = {0};
    sprintf(s1, "\rRead Time: %f, Count Time: %f\n", time_read, cpu_time_used - time_read);
    WriteConsole(stdHdl, s1, 64, &writtenChars, NULL);

    char s2[64] = {0};
    sprintf(s2, "\rTotal Time: %f\n", cpu_time_used);
    WriteConsole(stdHdl, s2, 64, &writtenChars, NULL);

    system("pause");
    return 0;
}
