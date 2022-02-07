#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define SIZE_OF_TLB 32
#define SIZE_OF_DISK 10000000
int TLB_HIT[20];    //the time of a process hit TLB
int TLB_LOOKUP[20]; //the time of a process look up TLB
int PAGE_FAULT[20]; //the time of a process paged fault
int REFERENCE[20];  //the time of a process been referenced
int TLB_POLICY;     //0 for Random, 1 for LRU
int PAGE_POLICY;    //0 for FIFO, 1 for Clock
int FRAME_POLICY;   //0 for Global, 1 for Local
int NUM_OF_PROCESS;
int NUM_OF_VIRTUAL_PAGE;
int NUM_OF_PHYSICAL_PAGE;
char RUNNING;
int CLOCK_HAND_COUNT = 0;
FILE *output_file = NULL;
FILE *test = NULL;

typedef struct node
{
    int index;     
    int logical_addr;
    int frame;    
    int disk_addr;
    char process;
    int p_bit;// 1: in physical memory, 0: in disk
    int r_bit;
    struct node *next;
} node;

int Disk[SIZE_OF_DISK][3];//0: in use or not 1: process id 2: VPN
node *TLB = NULL;       //point to first element(LRU) in TLB
node *PageTable[20];
node *localReplacementList[20];
node *globalReplacementList;
node *PhysicalMemory = NULL;
node *free_frame_list = NULL;
node *localClock[20];
node *globalclk = NULL;
node *pclk = NULL;

node *newnode()
{
    node *n;
    n = (node *) malloc(sizeof(node));
    n->disk_addr = -1;
    n->logical_addr = -1;
    n->frame = -1;
    n->p_bit = -1;
    n->r_bit = 0;
    n->next = NULL;
    return n;
}

node *reIndex(node *list, int m)  // 0 for page table , 1 for physical memory, 2 for TLB
{
    node *p = list;
    int index = 0;
    
    if(m==0){
        while(p)
        {
            p->logical_addr = index;
            index++;
            p = p->next;
        }
    }
     
    if(m==1){
        while(p)
        {
            p->frame = index;
            index++;
            p = p->next;
        }
    }
       
    if(m==2){
        while(p)
        {
            p->index = index;
            index++;
            p = p->next;
        }
    }
    
    return list;
}

node *enq(node *list, node *temp)
{
    if(PAGE_POLICY!=1)  temp->next = NULL;
    if(list==NULL)
    {
        list = temp;
        return list;
    }
    else
    {
        node *p = list;
        while(p->next)
        {
            p = p->next;
        }
        p->next = temp;
        return list;
    }
    return list;
}

node *deq(node *list, int VPN, char process)
{
    node *p=list;
    node *p_next=list->next;
    if(p->logical_addr==VPN&&p->process==process)   //deq from head
    {
        list = p_next;
        free(p);
        return list;
    }
    else
    {
        while(p_next!=NULL)
        {
            if(p_next->logical_addr==VPN&&p_next->process==process)
                break;
            p = p_next;
            p_next = p_next->next;
        }
        if(p_next->next == NULL)//沒有找到
        {
            p->next = NULL;
            free(p_next);
        }
        else
        {
            p->next = p_next->next;
            p_next = NULL;
        }
    }
    return list;
}

node *freeTLB(char process)
{
    node *p = TLB;
    while(p!=NULL)
    {
        p->logical_addr = -1;
        p->frame = -1;
        p->process = process;
        p = p->next;
    }
    return TLB;
}

node *search(node *list, int VPN, char process) //return page frame number or -1
{
    if(list==NULL)  return NULL;
    node *t = list;

    while(t!=NULL)
    {
        if(t->logical_addr==VPN && t->process==process)
            break;
        else
            t = t->next;
    }
    
    return t;
}

void nodeCopy(node **src, node **dest)
{
    (*dest)->p_bit = (*src)->p_bit;
    (*dest)->r_bit = (*src)->r_bit;
    (*dest)->logical_addr = (*src)->logical_addr;
    (*dest)->frame = (*src)->frame;
    (*dest)->disk_addr = (*src)->disk_addr;
    (*dest)->process = (*src)->process;
}

void fillPhysicalMemory(int PFN, int VPN, char process)
{
    node *p = PhysicalMemory;
    while(p)
    {
        if(p->frame == PFN)
        {
            p->logical_addr = VPN;
            p->process = process;
            p->r_bit = 1;
            break;
        }
        else
        {
            p = p->next;
        }
    }
}

int getid(char process)
{
    switch(process)
    {
    case 'A':
        return 0;
    case 'B':
        return 1;
    case 'C':
        return 2;
    case 'D':
        return 3;
    case 'E':
        return 4;
    case 'F':
        return 5;
    case 'G':
        return 6;
    case 'H':
        return 7;
    case 'I':
        return 8;
    case 'J':
        return 9;
    case 'K':
        return 10;
    case 'L':
        return 11;
    case 'M':
        return 12;
    case 'N':
        return 13;
    case 'O':
        return 14;
    case 'P':
        return 15;
    case 'Q':
        return 16;
    case 'R':
        return 17;
    case 'S':
        return 18;
    case 'T':
        return 19;
    }
    return -1;
}

char getprocess(int i)
{
    switch(i)
    {
    case 0:
        return 'A';
        break;
    case 1:
        return 'B';
        break;
    case 2:
        return 'C';
        break;
    case 3:
        return 'D';
        break;
    case 4:
        return 'E';
        break;
    case 5:
        return 'F';
        break;
    case 6:
        return 'G';
        break;
    case 7:
        return 'H';
        break;
    case 8:
        return 'I';
        break;
    case 9:
        return 'J';
        break;
    case 10:
        return 'K';
        break;
    case 11:
        return 'L';
        break;
    case 12:
        return 'M';
        break;
    case 13:
        return 'N';
        break;
    case 14:
        return 'O';
        break;
    case 15:
        return 'P';
        break;
    case 16:
        return 'Q';
        break;
    case 17:
        return 'R';
        break;
    case 18:
        return 'S';
        break;
    case 19:
        return 'T';
        break;
    default:
        return 'Z';
    }
}

void memoryHandler(char process, int VPN)
{
    int process_id = getid(process);
    node *pageLocation = search(PageTable[process_id], VPN, process);
    int diskLocation = 0;
    int PFN;
    int evictedVPN = -1;
    char evictedProcess = 'Z';
    node *replacement_list_ptr = newnode();

    //change running process to current process
    if(RUNNING != process)
    {
        TLB = freeTLB(process);
        RUNNING = process;
    }

    if(search(TLB, VPN, process)!= NULL) 
    {
        //TLB hit
        TLB_HIT[process_id]++;  //increment hit time
        TLB_LOOKUP[process_id]++;   //increment look up time
        node *tmp = search(TLB, VPN, process);
        node *tmpcopy = newnode();
        nodeCopy(&tmp, &tmpcopy);

        if(TLB_POLICY==1) //LRU
        {
            TLB = deq(TLB, VPN, process);
            TLB = enq(TLB, tmpcopy);
        }
        if(PAGE_POLICY==1) //clock algo
        {
            if(FRAME_POLICY==0)
            {
                //search in global frame list and set reference bit to 1
                replacement_list_ptr = search(globalReplacementList, VPN, process);
                replacement_list_ptr->r_bit=1;
            }
            else
            {
                //search in local frame list
                replacement_list_ptr = search(localReplacementList[process_id], VPN, process);
                replacement_list_ptr->r_bit=1;
            }
        }
        fprintf(output_file, "Process %c, TLB Hit, %d=>%d\n", process, VPN, tmp->frame);
        return;
    }

    else //TLB miss
    {
        TLB_LOOKUP[process_id]++;
        if (search(PageTable[process_id], VPN, process)->frame != -1)
        {
            //page hit
            PFN = search(PageTable[process_id], VPN, process)->frame;
            if(PAGE_POLICY==1) //clock algo 
            {
                if(FRAME_POLICY==0)
                {
                    replacement_list_ptr = search(globalReplacementList, VPN, process);
                    replacement_list_ptr->r_bit=1;
                }
                else
                {
                    replacement_list_ptr = search(localReplacementList[process_id], VPN, process);
                    replacement_list_ptr->r_bit=1;
                }
            }

            fprintf(output_file, "Process %c, TLB Miss, Page Hit, %d=>%d\n", process, VPN, PFN);
        }
        else //page fault
        {
            PAGE_FAULT[process_id]++;
            if(free_frame_list!= NULL) 
            {
                //get the first frame in free frame list and use the frame
                PFN = free_frame_list->frame;
                pageLocation->frame = free_frame_list->frame;
                pageLocation->r_bit = 1;
                pageLocation->process = process;
                pageLocation->p_bit = 1;
                pageLocation->logical_addr = VPN;
                node *copy = newnode();
                nodeCopy(&pageLocation, &copy);
                copy->frame = PFN;
                if(PAGE_POLICY==1)
                {
                    if(FRAME_POLICY==0)
                    {
                        if(PFN==(NUM_OF_PHYSICAL_PAGE-1))
                        {
                            copy->next = globalReplacementList;
                        }
                        else
                            copy->next = NULL;
                    }
                    else
                    {
                        if(PFN==NUM_OF_PHYSICAL_PAGE-1)
                            copy->next = localReplacementList[process_id];
                        else
                            copy->next = NULL;
                    }
                }
                else
                {
                    copy->next = NULL;
                }
                if(FRAME_POLICY==0)
                {
                    globalReplacementList = enq(globalReplacementList, copy);
                }
                else
                {
                    localReplacementList[process_id] = enq(localReplacementList[process_id], copy);
                }
                node *t=free_frame_list;
                free_frame_list = free_frame_list->next;
                free(t);
                fillPhysicalMemory(PFN, VPN, process);
                fprintf(output_file, "Process %c, TLB Miss, Page Fault, %d, Evict -1 of Process %c to -1, %d<<%d\n", process, pageLocation->frame, process, VPN, -1);
            }
            else //no free frame, select a victim
            {
                node *new_ptr = NULL;
                node *clk_ptr = NULL;
                if(FRAME_POLICY==0) //global
                {
                    new_ptr = globalReplacementList;
                    clk_ptr = globalReplacementList;
                }
                else    //local
                {
                    new_ptr = localReplacementList[process_id];
                    clk_ptr = localReplacementList[process_id];
                }

                if(PAGE_POLICY==1) //clock
                {
                    if(FRAME_POLICY==0) //global
                    {
                        if(globalclk!=NULL)
                        {
                            clk_ptr=globalclk;
                        }
                        while(1)
                        {
                            //check reference bit
                            if(clk_ptr->r_bit==1)
                            {
                                clk_ptr->r_bit = 0;
                                if(clk_ptr->next==NULL)
                                {
                                    clk_ptr = globalReplacementList;
                                }
                                else
                                {
                                    clk_ptr = clk_ptr->next;    //go to next page
                                }
                            }
                            else if(clk_ptr->r_bit==0)  //page out
                            {
                                globalclk = clk_ptr->next;
                                pclk = clk_ptr;
                                break;
                            }
                        }
                    }
                    else    //local
                    {
                        if(localClock[process_id]!=NULL)
                        {
                            clk_ptr=localClock[process_id];
                        }
                        while(1)    //check r_bit
                        {
                            if(clk_ptr->r_bit==1)
                            {
                                clk_ptr->r_bit = 0;
                                if(clk_ptr->next==NULL)
                                {
                                    clk_ptr = localReplacementList[process_id];
                                }
                                else
                                {
                                    clk_ptr = clk_ptr->next;
                                }
                            }
                            else if(clk_ptr->r_bit==0)
                            {
                                localClock[process_id] = clk_ptr->next;
                                pclk = clk_ptr;
                                break;
                            }
                        }
                    }
                }
                if(PAGE_POLICY==0)  //FIFO
                {
                    PFN = new_ptr->frame;
                    pageLocation->frame = new_ptr->frame;
                    pageLocation->r_bit = 1;
                    pageLocation->process = process;
                    pageLocation->p_bit = 1;
                }
                
                node *copy = newnode();
                nodeCopy(&pageLocation, &copy);
                if(PAGE_POLICY==0)
                {
                    if(FRAME_POLICY==0)
                    {
                        globalReplacementList = deq(globalReplacementList,new_ptr->logical_addr,  new_ptr->process);
                        globalReplacementList = enq(globalReplacementList, copy);
                    }
                    else
                    {
                        localReplacementList[process_id] = deq (localReplacementList[process_id], new_ptr->logical_addr, new_ptr->process);
                        localReplacementList[process_id] = enq(localReplacementList[process_id], copy);
                    }
                }
                else
                {
                    PFN = pclk->frame;
                    evictedProcess = pclk->process;
                    evictedVPN = pclk->logical_addr;
                    pclk->logical_addr = VPN;
                    pclk->process = process;
                    pclk->r_bit = 1;
                }
                if(PAGE_POLICY==0)
                {
                    //swap out original page to disk
                    search(PageTable[getid(new_ptr->process)], new_ptr->logical_addr, new_ptr->process)->frame = -1;
                    //write to physical memory
                    fillPhysicalMemory(PFN, VPN, process);
                    for (int i = 0; i < SIZE_OF_DISK; i++) //write victim to disk
                    {
                        if(Disk[i][0]==-1)
                        {
                            diskLocation = i;
                            Disk[i][0] = 1;
                            Disk[i][1] = getid(new_ptr->process);
                            Disk[i][2] = new_ptr->logical_addr;
                            break;
                        }
                    }
                    //檢查被evict的page有沒有在tlb
                    node *tlb_check = search(TLB, new_ptr->logical_addr, new_ptr->process);
                    if(tlb_check!=NULL)
                    {
                        tlb_check->logical_addr = -1;
                        tlb_check->frame = -1;
                    }

                    int originalDisk = -1;
                    for (int i = 0; i < SIZE_OF_DISK; i++)
                    {
                        //檢查本來有沒有在disk裡
                        if (Disk[i][1] == process_id && Disk[i][2] == VPN)
                        {
                            Disk[i][0] = -1; // 釋出新的位置
                            Disk[i][1] = -1; // 沒有被程序佔用
                            Disk[i][2] = -1; // 清空VPN
                            originalDisk = i;
                            break;
                        }
                    }
                    fprintf(output_file, "Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<%d\n",
                            process, PFN, new_ptr->logical_addr, new_ptr->process, diskLocation, VPN, originalDisk);
                }
                else
                {
                    search(PageTable[getid(evictedProcess)], evictedVPN, evictedProcess)->frame = -1;
                    //在physicalMemory紀錄
                    fillPhysicalMemory(PFN, VPN, process);
                    for (int i = 0; i < SIZE_OF_DISK; i++) //把被evict出去的東西塞進disk
                    {
                        if(Disk[i][0]==-1)
                        {
                            diskLocation = i;
                            Disk[i][0] = 1;
                            Disk[i][1] = getid(evictedProcess);
                            Disk[i][2] = evictedVPN;
                            break;
                        }
                    }
                    //檢查被evict的page有沒有在tlb
                    node *tlb_check = search(TLB, evictedVPN, evictedProcess);
                    if(tlb_check!=NULL)
                    {
                        tlb_check->logical_addr = -1;
                        tlb_check->frame = -1;
                    }
                    int originalDisk = -1;
                    for (int i = 0; i < SIZE_OF_DISK; i++)
                    {
                        //檢查本來有沒有在disk裡
                        if (Disk[i][1] == process_id && Disk[i][2] == VPN)
                        {
                            Disk[i][0] = -1; // 釋出新的位置
                            Disk[i][1] = -1; // 沒有被程序佔用
                            Disk[i][2] = -1; // 清空VPN
                            originalDisk = i;
                            break;
                        }
                    }
                    fprintf(output_file, "Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<%d\n",
                            process, PFN, evictedVPN, evictedProcess, diskLocation, VPN, originalDisk);
                }
            }
        }
        node *tp = TLB;//wirte to TLB
        while(tp)
        {
            if(tp->logical_addr==-1)
                break;
            else
                tp = tp->next;
        }
        if(tp==NULL) //full TLB, find a victim
        {
            if(TLB_POLICY==0) //random
            {
                int victim_id = rand() % SIZE_OF_TLB;
                int count = 0;
                node *rand_ptr = TLB;
                while(count < victim_id)
                {
                    //find a random node
                    rand_ptr = rand_ptr->next;
                    count++;
                }
                rand_ptr->frame = PFN;
                rand_ptr->logical_addr = VPN;
                rand_ptr->process = process;
            }
            else //LRU
            {
                TLB->frame = PFN;
                TLB->logical_addr = VPN;
                TLB->process = process;
            }
        }
        else
        {
            tp->logical_addr = VPN;
            tp->frame = PFN;
            tp->process = process;
        }
        memoryHandler(process, VPN);    //rerun again, TLB shall be hit
        return;
    }
}

void Analysis()
{
    FILE *fptr = fopen("analysis.txt", "w");
    for (int i = 0; i < NUM_OF_PROCESS; i++)
    {
        int m = 100;
        int t = 20;
        float hit_rate = (float)TLB_HIT[i] / (float)TLB_LOOKUP[i];
        float EAT = hit_rate * (m + t) + (1 - hit_rate) * (2 * m + t);
        float page_fault_rate = (float)PAGE_FAULT[i] / (float)REFERENCE[i];
        fprintf(fptr, "Process %c, Effective Access Time = %0.3f\n", getprocess(i), EAT);
        fprintf(fptr, "Process %c, Page Fault Rate = %0.3f\n", getprocess(i), page_fault_rate);
        // printf("Hit count:%d Lookup: %d page fault: %d\n", TLB_HIT[i], TLB_LOOKUP[i], PAGE_FAULT[i]);
    }
}

int main(void)
{
    srand(time(NULL));
    //readfile
    FILE *input_file = fopen("sys_config.txt", "r");
    output_file = fopen("trace_output.txt", "w");
    if(!input_file)
        exit(EXIT_FAILURE);
    char *PARAM = malloc(128 * sizeof(char));
    int ss;
    int size;
    size_t len = 0;
    for (int i = 0; i < 3; i++)
    {
        char tmp[20];
        ss=fscanf(input_file, "%[^\n] ", PARAM);
        sscanf(PARAM, "%*[^:]:%s", tmp);
        if (strcmp(tmp, "Random") == 0)    TLB_POLICY = 0;
        else if (strcmp(tmp, "LRU") == 0)    TLB_POLICY = 1;
        else if (strcmp(tmp, "FIFO") == 0)    PAGE_POLICY = 0;
        else if (strcmp(tmp, "Clock") == 0)    PAGE_POLICY = 1;
        else if (strcmp(tmp, "Global") == 0)    FRAME_POLICY = 0;
        else if (strcmp(tmp, "Local") == 0)    FRAME_POLICY = 1;
        // fprintf(output_file, "%s\n", tmp);
    }
    for (int i = 0; i < 3; i++)
    {
        fscanf(input_file, "%[^\n] ", PARAM);
        sscanf(PARAM, "%*[^:]:%d", &size);
        switch(i)
        {
        case 0:
            NUM_OF_PROCESS = size;
            break;
        case 1:
            NUM_OF_VIRTUAL_PAGE = size;
            break;
        case 2:
            NUM_OF_PHYSICAL_PAGE = size;
            break;
        default:
            break;
        }

    }
    fclose(input_file);

    //generate blank page table and TLB
    for (int j = 0; j < NUM_OF_PROCESS; j++)
    {
        for (int i = 0; i < NUM_OF_VIRTUAL_PAGE; i++)
        {
            node *temp = newnode();
            temp->process = getprocess(j);
            PageTable[j] = enq(PageTable[j], temp);
        }
        PageTable[j] = reIndex(PageTable[j], 0);
        TLB_HIT[j] = 0;
        TLB_LOOKUP[j] = 0;
        PAGE_FAULT[j] = 0;
        REFERENCE[j] = 0;
    }

    //generate blank physical page and free frame list
    for (int i = 0; i < NUM_OF_PHYSICAL_PAGE; i++)
    {
        node *tmp = newnode();
        node *tmp2 = newnode();
        tmp->process = 'Z';
        PhysicalMemory = enq(PhysicalMemory, tmp);
        free_frame_list = enq(free_frame_list, tmp2);
    }

    //make index correct
    PhysicalMemory = reIndex(PhysicalMemory, 1);
    free_frame_list = reIndex(free_frame_list, 1);

    //generate blank TLB and disk
    for (int i = 0; i < SIZE_OF_TLB; i++)
    {
        node *temp = newnode();
        TLB = enq(TLB, temp);
    }
    TLB = reIndex(TLB, 2);
    for (int i = 0; i < SIZE_OF_DISK; i++)
    {
        Disk[i][0] = -1;
        Disk[i][1] = -1;
        Disk[i][2] = -1;
    }

    //start tracing
    FILE *input_file2 = fopen("./trace.txt", "r");
    char process;
    int VPN;
    if(!input_file2)
        exit(EXIT_FAILURE);
    while(fscanf(input_file, "%[^\n] ", PARAM)!=EOF)
    {
        sscanf(PARAM, "Reference(%c,%d)", &process, &VPN);
        REFERENCE[getid(process)]++;
        //read process and VPN
        memoryHandler(process, VPN);
    }
    Analysis();
    fclose(input_file2);
    fclose(output_file);
}






