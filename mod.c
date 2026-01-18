#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shlwapi.h>
#include <time.h>
#include <stdbool.h>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "shlwapi.lib")

// Function: Calculate file MD5
BOOL GetFileMD5(const char* filename, BYTE* md5) {
    HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;

    HCRYPTPROV hProv = 0;
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CloseHandle(hFile);
        return FALSE;
    }

    HCRYPTHASH hHash = 0;
    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        CloseHandle(hFile);
        return FALSE;
    }

    BYTE buffer[1024];
    DWORD bytesRead;
    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        if (!CryptHashData(hHash, buffer, bytesRead, 0)) {
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            CloseHandle(hFile);
            return FALSE;
        }
    }

    DWORD hashLen = 16;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, md5, &hashLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        CloseHandle(hFile);
        return FALSE;
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);
    return TRUE;
}

// Function: Copy files with MD5 check
BOOL CopyFilesWithMD5Check(const char* sourceDir, const char* destDir) {
    char pattern[MAX_PATH];
    sprintf(pattern, "%s\\*", sourceDir);
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(pattern, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Failed to find files in %s\n", sourceDir);
        return FALSE;
    }

    do {
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) continue;
        char sourceFile[MAX_PATH];
        char destFile[MAX_PATH];
        sprintf(sourceFile, "%s\\%s", sourceDir, findFileData.cFileName);
        sprintf(destFile, "%s\\%s", destDir, findFileData.cFileName);

        // Check if file exists in destination
        if (PathFileExists(destFile)) {
            BYTE md5Source[16], md5Dest[16];
            if (!GetFileMD5(sourceFile, md5Source) || !GetFileMD5(destFile, md5Dest)) {
                printf("Failed to calculate MD5 for %s or %s\n", sourceFile, destFile);
                FindClose(hFind);
                return FALSE;
            }
            if (memcmp(md5Source, md5Dest, 16) == 0) {
                continue; // Skip copying if MD5 matches
            } else {
                if (!DeleteFile(destFile)) {
                    printf("Failed to delete existing file %s\n", destFile);
                    FindClose(hFind);
                    return FALSE;
                }
            }
        }

        // Copy file
        if (!CopyFile(sourceFile, destFile, FALSE)) {
            printf("Copy failed for %s, possibly file in use\n", sourceFile);
            FindClose(hFind);
            return FALSE;
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return TRUE;
}

int main() {
    char programDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, programDir); // Get program directory

    char parentDir[MAX_PATH];
    strcpy(parentDir, programDir);
    PathRemoveFileSpec(parentDir); // Get parent directory

    // Step 1: Go to parent directory
    if (!SetCurrentDirectory("..")) {
        printf("Failed to set working directory to parent.\n");
        system("pause");
        return 1;
    }

    // Step 2: Check for .minecraft directory
    if (!PathFileExists(".minecraft")) {
        printf(".minecraft directory not found.\n");
        system("pause");
        return 1;
    }
    if (!SetCurrentDirectory(".minecraft")) {
        printf("Failed to set working directory to .minecraft.\n");
        system("pause");
        return 1;
    }

    // Step 3: Check for versions directory
    if (!PathFileExists("versions")) {
        printf("versions directory not found.\n");
        system("pause");
        return 1;
    }
    if (!SetCurrentDirectory("versions")) {
        printf("Failed to set working directory to versions.\n");
        system("pause");
        return 1;
    }

    // Step 4: Check for BingoTech directory
    if (!PathFileExists("BingoTech")) {
        printf("BingoTech directory not found.\n");
        system("pause");
        return 1;
    }
    if (!SetCurrentDirectory("BingoTech")) {
        printf("Failed to set working directory to BingoTech.\n");
        system("pause");
        return 1;
    }

    // Step 5: Check for mods directory
    if (!PathFileExists("mods")) {
        printf("mods directory not found.\n");
        system("pause");
        return 1;
    }

    printf("Verification complete.\n");

    // Step 6: Go back to program directory and check mods_update
    if (!SetCurrentDirectory(programDir)) {
        printf("Failed to set working directory back to program directory.\n");
        system("pause");
        return 1;
    }
    if (!PathFileExists("mods_update")) {
        printf("mods_update directory not found.\n");
        system("pause");
        return 1;
    }
    if (!SetCurrentDirectory("mods_update")) {
        printf("Failed to set working directory to mods_update.\n");
        system("pause");
        return 1;
    }

    // Step 7: Copy resources to mods
    char resourcesDir[MAX_PATH];
    char modsDir[MAX_PATH];
    sprintf(resourcesDir, "%s\\mods_update", programDir);
    sprintf(modsDir, "%s\\.minecraft\\versions\\BingoTech\\mods", parentDir);
    if (!CopyFilesWithMD5Check(resourcesDir, modsDir)) {
        printf("Copy operation failed.\n");
        system("pause");
        return 1;
    }

    printf("Files copied successfully.\n");
    system("pause");
    return 0;
}