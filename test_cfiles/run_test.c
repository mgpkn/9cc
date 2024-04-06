
int main(){



    return 0;

}

void assert(int estimate,int filepath){


    int answer = 0;
    if(estimate == answer){
        printf("%s => %d",filepath,estimate);
        return;
    }
    printf("%s => $expected expected, but got %d",filepath,estimate);
    exit(1);

}