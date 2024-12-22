#include <deque>
#include <array>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unordered_map>

#include "hash.h"
#if defined PATH_MAX
#undef PATH_MAX
#endif
#define PATH_MAX 4096
#define BLOCK_SIZE 8192

struct DedupItem
{
    uint32_t linksToBlock_;;
    size_t blockSize_;
    char* blockData_;
};

struct RestoreItem
{
    std::string name_;
    std::deque<uint32_t> hashes_;
};

std::unordered_map<uint32_t,DedupItem> dedupContainer;

int main()
{
    char path[]="/home/yaroslav/Deadstream.avi";
    FILE* inPtr=fopen(path,"rb");
    if(!inPtr)
        return 1;
    RestoreItem restItem{"Restored.avi"};

    size_t readed=0;
    size_t realSize=0;
    size_t stepCounter=0;
    size_t dedupSize=0;
    char block[BLOCK_SIZE]="";
    while((readed=fread(block,sizeof(char),BLOCK_SIZE,inPtr)) > 0){
        ++stepCounter;
        realSize+=readed;
        uint32_t hash=SuperFastHash(block,readed);
        restItem.hashes_.push_back(hash);

        auto found=dedupContainer.find(hash);
        if(found!=dedupContainer.end())
            ++(found->second.linksToBlock_);
        else{
            DedupItem dedupItem {1};
            dedupItem.blockSize_=readed;
            dedupItem.blockData_=(char*)malloc(readed);
            memcpy(dedupItem.blockData_,block,readed);
            dedupContainer.emplace(hash,std::move(dedupItem));
            dedupSize+=readed;
        }
    }
    fclose(inPtr);
    printf("Real size: %zu; Deduplicated size: %zu; Steps: %zu; Chunks: %zu\n",
           realSize,dedupSize,stepCounter,dedupContainer.size());

    char outPath[PATH_MAX]="";
    sprintf(outPath,"/home/yaroslav/%s",restItem.name_.c_str());

    FILE* outPtr=fopen(outPath,"wb+");
    if(!outPtr)
        return 1;
    size_t realRestoredSize=0;
    while(!restItem.hashes_.empty()){
        uint32_t hash=restItem.hashes_.front();
        restItem.hashes_.pop_front();
        auto found=dedupContainer.find(hash);
        if(found!=dedupContainer.end()){
            size_t written=fwrite(found->second.blockData_,sizeof(char),found->second.blockSize_,outPtr);
            realRestoredSize+=written;
        }
    }
    fclose(outPtr);
    printf("File created success; Real restored size: %zu\n",realRestoredSize);
    return 0;
}
