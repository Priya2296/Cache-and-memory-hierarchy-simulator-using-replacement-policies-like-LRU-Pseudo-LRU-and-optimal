#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include<limits>
#include<typeinfo>

using namespace std;
string::size_type sz;

struct address1
{
    string tag;
    int index;
    string blockAdd;
} ;


address1 calculation(string Hex_Add, int block_size, int sets)
{
    // cout<<Hex_Add<<endl<<block_size<<endl<<sets<<endl;
    // cout<<typeid(Hex_Add).name()<<endl;
    int decimal = stoi(Hex_Add,&sz,16);
    //cout<<"DECIMAL"<<decimal<<endl;
    int block_bits = ceil(log2(block_size));
    //cout<<"BLOCK BITS"<<block_bits<<endl;
    int index_bits = ceil(log2(sets));
    //cout<<"INDEX_BITS"<<index_bits<<endl;
    int tag_bits=32-block_bits-index_bits;
    
    
    long denominator=pow(2, index_bits+block_bits);
    long tag_decimal = decimal / denominator;
    long denom1=pow(2, block_bits);
    long blockAdd  = decimal % denom1;
    blockAdd = decimal - blockAdd;
    int index = (decimal % denominator) / denom1;
    address1 new1;
    std::stringstream sstream;
    sstream << std::hex << tag_decimal;
    new1.tag = sstream.str();
    //cout<<"Tag is"<<new1.tag<<endl;
    std::stringstream sstream2;
    sstream2 << std::hex << blockAdd;
    new1.blockAdd =  sstream2.str();
   
    new1.index =  index;
    //cout<<"Index is"<<new1.index<<endl;
    return(new1);
}

//Parameters used throughout this program
int block_size;
int L1_Size;
int L1_Assoc;
int L2_Size;
int L2_Assoc;
int Replace_Policy;
int Inclusion_Policy;
string tracefile;
long victim;
long v1;
string victim1;

int L1_Read_Count = 0;
int L1_ReadMiss = 0;
int L1_Write_Count = 0;
int L1_WriteBacks=0;
int L1_WriteMiss = 0;
int L2_Read_Count=0;
int L2_ReadMiss=0;
int L2_Write_Count=0;
int L2_WriteMiss=0;
int L2_WriteBacks=0;
int L1_DirectWriteBacks=0;
float L1_MissRate=0;
float L2_MissRate=0;
long L1Sequence=0;
long L2Sequence=0;
int Total_MemoryTraffic=0;
long fptr=0;
int count1;






struct cache
{
    int index;
    string tag;
    string Hex_Add;
    bool valid;
    bool dirty;
    long seq_num;
    string blockAdd;
    int mru;
};

long L1_Sets;
cache** L1;
void L1_Create(int L1_Size,int block_size,int L1_Assoc)
{
    //cout<<"ENTERING L1";
    L1_Sets=(L1_Size/(block_size*L1_Assoc));
    //cout<<"L1 SETS"<<L1_Sets<<endl;
    
    L1=new cache*[L1_Sets];
    for(int i=0;i<L1_Sets;i++)
    {
        L1[i] = new cache[L1_Assoc];

    }
    for (int i=0; i<L1_Sets; i++)
    {
        for (int j=0; j<L1_Assoc; j++)
        {
            L1[i][j].tag="";
            L1[i][j].mru=0;
            
            L1[i][j].Hex_Add="";
            L1[i][j].index=i;
            L1[i][j].valid=0;
            L1[i][j].dirty=0;
            L1[i][j].seq_num=0;
            L1[i][j].blockAdd="";
            
        }

    }
}

long L2_Sets;
cache** L2;
void L2_Create(int L2_Size,int block_size,int L2_Assoc)
{
    //cout<<"ENTERING L2";
    L2_Sets=(L2_Size/(block_size*L2_Assoc));
    //cout<<"L2 Sets"<<L2_Sets<<endl;
    
    L2=new cache*[L2_Sets];
    for(int i=0;i<L2_Sets;i++)
    {
        L2[i] = new cache[L2_Assoc];

    }
    for (int i=0; i<L2_Sets; i++)
    {
        for (int j=0; j<L2_Assoc; j++)
        {
            L2[i][j].tag="";
            L2[i][j].mru=0;
            L2[i][j].Hex_Add="";
            L2[i][j].index=i;
            L2[i][j].valid=0;
            L2[i][j].dirty=0;
            L2[i][j].seq_num=0;
            L2[i][j].blockAdd="";
        }
    }
}


//Declaring global vectors
vector<string> op;
vector<string> addresses;

void BlocktobeReplaced_optimal2(int start,vector<string> addresses,vector<string> &InCache1,int block_size,int sets)
{
    //cout<<"HI"<<endl;
    for(int i=fptr;i<addresses.size();i++)
    {
        //cout<<"in cache size"<<InCache1.size()<<endl<<"addresses size"<<addresses.size()<<endl;

        if(InCache1.size()==1)
        {
            return;
        }
        
        for(int j=0;j<InCache1.size();j++)
        {
            address1 new1 = calculation(addresses[i], block_size, L2_Sets);
            //cout<<"Address in tracefile"<<new1.blockAdd<<endl<<"Address in Cache"<<InCache1[j]<<endl;
        
            if(InCache1[j].compare(new1.blockAdd)==0)
            {
                //cout<<"Match"<<endl;
                InCache1.erase(InCache1.begin()+j);
                //cout<<"After erasing"<<InCache1[0]<<endl;
                //cout<<"In cache size after deleting"<<InCache1.size();
                break;
            }
        }
    }
    return;

}
bool Invalidation(string Hex_Add)
{
    address1 new1 = calculation(Hex_Add, block_size, L1_Sets);
    for(int i=0;i<L1_Assoc;i++)
    {
        if(L1[new1.index][i].blockAdd==new1.blockAdd)
        {
            if(L1[new1.index][i].valid==true)
            {
                L1[new1.index][i].valid=false;
                L1[new1.index][i].tag="";
                if(L1[new1.index][i].dirty==true)
                {
                    L1_DirectWriteBacks++;
                    return(true);
                }
                else
                {
                    return(false);
                }
                


            }
        }
        
    }
    return(false);


}


void ApplyReplace_PolicyL2(string Hex_Add)
{
    //cout<<"WELCOME TO RP"<<endl;
    address1 new1=calculation(Hex_Add,block_size,L2_Sets);
    //cout<<"Inside Apply policy index"<<new1.index<<endl;

    if(Replace_Policy == 0)
    {
        //cout<<"LRU"<<endl;
        
        victim=2147483647;
        
        for (int i=0; i<L2_Assoc; i++)
        {
            //cout<<"Sequence numbers in that matching index are"<<L2[new1.index][i].seq_num<<endl;
            victim = min(victim, L2[new1.index][i].seq_num);
            
        }
        //cout<<"victim seq number"<<victim;
    }
    if(Replace_Policy == 1)
    {
        count1=0;
        for(int i=0;i<L2_Assoc;i++)
        {
            if(L2[new1.index][i].mru==1)
            {
                count1=count1+1;
            }
        }
        if(count1==L2_Assoc)
        {
            v1=-2147483648;
            for (int i=0; i<L2_Assoc; i++)
            {
                v1=max(L2[new1.index][i].seq_num,v1);
            }
            for(int i=0;i<L2_Assoc;i++)
            {
                if(L2[new1.index][i].seq_num != v1)
                {
                    
                    L2[new1.index][i].mru=0;
                    if(L2[new1.index][i].dirty==1)
                    {
                        if(Inclusion_Policy==1)
                        {
                            bool x= Invalidation(L2[new1.index][i].Hex_Add);
                            if(x==false||x==true)
                            {
                                L2_WriteBacks += 1;
                                L2[new1.index][i].valid=false;
                            }
                        }
                        
                        else
                        {
                            L2_WriteBacks += 1;
                            L2[new1.index][i].valid=false;
                        }
                        
                    
                        

                    }
                    else
                    {
                        if(Inclusion_Policy==1)
                        {
                            Invalidation(L2[new1.index][i].Hex_Add);
                            L2[new1.index][i].valid=false;
                        }
                    }
                }
            }
            
            
        }

                

    }
    if(Replace_Policy == 2)
    {
        vector<string> InCache;
        for(int i=0;i<L2_Assoc;i++)
        {
            InCache.push_back(L2[new1.index][i].blockAdd);
        }
        BlocktobeReplaced_optimal2(fptr, addresses, InCache, block_size, L2_Sets);
        string block_toBeReplaced=InCache[0];
        for(int i=0;i<L2_Assoc;i++)
        {
            if(L2[new1.index][i].blockAdd==block_toBeReplaced)
            {
                victim1=L2[new1.index][i].blockAdd;
               
            }
        }
    }
        
}




void InsertintoSlotL2(string Hex_Add,int flag)
{
    //cout<<"WELCOME TO INSERT"<<endl;
    address1 new1=calculation(Hex_Add,block_size,L2_Sets);
    //cout<<"Victim"<<victim<<endl;
    if(Replace_Policy == 0)
    {
        //cout<<"LRU INSERT"<<endl;
        for (int i=0; i<L2_Assoc; i++)
        {
            //cout<<"Victim check again"<<endl;
            //cout<<"Sequence number in cache"<<L2[new1.index][i].seq_num<<endl;
            
            if(L2[new1.index][i].seq_num==victim)
            {
                //cout<<"Sequence matched"<<endl;
                if(L2[new1.index][i].valid==true)
                {
                    if(L2[new1.index][i].dirty==1)
                    {
                        if(Inclusion_Policy==1)
                        {
                            bool x = Invalidation(L2[new1.index][i].Hex_Add);
                            if (x==false||x==true)
                            {
                                
                                L2_WriteBacks += 1;

                            }

                        }
                        else
                        {
                            L2[new1.index][i].valid=false;
                            
                            L2_WriteBacks += 1;
                            //cout<<"L2 Write backs:"<<L2_WriteBacks<<endl;

                        }
                    }
                    else
                    {
                        if(Inclusion_Policy==1)
                        {
                            Invalidation(L2[new1.index][i].Hex_Add);
                        }
                    }
                }
                L2[new1.index][i].valid=true;
                L2[new1.index][i].tag=new1.tag;
                L2[new1.index][i].seq_num=++L2Sequence;
                L2[new1.index][i].Hex_Add=Hex_Add;
                L2[new1.index][i].blockAdd=new1.blockAdd;
                if (flag==1)
                {
                //cout<<"L2 set dirty"<<endl;
                    L2[new1.index][i].dirty = 1;
                }
                else
                { 
                    L2[new1.index][i].dirty = 0;
                }
            break;
            }
        }
    }
    if(Replace_Policy==1)
    {
        for(int i=0;i<L2_Assoc;i++)
        {
            if(L1[new1.index][i].valid==false)
            {
                L2[new1.index][i].tag=new1.tag;
                L2[new1.index][i].index=new1.index;
                L2[new1.index][i].valid=true;
                L2[new1.index][i].blockAdd=new1.blockAdd;
                L2[new1.index][i].Hex_Add=Hex_Add;
                L2[new1.index][i].seq_num=++L2Sequence;
                L2[new1.index][i].mru=1;
                if(flag==1)
                {
                    L2[new1.index][i].dirty=1;
                }
                else
                {
                    L2[new1.index][i].dirty=0;
                    
                }
                break;
                
            }
        }
    }
    if(Replace_Policy==2)
    {
        for (int i=0; i<L2_Assoc; i++)
        {
            //cout<<L1[new1.index][i].blockAdd<<endl;
            if(L2[new1.index][i].blockAdd==victim1)
            {
                if(L2[new1.index][i].valid==true)
                {
                    if(L2[new1.index][i].dirty==1)
                    {
                        if(Inclusion_Policy==1)
                        {
                            bool x = Invalidation(L2[new1.index][i].Hex_Add);
                            if (x==false)
                            {
                                L2_WriteBacks += 1;
                            }

                        }
                        else
                        {
                            L2_WriteBacks += 1;

                        }
                    }
                    else
                    {
                        if(Inclusion_Policy==1)
                        {
                            Invalidation(L2[new1.index][i].Hex_Add);
                        }
                    }
                }
                L2[new1.index][i].valid=true;
                L2[new1.index][i].tag=new1.tag;
                L2[new1.index][i].seq_num=++L2Sequence;
                L2[new1.index][i].Hex_Add=Hex_Add;
                L2[new1.index][i].blockAdd=new1.blockAdd;
                if (flag==1)
                {
                //cout<<"L2 set dirty"<<endl;
                    L2[new1.index][i].dirty = 1;
                }
                else
                { 
                    L2[new1.index][i].dirty = 0;
                }
            break;

            }
        }
    }

    
}



void L2_Write(string Hex_Add,int L2_Assoc,int flag)
{
    if(L2_Size==0 && L2_Assoc==0)
    {
        return;
    }
    //cout<<"ENTERING L2 WRITE"<<endl;
    //cout<<Hex_Add<<endl<<L1_Assoc<<endl;
    address1 new1=calculation(Hex_Add,block_size,L2_Sets);
    // cout<<new1.index<<endl<<new1.tag<<endl;
    if(flag==1)
    {
        L2_Write_Count += 1;
        //cout<<"L1Write count"<<L2_Write_Count;
        for(int i=0;i<L2_Assoc;i++)
        {
            if(L2[new1.index][i].valid==true && L2[new1.index][i].tag==new1.tag)
            {
                //cout<<"L2 Write Hit";

                if(Replace_Policy==0)
                {
                    L2[new1.index][i].seq_num=++L2Sequence;
                }
                if(Replace_Policy==1)
                {
                    L2[new1.index][i].mru=1;
                    L2[new1.index][i].seq_num=++L2Sequence;

                }
                if(Replace_Policy==2)
                {
                    
                    L2[new1.index][i].seq_num=++L2Sequence;
                }
                //cout<<"Check dirt"<<endl;
                L2[new1.index][i].dirty = 1;
                //cout<<"Sequence num after write hit"<<L2[new1.index][i].seq_num<<endl;
                return;
            }
            
        }
        L2_WriteMiss += 1;

    }


    
    
    
    // cout<<"L2 Write miss"<<L2_WriteMiss<<endl;
    //else
    for(int i=0;i<L2_Assoc;i++)
    {
        //check for invalid slots, if available, place the tag in that slot
        if(L2[new1.index][i].valid==false)
        {
            //cout<<"Invalid block found"<<endl;
            L2[new1.index][i].tag=new1.tag;
            // cout<<new1.tag<<endl;
            L2[new1.index][i].index=new1.index;
            L2[new1.index][i].blockAdd=new1.blockAdd;
            L2[new1.index][i].valid=true;
            L2[new1.index][i].Hex_Add=Hex_Add;
            if(flag==1)
            {
                L2[new1.index][i].dirty=true;

            }
            else
            {
                L2[new1.index][i].dirty=false;
            }
            
            
            //cout<<"Dirty bit for invalid slot entry is"<<L2[new1.index][i].dirty<<endl;
            if(Replace_Policy==0)
            {
                //cout<<"Starting seq num"<<L2Sequence<<endl;
                
                L2[new1.index][i].seq_num=++L2Sequence;
                //cout<<"new seq"<<L2[new1.index][i].seq_num<<endl;
            }
            if(Replace_Policy==1)
            {
                L2[new1.index][i].mru=1;
                L2[new1.index][i].seq_num=++L2Sequence;
                    
            }
            if(Replace_Policy==2)
            {
                
                L2[new1.index][i].seq_num=++L2Sequence;
            }
            //cout<<"Sequence num while write"<<L2new1.index][i].seq_num<<endl;
            return;
        }
    }

    ApplyReplace_PolicyL2(Hex_Add);
    InsertintoSlotL2(Hex_Add,flag);

}                        


void BlocktobeReplaced_optimal(int start,vector<string> addresses,vector<string> &InCache1,int block_size,int sets)
{
    //cout<<"HI"<<endl;
    for(int i=fptr;i<addresses.size();i++)
    {
        //cout<<"in cache size"<<InCache1.size()<<endl<<"addresses size"<<addresses.size()<<endl;

        if(InCache1.size()==1)
        {
            return;
        }
        
        for(int j=0;j<InCache1.size();j++)
        {
            address1 new1 = calculation(addresses[i], block_size, L1_Sets);
            //cout<<"Address in tracefile"<<new1.blockAdd<<endl<<"Address in Cache"<<InCache1[j]<<endl;
        
            if(InCache1[j].compare(new1.blockAdd)==0)
            {
                //cout<<"Match"<<endl;
                InCache1.erase(InCache1.begin()+j);
                //cout<<"After erasing"<<InCache1[0]<<endl;
                //cout<<"In cache size after deleting"<<InCache1.size();
                break;
            }
        }
    }
    return;

}

void L2_Read(string Hex_Add, int L2_Assoc)
{
    if(L2_Size==0 && L2_Assoc==0)
    {
        return;
    }
    //Call "calculation" function to obtain the tag and index from the hexadecimal address
    //cout<<"ENTERING L2 READ"<<endl;
    //cout<<Hex_Add<<endl<<L2_Sets<<endl<<L2_Assoc<<endl;
    address1 new1=calculation(Hex_Add,block_size,L2_Sets);
    //cout<<"Tag calculated from address"<<new1.tag<<endl;

    //L2_Read_Count += 1;
    //cout<<L2_Read_Count;
    for(int i=0;i<L2_Assoc;i++)
    {
        //cout<<"Tags present in cache"<<L2[new1.index][i].tag<<endl;
        if(L2[new1.index][i].tag==new1.tag && L2[new1.index][i].valid==true)
        {
            //cout<<"Read Hit"<<endl;
            //update the LRU
            if(Replace_Policy==0)
            {
                L2[new1.index][i].seq_num=++L2Sequence;
            }
            if(Replace_Policy==1)
            {
                L2[new1.index][i].seq_num=++L2Sequence;
                L2[new1.index][i].mru=1;

            }
            if(Replace_Policy==2)
            {
                
                L2[new1.index][i].seq_num=++L2Sequence;
            }
            //cout<<"Seq_num after read hit"<<L1[new1.index][i].seq_num<<endl;
            return;
        }
        
        
    }
    L2_ReadMiss += 1;
    //cout<<" L2 Read miss, so write";
    L2_Write(Hex_Add,L2_Assoc,0);
}



void ApplyReplace_Policy(string Hex_Add)
{
    //cout<<"WELCOME TO RP"<<endl;
    address1 new1=calculation(Hex_Add,block_size,L1_Sets);
    //cout<<"Inside Apply policy index"<<new1.index<<endl;

    if(Replace_Policy == 0)
    {
        //cout<<"LRU"<<endl;
        
        victim=2147483647;
        
        for (int i=0; i<L1_Assoc; i++)
        {
            //cout<<"Sequence numbers in that matching index are"<<L1[new1.index][i].seq_num<<endl;
            victim = min(victim, L1[new1.index][i].seq_num);
            
        }
        //cout<<"victim seq number"<<victim;
    }
    if(Replace_Policy == 1)
    {
        count1=0;
        for(int i=0;i<L1_Assoc;i++)
        {
            //cout<<"MRUs"<<L1[new1.index][i].mru<<endl;
            if(L1[new1.index][i].mru==1)
            {
                count1 += 1;
            }
        }
        //cout<<"Count is"<<count1<<endl;
        if(count1==L1_Assoc)
        {
            v1=-2147483648;
            for (int i=0; i<L1_Assoc; i++)
            {
                v1=max(L1[new1.index][i].seq_num,v1);
            }
            //cout<<"V1 "<<v1<<endl;
            for(int i=0;i<L1_Assoc;i++)
            {
                if(L1[new1.index][i].seq_num != v1)
                {
                    L1[new1.index][i].mru=0;
                    if(L1[new1.index][i].dirty==1)
                    {
                        L1_WriteBacks += 1;
                        //cout<<"L1_Writebacks"<<L1_WriteBacks<<endl;
                        L2_Write(L1[new1.index][i].Hex_Add,L2_Assoc,1);
                        L1[new1.index][i].valid=false;

                    }
                    else
                    {
                        L1[new1.index][i].valid=false;
                    }
                }
            }
                    
        }
                

    }
    if(Replace_Policy == 2)
    {
        vector<string> InCache;
        for(int i=0;i<L1_Assoc;i++)
        {
            InCache.push_back(L1[new1.index][i].blockAdd);
        }
        //cout<<"Block Address"<<InCache[0]<<endl<<InCache[1]<<endl;
        //cout<<"current file pointer"<<fptr<<endl;
        BlocktobeReplaced_optimal(fptr, addresses, InCache, block_size, L1_Sets);
        //cout<<"IN CACHE[0]"<<InCache[0]<<endl;
        string block_toBeReplaced=InCache[0];
        //cout<<"Block address to be replaced"<<InCache[0]<<endl;

        for(int i=0;i<L1_Assoc;i++)
        {
            if(L1[new1.index][i].blockAdd==block_toBeReplaced)
            {
                victim1=L1[new1.index][i].blockAdd;
                //cout<<"victim1 "<<victim1<<endl;
               
            }
        }
    }
        
}
void InsertintoSlot(string Hex_Add,int flag)
{
    //cout<<"WELCOME TO INSERT"<<endl;
    address1 new1=calculation(Hex_Add,block_size,L1_Sets);
    //cout<<"Victim"<<victim<<endl;
    if(Replace_Policy == 0)
    {
        //cout<<"LRU INSERT"<<endl;
        for (int i=0; i<L1_Assoc; i++)
        {
            //cout<<"Victim check again"<<endl;
            //cout<<"Sequence number in cache"<<L1[new1.index][i].seq_num<<endl;
            
            if(L1[new1.index][i].seq_num==victim)
            {
                //cout<<"Sequence matched"<<endl;
                if(L1[new1.index][i].dirty==1)
                {
                    L1[new1.index][i].valid=false;
                    L1_WriteBacks += 1;
                    // cout<<"L1 HEX ADDRESS"<<L1[new1.index][i].Hex_Add<<endl;

                    L2_Write(L1[new1.index][i].Hex_Add,L2_Assoc,1);
                }
                else
                {
                    L1[new1.index][i].valid=false;
                }
                //Issuing a read request to next level of memory
                //cout<<"oNLY HEX ADDRESS"<<Hex_Add<<endl;
                L2_Read(Hex_Add,L2_Assoc);
                L1[new1.index][i].valid=true;
                L1[new1.index][i].tag=new1.tag;
                L1[new1.index][i].seq_num=++L1Sequence;
                L1[new1.index][i].blockAdd=new1.blockAdd;
                L1[new1.index][i].Hex_Add=Hex_Add;
                
                L1[new1.index][i].index=new1.index;
                //cout<<"Tag Present"<<L1[new1.index][i].tag<<endl;
                if(flag==1)
                {
                    L1[new1.index][i].dirty=true;
                
                }
                else
                {
                    L1[new1.index][i].dirty=false;
                }
                fptr++;

                

            }
            //cout<<"file pointer after inserting element"<<fptr<<endl;
        }
    }
    if(Replace_Policy==1)
    {
        for(int i=0;i<L1_Assoc;i++)
        {
            if(L1[new1.index][i].valid==false)
            {
                L2_Read(Hex_Add,L2_Assoc);
                L1[new1.index][i].tag=new1.tag;
                //cout<<new1.tag<<endl;
                L1[new1.index][i].valid=true;
                L1[new1.index][i].seq_num=++L1Sequence;
                L1[new1.index][i].blockAdd=new1.blockAdd;
                L1[new1.index][i].Hex_Add=Hex_Add;
                L1[new1.index][i].index=new1.index;
                L1[new1.index][i].mru=1;
                if(flag==1)
                {
                    L1[new1.index][i].dirty=true;
                    //cout<<"D"<<endl;

                }
                else
                {
                    L1[new1.index][i].dirty=false;
                }
            


            }
        }

    }
    if(Replace_Policy==2)
    {
        for (int i=0; i<L1_Assoc; i++)
        {
            //cout<<L1[new1.index][i].blockAdd<<endl;
            if(L1[new1.index][i].blockAdd==victim1)
            {
                if(L1[new1.index][i].dirty==1)
                {
                    L1[new1.index][i].valid=false;
                    L1_WriteBacks += 1;
                    L2_Write(L1[new1.index][i].Hex_Add,L2_Assoc,1);
                }
                else
                {
                    L1[new1.index][i].valid=false;
                }
                //Issuing a read request to next level of memory
                L2_Read(Hex_Add,L2_Assoc);
                L1[new1.index][i].valid=true;
                L1[new1.index][i].tag=new1.tag;
                L1[new1.index][i].seq_num=++L1Sequence;
                L1[new1.index][i].blockAdd=new1.blockAdd;
                //cout<<"Block add"<<L1[new1.index][i].blockAdd<<endl;
                
                L1[new1.index][i].index=new1.index;
                //cout<<"Tag Present"<<L1[new1.index][i].tag<<endl;
                if(flag==1)
                {
                    L1[new1.index][i].dirty=true;
                
                }
                else
                {
                    L1[new1.index][i].dirty=false;
                }
                fptr++;
            }
        }
        
        

    }

    
}
void L1_Write(string Hex_Add,int L1_Assoc,int flag)
{
    //cout<<"ENTERING L1 WRITE"<<endl;
    //cout<<Hex_Add<<endl<<L1_Assoc<<endl;
    address1 new1=calculation(Hex_Add,block_size,L1_Sets);
    // cout<<new1.index<<endl<<new1.tag<<endl;
    if(flag==1)
    {
        
        //cout<<"L1Write count"<<L1_Write_Count;
        for(int i=0;i<L1_Assoc;i++)
        {
            if(L1[new1.index][i].valid==true && L1[new1.index][i].tag==new1.tag)
            {
                //cout<<"L1 Write Hit";

                if(Replace_Policy==0)
                {
                    L1[new1.index][i].seq_num=++L1Sequence;
                }
                if(Replace_Policy==1)
                {
                    L1[new1.index][i].mru=1;
                    L1[new1.index][i].seq_num=++L1Sequence;

    
                }
                if(Replace_Policy==2)
                {
                    fptr++;
                    //cout<<"file ptr after write hit"<<fptr<<endl;
                    L1[new1.index][i].seq_num=++L1Sequence;
                }
                //cout<<"Check dirt"<<endl;
                L1[new1.index][i].dirty = 1;
                //cout<<"Sequence num after write hit"<<L1[new1.index][i].seq_num<<endl;
                return;
            }
            
        }
        L1_WriteMiss++;
        if(L2_Size!=0)
        {
            L2_Read_Count += 1;

        }
        
        //L2_Read(Hex_Add,L2_Assoc);

    }

    
    
    
    //cout<<"L1 Write miss"<<L1_WriteMiss<<endl;
    //else
    //cout<<"Hi"<<endl;
    for(int i=0;i<L1_Assoc;i++)
    {
        //check for invalid slots, if available, place the tag in that slot
        if(L1[new1.index][i].valid==false)
        {
            //cout<<"Invalid block found"<<endl;
            L2_Read(Hex_Add,L2_Assoc);
            L1[new1.index][i].tag=new1.tag;
            //cout<<new1.tag<<endl;
            L1[new1.index][i].valid=true;
            L1[new1.index][i].index=new1.index;
            L1[new1.index][i].blockAdd=new1.blockAdd;
            L1[new1.index][i].Hex_Add=Hex_Add;
            if(flag==1)
            {
                L1[new1.index][i].dirty=true;
                //cout<<"D"<<endl;

            }
            else
            {
                L1[new1.index][i].dirty=false;
            }
            
            
            //cout<<"Dirty bit for invalid slot entry is"<<L1[new1.index][i].dirty<<endl;
            if(Replace_Policy==0)
            {
                //cout<<"Starting seq num"<<L1Sequence<<endl;
                
                L1[new1.index][i].seq_num=++L1Sequence;
                //cout<<"new seq"<<L1[new1.index][i].seq_num<<endl;
            }
            if(Replace_Policy==1)
            {
                L1[new1.index][i].mru=1;
                //cout<<L1[new1.index][i].mru<<"MRU"<<endl;
                L1[new1.index][i].seq_num=++L1Sequence;
                //cout<<"L1 SEQUENCE IS"<<L1[new1.index][i].seq_num<<endl;
                //cout<<L1[new1.index][i].mru<<endl;   
                
            }
        
            if(Replace_Policy==2)
            {
                fptr++;
                L1[new1.index][i].seq_num=++L1Sequence;
            }
            //cout<<"Sequence num while write"<<L1[new1.index][i].seq_num<<endl;
            //cout<<"file ptr after writing in invalid slot"<<fptr<<endl;
            //cout<<"Hola";
            return;
        }
    }
        
    //cout<<Hex_Add<<endl;
    //cout<<new1.index<<endl;   
    ApplyReplace_Policy(Hex_Add);
    InsertintoSlot(Hex_Add,flag);

        
        

    
}


void L1_Read(string Hex_Add, int L1_Assoc)
{
    //Call "calculation" function to obtain the tag and index from the hexadecimal address
    //cout<<"ENTERING L1 READ"<<endl;
    //cout<<"read count"<<L1_Read_Count<<endl;
    address1 new1=calculation(Hex_Add,block_size,L1_Sets);
    //cout<<"Tag calculated from address"<<new1.tag<<endl;
    L1_Read_Count += 1;
    
    for(int i=0;i<L1_Assoc;i++)
    {
        //cout<<"Tags present in cache"<<L1[new1.index][i].tag<<endl;
        if(L1[new1.index][i].tag==new1.tag && L1[new1.index][i].valid==true)
        {
            //cout<<"Read Hit"<<endl;
            //update the LRU
            if(Replace_Policy==0)
            {
                L1[new1.index][i].seq_num=++L1Sequence;
            }
            if(Replace_Policy==1)
            {
                L1[new1.index][i].mru=1;
                L1[new1.index][i].seq_num=++L1Sequence;

            }
            if(Replace_Policy==2)
            {
                fptr++;
                L1[new1.index][i].seq_num=++L1Sequence;
            }
            //cout<<"Seq_num after read hit"<<L1[new1.index][i].seq_num<<endl;
            //cout<<"File ptr after read hit"<<fptr<<endl;

            return;
        }
        
        
    }
    //cout<<"Read miss before"<<L1_ReadMiss<<endl;
    L1_ReadMiss=L1_ReadMiss+1;
    //cout<<"Read miss after"<<L1_ReadMiss<<endl;
    //cout<<"Read miss, so write";
    if(L2_Size!=0)
        L2_Read_Count += 1;
    L1_Write(Hex_Add,L1_Assoc,0);
    L2_Read(Hex_Add,L2_Assoc);
    
    
}


void displayL1()
{
    cout<<"===== L1 contents ====="<<endl;
    for (int i=0; i<L1_Sets; i++){
        cout<<"Set"<<"\t"<<i<<":"<<"\t";
        for (int j=0; j<L1_Assoc; j++){
            cout<<L1[i][j].tag;
            if (L1[i][j].dirty == 1){
                cout<<" "<<"D";
            }
            cout<<"    ";
        }
        cout<<endl;
    }
}

void displayL2()
{
    if(L2_Size==0 and L2_Assoc==0)
    {
        return;
    }
    cout<<"===== L2 contents ====="<<endl;
    for (int i=0; i<L2_Sets; i++){
        cout<<"Set"<<"\t"<<i<<":"<<"\t";
        for (int j=0; j<L2_Assoc; j++){
            cout<<L2[i][j].tag;
            if (L2[i][j].dirty == 1){
                cout<<" "<<"D";
            }
            cout<<"    ";
        }
        cout<<endl;
    }
}

// void print(vector<string> &a) {
//    std::cout << "The vector elements are : ";

//    for(int i=0; i < a.size(); i++)
//    std::cout << a.at(i) << ' ';
// }

void calculateMissRate()
{
    L1_MissRate = float(L1_ReadMiss + L1_WriteMiss) / float(L1_Read_Count + L1_Write_Count);
    if(L2_Size>0)
    {
        L2_MissRate = float(L2_ReadMiss)/float(L2_Read_Count);
    }
}

void calculateMemoryTraffic()
{
    if (L2_Size==0){
        Total_MemoryTraffic = L1_ReadMiss + L1_WriteMiss + L1_WriteBacks;
    }

    if (L2_Size>0 && Inclusion_Policy==0){
        Total_MemoryTraffic = L2_ReadMiss + L2_WriteMiss + L2_WriteBacks;
    }

    if (L2_Size>0 && Inclusion_Policy==1){
        Total_MemoryTraffic = L2_ReadMiss + L2_WriteMiss + L2_WriteBacks + L1_DirectWriteBacks;
    }
}



int main(int argc, char* argv[])
{
    if(argc<8)
    {
        cout<<"Insufficient number of inputs";
        return 0;
    }

    block_size=atoi(argv[1]);
    L1_Size=atoi(argv[2]);
    L1_Assoc=atoi(argv[3]);
    L2_Size=atoi(argv[4]);
    L2_Assoc=atoi(argv[5]);
    Replace_Policy=atoi(argv[6]);
    Inclusion_Policy=atoi(argv[7]);
    tracefile=argv[8];

    //Initial configuration
    cout<<"===== Simulator configuration ====="<<"\n";
    cout<<"BLOCKSIZE:        	  "<<block_size<<"\n";
    cout<<"L1_SIZE:               "<<L1_Size<<"\n";
    cout<<"L1_ASSOC:              "<<L1_Assoc<<"\n";
    cout<<"L2_SIZE:               "<<L2_Size<<"\n";
    cout<<"L2_ASSOC:              "<<L2_Assoc<<"\n";
    
    if (Replace_Policy==0){
        cout<<"REPLACEMENT POLICY:    "<<"LRU"<<"\n";
    }
    else if (Replace_Policy==1){
        cout<<"REPLACEMENT POLICY:    "<<"Pseudo-LRU"<<"\n";
    }
    else if (Replace_Policy==2){
        cout<<"REPLACEMENT POLICY:    "<<"Optimal"<<"\n";
    
    }
    if (Inclusion_Policy==0){
        cout<<"INCLUSION PROPERTY:    "<<"non-inclusive"<<"\n";
    }
    else if (Inclusion_Policy==1){
        cout<<"INCLUSION PROPERTY:    "<<"inclusive"<<"\n";
    }
    cout<<"trace_file:            "<<tracefile<<"\n";

    
    

    L1_Create(L1_Size,block_size,L1_Assoc);
    if(L2_Size>0)
    {
        L2_Create(L2_Size,block_size,L2_Assoc);
    
    }
    
    // void Readfile()

    // {
        string ops;
        string Hex_Add;
        ifstream myfile(tracefile);
        if (myfile.is_open())
        {
            while (!myfile.eof())
            {
                myfile>>ops;
                //cout<<ops;
                op.push_back(ops);
                myfile>>Hex_Add;
                addresses.push_back(Hex_Add);
            }
            myfile.close();
        }
    // }
    //cout<<op.size()<<endl;
    //cout<<addresses.size()<<endl;

    //cout<<op[0]<<endl;
    //cout<<op[1]<<endl;
    //cout<<op[0].length()<<endl;
    for(int i=0;i<op[0].length();i++)
    {
        if(op[0][i]=='\xEF'||'\xBB'||'\xBF')
        {
            op[0].erase(i,3);
            //cout<<op[0]<<endl;
            break;
            
        }
    }


    



    //cout<<op.front()<<endl;
    //cout<<op.back()<<endl;

    



    
    for(int i=0;i<op.size();i++)
    {
        //cout<<"Hello"<<endl;
        
        //cout<<addresses[0]<<endl;

        if(op[i].compare("r")==0)
        {
            //L1_Read_Count=L1_Read_Count+1;
            L1_Read(addresses[i], L1_Assoc);
        }
        if(op[i].compare("w")==0)
        {
            //cout<<"write"<<endl;
            L1_Write_Count++;
            L1_Write(addresses[i],L1_Assoc,1);
        }
    }

    displayL1();
    displayL2();
    
    calculateMissRate();
    calculateMemoryTraffic();
    





    cout<<"===== Simulation results (raw) ====="<<endl;
    cout<<"a. "<<"number of L1 reads:             "<<L1_Read_Count<<endl;
    cout<<"b. "<<"number of L1 read misses:        "<<L1_ReadMiss<<endl;
    cout<<"c. "<<"number of L1 writes:             "<<L1_Write_Count<<endl;
    cout<<"d. "<<"number of L1 write misses:      "<<L1_WriteMiss<<endl;
    cout<<"e. "<<"L1 miss rate:                    "<<fixed<<setprecision(6)<<L1_MissRate<<endl;
    cout<<"f. "<<"number of L1 writebacks:         "<<L1_WriteBacks<<endl;
    cout<<"g. "<<"number of L2 reads:              "<<L2_Read_Count<<endl;
    cout<<"h. "<<"number of L2 read misses:        "<<L2_ReadMiss<<endl;
    cout<<"i. "<<"number of L2 writes:             "<<L2_Write_Count<<endl;
    cout<<"j. "<<"number of L2 write misses:       "<<L2_WriteMiss<<endl;
    if (L2_Size>0){
        cout<<"k. "<<"L2 miss rate:                    "<<fixed<<setprecision(6)<<L2_MissRate<<endl;
    }
    else{
        cout<<"k. "<<"L2 miss rate:                    "<<int(L2_MissRate)<<endl;
    }
    
    cout<<"l. "<<"number of L2 writebacks:         "<<L2_WriteBacks<<endl;
    cout<<"m. "<<"total memory traffic:            "<<Total_MemoryTraffic<<endl;



    
    return 0;
}


