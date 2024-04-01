#include "v8engine.h"
#include "libplatform/libplatform.h"
#include "v8.h"

using namespace std;
using namespace v8;

std::unique_ptr<v8::Platform> v8platform; //必须是全局的变量
const string target_func_name = "onReceiveBattleRsp";

void V8ConsoleMessageCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if(args.Length() == 0) {
        std::cout << "args length 0" << std::endl;
        return;
    }
    v8::HandleScope handle_scope(args.GetIsolate());
    v8::String::Utf8Value message(args.GetIsolate(), args[0].As<v8::String>());
    std::cout << "JS log: " << (*message) << std::endl;
}

void V8PrintException(v8::Isolate* isolate, v8::TryCatch* trycatch) {
    v8::HandleScope handle_scope(isolate);
    v8::String::Utf8Value exception(isolate, trycatch->Exception());
    v8::Local<v8::Message> message = trycatch->Message();
    
    if (message.IsEmpty()) {
        // 没有详细错误信息，只有异常信息
        std::cout << "JavaScript Syntax Error: " << *exception << std::endl;
    } else {
        // 有详细错误信息
        v8::String::Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());
        int linenum = message->GetLineNumber(isolate->GetCurrentContext()).FromJust();
        int start = message->GetStartColumn(isolate->GetCurrentContext()).FromJust();
        int end = message->GetEndColumn(isolate->GetCurrentContext()).FromJust();

        v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(isolate->GetCurrentContext()).ToLocalChecked());
        std::cout << "JS Syntax Error! filename: " << *filename << " line: " << linenum << " column: " << start << " sourceline=" << *sourceline << std::endl;

        v8::String::Utf8Value stack_trace(isolate, trycatch->StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
        if (stack_trace.length() > 0) {
            std::cout << "stack trace: " << *stack_trace << std::endl;
        }
    }
}

void V8PrintHeapStats(v8::Isolate* isolate, int index) {
    v8::HeapStatistics heap_stats;
    isolate->GetHeapStatistics(&heap_stats);
    std::cout << "V8PrintHeapStats index= " << index << " -----------------------------" << std::endl;
    std::cout << "Heap size limit: " << heap_stats.heap_size_limit() / 1024 << " KB" << std::endl;
    std::cout << "Total heap size: " << heap_stats.total_heap_size() / 1024 << " KB" << std::endl;
    std::cout << "Total heap size executable: " << heap_stats.total_heap_size_executable() / 1024 << " KB" << std::endl;
    std::cout << "Total physical size: " << heap_stats.total_physical_size() / 1024 << " KB" << std::endl;
    std::cout << "Used heap size: " << heap_stats.used_heap_size() / 1024 << " KB" << std::endl;
}

void v8engine::V8ExecuteScript(v8::Isolate* isolate, const char* jscode, int index) {
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);
    //V8PrintHeapStats(isolate, index);
    //设置console.log回调
    {
        v8::Local<v8::Object> globalObj = context->Global();
        v8::Local<v8::Object> console = v8::Object::New(isolate);
        auto b1 = globalObj->Set(context, v8::String::NewFromUtf8(isolate, "console").ToLocalChecked(), console);

        v8::Local<v8::FunctionTemplate> error_template = v8::FunctionTemplate::New(isolate, V8ConsoleMessageCallback);
        v8::Local<v8::Function> error_function = error_template->GetFunction(context).ToLocalChecked();
        b1 = console->Set(context, v8::String::NewFromUtf8(isolate, "log").ToLocalChecked(), error_function);
        b1 = console->Set(context, v8::String::NewFromUtf8(isolate, "error").ToLocalChecked(), error_function);
        b1 = console->Set(context, v8::String::NewFromUtf8(isolate, "warn").ToLocalChecked(), error_function);
    }

    //编译并执行js脚本
    v8::TryCatch trycatch(isolate);
    v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, jscode, v8::NewStringType::kNormal).ToLocalChecked();
    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(context, source).ToLocal(&script)) {
        V8PrintException(isolate, &trycatch);
        return;
    }
    v8::Local<v8::Value> result;
    if (!script->Run(context).ToLocal(&result)) {
        V8PrintException(isolate, &trycatch);
        return;
    }
    v8::Local<v8::Value> objValue2;
    auto b1 = context->Global()->Get(context, v8::String::NewFromUtf8(isolate, "goCallJs").ToLocalChecked()).ToLocal(&objValue2);
    if(objValue2.IsEmpty()) {
        std::cout << ("objValue2 empty!") << std::endl;
        return;
    }
    v8::Local<v8::Object> goObj = objValue2.As<v8::Object>();
    v8::Local<v8::Value> funcValue;
    auto b2 = goObj->Get(context, v8::String::NewFromUtf8(isolate, "onReceiveBattleRsp").ToLocalChecked()).ToLocal(&funcValue);
    if(funcValue.IsEmpty()) {
        std::cout << ("funcValue empty!") << std::endl;
        return;
    }

    if (funcValue->IsFunction()) {
        std::cout << "run script! index=" << index << std::endl;
        v8::Local<v8::Object> funcObj = funcValue.As<v8::Object>();
        //执行任务
        while (true)
        {
            tuple<string, uint32_t, std::function<void(string)>> tu;
            {
                //出作用域自动解锁无需调用unlock()
                std::unique_lock<std::mutex> guard(m_mutex);
                int i = index;
                m_condi.wait(guard, [this, i]() {
                    return (shutdown_ || !tasks_[i].empty());
                } );
                if (shutdown_) {
                    std::cout << "shutdown ntf! index=" << index << std::endl;
                    break;
                }
                if(tasks_[i].empty()) {
                    continue;
                }
                tu.swap(tasks_[i].front());
                tasks_[i].pop_front();
            }
            const string& str = std::get<0>(tu);
            auto& callback = std::get<2>(tu);
            if(str.empty()) {
                std::unique_lock<std::mutex> guard(m_mutexResult);
                results_.push_back({std::move(callback), std::move(str)});
                continue;
            }
            if(str == "showmem") {
                V8PrintHeapStats(isolate, index);
                std::unique_lock<std::mutex> guard(m_mutexResult);
                results_.push_back({std::move(callback), std::move(str)});
                continue;
            }
            if(str == "gc") {
                startGC(isolate, index);
                std::unique_lock<std::mutex> guard(m_mutexResult);
                results_.push_back({std::move(callback), std::move(str)});
                continue;
            }
            //执行一次任务
            {
                //在这个作用域加handlescope管理v8::Local变量
                v8::HandleScope handle_scope1(isolate);
                v8::Local<v8::Value> args[2] = { v8::Integer::New(isolate, 0), v8::String::NewFromUtf8(isolate, str.c_str()).ToLocalChecked() };
                v8::TryCatch trycatch(isolate);
                v8::MaybeLocal<v8::Value> fresult = funcObj->CallAsFunction(context, objValue2, 2, args);
                if (!fresult.IsEmpty()) {
                    auto val = v8::String::Utf8Value(isolate, fresult.ToLocalChecked());
                    string strResult(*val, val.length());
                    //InfoLn("Call result: " << strResult.length() << " statTaskNum_="<< statTaskNum_);
                    if(statTaskNum_ > 0 && (--statTaskNum_ <= 0)) {
                        std::cout << "all task done!!!!!!!!!!!!!!!!!!! cost=" << (GetMilliSeconds() - statTick_) << std::endl;
                    }
                    std::unique_lock<std::mutex> guard(m_mutexResult);
                    results_.push_back({std::move(callback), std::move(strResult)});
                }
                else {
                    v8::String::Utf8Value utf8Value(isolate, trycatch.Message()->Get());
                    std::cout << "call function didn't return a value. exception: " << *utf8Value << std::endl;
                }
                //检查一下堆栈大小
                //CheckHeapSize(isolate, index);
            }

        }
    }
    else {
        std::cout << "not find function! index= " << index << ", " << target_func_name.c_str() << std::endl;
    }
    std::cout << "thread done! index=" << index << std::endl;
}

bool v8engine::Create(int threadNum, const std::string& script, bool isReboot)
{
    //初始化数据
    shutdown_ = false;
    //读取js脚本
    jsScript_ = script;
    //初始化v8
    std::cout << "v8engine::Create isReboot=" << isReboot << " threadNum=" << threadNum << std::endl;
    if(!isReboot) {
        this->InitEnv();
        tasks_.resize(threadNum, std::list<TaskType>());
    }
    //启动工作线程
    for(int i = 0; i < threadNum; i++) {
        std::thread t1([this, i]() {
            v8::Isolate::CreateParams create_params;
            // create_params.constraints.set_max_old_generation_size_in_bytes((256*1024*1024));
            // create_params.constraints.set_max_young_generation_size_in_bytes((128*1024*1024));
            create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
            v8::Isolate* isolate = v8::Isolate::New(create_params);
            V8ExecuteScript(isolate, jsScript_.c_str(), i);
            isolate->Dispose();
            delete create_params.array_buffer_allocator;
        });
        workers_.push_back(std::move(t1));
    }
    return true;
}

void v8engine::Release()
{
    //关闭虚拟机
    this->CloseVM();
    //清理v8资源
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    v8platform.reset();
    std::cout << ("v8 engine release!") << std::endl;
}

void v8engine::PushTask(TaskType&& tu)
{
    std::unique_lock<std::mutex> guard(m_mutex);
    uint32_t index = std::get<1>(tu);
    int i = index % tasks_.size();
    tasks_[i].push_back(std::forward<TaskType>(tu));
    //std::cout << "push task! #str=" << str.length() << ", index= " << index << std::endl;
    m_condi.notify_all();
}

void v8engine::StartStat(int taskNum)
{
    statTaskNum_ = taskNum;
    statTick_ = GetMilliSeconds();
}

int64_t v8engine::GetMilliSeconds(){
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto val = now_ms.time_since_epoch().count();
    return val;
}

void v8engine::CheckHeapSize(v8::Isolate* isolate, int index)
{
    v8::HeapStatistics heap_stats;
    isolate->GetHeapStatistics(&heap_stats);
    size_t heapSize = heap_stats.total_heap_size();
    const size_t configsize = (450*1024*1024); //450M 根据实际测试在默认配置的情况下，超过500m就会触发heap上限从而导致宕机
    if (heapSize > configsize) {
        std::cout << "CheckHeapSize heap over limit!" << heapSize << std::endl;
        startGC(isolate, index);
    }
}

void v8engine::startGC(v8::Isolate* isolate, int index)
{
    int64_t tick1 = GetMilliSeconds();
    isolate->LowMemoryNotification();
    std::cout << "执行gc结束! cost=" << (GetMilliSeconds() - tick1) << ", index=" << index << std::endl;
}

void v8engine::InitEnv()
{
    //初始化v8
    v8::V8::InitializeICUDefaultLocation("");
    v8::V8::InitializeExternalStartupData("");
    v8platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(v8platform.get());
    v8::V8::Initialize();
}

void v8engine::GarbageCollect()
{
    for(size_t i = 1; i <= workers_.size(); i++) {
        PushTask({std::string("gc"), i, [](string){}});
    }
}

void v8engine::PrintMemoryInfo()
{
    for(size_t i = 1; i <= workers_.size(); i++) {
        PushTask({std::string("showmem"), i, [](string){}});
    }
}

void v8engine::CloseVM()
{
	shutdown_ = true;
    m_condi.notify_all();
	for (auto& t : workers_) {
		t.join();
	}
	workers_.clear();
}

void v8engine::GetResult(std::list<ResultType>& listResult)
{
    std::unique_lock<std::mutex> guard(m_mutexResult);
    listResult.swap(results_);
}