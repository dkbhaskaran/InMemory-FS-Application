#ifndef PERF_H_
#define PERF_H_

#include <iostream>

#if defined _WIN64 || defined _WIN32
#include <windows.h>
#elif defined __linux__
#include <time.h>
#endif

#if defined _WIN64 || defined _WIN32
struct ProfileImpl {
  double PCFreq = 1.0;

  ProfileImpl() {
    LARGE_INTEGER li;
    if (!QueryPerformanceFrequency(&li)) {
      std::cerr << "Failed to get performance frequency\n";
      exit(1);
    }
    PCFreq = li.QuadPart / 1e6;
  }

  double getTime() {
    LARGE_INTEGER li;
    if (!QueryPerformanceCounter(&li)) {
      std::cerr << "Failed to get time\n";
      exit(1);
    }

    return li.QuadPart/PCFreq;
  }
};
#elif defined __linux__
struct ProfileImpl {
  double PCFreq = 1.0;

  ProfileImpl() {
    struct timespec Res;
    if (clock_getres(CLOCK_MONOTONIC_RAW, &Res)) {
      std::cerr << "Failed to get performance frequency\n";
      exit(1);
    }

    PCFreq = (Res.tv_sec * 1e9 + Res.tv_nsec) * 1e3;
  }

  double getTime() {
    struct timespec Time;
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &Time)) {
      std::cerr << "Failed to get time\n";
      exit(1);
    }

    return (Time.tv_sec * 1e9 + Time.tv_nsec)/PCFreq;
  }
};
#endif

struct Profile : public ProfileImpl {
  int Calls = 0;
  std::string Tag;
  bool isUsed = false;
  double StartTime;
  double TotalDuration = 0.0;

  Profile(const std::string Name = "") : Tag(Name) { }

  void Start() {
    isUsed = true;
    StartTime = getTime();
  }

  void Stop() {
    TotalDuration += (getTime() - StartTime);
    Calls++;
  }

  ~Profile() {
    if (isUsed)
      std::cout << Tag << " took " << TotalDuration / Calls << " us per call"
                << std::endl;
  }
};

#endif //PERF_H_
