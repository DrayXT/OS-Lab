#include <iostream>
#include <mutex>

using namespace std;

class Monitor
{
    mutex mtx;
    condition_variable cv_producer, cv_consumer;
    bool event_ready = false, event_processed = true;
    int event_id = 0;

public:
    void producerEvent()
    {
        while (true)
        {
            this_thread::sleep_for(chrono::seconds(1));
            unique_lock<mutex> lock(mtx);
            cv_producer.wait(lock, [this] { return event_processed; });
            ++event_id;
            event_ready = true;
            event_processed = false;
            cout << "Producer sent event " << event_id << endl;
            cv_consumer.notify_one();
        }
    }

    void consumerEvent()
    {
        while (true)
        {
            unique_lock<mutex> lock(mtx);
            cv_consumer.wait(lock, [this] { return event_ready; });
            cout << "Consumer received event " << event_id << endl << endl;
            event_processed = true;
            event_ready = false;
            cv_producer.notify_one();
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
