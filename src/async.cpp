#include "icelander.hpp"
#include <stdexcept>

namespace icelander {
    namespace async {
        auto TaskScheduler::instance() -> TaskScheduler& {
            static TaskScheduler instance;
            return instance;
        }

        void TaskScheduler::schedule(std::function<void()> task) {
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                taskQueue_.push(std::move(task));
            }
            queueCv_.notify_one();
        }

        void TaskScheduler::start() {
            if (running_) {
                return;
            }

            running_ = true;

            auto hwThreads = std::thread::hardware_concurrency();
            size_t threadCount = hwThreads > 1 ? hwThreads : 1;
            workerThreads_.reserve(threadCount);

            for (size_t i = 0; i < threadCount; ++i) {
                workerThreads_.emplace_back(std::make_unique<std::thread>(&TaskScheduler::workerLoop, this));
            }
        }

        void TaskScheduler::stop() {
            if (!running_) {
                return;
            }

            running_ = false;
            queueCv_.notify_all();

            for (auto& thread : workerThreads_) {
                if (thread && thread->joinable()) {
                    thread->join();
                }
            }
            workerThreads_.clear();
        }

        bool TaskScheduler::isRunning() const {
            return running_;
        }

        void TaskScheduler::workerLoop() {
            while (running_) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queueMutex_);
                    queueCv_.wait(lock, [this] { return !taskQueue_.empty() || !running_; });

                    if (!running_) {
                        break;
                    }

                    if (!taskQueue_.empty()) {
                        task = std::move(taskQueue_.front());
                        taskQueue_.pop();
                    }
                }

                if (task) {
                    task();
                }
            }
        }

        auto sleepMs(int milliseconds) -> void {
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        }
    }
}