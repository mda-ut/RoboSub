#include "Sim.h"
#include <opencv2/opencv.hpp>
#include <pthread.h>
#include <unistd.h>

pthread_t* clientThread;
pthread_mutex_t* clientLock;

struct capsule{
    cv::Mat* frame;
    InputHandler* ih;
    bool* runSim;
    bool* threadAlive;
};

void* run(void* cap){
    //Sim sim ((cv::Mat*)cpsl->frame, cpsl->ih, cpsl->runSim, cpsl->threadAlive);
    Sim sim(cap);
    sim.start();
    pthread_exit(NULL);
}

int main()
{
    cv::Mat *f = new cv::Mat(240, 640, CV_8UC3);
    InputHandler* ih = new InputHandler(true);
    dataCap* cap = new dataCap;
    cap->frame = f;
    cap->ih = ih;
    cap->runSim = (bool*)true;
    cap->threadAlive = (bool*)true;

    pthread_t thread;
    int rc = pthread_create(&thread, NULL, run, (void*)cap);
    if (rc)
           std::cout<<"Couldnt create thread"<<std::endl;
    else
        std::cout<<"Created thread"<<std::endl;
    while (1){
        cv::imshow("Frame", *f);
        if (cv::waitKey(1) == 27){
            break;
        }
    }
    cap->runSim = (bool*)false;
    //usleep(1000000);

    while ((bool)cap->threadAlive){
        usleep(1000);
    }
    delete (f);
    delete (ih);
    return 0;
}
