#include "timer.h"

pInterrupt Timer::oldTimerInterrupt = 0;
volatile bool contextSwitchOnDemand = false;
volatile Time remainingTime = defaultTimeSlice;

//void tick();
volatile Reg tss;
volatile Reg tsp;
volatile Reg tbp;

void interrupt Timer::timerIntr(...){

    if(contextSwitchOnDemand == false){

        if(remainingTime > 0)
            remainingTime--;  

        //tick();
        asm int utilEntry
    }
    


    if(contextSwitchOnDemand == true || (remainingTime == 0 && lockVal == 0)){
        asm {
            mov tss, ss
            mov tsp, sp
            mov tbp, bp
        }
        running->ss = tss;
        running->sp = tsp;
        running->bp = tbp;
        running->myLockVal = lockVal;

        if(running->state == PCB::RUNNING && running != idlePCB){
            running->state = PCB::READY;
            Scheduler::put(running);
        }

        running = Scheduler::get();
        
        // if(running == nullptr){
        //     running = idlePCB;
        // }
        // else {
        //     running->state = PCB::RUNNING;
        // }

        if(running == nullptr || running->state != PCB::READY){
            running = idlePCB;
        } else {
            running->state = PCB::RUNNING;
        }

        DISABLED_INTR(
            cout << "Timer..." << endl;
        )

        lockVal = running->myLockVal;
        remainingTime = running->timeSlice;
        tss = running->ss;
        tsp = running->sp;
        tbp = running->bp;
        asm {
            mov ss, tss
            mov sp, tsp
            mov bp, tbp
        }
    }
    contextSwitchOnDemand = false;
}

void Timer::initializeTimerIntr(){
    DISABLED_INTR(
        Timer::oldTimerInterrupt = getvect(timerEntry);
        setvect(utilEntry, Timer::oldTimerInterrupt);
        setvect(timerEntry, Timer::timerIntr);
    )
}

void Timer::restoreTimerIntr(){
    DISABLED_INTR(
        setvect(timerEntry, Timer::oldTimerInterrupt);
    )
}


