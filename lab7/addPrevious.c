//CSC60 Dylan Prosser
int addPrevious(int inNum){
    static int svPrev = 0;
    int retVal = svPrev +  inNum;
    svPrev = inNum + svPrev;
    return(retVal);
}
