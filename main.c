#include <stdio.h>
#include <windows.h>
#include "HashMap.h"




static int hash(char *key){
    long hash = 5381;
    int c;

    while (c = *key++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

static bool equal(char *key1, char *key2) {
    return strcmp(key1, key2) == 0;
}


char* cut(char *str, char *cut){
    char * add = strstr(str, cut);
    if(add == NULL){
        return str;
    } else {
        unsigned int i = add - str;
        char * new_str = malloc(sizeof(char) * (i + 1));
        strncpy(new_str, str, i);
        *(new_str + i) = '\0';
        return new_str;
    }
}

void find(char * lpPath)
{
    char szFind[MAX_PATH], szFile[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    strcpy(szFind, lpPath);
    strcat(szFind, "\\*");
    HANDLE hFind= FindFirstFile(szFind, &FindFileData);
    if(INVALID_HANDLE_VALUE == hFind){
        return;
    } else {
        HashMap  hashMap = createHashMap(64, &hash, equal);
        while(TRUE) {
            if(FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
            {
                if(FindFileData.cFileName[0]!='.')
                {
                    strcpy(szFile,lpPath);
                    strcat(szFile,"");
                    strcat(szFile,FindFileData.cFileName);
                    find(szFile);
                }
            }
            else
            {

                fprintf(stdout, "%s\n", FindFileData.cFileName);
                fprintf(stdout, "%u\n", FindFileData.nFileSizeLow);

                char *keyFileName = cut(cut(FindFileData.cFileName, "."), "(1)");
                fprintf(stdout, "%s\n", keyFileName);

                WIN32_FIND_DATA *value = getFromHashMap(hashMap, keyFileName);
                if(value == NULL){
                    putIntoHashMap(hashMap, keyFileName, &FindFileData);
                } else {
                    if((*value).nFileSizeLow > FindFileData.nFileSizeLow){
                        DeleteFile(FindFileData.cFileName);
                    } else {
                        DeleteFile((*value).cFileName);
                    }
                }


                //printf("%s\n",FindFileData.dwFileAttributes);
                //printf("%d\n",FindFileData.ftCreationTime.dwHighDateTime);
                //printf("%d\n",FindFileData.ftCreationTime.dwLowDateTime);

                fflush(stdout);
            }
            if(!FindNextFile(hFind,&FindFileData)){
                break;
            }
        }

        FindClose(hFind);
    }


}
void main()
{
    char filepath[MAX_PATH]="/var/MusicDR";
    find(filepath);

//    char *cutStr = cut("jljlkl(1)", "(1)");
//    printf("%s\n", cutStr);
//    free(cutStr);
    system("PAUSE");
}