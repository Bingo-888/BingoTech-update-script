#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shlwapi.h>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "shlwapi.lib")

// 函数：计算文件MD5
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

int main() {
    char exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    char wd[MAX_PATH];
    strcpy(wd, exePath);
    PathRemoveFileSpec(wd); // wd = exe目录

    char parent[MAX_PATH];
    strcpy(parent, wd);
    PathRemoveFileSpec(parent); // parent = exe目录的父目录

    // 步骤1: 以parent为wd
    if (!SetCurrentDirectory(parent)) {
        printf("Incomplete result\n");
        system("pause");
        return 1;
    }

    // 步骤2: 找 .minecraft
    char minecraftPath[MAX_PATH];
    sprintf(minecraftPath, "%s\\.minecraft", parent);
    DWORD attr = GetFileAttributes(minecraftPath);
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        printf("Incomplete result\n");
        system("pause");
        return 1;
    }
    if (!SetCurrentDirectory(minecraftPath)) {
        printf("Incomplete result\n");
        system("pause");
        return 1;
    }

    // 步骤3: 找 versions
    char versionsPath[MAX_PATH];
    sprintf(versionsPath, "%s\\versions", minecraftPath);
    attr = GetFileAttributes(versionsPath);
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        printf("Incomplete result\n");
        system("pause");
        return 1;
    }
    if (!SetCurrentDirectory(versionsPath)) {
        printf("Incomplete result\n");
        system("pause");
        return 1;
    }

    // 步骤4: 找 BingoTech
    char bingoTechPath[MAX_PATH];
    sprintf(bingoTechPath, "%s\\BingoTech", versionsPath);
    attr = GetFileAttributes(bingoTechPath);
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        printf("Incomplete result\n");
        system("pause");
        return 1;
    }
    if (!SetCurrentDirectory(bingoTechPath)) {
        printf("Incomplete result\n");
        system("pause");
        return 1;
    }

    // 步骤5: 找 mods
    char modsPath[MAX_PATH];
    sprintf(modsPath, "%s\\mods", bingoTechPath);
    attr = GetFileAttributes(modsPath);
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        printf("Incomplete result\n");
        system("pause");
        return 1;
    }
    printf("Inspection completed\n");

    // 步骤6: 回到初始wd，找resources
    if (!SetCurrentDirectory(wd)) { // wd是exe目录
        printf("Incomplete result\n");
        system("pause");
        return 1;
    }
    char resourcesPath[MAX_PATH];
    sprintf(resourcesPath, "%s\\resources", wd);
    attr = GetFileAttributes(resourcesPath);
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        printf("Incomplete result\n");
        system("pause");
        return 1;
    }

    // 步骤7: 拷贝resources到mods
    // 遍历resources
    char searchPath[MAX_PATH];
    sprintf(searchPath, "%s\\*", resourcesPath);
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Incomplete result\n");
        system("pause");
        return 1;
    }
    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) continue;
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            // 是文件
            char srcFile[MAX_PATH];
            sprintf(srcFile, "%s\\%s", resourcesPath, findData.cFileName);
            char dstFile[MAX_PATH];
            sprintf(dstFile, "%s\\%s", modsPath, findData.cFileName);
            // 检查dst是否存在
            if (GetFileAttributes(dstFile) != INVALID_FILE_ATTRIBUTES) {
                // 存在，比较MD5
                BYTE md5Src[16], md5Dst[16];
                if (!GetFileMD5(srcFile, md5Src) || !GetFileMD5(dstFile, md5Dst)) {
                    printf("Incomplete result\n");
                    system("pause");
                    return 1;
                }
                if (memcmp(md5Src, md5Dst, 16) == 0) {
                    // 相同，不拷贝
                    continue;
                } else {
                    // 不同，删除dst
                    if (!DeleteFile(dstFile)) {
                        printf("Incomplete result\n");
                        system("pause");
                        return 1;
                    }
                }
            }
            // 拷贝
            if (!CopyFile(srcFile, dstFile, FALSE)) {
                printf("Incomplete result\n");
                system("pause");
                return 1;
            }
        }
    } while (FindNextFile(hFind, &findData));
    FindClose(hFind);

    printf("Inspection completed\n");
    system("pause");
    return 0;
}