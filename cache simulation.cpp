/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size)  t=32-s-b
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>
#include <vector>

using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss

unsigned long globalTime = 0; //clock

unsigned long getTime()
{
    globalTime++;
    return globalTime;
}


struct config{
       int L1blocksize;
       int L1setsize;
       int L1size;
       int L2blocksize;
       int L2setsize;
       int L2size;
       };

/* you can define the cache class here, or design your own data structure for L1 and L2 cache
class cache {
      
      }
*/
typedef struct addL1 {
    unsigned int tagbits;
    unsigned int indexbits;
    unsigned int tag;
    unsigned int index;
    unsigned int blockoffset;
} AddrL1;//define addrL1

typedef struct addL2 {
    unsigned int tagbits;
    unsigned int indexbits;
    unsigned int tag;
    unsigned int index;
    unsigned int blockoffset;
} AddrL2;//define addrL2

// # bits in block
// # bits representing set
// # bits in tag
// cache --> sets --> line

typedef struct myLine {
    unsigned long tag;
    bool valid;
    unsigned long time;
} Line; //define a line

typedef struct mySet {
    std::vector<Line> myLines;
    unsigned int numLines; // numLines-way associative cache
} Set; //define a set

typedef struct myL1Cache {
    std::vector<Set> mySets;
    unsigned int numSets; // cache-size / block-size / numLines
} L1Cache; //define L1cache


typedef struct myL2Cache {
    std::vector<Set> mySets;
    unsigned int numSets; // cache-size / block-size / numLines
} L2Cache; //define L2 cache


unsigned int powTwo(unsigned int n)
{
    if (n == 0) {
        return 1;
    }

    return 2 * powTwo(n-1);
}// powTwo computes 2 ** n


// nOnes outputs an unsigned number that equals n consecutive ones in binary representation.
// assume n > 0
unsigned int nOnes(unsigned int n)
{
    return powTwo(n) - 1;
}

// get tag, set, and block offset
unsigned int getSet(unsigned int x, unsigned int offsetBits, unsigned int setBits)
{
    return (x >> offsetBits) & nOnes(setBits);
}

unsigned int getTag(unsigned int x, unsigned int offsetBits, unsigned int setBits,unsigned int tagBits )
{
    return (x >> (offsetBits+setBits)) & nOnes(tagBits);
}

unsigned int log2(unsigned int x) {
    unsigned int ret = 0;
    while (x >>= 1) ret++;

    return ret;
}

unsigned int s(unsigned int a, unsigned int b, unsigned int c)
{
    return log2(a) + 10 - log2(b) - log2(c);
} //funtion calculate index bits

unsigned int b(unsigned int a)
{
    return log2(a);
}//calculate blockoffset bits

unsigned int t(unsigned int a, unsigned int b)
{
    return 32-a-b;
}//calculate tag bits

int main(int argc, char* argv[]){

    //config cacheconfig; // = {8, 1, 16, 16, 4, 32};
    AddrL1 addrL1;
    AddrL2 addrL2;
    //Line myLine ;
    Set mySet;
    //L1Cache myL1Cache;
    //L2Cache myL2Cache;
    //initialize
    /*
    ifstream cache_params ("~/Desktop/cacheconfig.txt");
    string dummyLine;
    //cache_params.open("~/Desktop/cacheconfig.txt");
    //while(!cache_params.eof())  // read config file
    //{
//      cache_params>>dummyLine;
//      cache_params>>cacheconfig.L1blocksize;
//      cache_params>>cacheconfig.L1setsize;
//      cache_params>>cacheconfig.L1size;
//      cache_params>>dummyLine;
//      cache_params>>cacheconfig.L2blocksize;
//      cache_params>>cacheconfig.L2setsize;
//      cache_params>>cacheconfig.L2size;
    //}

    std::vector<int> nums = std::vector<int>(6);
    std::string str;
    for (int i = 0; i < 6; i++)
    {
        std::getline(cache_params, str);
        nums[i] = std::stoi(str);
    }
     */

    ifstream cc;
    string l;
    cc.open(argv[1]);
    std::vector<int> nums = std::vector<int>(10);
    int idx = 0;
    if (cc.is_open()) {
        while (getline(cc, l)) {
            if (idx != 0 && idx != 4) {
                std::istringstream ss(l);
                ss >> nums[idx];
            }

            idx++;
        }
    }

    config cacheconfig = {nums[1], nums[2], nums[3], nums[5], nums[6], nums[7]};
    // compute cache bits
    addrL1.indexbits=s(cacheconfig.L1size, cacheconfig.L1setsize, cacheconfig.L1blocksize);
    addrL1.blockoffset=b(cacheconfig.L1blocksize);
    addrL1.tagbits= t( addrL1.blockoffset , addrL1.indexbits); //calculate the addrL1 bits

    addrL2.indexbits=s( cacheconfig.L2size,cacheconfig.L2setsize, cacheconfig.L2blocksize );
    addrL2.blockoffset= b( cacheconfig.L2blocksize);
    addrL2.tagbits=t( addrL2.blockoffset,addrL2.indexbits);//calculate the addrL2bits


    //initialize L1
    L1Cache L1 = {};
    L1.numSets = powTwo(addrL1.indexbits);
    L1.mySets = std::vector<Set> (L1.numSets);
    for (unsigned int i = 0; i < L1.numSets; i++)
    {
        // initialize cache structs
        Set s = {};
        s.myLines = std::vector<Line>(cacheconfig.L1setsize);
        s.numLines = cacheconfig.L1setsize;

        std::vector<Line> lines = s.myLines;
        for (unsigned int j = 0; j < s.numLines; j++)
        {
            // initialize every line
            lines[j].valid = false;
            lines[j].tag=0;
        }

        L1.mySets[i] = s;
    }


    //initialize L2
    L2Cache L2={};
    L2.numSets=powTwo(addrL2.indexbits);
    L2.mySets=std::vector<Set>(L2.numSets);
    for (unsigned int i = 0; i < L2.numSets; ++i)
    {
     Set s ={};
     s.myLines=std::vector<Line>(cacheconfig.L2setsize);
     s.numLines=cacheconfig.L2setsize;

     std::vector<Line> lines=s.myLines;
     for (unsigned int j = 0; j < s.numLines; ++j)
        {
            lines[j].valid=false;
            lines[j].tag=0;
        }
        L2.mySets[i]=s;
    }

   
  int L1AcceState =0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
  int L2AcceState =0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;
   
   
    ifstream traces ;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";
    
    traces.open(argv[2]);
    tracesout.open(outname);
    
    string line;
    string accesstype;  // the Read/Write access type from the memory trace;
    string xaddr;       // the address from the memory trace store in hex;
    unsigned int addr;  // the address from the memory trace store in unsigned int;        
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;


    unsigned long lineNum = 0;
    if (traces.is_open()&&tracesout.is_open()){    
        while (getline (traces,line)){   // read mem access file and access Cache
            lineNum++;
            
            istringstream iss(line); 
            if (!(iss >> accesstype >> xaddr)) {break;}
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32> (addr);

            addrL1.tag=getTag( addr, addrL1.blockoffset, addrL1.indexbits, addrL1.tagbits);
            addrL1.index=getSet(addr, addrL1.blockoffset,addrL1.indexbits);
            //get L1 tag and index

            addrL2.tag=getTag(addr,addrL2.blockoffset,addrL2.indexbits, addrL2.tagbits);
            addrL2.index=getSet( addr,addrL2.blockoffset,addrL2.indexbits);
            //get L2 tag and index
           
           
           // access the L1 and L2 Cache according to the trace;
              if (accesstype.compare("R")==0)
              {
                 Set thisL1Set = L1.mySets[addrL1.index];
                 bool foundL1 = false;
                 bool foundL2 = false;
                 for (int i=0;i<thisL1Set.numLines; i++)
                 {
                     Line myLine = thisL1Set.myLines[i];
                     unsigned long t = myLine.tag;
                     if (t==addrL1.tag)
                     {
                         if (myLine.valid==1)
                         {
                             L1AcceState=RH;
                             L2AcceState=NA;
                             myLine.time=getTime();//update LRU
                             foundL1 = true;
                         }

                     }

                 }

                 if (!foundL1)
                 {
                     L1AcceState=RM;
                     Set thisL2Set =L2.mySets[addrL2.index];
                     for(int i=0;i<thisL2Set.numLines;i++)
                     {
                         Line myLine =thisL2Set.myLines[i];
                         unsigned long t = myLine.tag;
                         if(t==addrL2.tag)
                         {
                             if (myLine.valid==1)
                             {
                                 L2AcceState=RH;
                                 myLine.time=getTime();

                                 bool empty= false;
                                 for(int j=0;j<thisL1Set.numLines;j++)
                                 {
                                     if(thisL1Set.myLines[j].valid==0)
                                     {
                                         // TODO: debug changed this
                                         L1.mySets[addrL1.index].myLines[j].tag=addrL1.tag;
                                         L1.mySets[addrL1.index].myLines[j].valid=1;
                                         L1.mySets[addrL1.index].myLines[j].time=getTime();
                                         empty=true;
                                         break;
                                     }//if there is a empty line
                                 }
                                 if(!empty)
                                 {
                                     int leastIndex = 0; //index of line so far with earliest timestamp
                                     unsigned long leastValue = getTime(); // timestamp of line so far with earliest timestamp

                                     for(int j=0;j<thisL1Set.numLines;j++)
                                     {
                                         if (thisL1Set.myLines[j].time<leastValue)
                                         {
                                             leastValue=thisL1Set.myLines[j].time;
                                             leastIndex=j;
                                         }
                                     }
                                     L1.mySets[addrL1.index].myLines[leastIndex].tag=addrL1.tag;
                                     L1.mySets[addrL1.index].myLines[leastIndex].valid=1;
                                     L1.mySets[addrL1.index].myLines[leastIndex].time=getTime();
                                 }//do LRU TODO:?

                                 foundL2=true;
                             }
                         }

                     }
                     if (!foundL2)
                     {
                         L2AcceState=RM;
                         
                         bool emptyL1= false;
                         for(int j=0;j<thisL1Set.numLines;j++)
                         {
                             if(thisL1Set.myLines[j].valid==0)
                             {
                                 // TODO: changed this
                                 L1.mySets[addrL1.index].myLines[j].tag=addrL1.tag;
                                 L1.mySets[addrL1.index].myLines[j].valid=1;
                                 L1.mySets[addrL1.index].myLines[j].time=getTime();
                                 emptyL1  =true;
                                 break;
                             }//if there is a empty line
                         }
                         if(!emptyL1)
                         {
                             int leastIndex = 0; //index of line so far with earliest timestamp
                             unsigned long leastValue = getTime(); // timestamp of line so far with earliest timestamp

                             for(int j=0;j<thisL1Set.numLines;j++)
                             {
                                 if (thisL1Set.myLines[j].time<leastValue)
                                 {
                                     leastValue=thisL1Set.myLines[j].time;
                                     leastIndex=j;
                                 }
                             }
                             // TODO: I changed this
                             L1.mySets[addrL1.index].myLines[leastIndex].tag=addrL1.tag;
                             L1.mySets[addrL1.index].myLines[leastIndex].valid=1;
                             L1.mySets[addrL1.index].myLines[leastIndex].time=getTime();
                         }//do LRU

                         bool emptyL2= false;
                         for(int j=0;j<thisL2Set.numLines;j++)
                         {
                             if(thisL2Set.myLines[j].valid==0)
                             {
                                 //TODO: I changed this
                                 L2.mySets[addrL2.index].myLines[j].tag=addrL2.tag;
                                 L2.mySets[addrL2.index].myLines[j].valid=1;
                                 L2.mySets[addrL2.index].myLines[j].time=getTime();
                                 emptyL2=true;
                                 break;
                             }//if there is a empty line
                         }
                         if(!emptyL2)
                         {
                             int leastIndex = 0; //index of line so far with earliest timestamp
                             unsigned long leastValue = getTime(); // timestamp of line so far with earliest timestamp

                             for(int j=0;j<thisL2Set.numLines;j++)
                             {
                                 if (thisL2Set.myLines[j].time<leastValue)
                                 {
                                     leastValue=thisL2Set.myLines[j].time;
                                     leastIndex=j;
                                 }
                             }
                             // TODO: I changed this
                             L2.mySets[addrL2.index].myLines[leastIndex].tag=addrL2.tag;
                             L2.mySets[addrL2.index].myLines[leastIndex].valid=1;
                             L2.mySets[addrL2.index].myLines[leastIndex].time=getTime();
                         }//do L2 LRU
                         
                         //
                     }
                 }
              }
              //TODO;WRITE
              else
              {
                  Set thisL1Set = L1.mySets[addrL1.index];
                  bool foundL1 = false;
                  bool foundL2 = false;
                  for (int i=0;i<thisL1Set.numLines; i++)
                  {
                      Line myL1Line = thisL1Set.myLines[i];
                      unsigned long t = myL1Line.tag;
                      if (t == addrL1.tag)
                      {
                          if (myL1Line.valid == 1)
                          {
                              L1AcceState = WH;
                              L2AcceState = WH;
                              thisL1Set.myLines[i].time=getTime();//update L1 LRU
                              Set thisL2Set=L2.mySets[addrL2.index];
                              for(int j=0;j<thisL2Set.numLines;j++)
                              {
                                  Line myL2Line=thisL2Set.myLines[j];
                                  if (myL2Line.tag == addrL2.tag)
                                  {
                                      L2.mySets[addrL2.index].myLines[j].time=getTime();//update L2 LRU
                                  }
                              }
                              foundL1 = true;
                          }
                      }
                  }
                  if(!foundL1)
                  {
                      L1AcceState=WM;
                      Set thisL2Set=L2.mySets[addrL2.index];
                      for(int j=0;j<thisL2Set.numLines;j++)
                      {
                          Line myL2Line=L2.mySets[addrL2.index].myLines[j];
                          if (myL2Line.tag == addrL2.tag)
                          {
                              if(myL2Line.valid==1)
                              {
                                  L2AcceState=WH;
                                  L2.mySets[addrL2.index].myLines[j].time=getTime();
                                  foundL2 = true;
                              }
                          }

                      }
                      if(!foundL2)
                      {
                          L2AcceState=WM;
                      }
                  }

              }
              
              
             
            tracesout<< L1AcceState << " " << L2AcceState << endl;  // Output hit/miss results for L1 and L2 to the output file;
             
             
        }
        traces.close();
        tracesout.close(); 
    }
    else cout<< "Unable to open trace or traceout file ";


   
    
  

   
    return 0;
}

