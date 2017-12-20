#include <stdio.h>
#include <windows.h>
#include <HashMap.h>

/**
 * hash鍑芥暟
 * @param keyVoid
 * @return
 */
static int hash(void *keyVoid){
    char * key = keyVoid;
    int h = 5381;
    for (int i = 0; ; i++) {
        if(*(key + i) == '\0'){
            break;
        }

        h = (h << 5) + h +  *(key + i) ;
    }

    return h;
}

static bool equal(void *key1Void, void *key2Void) {
    char * key1 = key1Void;
    char * key2 = key2Void;

    return strcmp(key1, key2) == 0;
}

int cut(char *str, char *cut, char * newStr){
    char * add = strstr(str, cut);
    if(add == NULL){
        strcpy(newStr, str);
        return 0;
    } else {
        unsigned long i = add - str;
        strncpy(newStr, str, i);
        strcat(newStr, "\0");
        return 0;
    }
}


void find(char *dir, char *pattern, bool verbose) {


    char *LookUpPath = calloc(MAX_PATH, sizeof(char));
    strcpy(LookUpPath, dir);
    strcat(LookUpPath, "/*");


    WIN32_FIND_DATA findData;
    WIN32_FIND_DATA * pFindData = &findData;

    HANDLE hFind= FindFirstFile(LookUpPath, pFindData);

    if(INVALID_HANDLE_VALUE == hFind){
        return;
    } else {
        HashMap  hashMap = CreateHashMap(64, &hash, &equal);

        while(TRUE) {

            //recursion in dir
            if(findData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
            {
                if(findData.cFileName[0] != '.') {
                    char *subDir = calloc(MAX_PATH, sizeof(char));
                    strcpy(subDir, dir);
                    strcat(subDir,"/");
                    strcat(subDir, findData.cFileName);
                    find(subDir, pattern, verbose);
                    free(subDir);
                }
            }
            else
            {
                //ignore start with .
                if(findData.cFileName[0] == '.'){
                    if(!FindNextFile(hFind,&findData)){
                        break;
                    }
                }

                //core name
                char *KeyFileName0 = calloc(MAX_PATH, sizeof(char));
                cut(findData.cFileName, ".", KeyFileName0);
                char *KeyFileName = calloc(MAX_PATH, sizeof(char));
                cut(KeyFileName0, pattern, KeyFileName);
                free(KeyFileName0);

                WIN32_FIND_DATA *value = GetHashMap(hashMap, KeyFileName);

                if(value == NULL){
                    //copy and store
                    WIN32_FIND_DATA *currentFindFileData = calloc(1, sizeof(WIN32_FIND_DATA));
                    currentFindFileData->nFileSizeLow = findData.nFileSizeLow;
                    strcpy(currentFindFileData->cFileName, findData.cFileName);
                    PutHashMap(hashMap, KeyFileName, currentFindFileData);

                    if (verbose) {
                        fprintf(stdout, "find %s,%u\n", currentFindFileData->cFileName, currentFindFileData->nFileSizeLow);
                        fflush(stdout);
                    }
                } else {

                    WIN32_FIND_DATA more;
                    WIN32_FIND_DATA less;
                    //delete less
                    if((*value).nFileSizeLow > findData.nFileSizeLow){
                        more = *value;
                        less = findData;
                    } else {
                        less = *value;
                        more = findData;
                    }

                    if (verbose) {
                        fprintf(stdout, "compare %s,%u greater than %s,%u\n", more.cFileName, more.nFileSizeLow, less.cFileName, less.nFileSizeLow);
                        fflush(stdout);
                    }

                    char *name2Delete = calloc(MAX_PATH, sizeof(char));
                    strcpy(name2Delete, dir);
                    strcat(name2Delete, "/");
                    strcat(name2Delete, less.cFileName);
                    DeleteFile(name2Delete);

                    fprintf(stdout, "delete %s,%u\n", name2Delete, less.nFileSizeLow);
                    fflush(stdout);
                    free(name2Delete);

                    //copy and store
                    WIN32_FIND_DATA *currentFindFileData = calloc(1, sizeof(WIN32_FIND_DATA));
                    currentFindFileData->nFileSizeLow = more.nFileSizeLow;
                    strcpy(currentFindFileData->cFileName, more.cFileName);
                    PutHashMap(hashMap, KeyFileName, currentFindFileData);

                    if (verbose) {
                        fprintf(stdout, "keep %s,%u\n", currentFindFileData->cFileName, currentFindFileData->nFileSizeLow);
                        fflush(stdout);
                    }
                    free(value);
                }

                fflush(stdout);
            }


            if(!FindNextFile(hFind,&findData)){
                break;
            }
        }

        //rename
        unsigned int size = SizeHashMap(hashMap);
        KvPairHashMap *p = PairListHashMap(hashMap);

        for (int i = 0; i < size; i++){
            WIN32_FIND_DATA more;
            WIN32_FIND_DATA * moreP = (p + i)->value;
            more = *moreP;

            if (verbose) {
                fprintf(stdout, "rename begin %s\n", more.cFileName);
                fflush(stdout);
            }

            char * suffix = strchr(more.cFileName, '.');
            int    suffixLen;
            if(suffix == NULL){
                suffixLen = 0;
            } else {
                suffixLen = strlen(suffix);
            }


            char *renameFr = calloc(MAX_PATH, sizeof(char));
            strcpy(renameFr, dir);
            strcat(renameFr, "/");
            strcat(renameFr, more.cFileName);

            if (verbose) {
                fprintf(stdout, "rename from %s\n", renameFr);
                fflush(stdout);
            }

            char *KeyFileName0 = calloc(MAX_PATH, sizeof(char));;
            cut(more.cFileName, ".", KeyFileName0);
            char *KeyFileName = calloc(MAX_PATH, sizeof(char));;
            cut(KeyFileName0, pattern, KeyFileName);
            free(KeyFileName0);

            char *renameTo = calloc(MAX_PATH, sizeof(char));
            strcpy(renameTo, dir);
            strcat(renameTo, "/");
            strcat(renameTo, KeyFileName);

            if(suffixLen > 0){
                strcat(renameTo, suffix);
            }

            if (verbose) {
                fprintf(stdout, "rename to   %s\n", renameTo);
                fflush(stdout);
            }

            if(equal(renameFr, renameTo)){
                if(verbose){
                    fprintf(stdout, "rename ignore\n");
                    fflush(stdout);
                }
            } else {
                char fr[MAX_PATH];
                char to[MAX_PATH];
                strcpy(fr, renameFr);
                strcpy(to, renameTo);
                MoveFile(fr, to);
                fprintf(stdout, "rename %s >>> %s\n", fr, to);
                fflush(stdout);
            }

            free(renameFr);
            free(renameTo);
            free((p + i)->key);
            free((p + i)->value);
        }
        DestroyHashMap(hashMap);
        FindClose(hFind);
    }

    free(LookUpPath);

}


int main()
{
    fprintf(stdout, "please input the music directory\n");
    fflush(stdout);
    char *filepath = calloc(MAX_PATH, sizeof(char));
    fgets(filepath, MAX_PATH, stdin);

    fprintf(stdout, "please input the redundant pattern\n");
    fflush(stdout);
    char *pattern = calloc(MAX_PATH, sizeof(char));
    fgets(pattern, MAX_PATH, stdin);

    fprintf(stdout, "show verbose log? (Y/N)\n");
    fflush(stdout);
    char *verbose  = calloc(MAX_PATH, sizeof(char));
    fgets(verbose , MAX_PATH, stdin);

    if ((strlen(filepath) > 0) && (filepath[strlen (filepath) - 1] == '\n')){
        filepath[strlen (filepath) - 1] = '\0';
    }

    if ((strlen(pattern) > 0) && (pattern[strlen (pattern) - 1] == '\n')){
        pattern[strlen (pattern) - 1] = '\0';
    }

    if ((strlen(verbose) > 0) && (verbose[strlen (verbose) - 1] == '\n')){
        verbose[strlen (verbose) - 1] = '\0';
    }

    find(filepath, pattern, strcmp(verbose, "Y") == 0);

    free(filepath);
    free(pattern);
    free(verbose);

    fprintf(stdout, "press any key to exit\n");
    fflush(stdout);
    getchar();

    return 0;
}