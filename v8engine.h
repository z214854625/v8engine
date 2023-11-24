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
//#include "v8-isolate.h"

namespace v8 {
    class Isolate;
}

class v8engine
{
public:
    v8engine() = default;
    ~v8engine() = default;

    bool Create(int);

    void Release();

    //添加任务
    void PushTask(const std::string&, int);

    //执行脚本
    void V8ExecuteScript(v8::Isolate* isolate, const char* script, int index);

    //检查开始统计
    void StartStat(int );

    //垃圾回收
    void GarbageCollect();

    //打印vm内存信息
    void PrintMemoryInfo();

protected:
    //读取文件
    std::string ReadFile(const std::string &filename);
    
    //获取系统毫秒
    int64_t GetMilliSeconds();

    //检查堆栈大小
    void CheckHeapSize(v8::Isolate* isolate, int index);

    //执行GC
    void startGC(v8::Isolate* isolate, int index);

private:
    std::vector<std::thread> threads_;
    std::mutex m_mutex;
    std::condition_variable m_condi;
    bool shutdown_;
    std::vector<std::list<std::string>> tasks_;
    std::string jsScript_;
    std::atomic<int> statTaskNum_;
    int64_t statTick_;
};
