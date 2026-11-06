#include<bits/stdc++.h>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include <unistd.h>
#include<cstdio>
using namespace std;

#define printer_number 4
#define Binding_station_number 2
#define Reader_number 2
#define MAX_GROUP 1000

int N=1000,M=1000,w,x,y;
int total_group_number;

//student info

struct student_info
{
      int group_num;
      int studentid;
      
};


//time calculation
time_t start,current;


//printer

vector<struct student_info* >printer[printer_number+1];//printer waiting list
vector<pthread_mutex_t>printer_lock(printer_number+1);
bool isfree[printer_number+1];



//semaphore for  every  student (for chatarmatha dining philosopher)
vector<sem_t>student(N);


//group semaphore
vector<sem_t>group_sem(MAX_GROUP+1);
vector<pthread_mutex_t>group_lock(MAX_GROUP+1);
vector<struct student_info >group_members[MAX_GROUP+1];

//binding semaphore
sem_t  bind_sem;


//submit to library
int record_in_the_entrybook=0;
pthread_mutex_t entry_mutex;

//reader
int currentlyReading=0;
pthread_mutex_t reader_mutex;




//all the functions

int currentTime()
{
    time(&current);
    return current-start;
}

void Release_printer(struct student_info *info)
{
    pthread_mutex_lock(&printer_lock[(info->studentid%printer_number)+1]);
    bool got=false;
    bool first=true;
    for(int i=0;i<printer[(info->studentid%printer_number)+1].size();i++)
    {
        if(printer[(info->studentid%printer_number)+1][i]->studentid!=info->studentid &&(printer[(info->studentid%printer_number)+1][i]->group_num==info->group_num || first))
      {
         got=true;
         first=false;
         sem_post(&student[printer[(info->studentid%printer_number)+1][i]->studentid]);
      }
    }
    
    if(!got)
    {
        
        isfree[(info->studentid%printer_number)+1]=true;
    }
    pthread_mutex_unlock(&printer_lock[(info->studentid%printer_number)+1]);

}



void * printing(void * args)
{
    
     struct student_info *info=(struct student_info *)args;
     //printf("group number:%d student id:%d\n",info->group_num,info->studentid);
     sem_wait(&student[info->studentid]);
     //printf("in %d\n",info->studentid);
     printf("Student %d has arrived at the print station at time %d\n",info->studentid,currentTime());
     sleep(w);
     pthread_mutex_lock(&printer_lock[(info->studentid%printer_number)+1]);
     //cout<<w<<endl;
        

     //printf("in %d ha ha\n",info->studentid);
     int i=0;
     for(;i<printer[(info->studentid%printer_number)+1].size();i++)
     {
      if(printer[(info->studentid%printer_number)+1][i]->studentid==info->studentid)
      {
            break;
      }
     }
     printer[(info->studentid%printer_number)+1].erase(printer[(info->studentid%printer_number)+1].begin()+i);
     pthread_mutex_unlock(&printer_lock[(info->studentid%printer_number)+1]);
     printf("Student %d has finished printing at time %d\n",info->studentid,currentTime());
     fflush(stdout);
     Release_printer(info);


       //this part is for binding activation
     pthread_mutex_lock(&group_lock[info->group_num]);
     group_members[info->group_num].push_back(*info);
     if(group_members[info->group_num].size()>=M)
     {  
        sem_post(&group_sem[info->group_num]);
     }
     pthread_mutex_unlock(&group_lock[info->group_num]);



     sem_post(&student[info->studentid]);
        
}
void* group_formation(void * args)
{
  
   bool done[printer_number+1];
   pthread_t  thread[N+1];

   for(int i=1;i<printer_number+1;i++)
   {
      done[i]=false;
   }
   
   default_random_engine generator(std::random_device{}());
   poisson_distribution<int> distribution(3);


   int gnum=1;  

   for(int i=1;i<=N;i++)
   {    
        int num=distribution(generator);
        sleep(num);
        struct student_info *info;
        info = (struct student_info *)malloc(sizeof(struct student_info));
        info->group_num=gnum;
        info->studentid=i;
        //printf("group number:%d student id:%d\n",info->group_num,info->studentid);
        if(i%M==0)
        {
            gnum++;
        }

        pthread_mutex_lock(&printer_lock[(i%printer_number)+1]);
        	
        if(done[(i%printer_number)+1]==false || isfree[(i%printer_number)+1])
        {
            //printf("%d\n",i);
           done[(i%printer_number)+1]=true;
           isfree[(i%printer_number)+1]=false;
        }
        else
        {
            sem_wait(&student[i]); 
            //cout<<"lalalala "<<i<<endl;
        }
        printer[(i%printer_number)+1].push_back(info);
        pthread_mutex_unlock(&printer_lock[(i%printer_number)+1]);	

       

        pthread_create(&thread[i],NULL,printing,(void *)info);
        
          
         
   }
   
   for(int i=1;i<=N;i++)
   {
        pthread_join(thread[i],NULL);
   }
     
   
}
void * writing_to_entry_book(void * args)
{
    int group_number=*((int *)args);
    pthread_mutex_lock(&entry_mutex);
    printf("Group %d has started writing to the entry book at time %d\n",group_number,currentTime());
    sleep(y);
    record_in_the_entrybook++;
    printf("Group %d has finished writing to the entry book at time %d\n",group_number,currentTime());
    pthread_mutex_unlock(&entry_mutex);

}
void * binding_station(void * args)
{
    int group_number=*((int *)args);
    // printf("in the group %d\n",group_number);
    sem_wait(&group_sem[group_number]);
    sem_wait(&bind_sem);
    printf("Group %d has started binding at time %d\n",group_number,currentTime());
    sleep(x);
    printf("Group %d has finished binding at time %d\n",group_number,currentTime());
    sem_post(&bind_sem);
    sem_post(&group_sem[group_number]);


    //going to library to complete submission by writing to entry book
    pthread_t write;
    int *p=(int *) malloc(sizeof(int));
    *p=group_number;
    pthread_create(&write,NULL,writing_to_entry_book,(void *)p);
    pthread_join(write,NULL);
}

void * stuff_reading(void *args)
{
    int stuff_number=*((int *) args);
    default_random_engine generator(std::random_device{}());
    poisson_distribution<int> distribution(3);
   while (true)
   {   
      int num=distribution(generator);
      sleep(num);
      pthread_mutex_lock(&reader_mutex);
      currentlyReading++;
      if(currentlyReading==1)
      {
        pthread_mutex_lock(&entry_mutex);
      }
      pthread_mutex_unlock(&reader_mutex);
      printf("Staff %d has started reading the entry book at time %d. No. of submission = %d\n",stuff_number,currentTime(),record_in_the_entrybook);
      sleep(y);
      pthread_mutex_lock(&reader_mutex);
      currentlyReading--;
      if(currentlyReading==0)
      {
        pthread_mutex_unlock(&entry_mutex);
      }
      pthread_mutex_unlock(&reader_mutex);


   }
   
}

int main()
{

      freopen("out.txt","w",stdout);
      freopen("in.txt","r",stdin);
      
      cin>>N>>M>>w>>x>>y;
      //cout<<N<<endl<<M<<endl<<w<<endl<<x<<endl<<y;
      total_group_number=N/M;
      
      time(&start);

      
     
      //lock init
      for(int i=1;i<printer_number+1;i++)
      {
          pthread_mutex_init(&printer_lock[i],0);
          isfree[i]=true;
      }
      // student semaphore init for dining philosopher

      for(int i=1;i<=N;i++)
      {
          sem_init(&student[i],0,1);
      }
      
     //semaphore init for binding philosopher

     sem_init(&bind_sem,0,Binding_station_number);

     //semaphore and lock init for group

     for(int i=1;i<=total_group_number;i++)
     {
          pthread_mutex_init(&group_lock[i],0);
          sem_init(&group_sem[i],0,1);
          sem_wait(&group_sem[i]);
     }
    
    //reader and entrybook mutex init
     pthread_mutex_init(&reader_mutex,0);
     pthread_mutex_init(&entry_mutex,0);

      pthread_t print;
      pthread_t groupthread[total_group_number+1],reader[Reader_number+1];

      pthread_create(&print,NULL,group_formation,NULL);
     

    for(int i=1;i<=total_group_number;i++)
    {   
        int *p=(int *) malloc(sizeof(int));
        *p=i;
        pthread_create(&groupthread[i],NULL,binding_station,(void *)p);
        
    }
    
    for(int i=1;i<=Reader_number;i++)
    {
        int *p=(int *) malloc(sizeof(int));
        *p=i;
        pthread_create(&reader[i],NULL,stuff_reading,(void *)p);

    }
      
      pthread_join(print,NULL);

      for(int i=1;i<=total_group_number;i++)
    {
       
        pthread_join(groupthread[i],NULL);

    }
   
      

}