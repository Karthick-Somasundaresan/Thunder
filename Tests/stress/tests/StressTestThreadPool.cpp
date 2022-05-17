/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 Metrological
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "StressTestAdapters.h"
#include "StressTestMacros.h"
#include <algorithm>

namespace WPEFramework{
namespace StressTest{

  class Dispatcher: public Core::ThreadPool::IDispatcher {
    public:
      Dispatcher() = default;
      Dispatcher(const Dispatcher&) = delete;
      Dispatcher& operator=(const Dispatcher&) = delete;
      ~Dispatcher() = default;
      void Initialize() override {}
      void Deinitialize() override {}
      void Dispatch(Core::IDispatch* job) {
        job->Dispatch();
      }
  };
  class Scheduler: public Core::ThreadPool::IScheduler {
    public:
      Scheduler () = default;
      Scheduler (const Scheduler&) = delete;
      Scheduler& operator=(const Scheduler&) = delete;
      ~Scheduler() = default;
      void Schedule(const Core::Time& Time, const Core::ProxyType<Core::IDispatch>& job) {
        job->Dispatch();
      }
  };

  class MyJob;

  struct INotifyJobComplete {
    virtual void JobComplete(MyJob&) = 0;
  };
  class MyJob : public Core::IDispatch {
    public:
      enum class State : uint32_t {
        IDLE,
	SUBMITTED,
	EXECUTING,
	COMPLETED,
	CANCELLED
      };
    public:
      MyJob():_a(0), _state(State::IDLE), _parent(nullptr) {}
      MyJob(INotifyJobComplete* parent): _a(0), _state(State::IDLE), _parent(parent){}
      MyJob& operator=(const MyJob&) = default;
      MyJob(const MyJob&) = default;
      MyJob(int a): _a(a), _state(State::IDLE) {}
      void Dispatch() override{
        std::cout<<"["<< pthread_self()<< "] ["<<_a<< "] Executing Task!!\n";
	_state = State::EXECUTING;
	usleep(1000);
	//sleep(10);
        std::cout<<"["<< pthread_self()<< "] ["<<_a<<"] End Task!!\n";
	_state = State::COMPLETED;
	_parent->JobComplete(*this);
      }
      void SetState(const State state) {
        _state = state;
      }
      State GetState() const { 
      	return _state;
      }
      void SetValue(int val) {
        _a = val;
      }
      int GetValue() const {
      	return _a;
      }
      void SetParent(INotifyJobComplete* parent) {
        _parent = parent;
      }
      bool operator==(const MyJob& other) {
        return (other._a == _a && other._state == _state);
      }
    private:
      int _a;
      State _state;
      INotifyJobComplete* _parent;
  };


  class JobPoolManager: public INotifyJobComplete{
    private:
      JobPoolManager():_jobPool(1), _jobList(), _adminLock() {}
    public:
      static JobPoolManager& Instance() {
        static JobPoolManager jpm;
	return jpm;
      }
      void JobComplete(MyJob& completedJob) override {
	_adminLock.Lock();
        for (std::vector<Core::ProxyType<Core::ThreadPool::JobType<MyJob>>>::iterator iter = _jobList.begin(); iter != _jobList.end() ; ){
	  if(completedJob == (MyJob&)*(*iter)){
	    iter = _jobList.erase(iter);
	    break;
	  }
	  iter++;
	}
	_adminLock.Unlock();
      }

      void PrintPoolStats() {
	std::cout<<"=============== ProxyPool Stat [BEGIN] =================\n";
        std::cout<<"Created Elements: "<<_jobPool.CreatedElements()<<" \n";
        std::cout<<"Queued Elements: "<<_jobPool.QueuedElements()<<"\n";
        std::cout<<"Count: "<<_jobPool.Count()<<"\n";
	std::cout<<"=============== ProxyPool Stat [END] =================\n";
      }

      void PrintAllJobs() {
	std::cout<<"Number of Jobs: "<<_jobList.size()<<"\n";
        for (std::vector<Core::ProxyType<Core::ThreadPool::JobType<MyJob>>>::iterator iter = _jobList.begin(); iter != _jobList.end() ; iter++){
	  std::cout<<"Job id: "<<(uint32_t)(((MyJob&)*(*iter)).GetValue())<<" State:"<<(uint32_t)(((MyJob&)*(*iter)).GetState())<<"\n";
	}
      }
    
      Core::ProxyType<Core::ThreadPool::JobType<MyJob>> GetNewJob() {
        Core::ProxyType<Core::ThreadPool::JobType<MyJob>> job = _jobPool.Element();
	if(job.IsValid()){
	  ((MyJob&)(*job)).SetParent(this);
	  _adminLock.Lock();
          _jobList.push_back(job);
	  _adminLock.Unlock();
          //PrintPoolStats();
	}
	return job;
      }

      void GetSubmittedJob(Core::ProxyType<Core::ThreadPool::JobType<MyJob>>&  submittedJob){
	_adminLock.Lock();
        for (std::vector<Core::ProxyType<Core::ThreadPool::JobType<MyJob>>>::iterator iter = _jobList.begin(); iter !=   _jobList.end(); iter++){
	  if(((MyJob&)*(*iter)).GetState() ==  MyJob::State::SUBMITTED || ((MyJob&)*(*iter)).GetState() ==  MyJob::State::EXECUTING  ){
	    submittedJob = *iter;  
	    break;
	  }
	}
        _adminLock.Unlock();
      }

      void DeleteJob(Core::ProxyType<Core::ThreadPool::JobType<MyJob>>& job){
	_adminLock.Lock();
        for (std::vector<Core::ProxyType<Core::ThreadPool::JobType<MyJob>>>::iterator iter = _jobList.begin(); iter != _jobList.end() ; ){
	  if((MyJob&)(*job) == (MyJob&)*(*iter)){
	    ((MyJob&)(*job)).SetState(MyJob::State::CANCELLED);
	    iter = _jobList.erase(iter);
	    break;
	  }
	  iter++;
	}
	_adminLock.Unlock();
      }

      uint32_t GetSubmittedJobCount() {
        _adminLock.Lock();
	uint32_t submittedCount = 0;
	for (std::vector<Core::ProxyType<Core::ThreadPool::JobType<MyJob>>>::iterator iter = _jobList.begin(); iter != _jobList.end() ; ){
          if(((MyJob&)*(*iter)).GetState() != MyJob::State::EXECUTING) {
	    submittedCount++;
          } 
          iter++;
	}
        _adminLock.Unlock();
	return submittedCount;
      }
    private:
      Core::ProxyPoolType<Core::ThreadPool::JobType<MyJob>> _jobPool;
      std::vector<Core::ProxyType<Core::ThreadPool::JobType<MyJob>>> _jobList;
      Core::CriticalSection _adminLock;
  };
  
  class StressTestThreadPool : public StressTestInterface {
    public:
      StressTestThreadPool(uint32_t count, uint32_t stackSize, uint32_t queueSize, Core::ThreadPool::IDispatcher* dispatcher, Core::ThreadPool::IScheduler* scheduler): _threadPool(count, stackSize, queueSize, dispatcher, scheduler){}
      void IncreaseLoad(uint32_t loadFactor = 0) override {
	static uint32_t jobId = 0;
	//std::cout<<"["<<pthread_self()<<"] Increase Load by factor:"<<loadFactor<<"\n";
	//std::cout<<"Increase Load by factor:"<<loadFactor<<"\n";
        Core::ProxyType<Core::ThreadPool::JobType<MyJob>> myJob ;
	while (loadFactor > 0) {
	  myJob = JobPoolManager::Instance().GetNewJob();
	  ((MyJob&)(*myJob)).SetValue(jobId++);
	  ((MyJob&)(*myJob)).SetState(MyJob::State::SUBMITTED);
	  //std::cout<<"Submitting Job id:"<<jobId<<"\n";
	  _threadPool.Submit(myJob->Submit(), 0);
	  loadFactor--;
	}
      }

      void DecreaseLoad(uint32_t loadFactor = 0) override {
	//std::cout<<"["<<pthread_self()<<"] Decrease Load by factor:"<<loadFactor<<"\n";
	Core::ProxyType<Core::ThreadPool::JobType<MyJob>> myJob;
        while(loadFactor > 0 ){

          //std::cout<<"["<<pthread_self()<<"] Submitted JobCount: "<<JobPoolManager::Instance().GetSubmittedJobCount()<<"\n";
          if(JobPoolManager::Instance().GetSubmittedJobCount() > 0) {
            //std::cout<<"Submitted jobCount: "<<JobPoolManager::Instance().GetSubmittedJobCount()<<'\n';
	    JobPoolManager::Instance().GetSubmittedJob(myJob);
	    if(myJob.IsValid()){
	      //std::cout<<"Revoking Job: "<<((MyJob&)(*myJob)).GetValue()<<"\n";
	      _threadPool.Revoke(myJob->Revoke(), 0);
	      JobPoolManager::Instance().DeleteJob(myJob);
	      myJob.Release();
	    } else {
	      //std::cout<<"Got invalid Job to Revoke\n";
	    }
	  } else {
            //std::cout<<"There are no Submitted jobs\n";
            break;
          }

	  loadFactor--;
	}
      }
      void MaxLoad() override {
        IncreaseLoad(ConfigReader::Instance().ThreadPoolMaxJobThreshold());
      }
      void NoLoad() override {
        DecreaseLoad(JobPoolManager::Instance().GetSubmittedJobCount()); 
      }
      string GetClassName() const override { return "StressTestThreadPool";}
      void Cleanup() override {
	std::cout<<"Cleanup Called\n";
        NoLoad();
        std::cout<<"Cleanup Completed\n";
      }
      bool Validate() override {return true;}
    private:
      Core::ThreadPool _threadPool;
  };
  Dispatcher dispatcher;
  Scheduler scheduler;


  //LOAD_TEST_WITH_CUSTOM_THREADS(4, StressTestThreadPool, 4, 0, 1024, &dispatcher, &scheduler);
  //LOAD_TEST(StressTestThreadPool, 4, 0, 1024, &dispatcher, &scheduler);
  STRESS_TEST(StressTestThreadPool, 4, 0, 1024, &dispatcher, &scheduler);
  
}//StressTest
}//WPEFrameworCriticalSectionk
