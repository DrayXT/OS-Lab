#include <iostream>
#include <mutex>

using namespace std;

class Monitor
{
    mutex mtx;
    condition_variable cv;
    bool event_ready = false;
    int event_id = 0;

public:
    void producerEvent()
    {
        while (true)
        {
            this_thread::sleep_for(chrono::seconds(1));
            unique_lock<mutex> lock(mtx);
            event_ready = true;
            ++event_id;
            cout << "Producer sent event " << event_id << endl;
            cv.notify_one();
        }
    }

    void consumerEvent()
    {
        while (true)
        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [this] { return event_ready; });
            cout << "Consumer received event " << event_id << endl << endl;
            event_ready = false;
        }
    }
};

void main()
{
    Monitor monitor;

    thread producer(&Monitor::producerEvent, &monitor);
    thread consumer(&Monitor::consumerEvent, &monitor);

    producer.join();
    consumer.join();
}
