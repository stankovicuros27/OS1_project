#include "pcb.h"
#include "SCHEDULE.H"
#include <dos.h>
#include <iostream.h>

ID PCB::currentID = 0;
List<PCB*> allPCBs;

PCB::PCB(StackSize size, Time timeSl, Thread *myThr, State s){
    timeSlice = timeSl;
    myThread = myThr;
    myLockVal = 0;    
    myID = ++currentID;
    state = s; 
   
    if (size > MAX_STACK) size = MAX_STACK; 
    if (size < MIN_STACK) size = MIN_STACK;
    stackSize = size / sizeof(Word); 
    initializeStack(runWrapper);

    allPCBs.insertBack(this);
}

PCB::PCB(int mainPCB){  //used only for creating mainPCB
    state = PCB::RUNNING; 
    stack = nullptr;
    sp = 0;
    ss = 0;
    bp = 0;
    timeSlice = defaultTimeSlice;
    myLockVal = 0;
    myID = ++currentID;
    allPCBs.insertBack(this);
}

PCB::PCB(){} //used only to make idlePCB

PCB::~PCB(){                        //mozda da dodam da se brise iz liste svih PCBova?
    awakeMyAsleep();
    if (stackSize != 0 || stack != nullptr) delete[] stack;
}

void PCB::initializeStack(pFunction fp){
    LOCKED(
        stack = new Word[stackSize];
    )
    stack[stackSize - 1] = INIT_PSW;       
    stack[stackSize - 2] = FP_SEG(fp);
    stack[stackSize - 3] = FP_OFF(fp);
    ss = FP_SEG(stack + stackSize - 12);
    sp = FP_OFF(stack + stackSize - 12);
    bp = sp;
}

void PCB::startPCB(){
    LOCKED(
        if (state == PCB::CREATED){
            setState(PCB::READY);
            Scheduler::put(this);
        } 
    )
}

void PCB::unblockPCB(){
    LOCKED(
        if(state == PCB::BLOCKED){
            setState(PCB::READY);
            Scheduler::put(this);
        }
    )
}

void PCB::blockPCB(){
    LOCKED(
        if((state != PCB::TERMINATED) && (state != PCB::IDLE)){
            setState(PCB::BLOCKED);
        }
    )
}

void PCB::awakeMyAsleep(){
    LOCKED( 
        while(!waitingForMe.isEmpty()){
            PCB *toStart = waitingForMe.getFront();
            waitingForMe.deleteFront();
            toStart->unblockPCB(); 
        }
    )
    //ovde sam obrisao if(toStart != nullptr) ispred toStart->
}

void PCB::waitToComplete(){
    //pazi ovde uslove!!!!!!
    LOCKED(
        if (running != this && 
        this->state != PCB::TERMINATED && 
        this->state != PCB::IDLE && 
        this->state != PCB::CREATED 
        /*&& !this->isWaitingForMe()*/ ) /* used for deadlocks */
        {        
            this->waitingForMe.insertBack(running);
            running->blockPCB();
            dispatch();
        }
    )
}

//pozivam iz runninga za thread za koji zelim da vidim da li me ceka
bool PCB::isWaitingForMe(){
    bool ret = false;
    LOCKED(
        List<PCB*>::Iterator iter = running->waitingForMe.begin();
        for(;iter != running->waitingForMe.end(); iter++){
            if ((*iter)->getId() == this->getId()){
                ret = true;
                break;
            }
        }
    )
    return ret;
}

void PCB::waitAll(){
    //pazi ovde uslove!!!!!!
    if(running != mainPCB) return;  //only mainPCB can wait for all other threads
    LOCKED(
        List<PCB*>::Iterator iter = allPCBs.begin();
        for(;iter != allPCBs.end(); iter++){
            (*iter)->waitToComplete();
        }
    )
}

PCB* PCB::getPCBById(ID id){
    List<PCB*>::Iterator iter = allPCBs.begin();
    for(;iter != allPCBs.end(); iter++){
        if ((*iter)->getId() == id) return (*iter);
    }
    return nullptr;
}

void PCB::runWrapper(){
    running->getMyThread()->run();
    LOCKED(
        running->awakeMyAsleep();
        running->setState(PCB::TERMINATED);
        dispatch();
    )
}

//---Signals---

void PCB::signal(SignalId signal){

}
void PCB::registerHandler(SignalId signal, SignalHandler handler){

}
void PCB::unregisterAllHandlers(SignalId id){

}
void PCB::swap(SignalId id, SignalHandler hand1, SignalHandler hand2){

}
void PCB::blockSignal(SignalId signal){

}
void PCB::blockSignalGlobally(SignalId signal){

}
void PCB::unblockSignal(SignalId signal){

}
void PCB::unblockSignalGlobally(SignalId signal){

}
bool PCB::handleSignals(){

}
void PCB::kill(PCB *pcb){

}