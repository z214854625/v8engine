/**
 * @brief v8引擎类
 * @date 2023-11-15
 * @author ccy
*/
#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <condition_variable>
#include <atomic>
#include <tuple>
#include <functional>

namespace v8 {
    class Isolate;
}

using TaskType = std::tuple<std::string, uint32_t, std::function<void(std::string)>>;
using ResultType = std::tuple<std::function<void(std::string)>, std::string>;

class v8engine
{
public:

    v8engine() = default;
    ~v8engine() = default;

    bool Create(int threadNum, const std::string& script, bool isReboot);

    void Release();

    //添加任务
    void PushTask(TaskType&&);

    //执行脚本
    void V8ExecuteScript(v8::Isolate* isolate, const char* script, int index);

    //检查开始统计
    void StartStat(int );

    //垃圾回收
    void GarbageCollect();

    //打印vm内存信息
    void PrintMemoryInfo();

    //关闭vm
    void CloseVM();

    void GetResult(std::list<ResultType>& listResult);

protected:
    //获取系统毫秒
    int64_t GetMilliSeconds();

    //检查堆栈大小
    void CheckHeapSize(v8::Isolate* isolate, int index);

    //执行GC
    void startGC(v8::Isolate* isolate, int index);

    //初始化v8环境，全局只能一次，重启不能重复调用
    void InitEnv();

private:
    std::vector<std::thread> workers_;
    std::mutex m_mutex;
    std::condition_variable m_condi;
    bool shutdown_;
    std::vector<std::list<TaskType>> tasks_;
    std::string jsScript_;
    std::atomic<int> statTaskNum_;
    int64_t statTick_;
    std::mutex m_mutexResult;
    std::list<ResultType> results_;
};
