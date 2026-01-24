#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "shlwapi.lib")
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#endif

// Function: Calculate file MD5
BOOL GetFileMD5(const char* filename, BYTE* md5) {
#ifdef _WIN32
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
#else
    FILE* file = fopen(filename, "rb");
    if (!file) return FALSE;

    MD5_CTX ctx;
    MD5_Init(&ctx);

    unsigned char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        MD5_Update(&ctx, buffer, bytesRead);
    }

    MD5_Final(md5, &ctx);
    fclose(file);
    return TRUE;
#endif
}

// Function: Copy files with MD5 check
BOOL CopyFilesWithMD5Check(const char* sourceDir, const char* destDir) {
#ifdef _WIN32
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
#else
    DIR* dir = opendir(sourceDir);
    if (!dir) {
        printf("Failed to open directory %s\n", sourceDir);
        return FALSE;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char sourceFile[PATH_MAX];
        char destFile[PATH_MAX];
        sprintf(sourceFile, "%s/%s", sourceDir, entry->d_name);
        sprintf(destFile, "%s/%s", destDir, entry->d_name);

        // Check if it's a regular file
        struct stat st;
        if (stat(sourceFile, &st) != 0 || !S_ISREG(st.st_mode)) continue;

        // Check if file exists in destination
        if (access(destFile, F_OK) == 0) {
            BYTE md5Source[16], md5Dest[16];
            if (!GetFileMD5(sourceFile, md5Source) || !GetFileMD5(destFile, md5Dest)) {
                printf("Failed to calculate MD5 for %s or %s\n", sourceFile, destFile);
                closedir(dir);
                return FALSE;
            }
            if (memcmp(md5Source, md5Dest, 16) == 0) {
                continue; // Skip copying if MD5 matches
            } else {
                if (remove(destFile) != 0) {
                    printf("Failed to delete existing file %s\n", destFile);
                    closedir(dir);
                    return FALSE;
                }
            }
        }

        // Copy file manually
        FILE* src = fopen(sourceFile, "rb");
        if (!src) {
            printf("Failed to open source file %s\n", sourceFile);
            closedir(dir);
            return FALSE;
        }
        FILE* dst = fopen(destFile, "wb");
        if (!dst) {
            printf("Failed to open destination file %s, possibly in use\n", destFile);
            fclose(src);
            closedir(dir);
            return FALSE;
        }
        char buffer[1024];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
            if (fwrite(buffer, 1, bytes, dst) != bytes) {
                printf("Failed to write to %s\n", destFile);
                fclose(src);
                fclose(dst);
                closedir(dir);
                return FALSE;
            }
        }
        fclose(src);
        fclose(dst);
    }

    closedir(dir);
    return TRUE;
#endif
}

int main() {
    char programDir[PATH_MAX];
#ifdef _WIN32
    GetCurrentDirectory(PATH_MAX, programDir);
#else
    getcwd(programDir, PATH_MAX);
#endif

    // Step 1: Go to parent directory
    char parentDir[PATH_MAX];
    strcpy(parentDir, programDir);
#ifdef _WIN32
    PathRemoveFileSpec(parentDir);
    if (!SetCurrentDirectory("..")) {
        printf("Failed to set working directory to parent.\n");
        goto pause_exit;
    }
#else
    char* lastSlash = strrchr(parentDir, '/');
    if (lastSlash) *lastSlash = '\0';
    if (chdir("..") != 0) {
        printf("Failed to set working directory to parent.\n");
        goto pause_exit;
    }
#endif

    // Step 2: Check for .minecraft directory
#ifdef _WIN32
    if (!PathFileExists(".minecraft")) {
        printf(".minecraft directory not found.\n");
        goto pause_exit;
    }
    if (!SetCurrentDirectory(".minecraft")) {
        printf("Failed to set working directory to .minecraft.\n");
        goto pause_exit;
    }
#else
    if (!dir_exists(".minecraft")) {
        printf(".minecraft directory not found.\n");
        goto pause_exit;
    }
    if (chdir(".minecraft") != 0) {
        printf("Failed to set working directory to .minecraft.\n");
        goto pause_exit;
    }
#endif

    // Step 3: Check for versions directory
#ifdef _WIN32
    if (!PathFileExists("versions")) {
        printf("versions directory not found.\n");
        goto pause_exit;
    }
    if (!SetCurrentDirectory("versions")) {
        printf("Failed to set working directory to versions.\n");
        goto pause_exit;
    }
#else
    if (!dir_exists("versions")) {
        printf("versions directory not found.\n");
        goto pause_exit;
    }
    if (chdir("versions") != 0) {
        printf("Failed to set working directory to versions.\n");
        goto pause_exit;
    }
#endif

    // Step 4: Check for BingoTech directory
#ifdef _WIN32
    if (!PathFileExists("BingoTech")) {
        printf("BingoTech directory not found.\n");
        goto pause_exit;
    }
    if (!SetCurrentDirectory("BingoTech")) {
        printf("Failed to set working directory to BingoTech.\n");
        goto pause_exit;
    }
#else
    if (!dir_exists("BingoTech")) {
        printf("BingoTech directory not found.\n");
        goto pause_exit;
    }
    if (chdir("BingoTech") != 0) {
        printf("Failed to set working directory to BingoTech.\n");
        goto pause_exit;
    }
#endif

    // Step 5: Check for mods directory
#ifdef _WIN32
    if (!PathFileExists("mods")) {
        printf("mods directory not found.\n");
        goto pause_exit;
    }
#else
    if (!dir_exists("mods")) {
        printf("mods directory not found.\n");
        goto pause_exit;
    }
#endif

    printf("Verification complete.\n");

    // Step 6: Go back to program directory and check mods_update
#ifdef _WIN32
    if (!SetCurrentDirectory(programDir)) {
        printf("Failed to set working directory back to program directory.\n");
        goto pause_exit;
    }
    if (!PathFileExists("mods_update")) {
        printf("mods_update directory not found.\n");
        goto pause_exit;
    }
    if (!SetCurrentDirectory("mods_update")) {
        printf("Failed to set working directory to mods_update.\n");
        goto pause_exit;
    }
#else
    if (chdir(programDir) != 0) {
        printf("Failed to set working directory back to program directory.\n");
        goto pause_exit;
    }
    if (!dir_exists("mods_update")) {
        printf("mods_update directory not found.\n");
        goto pause_exit;
    }
    if (chdir("mods_update") != 0) {
        printf("Failed to set working directory to mods_update.\n");
        goto pause_exit;
    }
#endif

    // Step 7: Copy files from mods_update to mods
    char modsDir[PATH_MAX];
#ifdef _WIN32
    sprintf(modsDir, "%s\\.minecraft\\versions\\BingoTech\\mods", parentDir);
#else
    sprintf(modsDir, "%s/.minecraft/versions/BingoTech/mods", parentDir);
#endif
    if (!CopyFilesWithMD5Check(".", modsDir)) {
        printf("Copy operation failed.\n");
        goto pause_exit;
    }

    printf("Files copied successfully.\n");

pause_exit:
#ifdef _WIN32
    system("pause");
#else
    printf("Press Enter to continue...");
    getchar();
#endif
    return 0;
}