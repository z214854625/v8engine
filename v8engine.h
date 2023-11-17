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
#include "v8-isolate.h"

class v8engine
{
public:
    v8engine() = default;
    ~v8engine() = default;

    bool Create(int);

    void Release();

    //添加任务
    void PushTask(const std::string&, int);

    //工作线程函数
    void WorkerRoutine(int );

    void V8ExecuteScript(v8::Isolate* isolate, const char* script, int index);

protected:
    //读取文件
    std::string ReadFile(const std::string &filename);

private:
    std::vector<std::thread> threads_;
    std::mutex m_mutex;
    std::condition_variable m_condi;
    bool shutdown_;
    std::vector<std::list<std::string>> tasks_;
    std::string jsScript_;
};
