////////////////////////////////////////////
//                                        //
//        LRU replacement policy          //
//     Jinchun Kim, cienlux@tamu.edu      //
//                                        //
////////////////////////////////////////////

#include "../inc/champsim_crc2.h"
#include <iostream>
#include <random>
#include <cstdlib>
#include <ctime>

#define NUM_CORE 1
#define LLC_SETS NUM_CORE*2048
#define LLC_WAYS 16

uint32_t lru[LLC_SETS][LLC_WAYS];



// ----DO NOT CHANGE THESE VARIABLE ASSIGNMENTS-----
int *leader = new int[64];  			//leader array
int leader_array_populating = 1;   		//becomes 0 after executing once when array is populated with rands
int leader_set_switch = 1;				//a switch for accessing leader set...
int distinctive =1;						//a switch to check if leader array is being populated by distinctive set number

int policy_toggle =1;					// 1 = policy#1    2 = policy#2......starts with policy#1
int leader_set_access =0;				//a counter to count leader set access..when it turns 64, it decides which policy worked the best
int policy_1_hit=0;
int policy_2_hit=0;
//-----DO NOT CHANGE THESE VARIABLE ASSIGNMENTS-----






// initialize replacement state
void InitReplacementState(){	


	if(leader_array_populating==1){             //populates the leader array with random number between 1 and 1024
	
        for(int i =0;i<64;i++){                 //initialize the leader array with all -1s initially
            leader[i]= -1;
        }
		
	for(int i =0;i<64;i++){						//initialize the leader array with distinctive random numbers
            distinctive = 0;
            while(distinctive == 0){
                int rand_num = (rand () % 1024) + 0;
                for(int j =0;j<64;j++){
                    if(rand_num==leader[j]){
                        distinctive = 0;
                    }
                    else{
                        distinctive = 1;
                    }
                }
                if(distinctive == 1){
                    leader[i] = rand_num;
                }
            } 
        }
        
        leader_array_populating = 0;			// switched to 0 so that this portion of code doesnt get called anymore
    }
	
	
	cout << "Initialize LRU replacement state" << endl;

    for (int i=0; i<LLC_SETS; i++) {
        for (int j=0; j<LLC_WAYS; j++) {
            lru[i][j] = j;
        }
    }
	
	cout << "Initialize LRU replacement state has been completed " << endl;
}




// find replacement victim
// return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSet (uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type)
{
	if(leader_set_switch==1){			//leader_set_switch == 1 means we can compare leader sets 
		for(int i =0;i<64;i++){
			if(set == leader[i]){
				leader_set_access++;		//increment leader_set_access counter...
				
				// set number matches with one of the leader sets ...so we switch policy
				if(policy_toggle==1){
					policy_toggle =2;
				}
				else if(policy_toggle==2){
					policy_toggle =1;
				}
			}
		}
		
	}
		
	
	
	if(policy_toggle == 1){						//policy_1 = lru eviction
		for (int i=0; i<LLC_WAYS; i++){
			if (lru[set][i] == (LLC_WAYS-1)){
				return i;
			}
		}
		
	}
	
	else if(policy_toggle == 2){				//policy_2 = lru eviction
		for (int i=0; i<LLC_WAYS; i++){
			if (lru[set][i] == (LLC_WAYS-1)){
				return i;
			}
		}
		
	}
	
    return 0;
}



// called on every cache hit and cache fill
void UpdateReplacementState (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{	
	if(hit==1){
		if(policy_toggle == 1){
			policy_1_hit++;
			
		}
		
		else if(policy_toggle == 2){
			policy_2_hit++;
			
		}
	}
	
	
		
	if(leader_set_access == 64){
		if(policy_1_hit>policy_2_hit){
			policy_toggle = 1;
			leader_set_switch = 0;  //switch turned into 0 means no more comparing with leader sets..we have found our ultimate policy
			leader_set_access = -1; //initialized to -1 to avoid entering this portion of code anymore.
		}
		else if(policy_1_hit<policy_2_hit){
			policy_toggle = 2;
			leader_set_switch = 0;
			leader_set_access = -1;
		}
	}
		

		
	
	
	if(policy_toggle == 1){					//lru insertion
			
		for (uint32_t i=0; i<LLC_WAYS; i++) {
			if (lru[set][i] < lru[set][way]) {
				lru[set][i]++;
				if (lru[set][i] == LLC_WAYS){
					assert(0);
				}
			}
		}
		lru[set][way] = 0; // promote to the MRU position
	}
	
	
	else if(policy_toggle == 2){					//mru insertion
			
		for (uint32_t i=0; i<LLC_WAYS; i++) {
			if (lru[set][i] > lru[set][way]) {
				lru[set][i]--;
				if (lru[set][i] == LLC_WAYS){
					assert(0);
				}
			}
		}
		lru[set][way] = LLC_WAYS-1; // promote to the LRU position

	}
		
	
	
}



// use this function to print out your own stats on every heartbeat 
void PrintStats_Heartbeat()
{

}

// use this function to print out your own stats at the end of simulation
void PrintStats()
{
	
}
