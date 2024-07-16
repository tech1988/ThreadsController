#ifndef THREADSCONTROLLER_H
#define THREADSCONTROLLER_H

#include <thread>
#include <atomic>
#include <functional>
#include <chrono>

#include <iostream>

int workThreadsCount50()
{
    unsigned int count = std::thread::hardware_concurrency();
    if(count == 1) return count;
    return count/2;
}

std::vector<std::vector<int>> makeTasksForThreads(const int listSize, const int threadsCount)
{
    double quotient = static_cast<double>(listSize)/threadsCount;

    std::vector<std::vector<int>> ret;

    if(quotient <= 1.0)
    {
       for(int i = 0; i < listSize; i++) ret.push_back(std::vector<int>{i});
       return ret;
    }

    int taskX = 0, countInThread = quotient;

    for(int threadN = 0; threadN < threadsCount; threadN++)
    {
        std::vector<int> vectorTasks;
        for(int i = 0; i < countInThread; i++, taskX++) vectorTasks.push_back(taskX);
        ret.push_back(std::move(vectorTasks));
    }

    quotient -= countInThread;

    if(quotient > 0)
    {
       for(int i = 0, end = listSize - taskX; i < end; i++, taskX++) ret[i].push_back(taskX);
       return ret;
    }

    return ret;
}

class Controller
{
public:
    explicit Controller(){}
    virtual ~Controller(){}
    virtual bool isStop() const = 0;

    void sleep(std::chrono::milliseconds ms, std::chrono::milliseconds tmSegment = std::chrono::milliseconds(10)) const
    {
         std::chrono::milliseconds sum(0);

         while(!isStop() && sum < ms)
         {
               sum += tmSegment;
               std::this_thread::sleep_for(std::chrono::milliseconds(tmSegment));
         }
    }

    void sleep(std::chrono::seconds s, std::chrono::milliseconds tmSegment = std::chrono::milliseconds(10)) const
    {
         std::chrono::milliseconds sum(0);

         while(!isStop() && sum < s)
         {
               sum += tmSegment;
               std::this_thread::sleep_for(std::chrono::milliseconds(tmSegment));
         }
    }

    void sleep(std::chrono::minutes m, std::chrono::milliseconds tmSegment = std::chrono::milliseconds(10)) const
    {
         std::chrono::milliseconds sum(0);

         while(!isStop() && sum < m)
         {
               sum += tmSegment;
               std::this_thread::sleep_for(std::chrono::milliseconds(tmSegment));
         }
    }

    void sleep(std::chrono::hours h, std::chrono::milliseconds tmSegment = std::chrono::milliseconds(10)) const
    {
         std::chrono::milliseconds sum(0);

         while(!isStop() && sum < h)
         {
               sum += tmSegment;
               std::this_thread::sleep_for(std::chrono::milliseconds(tmSegment));
         }
    }

    void sleep(std::chrono::days s, std::chrono::milliseconds tmSegment = std::chrono::milliseconds(10)) const
    {
         std::chrono::milliseconds sum(0);

         while(!isStop() && sum < s)
         {
               sum += tmSegment;
               std::this_thread::sleep_for(std::chrono::milliseconds(tmSegment));
         }
    }
};

template<typename T, typename V> class ThreadsController : public Controller
{
    std::atomic_uint32_t count;
    std::atomic_bool exit, stopped;

public:
    explicit ThreadsController(){}

    template<typename C> bool run(const C & tasksList, V value, const std::function<void(const Controller &, T, V)> & threadFunc)
    {
        if(tasksList.size() == 0 || !threadFunc || count.load() > 0) return false;

        count = tasksList.size();

        exit = false;
        stopped = false;

        for(T task : tasksList)
        {
            std::thread th(&ThreadsController::executeTask, this, task, value, std::ref(threadFunc));
            th.detach();
        }

        if(count.load() > 0) exit.wait(false);

        return true;
    }

    bool isRunning() const { return count.load() > 0; }
    void stop(){ stopped = true; }
    bool isStop() const override { return stopped.load(std::memory_order_relaxed); }

private:
    static void executeTask(ThreadsController * self, T task, V value, const std::function<void(const Controller &, T, V)> & threadFunc)
    {
        threadFunc(*self, task, value);
        self->count--;

        if(self->count.load() == 0)
        {
           self->exit = true;
           self->exit.notify_one();
        }
    }
};

template<typename V> class ListThreadsController : public Controller
{
    std::atomic_uint32_t count;
    std::atomic_bool exit, stopped;

public:
    explicit ListThreadsController(){}
    ~ListThreadsController(){ exec(); }

    bool add(V value, const std::function<void(const Controller &, V)> & threadFunc)
    {
        if(!threadFunc) return false;

        if(count.load() == 0) stopped = false;
        count++;
        exit = false;

        std::thread th(&ListThreadsController::executeTask, this, value, threadFunc);
        th.detach();

        return true;
    }

    bool isRunning() const { return count.load() > 0; }
    void stop(){ stopped = true; }
    bool isStop() const override { return stopped.load(std::memory_order_relaxed); }

    void exec()
    {
         if(count.load() == 0) return;
         do{ exit.wait(false); }while(count.load() > 0);
    }

private:
    static void executeTask(ListThreadsController * self, V value, const std::function<void(const Controller &, V)> threadFunc)
    {
        threadFunc(*self, value);

        self->count--;

        if(self->count.load() == 0)
        {
           self->exit = true;
           self->exit.notify_one();
        }
    }
};

#endif // THREADSCONTROLLER_H
