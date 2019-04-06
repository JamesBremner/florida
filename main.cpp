#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <cmath>
#include <queue>

using namespace std;

class cServer;
class cJob;

/**
Client

To simulate multiple clients you must preload the data from FloridaCounties.csv into a single client.
The client must store the client data in a single data structure in which each record is stored as a single string variable (i.e. perhaps an array or linked list of strings).
 You are not allowed to preprocess the data into its constituent parts.
 However you will need to read the first record from the string, which for simulation purposes contains a “wait” number.
 This number ranges from 0 to 5 and represents the number of seconds you should wait before attempting to pass the next job to the scheduler. Note: Many jobs have wait time of 0. In this case (to simplify the problem) you must just pass the jobs consecutively with no deliberate wait time.

The client must run on its own thread throughout the entire simulation. You should not start up the simulation until the client object is loaded i.e. all records are read in and stored.

The client is responsible for passing the entire string to the scheduler. Each such request is called a ‘job’.
*/
class cClient
{
public:
    cClient( cServer& server );
    void Read();
    void Send();
private:
    std::vector< std::string > vl;
    cServer& myServer;
};

class cServer
{
public:
    cServer();
    void Rcvr( const std::string& job );
    void DoWork();
    void Task1JobCreation( cJob& theJob, const std::string& job );
    void Task2StatelessCounter( cJob& theJob );
    void Task3VicinityComputer( cJob& theJob );
    void Task4TallyType( cJob& theJob );
    void Task5RegisterExpensiveHouses( cJob& theJob );
private:
    int myTally[2];
    std::queue< cJob > myJobQueue;
    std::thread myWorkerThread;
};

class cJob
{
public:
    cJob( int count )
        : myArrival( std::chrono::steady_clock::now() )
        , myCount( count )
    {
    }
    void push_back( const std::string& field )
    {
        vField.push_back( field );
    }
    int FieldCount()
    {
        return (int) vField.size();
    }
    void setTasks();

    int NextTask();

    int getSeconds()
    {
        return mySeconds;
    }
    int getCount()
    {
        return myCount;
    }

    int getPointGranularity() const
    {
        return atoi( vField[17].c_str() );
    }
    int getEqLimit() const
    {
        return atoi( vField[3].c_str() );
    }
    int getHuLimit() const
    {
        return atoi( vField[4].c_str() );
    }
    int getFlLimit() const
    {
        return atoi( vField[5].c_str() );
    }
    int getFrLimit() const
    {
        return atoi( vField[6].c_str() );
    }
    double getLatitude() const
    {
        return atof( vField[13].c_str() );
    }
    double getLongitude() const
    {
        return atof( vField[14].c_str() );
    }
    std::string getLine() const
    {
        return vField[ 15 ];
    }
    std::string getConstruction() const
    {
        return vField[16 ];
    }
private:
    int myCount;
    std::vector<  std::string > vField;     // The fields in the job specification sent by client
    std::vector< int > myTodo;              // The tasks still to be done
    int myDone;                             // the number of tasks completed
    std::chrono::steady_clock::time_point myArrival;                       // arrival time
    int mySeconds;                          // seconds needed to complete this job
    static int theTotalSeconds;              // seconds for all jobs
};


cClient::cClient( cServer& server )
    : myServer( server )
{

}

void cClient::Read()
{
    ifstream in( "FloridaCounties.csv");
    if( ! in.is_open() )
    {
        std::cout << "Cannot read\n";
        exit(1);
    }
    std::string l;

    // skip first line, it is headers
    getline( in, l );

    while( getline( in, l ))
    {
        vl.push_back( l );
    }
    std::cout << "Client read " << vl.size() << " records\n";
}

void cClient::Send()
{
    for( auto& l : vl )
    {
        myServer.Rcvr( l );

        int wait = atoi( l.c_str() );
        if( wait )
            std::this_thread::sleep_for (std::chrono::seconds( wait ));

    }
}

cServer::cServer()
    : myWorkerThread( &cServer::DoWork, this )
{
}

void cServer::Rcvr( const std::string& job )
{
    static int jobtotaltime = 0;
    static int jobcount = 0;
    jobcount++;

    auto timenow =
        chrono::system_clock::to_time_t(chrono::system_clock::now());
    cout << ctime(&timenow);

    std::cout << "\nclient sent job: " << job.substr(0,10) << " ... " << job.substr(job.length()-10) << "\n";

    cJob theJob( jobcount );
    Task1JobCreation( theJob, job );
    theJob.setTasks();
    myJobQueue.push( theJob );
    std::cout << "Job queue length " << myJobQueue.size() << "\n";

}

void cServer::DoWork()
{
    while( 1 )
    {
        if( ! myJobQueue.size() )
        {
            std::cout << "DoWork waiting\n";
            std::this_thread::sleep_for (std::chrono::seconds( 1 ));
        }
        else
        {
            int next = myJobQueue.front().NextTask();
            if( next == -1 )
            {
               // std::cout << "\nDoWork job complete\n";
                myJobQueue.pop();
            }
            else
            {
                //std::cout << "DoWork task " << next << "\n";
                switch( next )
                {
                case 2:
                    Task2StatelessCounter( myJobQueue.front() );
                    break;
                case 3:
                    Task3VicinityComputer( myJobQueue.front() );
                    break;
                case 4:
                    Task4TallyType( myJobQueue.front() );
                    break;
                case 5:
                    Task5RegisterExpensiveHouses( myJobQueue.front() );
                    break;
                }
            }
        }
    }

}

void cServer::Task1JobCreation( cJob& theJob, const std::string& job )
{
    std::stringstream sst(job);
    std::string a;
    while( getline( sst, a, ',' ) )
        theJob.push_back(a);
    if( theJob.FieldCount() < 18 )
        throw std::runtime_error("Task1JobCreation insufficient fields");
    std::cout << "Task1 ";
}
/*
Task 2: Stateless Counter (pretty useless actually!)

1.	Take a house object as input.

2.	Count from 0 to eq_site_limit
3.	Count from 0 to fl_site_limit
4.	Count from 0 to hu_site_limit
5.	Count from 0 to fl_site_limit
6.	Return nothing
*/

void cServer::Task2StatelessCounter( cJob& theJob )
{
    for( int k = 0; k < theJob.getEqLimit(); k++ )
        ;
    for( int k = 0; k < theJob.getHuLimit(); k++ )
        ;
    for( int k = 0; k < theJob.getFlLimit(); k++ )
        ;
    for( int k = 0; k < theJob.getFrLimit(); k++ )
        ;
    std::cout <<"Job"<< theJob.getCount() << "Task2 ";
}

void cServer::Task3VicinityComputer( cJob& theJob )
{
    double dlo = theJob.getLongitude() + 81;
    double dla = theJob.getLatitude() - 30;
    double distance = sqrt( dlo * dlo + dla * dla );
    std::cout <<"Job"<< theJob.getCount() << "Task3 ";
}
/*
Task 4: Tally by Type (stateful)

1.	Simulate access to a persistent data store by use of a shared data structure. To simulate remote database accesses add 1 second of wait time to each update.
2.	Tally by type i.e. increment counter for each type i.e. Residence:Wood or Residence:Masonry (found in fields “line” and “construction”).
3.	Think BASE, i.e. basicallyavailable, soft state, eventually consistent.
4.	At the end of the simulation PRINT the contents of this data structure to a file or screen.
*/
void cServer::Task4TallyType( cJob& theJob )
{
    std::this_thread::sleep_for (std::chrono::seconds( 1 ));
    if( theJob.getLine() == "Residential" )
    {
        if( theJob.getConstruction() == "Masonry ")
            myTally[0]++;
        else if ( theJob.getConstruction() == "Wood" )
            myTally[1]++;
    }
    std::cout <<"Job"<< theJob.getCount() << "Task4 ";
}

void cServer::Task5RegisterExpensiveHouses( cJob& theJob )
{
    // specs say "eq_site_license " but no such field name
    // use EqLimit instead until clarfied
    if( theJob.getEqLimit() > 800000 )
        std::this_thread::sleep_for (std::chrono::seconds( 10 ));
    std::cout <<"Job"<< theJob.getCount() << "Task5 ";
}

int cJob::theTotalSeconds = 0;

void cJob::setTasks()
{
    /*
        /*
    Tasks are completed in different orders depending on thepoint_granularity

    (PG) field as follows:
    PG=1: T1, T2, T3, T4, T5
    PG=2: T1, T3, T4, T5, T2
    PG=3: T1, T5, T4, T3, T2
    PG=4: T1, T4, T2, T5, T3
    PG=5: T1, T2, T5, T3, T4
    PG=7: T1, T3, T2, T5, T4
    (There are no jobs for PG=6)

    */
    switch( getPointGranularity() )
    {
    case 1:
        myTodo.push_back( 2 );
        myTodo.push_back( 3 );
        myTodo.push_back( 4 );
        myTodo.push_back( 5 );
        break;
    case 2:
        myTodo.push_back( 3 );
        myTodo.push_back( 4 );
        myTodo.push_back( 5 );
        myTodo.push_back( 2 );
        break;
    case 3:
        myTodo.push_back( 5 );
        myTodo.push_back( 4 );
        myTodo.push_back( 3 );
        myTodo.push_back( 2 );
        break;
    case 4:
        myTodo.push_back( 4 );
        myTodo.push_back( 2 );
        myTodo.push_back( 5 );
        myTodo.push_back( 3 );
        break;
    case 5:
        myTodo.push_back( 2 );
        myTodo.push_back( 5 );
        myTodo.push_back( 3 );
        myTodo.push_back( 4 );
        break;
    case 7:
        myTodo.push_back( 3 );
        myTodo.push_back( 2 );
        myTodo.push_back( 5 );
        myTodo.push_back( 4 );
        break;
    }
    myDone = 0;
}

int cJob::NextTask()
{
    // check for more tasks
    if( ( ! myTodo.size() ) || myDone >= myTodo.size() )
    {
        mySeconds = chrono::duration_cast<chrono::seconds>(
                        std::chrono::steady_clock::now() - myArrival).count();
        std::cout  << "\nJob " << myCount << " complete in " << mySeconds << " secs";
        theTotalSeconds += mySeconds;
        cout <<"( " << myCount << " jobs completed in " << theTotalSeconds << " secs )\n";
        return -1;
    }

    int next = myTodo[ myDone ];
    myDone++;
    return next;
}

int main()
{
    cServer theServer;
    cClient theClient( theServer );
    theClient.Read();
    theClient.Send();
    return 0;
}
