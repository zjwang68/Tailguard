
//setup simulation enviroment and define global variables and functions
#include "query.h"

List<query> PQ[TNP]; //define NTP queues, each stores all the queries which have a task served in their task server
int NTASK[TNP];      //define NTP counters, each  counts the number of tasks in a task server
query HQ[TNP];       //store the query, its task is in the head of the task queue and being served
int JTP[NQ];         //the query class
double ULTAIL[FAN];   //store unloaded query tail latency, input file, measured from simulated or real systems
int QID;             //the query ID indicator
double runtime;      //indicate the next runtime
int event;           //record the current or next event 0--next event is a query arrival process, 1--complete tasks in task servers
double tot_fan;     //total number of fanout for all queries

//task scheduling for tailguard
void new_query_budget(query qer, double nowtime){
	short int markp[TNP];
	int i,j,k,fan,key,tp;
	double tm,tail,x,xx,yy;
	query tqer;
	
	for(i=0;i<TNP;i++) {markp[i]=0;}   
	x=get_random();
	 
    for(i=0;i<CLS; i++){ //find the query class
	   if(x<PCT[i]) { tp=i; break;}
       }
 
     //this is to set up query fanout 
	 //here is case 1, query fanouts are 1, 10 and 100
     if(tp==0) { fan=1;}
     else if(tp==1) { fan=10;}
     else {fan=100;}

	 //here is the case all query has the same fanout
	 //fan=FAN;
  
        JTP[QID-1]=tp;  //set the fanout of the query
       
	//set the query information
	   k=0; qer.fanout=fan; qer.qtype=tp; qer.task_time=0; tot_fan+=1.0*fan;
	
	   tail=ULTAIL[fan-1];  //find the unloaded tail latency
        xx=RTAIL[tp]-tail; //computed task budget in the query
    
        xx=nowtime+xx;  //set the task pre-dequeuing deadline
        qer.btime=xx;
	
	while(k<fan){
		//randomly select task servers if the query fanout is less than the number of task servers
		if(fan<TNP){
		   j=(int)(TNP*get_random()); if(j==TNP) {j--;}
		   if(markp[j]==1) {continue;}  //the task server is already selected
	       }
               else { //if fanout=number of task servers, send a task to each task server
                  j=k; 
              }
		
		markp[j]=1; //send the task to selected task server j
		if(NTASK[j]==-1) { //no query in the task server is served, serve the query immediately
				tm=get_tailbench_time();  //get the task service time
		        qer.task_time=qer.start_time+tm;
                 HQ[j]=qer; NTASK[j]=0; 
                 
        	}
		else { //the task server is busy, add the  task to task queue based on the task deadline
                   qer.task_time=0;  key=0;
		   if(PQ[j].Empty()) { PQ[j].EnQueue(qer); key=1;}  //no task in the queue, add it to the queue
		   else { PQ[j].SetToTail(); tqer=*PQ[j].Access();  //some tasks in the queue, insert the task into the queue based on its pre-dequeuing deadline
		          if(qer.btime>=tqer.btime) { PQ[j].Insert(qer); key=1;}
				  else {
					   while(PQ[j].Back()) { tqer=*PQ[j].Access();
					       if(qer.btime>=tqer.btime) { PQ[j].Insert(qer); key=1; break;}
					   }
				  }
				  if(key==0) { PQ[j].EnQueue(qer); }
		   } //end second else	
                   NTASK[j]++;  
		}
		
		k++; 
	}//end while
	
	for(i=0;i<TNP;i++) {
		
	   if(NTASK[i]>=0 && HQ[i].task_time<runtime) { event++;} 
	}
	
	return;
}

//Task scheduling for FIFO queuing
void new_query_fifo(query qer){
	short int markp[TNP];
	int i,j,k,fan,tp;
	double tm,x,xx;
	
	for(i=0;i<TNP;i++) {markp[i]=0;}   
	x=get_random();

     
  for(i=0;i<CLS; i++){ //find the query class
	   if(x<PCT[i]) { tp=i; break;}
       }
 
     //this is to set up query fanout 
	 //here is case 1, query fanouts are 1, 10 and 100
     if(tp==0) { fan=1;}
     else if(tp==1) { fan=10;}
     else {fan=100;}

	 //here is the case all query has the same fanout
	 //fan=FAN;

    JTP[QID-1]=tp;
    
    //set the query information
	k=0; qer.fanout=fan; qer.qtype=tp; qer.task_time=0;  qer.btime=0; tot_fan+=1.0*fan;
	
	while(k<fan){
               if(fan<TNP){
	        	//randomly select a task server if the query fanout is smaller than the number of taskservers
	        	j=(int)(TNP*get_random()); if(j==TNP) {j--;}
	        	if(markp[j]==1) {continue;}  //the task server is already selected
		}
                else { j=k;} //assign a task to each task server if the query fanout= number of task servers

		
		markp[j]=1; //select pod j 
		if(NTASK[j]==-1) { //no query is served, serve the query immediately
		   tm=get_tailbench_time();
		   qer.task_time=qer.start_time+tm;
                   HQ[j]=qer; NTASK[j]=0; 
        	}
		else { //the task server is busy, add the task to the tail of the queue (FIFO)
                   qer.task_time=0;
		   if(PQ[j].Empty()) { PQ[j].EnQueue(qer); }
		   else {PQ[j].SetToTail(); PQ[j].Insert(qer);}
                   NTASK[j]++;
		}
		
		k++; 
	}//end while
	
	for(i=0;i<TNP;i++) { //set  event value
	   if(NTASK[i]>=0 && HQ[i].task_time<runtime) { event++;} 
	}
	
	return;
}

//Task scheduling for priority queuing 
void new_query_priq(query qer){
	short int markp[TNP];
	int i,j,k,fan,tp,key;
	double tm,x,xx;
	query tqer;
	
	for(i=0;i<TNP;i++) {markp[i]=0;}   
	x=get_random();

    for(i=0;i<CLS; i++){ //find the query class
	   if(x<PCT[i]) { tp=i; break;}
       }
 
     //this is to set up query fanout 
	 //here is case 1, query fanouts are 1, 10 and 100
     if(tp==0) { fan=1;}
     else if(tp==1) { fan=10;}
     else {fan=100;}

	 //here is the case all query has the same fanout
	 //fan=FAN;
        fan=FAN;
        JTP[QID-1]=tp; 

     //set the query information
	k=0; qer.fanout=fan; qer.qtype=tp; qer.task_time=0;  qer.btime=0; tot_fan+=1.0*fan;
	 
	while(k<fan){
              if(fan<TNP){
	        	//randomly select a task server if the query fanout is smaller than the number of taskservers
	        	j=(int)(TNP*get_random()); if(j==TNP) {j--;}
	        	if(markp[j]==1) {continue;}  //the task server is already selected
		}
        else { j=k;} //assign a task to each task server if the query fanout= number of task servers

		markp[j]=1; //select pod j 
		if(NTASK[j]==-1) { //no query is served, serve the query immediately
		     tm=get_tailbench_time();
		   qer.task_time=qer.start_time+tm;
                   HQ[j]=qer; NTASK[j]=0; 
        	}
		else { //task server is busy, add the query to priority queue based on class
                   qer.task_time=0; key=0;
		   if(PQ[j].Empty()) { PQ[j].EnQueue(qer); key=1;}
		   else {PQ[j].SetToTail(); tqer=*PQ[j].Access();
		          if(qer.qtype>=tqer.qtype) { PQ[j].Insert(qer); key=1;}
			  else {
					   while(PQ[j].Back()) { tqer=*PQ[j].Access();
					       if(qer.qtype>=tqer.qtype) { PQ[j].Insert(qer); key=1; break;}
					   }
                      	   }
                     }
                         
				  if(key==0) { PQ[j].EnQueue(qer); }
                   NTASK[j]++;
		}
		
		k++; 
	}//end while
	
	for(i=0;i<TNP;i++) { //set  event value
	   if(NTASK[i]>=0 && HQ[i].task_time<runtime) { event++;} 
	}
	
	return;
}

void Show_Data()
{ int i;
  query qer;

  for(i=0;i<TNP;i++) {
	 cout<<"Pod: "<<i<<" has "<<NTASK[i]<<" task(s)"<<endl;
         if(NTASK[i]>=0) { cout<<"The handling query is: "; print_query(HQ[i]);}
	
         if(!PQ[i].Empty()){
		cout<<"Queued query:"<<endl;
		PQ[i].SetToHead();
		qer=*PQ[i].Access(); print_query(qer);
		 while(PQ[i].Fore()) { qer=*PQ[i].Access(); print_query(qer);}
	}
       cout<<"*****************"<<endl;
  }
}

