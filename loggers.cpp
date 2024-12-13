#include "loggers.hpp"

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

std::string HumanReadableTime(int64_t ns) {
  // round the the most significant unit
  if (ns < 1000) {
    return std::to_string(ns) + "ns";
  }
  ns /= 1000;

  if (ns < 1000) {
    return std::to_string(ns) + "us";
  }
  ns /= 1000;

  if (ns < 1000) {
    return std::to_string(ns) + "ms";
  }
  ns /= 1000;
  if (ns < 60) {
    return std::to_string(ns) + "s";
  }
  ns /= 60;
  return std::to_string(ns) + "m";
}

PeriodicLogger::PeriodicLogger(std::function<void()> log) : m_log(log) {
  m_loggingThread = std::jthread([this](std::stop_token st) {
    while (!st.stop_requested()) {
      std::unique_lock lk(m_mutex);
      m_cv.wait_for(lk, std::chrono::seconds(10),
                    [&st]() { return st.stop_requested(); });
      m_log();
    }
  });
}

void PeriodicLogger::manualLog() {
  std::unique_lock lk(m_mutex);
  m_log();
}

PeriodicLogger::~PeriodicLogger() {
  m_loggingThread.request_stop();
  m_cv.notify_all();
}

void TimingLogger::Timer::tic() {
  m_start = std::chrono::high_resolution_clock::now();
}
void TimingLogger::Timer::toc() {
  m_ns_timer->fetch_add(std::chrono::duration_cast<std::chrono::nanoseconds>(
                            std::chrono::high_resolution_clock::now() - m_start)
                            .count());
}
TimingLogger::Timer::Timer(std::atomic<int64_t> &ns_timer)
    : m_ns_timer(&ns_timer) {}

TimingLogger::Timer TimingLogger::getTimer(const std::string &name) {
  std::unique_lock lk(m_mutex);
  return Timer(m_ns_timers[name]);
}

std::string TimingLogger::logString() {
  std::unique_lock lk(m_mutex);
  return logStringLocked();
}

TimingLogger::TimingLogger() {
  m_loggingThread = std::jthread([this](std::stop_token st) {
    while (!st.stop_requested()) {
      std::unique_lock lk(m_mutex);
      m_cv.wait_for(lk, std::chrono::seconds(10),
                    [&st]() { return st.stop_requested(); });
      auto s = logStringLocked();
      (void)write(1, s.c_str(), s.size());
    }
  });
}

TimingLogger::~TimingLogger() {
  m_loggingThread.request_stop();
  m_cv.notify_all();
}

std::string TimingLogger::logStringLocked() {
  std::stringstream ss;
  for (const auto &[name, timer] : m_ns_timers) {
    ss << name << HumanReadableTime(timer.load()) << " ";
  }
  return ss.str();
}

TimingLogger& TimingLogger::instance() {
    static TimingLogger logger;
    return logger;
}