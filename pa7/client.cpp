/*
    Based on original assignment by: Dr. R. Bettati, PhD
    Department of Computer Science
    Texas A&M University
    Date  : 2013/01/31
 */


#include <iostream>
#include <fstream>
#include <cstring>
#include <csignal>
#include <string>
#include <sstream>
#include <iomanip>

#include <sys/time.h>
#include <cassert>
#include <assert.h>

#include <cmath>
#include <numeric>
#include <algorithm>

#include <list>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "reqchannel.h"
#include "SafeBuffer.h"
#include "Histogram.h"
using namespace std;

struct vals_passed {
    string name;
    int n;
    SafeBuffer* buff;
};

struct vals_for_workers {
    SafeBuffer* buff;
    SafeBuffer* buffJoe;
    SafeBuffer* buffJohn;
    SafeBuffer* buffJane;
    //Histogram* hist;
    NetworkRequestChannel* req_chan;
};

struct vals_for_stat {
    int which_person;
    SafeBuffer* buff;
    Histogram* hist;
};

string get_time_diff(struct timeval * tp1, struct timeval * tp2) {
  /* Returns a string containing the difference, in seconds and micro seconds, between two timevals. */
  long sec = tp2->tv_sec - tp1->tv_sec;
  long musec = tp2->tv_usec - tp1->tv_usec;
  if (musec < 0) {
    musec += (int)1e6;
    sec--;
  }
  stringstream ss;
  ss<< " [sec = "<< sec <<", musec = "<<musec<< "]";
  return ss.str();
}

void* request_quits(void* arg) {
    for(int i = 0; i < ((vals_passed*)arg)->n; i++) { // should be passed w
        (((vals_passed*)arg)->buff)->push("quit");
    }
    pthread_exit(NULL);
    return (void*)NULL;
}

void* request_thread_function(void* arg) {
    /*
        Fill in this function.

        The loop body should require only a single line of code.
        The loop conditions should be somewhat intuitive.

        In both thread functions, the arg parameter
        will be used to pass parameters to the function.
        One of the parameters for the request thread
        function MUST be the name of the "patient" for whom
        the data requests are being pushed: you MAY NOT
        create 3 copies of this function, one for each "patient".
     */

    for(int i = 0; i < ((vals_passed*)arg)->n; i++) {
        (((vals_passed*)arg)->buff)->push("data "+((vals_passed*)arg)->name);
    }
    pthread_exit(NULL);
    return (void*)NULL;
}

void* worker_thread_function(void* arg) {
    /*
        Fill in this function. 

        Make sure it terminates only when, and not before,
        all the requests have been processed.

        Each thread must have its own dedicated
        RequestChannel. Make sure that if you
        construct a RequestChannel (or any object)
        using "new" that you "delete" it properly,
        and that you send a "quit" request for every
        RequestChannel you construct regardless of
        whether you used "new" for it.
     */

    NetworkRequestChannel* workerChannel = (((vals_for_workers*)arg)->req_chan);

    while(true) {
        string request = (((vals_for_workers*)arg)->buff)->pop();
        //workerChannel->cwrite(request);

        if(request == "quit") { // will be pushed after request threads are joined
            workerChannel->cwrite("!"); // backend logic
            break;
        } else{
            workerChannel->cwrite(request);
            string response = workerChannel->cread();
            if(request == "data Joe Smith"){
                // buff Joe
                (((vals_for_workers*)arg)->buffJoe)->push(response);
            } else if(request == "data John Smith"){
                // buff John
                (((vals_for_workers*)arg)->buffJohn)->push(response);
            } else if(request == "data Jane Smith"){
                // buff Jane
                (((vals_for_workers*)arg)->buffJane)->push(response);
            }
            //(((vals_for_workers*)arg)->hist)->update(request, response);
        }
    }
    pthread_exit(NULL);
    return (void*)NULL;
}

void* stat_thread_function(void* arg) {
    /*
        Fill in this function. 

        There should 1 such thread for each person. Each stat thread 
        must consume from the respective statistics buffer and update
        the histogram. Since a thread only works on its own part of 
        histogram, does the Histogram class need to be thread-safe????

     */

    //RequestChannel* workerChannel = (((vals_for_workers*)arg)->req_chan);

    while(true) {
        string response = (((vals_for_stat*)arg)->buff)->pop();
        //workerChannel->cwrite(request);

        if(response == "quit") { // will be pushed after worker threads are joined
            break;
        } else{
            if(((vals_for_stat*)arg)->which_person == 0){ // John
                (((vals_for_stat*)arg)->hist)->update("data John Smith", response);
            } else if(((vals_for_stat*)arg)->which_person == 1){ // Jane
                (((vals_for_stat*)arg)->hist)->update("data Jane Smith", response);
            } else if(((vals_for_stat*)arg)->which_person == 2){ // Joe
                (((vals_for_stat*)arg)->hist)->update("data Joe Smith", response);
            }
        }
    }
    pthread_exit(NULL);
    return (void*)NULL;
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

Histogram hist = Histogram();

void animate_hist(int signa){
    alarm(2);
    system("clear");
    hist.print();
}

int main(int argc, char * argv[]) {
    int n = 100; //default number of requests per "patient"
    int w = 1; //default number of worker threads
    int b = 3 * n; // default capacity of the request buffer, you should change this default
    int opt = 0;
    char* h = "127.0.0.1"; // default to local host
    char* p = "8080"; // default to port 8080
    bool run_local = true; // default to running locally

    char* lcl_hst1 = "localhost";
    char* lcl_hst2 = "127.0.0.1";

    while ((opt = getopt(argc, argv, "n:w:b:i:h:p:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg); //This won't do a whole lot until you fill in the worker thread function
                break;
            case 'b':
                b = atoi(optarg);
                if(b < 3){
                    b = 3;
                }
                break;
            case 'i':
                // only using network request channel so no need
                break;
            case 'h':
                h = optarg;
                if(strcmp(h,lcl_hst1)==0 || strcmp(h,lcl_hst2)==0){
                    cout << "\nClient program assumes that you have not started a server instance and will start one for you at 127.0.0.1" << endl;
                    h = "127.0.0.1";
                    run_local = true;
                } else {
                    cout << "\nClient program assumes that you have started a server instance at " << h << endl;
                    run_local = false;
                }
                break;
            case 'p':
                p = optarg;
                break;
        }
    }
    int pid = -1;
    if(run_local){
        pid = fork();
    }
    if (pid == 0 && run_local){
        const char* send_it = p;
        //cout << send_it << endl;
        execl("dataserver", "dataserver", send_it, (char*) NULL);
    }
    else {
        string temp_crap_never = "";
        cout << "\n\n\n" << endl << "Please confirm by typing Y" << endl;
        cin >> temp_crap_never;

        hist = Histogram(); // just for safety
        signal(SIGALRM, animate_hist);
        alarm(2);

        struct timeval tp_start, tp_end; /* Used to compute elapsed time. */
        gettimeofday(&tp_start, 0); // start timer

        const int NUM_PATIENTS = 3;
        //cout << "n == " << n << endl;
        //cout << "w == " << w << endl;

        //cout << "CLIENT STARTED:" << endl;
        //cout << "Establishing control channel... " << flush;
        NetworkRequestChannel *chan = new NetworkRequestChannel("control", RequestChannel::CLIENT_SIDE, h, p);
        //cout << "done." << endl<< flush;

        int size_of_buffer = b;
        SafeBuffer request_buffer(size_of_buffer);
        int stat_buff_size = size_of_buffer / 3;
        SafeBuffer r_buffJoe(stat_buff_size);
        SafeBuffer r_buffJohn(stat_buff_size);
        SafeBuffer r_buffJane(stat_buff_size);

        ///////////////////////////////////////////////////////////// ADDING REQUESTS

        vals_passed temp_vals[NUM_PATIENTS];
        string patients[NUM_PATIENTS] = {"Joe Smith", "John Smith", "Jane Smith"};
        pthread_t patients_thread[NUM_PATIENTS];
        int ret_val;

        for(int i = 0; i < NUM_PATIENTS; i++) {
            temp_vals[i].n = n;
            temp_vals[i].buff = &request_buffer;
            temp_vals[i].name = patients[i];
            ret_val =  pthread_create(&patients_thread[i], NULL, &request_thread_function, (void*)&temp_vals[i]); // might be a problem using same temp vals
            if(ret_val != 0){
                cerr << "Seems like creating thread for " << patients[i] << " failed" << endl;
            }
            //request_buffer.push("data John Smith");
            //request_buffer.push("data Jane Smith");
            //request_buffer.push("data Joe Smith");
        }

        ///////////////////////////////////////////////////////////// ADDING STAT THREADS

        vals_for_stat temp_pats[NUM_PATIENTS];
        SafeBuffer* pats_buffs[NUM_PATIENTS] = {&r_buffJohn, &r_buffJane, &r_buffJoe};
        pthread_t patients_stat_thread[NUM_PATIENTS];
        for(int i = 0; i < NUM_PATIENTS; i++) {
            temp_pats[i].which_person = i;
            temp_pats[i].buff = pats_buffs[i];
            temp_pats[i].hist = &hist;
            ret_val =  pthread_create(&patients_stat_thread[i], NULL, &stat_thread_function, (void*)&temp_pats[i]); // might be a problem using same temp vals
            if(ret_val != 0){
                cerr << "Seems like creating thread for stat " << i << " failed" << endl;
            }
            //request_buffer.push("data John Smith");
            //request_buffer.push("data Jane Smith");
            //request_buffer.push("data Joe Smith");
        }

        ///////////////////////////////////////////////////////////// Create workers
        /*
        chan->cwrite("newchannel");
        string s = chan->cread ();
        RequestChannel *workerChannel = new RequestChannel(s, RequestChannel::CLIENT_SIDE);

        while(true) {
            string request = request_buffer.pop();
            workerChannel->cwrite(request);

            if(request == "quit") {
                delete workerChannel;
                break;
            }else{
                string response = workerChannel->cread();
                hist.update (request, response);
            }
        }*/

        pthread_t worker_threads[w];
        vector<NetworkRequestChannel*> workerChannels;
        vals_for_workers* temp_worker_vals = new vals_for_workers[w];
        bool no_error = true;

        //cout << "creating worker threads" << endl;

        for(int i = 0; i < w; i++) {
            // create workers
            try{
                temp_worker_vals[i].buff = &request_buffer;
                // TO DO: add in buffer for each patient
                temp_worker_vals[i].buffJoe = &r_buffJoe;
                temp_worker_vals[i].buffJohn = &r_buffJohn;
                temp_worker_vals[i].buffJane = &r_buffJane;
                //temp_worker_vals[i].hist = &hist;
                //chan->cwrite("newchannel");
                //string s = chan->cread();
                workerChannels.push_back(new NetworkRequestChannel("none", RequestChannel::CLIENT_SIDE, h, p));
                temp_worker_vals[i].req_chan = workerChannels[i];
                ret_val =  pthread_create(&worker_threads[i], NULL, &worker_thread_function, (void*)&temp_worker_vals[i]); // might be a problem using same temp vals
                if(ret_val != 0){
                    cerr << "Seems like creating thread for worker_thread[" << i << "] failed" << endl;
                    throw runtime_error("Well you need more file space...");
                }
            } catch(exception& e){
                execl("rmv", (char*) NULL);
                no_error = false;
                break;
            }
        }


        ///////////////////////////////////////////////////////////// JOINING REQUESTS
        for(int i = 0; i < NUM_PATIENTS; i++) {
            // confirm threads are done
            if(pthread_join(patients_thread[i], NULL) != 0){
                cerr << "something went wrong with the join on request threads " << i << endl;
            }
        }

        //cout << "Done populating request buffer" << endl;

        ///////////////////////////////////////////////////////////// ADDING QUIT REQUESTS FOR WORKERS
        vals_passed temp_val;
        //cout << "approx: " << request_buffer.size() << " should be less than or equal to " <<  size_of_buffer << endl;
        //cout << "Pushing quit requests... ";
        temp_val.n = w; // for quit requests
        temp_val.buff = &request_buffer;
        temp_val.name = "not needed";
        pthread_t quit_thread;

        ret_val =  pthread_create(&quit_thread, NULL, &request_quits, (void*)&temp_val); // might be a problem using same temp vals
        if(ret_val != 0){
            cerr << "Seems like creating thread for quit failed" << endl;
        }
        /*for(int i = 0; i < w; ++i) {
            request_buffer.push("quit");
        }*/
        //cout << "done." << endl;

        ///////////////////////////////////////////////////////////// JOINING QUIT REQUEST FOR WORKERS
        if(no_error && pthread_join(quit_thread, NULL) != 0){
            cerr << "something went wrong with the join on quit request threads " << endl;
        }

        ///////////////////////////////////////////////////////////// JOINING WORKERS
        for(int i = 0; i < w; i++) {
            // confirm worker threads are done
            if(no_error && pthread_join(worker_threads[i], NULL) != 0){
                cerr << "something went wrong with the join on worker thread: " << i << endl;
            }
        }
        gettimeofday(&tp_end, 0);   // stop timer
        chan->cwrite("!!"); // custom backend logic to kill the server and the host
        delete chan;
        ///////////////////////////////////////////////////////////// ADDING QUIT REQUESTS FOR STATS

        // possib error, just make unsafe method because these below should be thread safe
        r_buffJohn.push("quit"); // for quit requests
        r_buffJane.push("quit"); // for quit requests
        r_buffJoe.push("quit"); // for quit requests

        ///////////////////////////////////////////////////////////// JOINING FOR STATS
        
        for(int i = 0; i < NUM_PATIENTS; i++) {
            // confirm threads are done
            if(pthread_join(patients_stat_thread[i], NULL) != 0){
                cerr << "something went wrong with the join on stat threads " << i << endl;
            }
        }    
        ///////////////////////////////////////////////////////////// DONE
        

        delete[] temp_worker_vals;
        for(int i = 0; i < workerChannels.size(); i++) {
            delete workerChannels[i];
        }
        //cout << "All Done!!!" << endl; 
        system("clear");
        hist.print();
        cout<< get_time_diff(&tp_start, &tp_end) << endl; // <<"Time taken: "

        cout << "\n" << endl << "Please allow time (30s) before using the same port again as it is in the process of clearing up memory and exitting server process with exit(0) and the port needs time to open again as it propegates through the system.\n" << endl;
    }
    return 0;
}
