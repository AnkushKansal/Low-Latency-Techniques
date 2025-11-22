#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <ranges>
#include <mutex>
#include <condition_variable>
#include <functional>

//  Importance :
//  Need for increasing clients, worker threads takes time to launch thread
//  Death of thread is a waste of thread lifecycle
//  Thread will not be blocked and moved to other task using non blocking api's

class ThreadPool
{

    unsigned short m_threads;
    std::vector<std::thread> threads_coll;
    std::queue<std::function<void(int)>> tasks_queue;

    std::mutex mu;
    std::condition_variable cond;

    bool finish_interrupt;

public:
    explicit ThreadPool(unsigned short poolsize) noexcept : m_threads(poolsize), finish_interrupt(false)
    {
        for (auto index : std::views::iota((size_t)0, poolsize))
        {
            threads_coll.emplace_back([this]()
                                      { 
                                        while(true) // let not thread die
                                            {
                                                std::unique_lock<std::mutex> u_lock(mu);
                                                cond.wait(u_lock, [this]{ return finish_interrupt || !tasks_queue.empty(); });

                                                if(finish_interrupt)
                                                    return;

                                                auto task = std::move(tasks_queue.front());
                                                tasks_queue.pop();
                                                u_lock.unlock();

                                                task(2);
                                            } });
        }
    }

    void execute() {}
};

int main()
{

    auto pool_size = static_cast<unsigned short>(std::thread::hardware_concurrency());
    std::cout << "H/D cores = " << std::thread::hardware_concurrency() << "\n";

    ThreadPool pool{pool_size};
    pool.execute(some_task);

    return 0;
}