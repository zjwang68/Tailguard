
/*#########################################################################################
The simulator of tailguard, you are free to use and modify the codes
###########################################################################################*/

#include <iomanip>
#include <math.h>
#include <fstream>
#include <iostream>
#include "global.h"

using namespace std;

int main(int argc, char** argv)
{
    int i,j,k,ii,jj,kk,ll,stop,count_load, counter;
    double  x,y,z,xx,yy,zz,tm, tim, nowtm,nowtime,qtail[CLS];
    double tot_load, tp_load, vt[CLS], vtt[CLS],tat[CLS], mst[CLS], var[CLS];
    int jobc[CLS];
    query qer;
    int CTM[CLS][NR];             //used for p-th percentile calculation
    double ATime[NQ],JFTime[NQ];  // Atime record the job arrival and JTime record the query completion time
    ifstream inf1, inf2;
	
    inf1.open("/RUT_Mastree_DV");         //Load the unloaded tail latency file
    inf2.open("/Mastree_cdf");
    for(i=0;i<FAN; i++){ inf1>>xx; ULTAIL[i]=xx; }    //ULTAIL stores the unloaded query tail latency for query with fanout i+1 (measured from simulation or real system)
    for(i=0;i<TN;i++) { inf2>>xx; TASK_CDF[i]=xx; }   //CDF of task service time, measured from task serivce samples

    
    srand(time(NULL));     // seed of random number

	for(i=0;i<TNP;i++) { NTASK[i]=-1; } //no task is serving in each task queue, -1 indicate no serving task
	
	//Initial the query completion time to be 0 and set NQ arrival time
	tm=0;
	for(i=0;i<NQ;i++){
	  tm+=get_next_time(ARATE);  //Poisson arrival processing
	  ATime[i]=tm;               //initiate all query arrival times (this way can fast simulation)
    JFTime[i]=0;               //initiate all query completion times to be 0
	}
        
  for(i=0;i<CLS; i++) { tat[i]=0; vtt[i]=0;}   //tat records task service time, vtt records square of task service time. Both are used for task service mean and variance calculation
	
    //setup the first arrived query
    QID=0; qer.QID=QID; QID++;  tm=ATime[qer.QID]; qer.start_time=tm;
    runtime=ATime[QID]; event=0; 
    tot_load=tp_load=tot_fan=0; count_load=counter=0;
    cout<<"Runtime: "<<runtime<<endl;

    new_query_budget(qer,runtime);  //The first query arrival, this is Tailguard scheduling, and T-EDFQ(need to change some codes in this function)
   // new_query_fifo(qer);            //The first query arrival, this is FIFO scheduling
   // new_query_priq(qer);            //The first query arrival, this is FIFO scheduling

    //main process for  query handle
	while(1){ 
	    counter++;  //the number of times the process is done
	    if(event==0) { //the event is the next arrival query
			//setup the next arrived query
	         if(QID<NQ) { //the query is not the last query, find the current query arrive time 
                    qer.QID=QID; QID++;  tm=ATime[qer.QID]; 
                    qer.start_time=tm; nowtime=runtime;
                    if(QID<NQ) {runtime=ATime[QID]; }   //set the next query arrival time
		     else {runtime=INF*1.0; } //This is the last query, no next query will arrive 
                      new_query_budget(qer,nowtime);  //Initate the current query
		 }
		 else { //no more query, no query will arrive
			runtime=INF*1.0; 
		  }
	       }//end if
	    else { //The event is to serve tasks in the task queues
	        event=0; //set the next query arrival as default event
    
    // Process the task scheduled in task servers
		for(ii=0;ii<TNP;ii++){ //task scheduling process in each task server
		  	if(NTASK[ii]>=0){ //the task queue ii has (a) task(s) in the queue
			   if(HQ[ii].task_time<=runtime){ //schedule all tasks that are completed before next query arrival time, HQ record the query completed
			     jj=HQ[ii].QID; nowtm=HQ[ii].task_time;   //find the serving query ID, task finish time
			     tm= nowtm-HQ[ii].start_time;   //find the task response time
            tat[JTP[jj]]+=tm;   //record the task respone time in a class of queries
            vtt[JTP[jj]]+=tm*tm; //record the square of task response time for variance calculation

			    if(JFTime[jj]<tm) {JFTime[jj]=tm;}  //record the query completion time if the task completion time is longer (the query completition time the maximum task completion time uin the query)
				  NTASK[ii]--;    //The number of tasks in task queue ii is reduced by 1 
			         if(NTASK[ii]>=0 && !PQ[ii].Empty()){ //the  number of tasks in task queue ii is more than 0,  get the next task from PQ[ii] and stored HQ
				        PQ[ii].SetToHead(); qer=*PQ[ii].Access(); PQ[ii].Remove();
                tim=get_tailbench_time();  //get the next task service  time 
		          	qer.task_time=nowtm+tim;   //set the next task completion time
                HQ[ii]=qer; 
                if(qer.task_time<=runtime) { event++; } //if any task completion time is eariler than next query arrival time, the next event is task scheduling
					   }
				      }//end if(HQ[ii]
			          }
		            }//end for loop
	      }//end else
		
      //show middle stage simulation status
      if(counter%400000==0) { cout<<"Start query "<<QID<<" runtime: "<<runtime<<" mean fanout: "<<tot_fan/(1.0*QID)<<endl; 
                       for(i=0;i<TNP;i++) { if(NTASK[i]>=0) {tot_load+=1.0; tp_load+=1.0; }}
                       count_load++;
                       
                       xx=tp_load/(1.0*TNP);  yy=tot_load/(1.0*TNP*count_load);
                       cout<<"  Load at this time: "<<xx<<" overall load: "<<yy<<endl<<endl;
                       tp_load=0;
                  }

	 if(runtime>=INF*0.95 && event==0) { break;} //finish simulation process
  }//end while

  //calculate the p-th tail latency
  for(j=0;j<CLS;j++){ jobc[j]=0; vt[j]=0;
      for(i=0;i<NR;i++) { CTM[j][i]=0;}
  }
  
  z=0.002; //minimum time interval for tail latency count, here it means the minimum time is 0.002 unit time
//count the nnumber of jobs in each class
  for(i=0;i<NQ;i++){ j=JTP[i]; jobc[j]++;  } cout<<jobc[0]<<"  "<<jobc[1]<<endl;

  for(i=0;i<NQ;i++){  
     j=(int)(JFTime[i]/z); if(j>=NR) {j=NR-1; }
     k=JTP[i]; vt[k]+=JFTime[i];
     CTM[k][j]++;			  
  }
		  

 for(j=0;j<CLS;j++) { kk=0;
  for(i=0;i<NR;i++){ 
     kk+=CTM[j][i];
     if(1.0*kk/(1.0*jobc[j])>=pth/100.) { qtail[j]=i*z; break;}
   }
 }
 
 kk=FAN;

 for(i=0;i<CLS; i++){ 
  //if(i==0) { kk=1; }
  //else if(i==1) { kk=10; }
  //else { kk=100;}

   x=1.0*jobc[i]*kk;
 //calculate mean and variance of task response time
  mst[i]=tat[i]/x;  //cout<<"mean: "<<mst[i]<<" variance: "<<vt[i]/jobc[i]<<endl;
  var[i]=vtt[i]/x-mst[i]*mst[i];


  cout<<"Mean fanout degree: "<<tot_fan/(1.0*NQ)<<endl;
  cout<<"The "<<pth<<" percentile tail for job class "<<i<<" is: "<<qtail[i]<<" total # of jobs is "<<jobc[i]<<endl;
  cout<<"The mean task completion time is: "<<mst[i]<<endl;
  cout<<"The sqrt variance of completion time is: "<<sqrt(fabs(var[i]))<<" ratio: "<<sqrt(fabs(var[i]))/mst[i]<<endl;
 }

 
    return 0;
}       

