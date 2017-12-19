#include <stdio.h>
#include <windows.h>
#include <HashMap.h>

/**
 * hash函数
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


void find(char *dir, char *pattern) {

    /*搜索路径*/
    char *LookUpPath = calloc(MAX_PATH, sizeof(char));
    strcpy(LookUpPath, dir);
    strcat(LookUpPath, "/*");

    /*找到的文件*/
    WIN32_FIND_DATA findData;
    WIN32_FIND_DATA * pFindData = &findData;

    HANDLE hFind= FindFirstFile(LookUpPath, pFindData);

    if(INVALID_HANDLE_VALUE == hFind){
        return;
    } else {
        HashMap  hashMap = CreateHashMap(64, &hash, &equal);

        while(TRUE) {
            if(findData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
            {
//                if(findData.cFileName[0] != '.')
//                {
//                    strcpy(szFile,dir);
//                    strcat(szFile,"");
//                    strcat(szFile,findData.cFileName);
//                    find(szFile);
//                }
            }
            else
            {

                //start with .
                if(findData.cFileName[0] == '.'){
                    if(!FindNextFile(hFind,&findData)){
                        break;
                    }
                }

				//获取核心名称
                char *KeyFileName0 = calloc(MAX_PATH, sizeof(char));
                cut(findData.cFileName, ".", KeyFileName0);
                char *KeyFileName = calloc(MAX_PATH, sizeof(char));
                cut(KeyFileName0, pattern, KeyFileName);
                free(KeyFileName0);

                WIN32_FIND_DATA *value = GetHashMap(hashMap, KeyFileName);

                if(value == NULL){
					//复制到新的结构体，放进map
                    WIN32_FIND_DATA *currentFindFileData = calloc(1, sizeof(WIN32_FIND_DATA));
                    currentFindFileData->nFileSizeLow = findData.nFileSizeLow;
                    strcpy(currentFindFileData->cFileName, findData.cFileName);
                    PutHashMap(hashMap, KeyFileName, currentFindFileData);

                    fprintf(stdout, "find %s,%u\n", currentFindFileData->cFileName, currentFindFileData->nFileSizeLow);
                } else {

                    WIN32_FIND_DATA more;
                    WIN32_FIND_DATA less;
                    //删除小的
                    if((*value).nFileSizeLow > findData.nFileSizeLow){
                        more = *value;
                        less = findData;
                    } else {
                        less = *value;
                        more = findData;
                    }

                    fprintf(stdout, "compare %s,%u greater than %s,%u\n", more.cFileName, more.nFileSizeLow, less.cFileName, less.nFileSizeLow);

                    char *name2Delete = calloc(MAX_PATH, sizeof(char));
                    strcpy(name2Delete, dir);
                    strcat(name2Delete, "/");
                    strcat(name2Delete, less.cFileName);
                    DeleteFile(name2Delete);

                    fprintf(stdout, "delete %s,%u\n", less.cFileName, less.nFileSizeLow);
                    free(name2Delete);

					//大的复制到新的结构体，放进map
                    WIN32_FIND_DATA *currentFindFileData = calloc(1, sizeof(WIN32_FIND_DATA));
                    currentFindFileData->nFileSizeLow = more.nFileSizeLow;
                    strcpy(currentFindFileData->cFileName, more.cFileName);
                    PutHashMap(hashMap, KeyFileName, currentFindFileData);

                    fprintf(stdout, "keep %s,%u\n", currentFindFileData->cFileName, currentFindFileData->nFileSizeLow);

                    free(value);
                }

                fflush(stdout);
            }

           
            if(!FindNextFile(hFind,&findData)){
                break;
            }
        }
		                    
		//重命名

		unsigned int size = SizeHashMap(hashMap);
        KvPairHashMap *p = PairListHashMap(hashMap);

		for (int i = 0; i < size; i++){
			WIN32_FIND_DATA more;
            WIN32_FIND_DATA * moreP = (p + i)->value;
            more = *moreP;

            fprintf(stdout, "rename before %s\n", more.cFileName);

			char * suffix = strchr(more.cFileName, '.');
            int    suffixLen;
            if(suffix == NULL){
                suffixLen = 0;
            } else {
                suffixLen = strlen(suffix);
            }

            //fprintf(stdout, "rename suffix %s\n", suffix);

			char *renameFr = calloc(MAX_PATH, sizeof(char));
			strcpy(renameFr, dir);
			strcat(renameFr, "/");
			strcat(renameFr, more.cFileName);

            fprintf(stdout, "rename from %s\n", renameFr);

            //获取核心名称
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

            fprintf(stdout, "rename to   %s\n", renameTo);

            if(equal(renameFr, renameTo)){
                fprintf(stdout, "rename ignore\n");
            } else {
                char fr[MAX_PATH];
                char to[MAX_PATH];
                strcpy(fr, renameFr);
                strcpy(to, renameTo);
                MoveFile(fr, to);
                fprintf(stdout, "rename success\n");
            }

            free(renameFr);
            free(renameTo);
            free(p->key);
            free(p->value);
            free(moreP);
		}
        fflush(stdout);
        DestroyHashMap(hashMap);
        FindClose(hFind);
    }

    free(LookUpPath);

}


int main()
{
    fprintf(stdout, "请输入音乐目录\n");
    fflush(stdout);
    char *filepath = calloc(MAX_PATH, sizeof(char));
    fgets(filepath, MAX_PATH, stdin);

    fprintf(stdout, "请输入音乐名不重复的部分开头\n");
    fflush(stdout);
    char *pattern = calloc(MAX_PATH, sizeof(char));
    fgets(pattern, MAX_PATH, stdin);

    if ((strlen(filepath) > 0) && (filepath[strlen (filepath) - 1] == '\n')){
        filepath[strlen (filepath) - 1] = '\0';
    }

    if ((strlen(pattern) > 0) && (pattern[strlen (pattern) - 1] == '\n')){
        pattern[strlen (pattern) - 1] = '\0';
    }

    find(filepath, pattern);

    free(filepath);
    free(pattern);

    fprintf(stdout, "按任意键退出\n");
    fflush(stdout);
    getchar();//等待终端输入任意字符

    return 0;//退出程序。
}