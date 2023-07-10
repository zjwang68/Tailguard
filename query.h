///////////////////////////////////STITCH simulation code/////////////
//////////////////////////////////Defintion head file////////////////
//////////////////////////////////Setup parameters defined//////////

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include "List.cpp"
#include <string.h>
#include <assert.h>

using namespace std;

//system parameters
#define  TNP 100                 //total number of task servers in the system
#define  NQ  400000              //Number of queries to be simulated
#define  NR 100000               //number of counters for p-th tail latency
#define  CLS  3                  //number of classes for SLO jobs 
#define  FAN  100                //maximum query fanout degree 

#define  ATET   1.0              //average task execution time for exponential distribution
#define ARATE  0.7*100./63.1     //average query arrival rate per unit time, this value is determined by the means task service time, query fanout and query arrival time, you need to calculate it based on your system set up.
#define pth   99                 //the p-th percentile tail latency

#define INF  800000000            //defined as an infinite number
double RTAIL[CLS]={20.,20.,20.};  //Tail latency SLO of CLS of queries
double PCT[CLS]={0.1,0.4, 1.0};   //percentage of each class of queries
#define TN 10000                  //number of entries for CDF       
double TASK_CDF[TN];                //store CDF data of task service time

/****************************************************************
define struct of query
QID: query identifier
start_time: query_start time
fanout: fanout degree
qtype: query class
task_time: task execution finish time
btime: task budget time
******************************************************************/
struct query {int QID; double start_time; int fanout; int qtype; double task_time; double btime;};

/****************************************************
Random double number generator Function between 0 and 1.
***************************************************/
double get_random()
{
  unsigned int xx;
  double y;

  y=(double)rand()/(double)RAND_MAX;
  if(y==1.0) { y-=10E-50;}

  return y;
}

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Next Request Generator--exponential distribution
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
double get_next_time(double arate)
{
   double  dice,tm;
   dice=get_random();
  if(dice<10E-50) { dice=10E-50;}
   tm=-log(dice)/arate;

   return tm;
}

/*****************************************************
 Generate task time based on input CDF
******************************************************/
double get_tailbench_time()
{
  double x, dice, tm;
  int ii;

   dice=get_random();
   ii=(int)(dice*TN); if(ii==TN) {ii--;}

   if(ii==0) { tm=TASK_CDF[ii]*(1-get_random()/2.0); }
   else if(ii==TN-1) {
            x=-log(get_random())*TASK_CDF[ii]/2; 
           tm=x+TASK_CDF[ii];
   }
   else {tm=TASK_CDF[ii]+(TASK_CDF[ii+1]-TASK_CDF[ii])*(get_random()-0.5);}

   return tm;
}


/*******maxv function: return the larger number value******/
double maxv(double x, double y)
{
	if(x>=y) { return x; }
	else { return y;}
}


void print_query(query qer)
	{
	cout<<"Query QID: "<<qer.QID<<" start time: "<<qer.start_time<<" fanout: "<<qer.fanout;
	cout<<" Task_time: "<<qer.task_time<<" budget time: "<<qer.btime<<" type: "<<qer.qtype<<endl;
		
	}
