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
#include <sys/select.h>
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
    RequestChannel* req_chan;
};

struct val_for_workers {
    SafeBuffer* buff;
    SafeBuffer* buffJoe;
    SafeBuffer* buffJohn;
    SafeBuffer* buffJane;
    //Histogram* hist;
    vector<RequestChannel*> *req_chans;
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

    vector<RequestChannel*> *chans_ptr = (((val_for_workers*)arg)->req_chans);
    int num_chans = chans_ptr->size();
    int cnt_for_quit = 0;
    bool quit_yet = false;
    vector<bool> quit;
    for (int i=0; i<num_chans; i++){
        quit.push_back(false);
    }
    vector<string> requests;
    for (int i=0; i<num_chans; i++){
        requests.push_back("");
    }
    fd_set rfds;
    int rv = 0;
    int n_temp, temps;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 50000;

    while(!quit_yet){
        for(int i = 0; i < num_chans; i++){
            if(!quit[i]){
                if(requests[i] == ""){// possib error, requests
                    requests[i] = (((val_for_workers*)arg)->buff)->pop();
                    chans_ptr->at(i)->cwrite(requests[i]);
                    if(requests[i] == "quit") { // will be pushed after request threads are joined
                        quit[i] = true;
                        cnt_for_quit++;
                        if(cnt_for_quit == num_chans){
                            quit_yet = true;
                        }
                    } 
                }
            }
        }
        if(quit_yet){
            break; // so as to not waste stuff waiting or getting stuck if all have quit
        }
        fd_set rfds;
        FD_ZERO(&rfds);
        n_temp = 0;
        for(int i = 0; i < num_chans; i++){
            if(!quit[i]){
                FD_SET(chans_ptr->at(i)->read_fd(), &rfds);
                if(n_temp < chans_ptr->at(i)->read_fd()){
                    n_temp = chans_ptr->at(i)->read_fd();
                }
            }
        }
        rv = select(n_temp + 1, &rfds, NULL, NULL, &tv);

        if(rv == -1){ // fake it till you make it...
            cerr << "error with select " << n_temp << " " << FD_ISSET(temps, &rfds) << endl;
        } else if(rv == 0){
            cerr << "timeout shouldn't happen" << n_temp << endl;
            quit_yet = true;
        } else{
            for(int i = 0; i < num_chans; i++){
                if(!quit[i]){
                    if(FD_ISSET(chans_ptr->at(i)->read_fd(), &rfds)){ // found a fd that is set
                        string response = chans_ptr->at(i)->cread(); // handle response since it exists
                        if(requests[i] == "data Joe Smith"){
                            // buff Joe
                            (((val_for_workers*)arg)->buffJoe)->push(response);
                        } else if(requests[i] == "data John Smith"){
                            // buff John
                            (((val_for_workers*)arg)->buffJohn)->push(response);
                        } else if(requests[i] == "data Jane Smith"){
                            // buff Jane
                            (((val_for_workers*)arg)->buffJane)->push(response);
                        } else{
                            cerr << "requests was unexpected: " << requests[i]  << " |" << endl;
                        }
                        //cout << response << endl;
                        requests[i] = "";
                    }
                }
            } 
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
    int num_worker_threadss = 3;
    int n = 100; //default number of requests per "patient"
    int w = 1; //default number of worker threads
    int b = 3 * n; // default capacity of the request buffer, you should change this default
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:w:b:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg); //This won't do a whole lot until you fill in the worker thread function
                break;
            case 'b':
                b = atoi (optarg);
                if(b < 3){
                    b = 3;
                }
                break;
        }
    }

    int pid = fork();
	if (pid == 0){
		execl("dataserver", (char*) NULL);
	}
	else {
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
        RequestChannel *chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
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

        vector<pthread_t*> worker_threadss;
        worker_threadss.push_back(new pthread_t);
        vector<RequestChannel*> workerChannels;
        val_for_workers* temp_worker_val = new val_for_workers;
        bool no_error = true;

        if(num_worker_threadss > w){
            num_worker_threadss = w;
        }
        int div_w = w / num_worker_threadss;
        if(div_w < 1){
            div_w = 1;
        }
        


        for(int i = 0; i < w; i++) {
                // create workers
                try{
                    //temp_worker_vals[i].hist = &hist;
                    chan->cwrite("newchannel");
                    string s = chan->cread();
                    workerChannels.push_back(new RequestChannel(s, RequestChannel::CLIENT_SIDE));
                    //cout << workerChannels.at(w_i).at(i) << endl;
                } catch(exception& e){
                    execl("rmv", (char*) NULL);
                    no_error = false;
                    break;
                }
        }
        temp_worker_val->buff = &request_buffer;
        temp_worker_val->buffJoe = &r_buffJoe;
        temp_worker_val->buffJohn = &r_buffJohn;
        temp_worker_val->buffJane = &r_buffJane;
        temp_worker_val->req_chans = &workerChannels;

        cout << "num worker threads: " << num_worker_threadss << "\nnum of chans per: " << div_w << endl;
        
        for(int w_i = 0; w_i < worker_threadss.size(); w_i++){
            ret_val = pthread_create(worker_threadss.at(w_i), NULL, &worker_thread_function, (void*)temp_worker_val); // might be a problem using same temp vals
            if(ret_val != 0){
                cerr << "Seems like creating thread for worker_thread failed" << endl;
                throw runtime_error("Well you need more file space...");
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

        ///////////////////////////////////////////////////////////// JOINING WORKER(S)
        for(int i = 0; i < worker_threadss.size(); i++) {
            // confirm worker threads are done
            if(no_error && pthread_join(*(worker_threadss.at(i)), NULL) != 0){
                cerr << "something went wrong with the join on worker thread" << endl;
            }
        }

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
        gettimeofday(&tp_end, 0);   // stop timer
        delete temp_worker_val;
        for(int i = 0; i < worker_threadss.size(); i++){
            delete worker_threadss.at(i);
        }
        for(int i = 0; i < workerChannels.size(); i++) {
            delete workerChannels.at(i);
        }
        chan->cwrite ("quit");
        delete chan;
        cout << "All Done!!!" << endl; 
		system("clear");
        hist.print();
        cout<< get_time_diff(&tp_start, &tp_end) << endl; // <<"Time taken: "
    }
    return 0;
}
