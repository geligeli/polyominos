#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_set>
#include <utility>


std::string HumanReadableTime(int64_t ns);

class PeriodicLogger {
public:
  PeriodicLogger(std::function<void()> log);
  void manualLog();
  ~PeriodicLogger();

private:
  std::function<void()> m_log;
  std::condition_variable m_cv;
  std::mutex m_mutex;
  std::jthread m_loggingThread;
};

class TimingLogger {
public:
  static TimingLogger& instance();
  class Timer {
  public:
    void tic();
    void toc();
    explicit Timer(std::atomic<int64_t> &ns_timer);

  private:
    std::chrono::high_resolution_clock::time_point m_start;
    std::atomic<int64_t> *m_ns_timer;
  };

  Timer getTimer(const std::string &name);
  std::string logString();
  TimingLogger();
  ~TimingLogger();

private:
  std::string logStringLocked();

  std::condition_variable m_cv;
  std::mutex m_mutex;
  std::unordered_map<std::string, std::atomic<int64_t>> m_ns_timers;
  std::jthread m_loggingThread;
};
