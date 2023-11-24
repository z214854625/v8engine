#include "v8engine.h"
#include "libplatform/libplatform.h"
#include "v8.h"
#include "base64.h"

using namespace std;
using namespace v8;

std::unique_ptr<v8::Platform> v8platform; //必须是全局的变量
const string target_func_name = "onReceiveBattleRsp";

void V8ConsoleMessageCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::HandleScope handle_scope(args.GetIsolate());
    v8::String::Utf8Value message(args.GetIsolate(), args[0].As<v8::String>());
    printf("JavaScript Console.log: %s\n", *message);
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
        
        std::cout << "JavaScript Syntax Error in " << *filename << " at line " << linenum << ", column " << start << "-" << end << std::endl;
        
        v8::String::Utf8Value sourceline(isolate, message->GetSourceLine(isolate->GetCurrentContext()).ToLocalChecked());
        std::cout << *sourceline << std::endl;
        
        for (int i = 0; i < start; i++) {
            std::cout << " ";
        }
        
        for (int i = start; i < end; i++) {
            std::cout << "^";
        }
        
        std::cout << std::endl;

        v8::String::Utf8Value stack_trace(isolate, trycatch->StackTrace(isolate->GetCurrentContext()).ToLocalChecked());
        if (stack_trace.length() > 0) {
            std::cout << *stack_trace << std::endl;
        }
    }
}

void V8PrintHeapStats(v8::Isolate* isolate, int index) {
    v8::HeapStatistics heap_stats;
    isolate->GetHeapStatistics(&heap_stats);
    std::cout << "V8PrintHeapStats index=" << index << " -----------------------------" << std::endl;
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
    V8PrintHeapStats(isolate, index);
    //设置console.log回调
    {
        v8::Local<v8::Object> globalObj = context->Global();
        v8::Local<v8::Object> console = v8::Object::New(isolate);
        auto b1 = globalObj->Set(context, v8::String::NewFromUtf8(isolate, "console").ToLocalChecked(), console);

        v8::Local<v8::FunctionTemplate> error_template = v8::FunctionTemplate::New(isolate, V8ConsoleMessageCallback);
        v8::Local<v8::Function> error_function = error_template->GetFunction(context).ToLocalChecked();
        auto b2 = console->Set(context, v8::String::NewFromUtf8(isolate, "log").ToLocalChecked(), error_function);
        b2 = console->Set(context, v8::String::NewFromUtf8(isolate, "error").ToLocalChecked(), error_function);
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
        std::cout << "objValue2 empty! " << std::endl;
        return;
    }
    v8::Local<v8::Object> goObj = objValue2.As<v8::Object>();
    v8::Local<v8::Value> funcValue;
    auto b2 = goObj->Get(context, v8::String::NewFromUtf8(isolate, "onReceiveBattleRsp").ToLocalChecked()).ToLocal(&funcValue);
    if(funcValue.IsEmpty()) {
        std::cout << "funcValue empty! " << std::endl;
        return;
    }

    if (funcValue->IsFunction()) {
        std::cout << "run script! index= " << index << std::endl;
        v8::Local<v8::Object> funcObj = funcValue.As<v8::Object>();
        //执行任务
        while (true)
        {
            string str;
            {
                //出作用域自动解锁无需调用unlock()
                std::unique_lock<std::mutex> guard(m_mutex);
                int i = index % tasks_.size();
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
                str = std::move(tasks_[i].front());
                tasks_[i].pop_front();
            }
            if(str == "showmem") {
                V8PrintHeapStats(isolate, index);
                continue;
            }
            if(str == "gc") {
                startGC(isolate, index);
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
                    std::cout << "Call result: " << *(v8::String::Utf8Value(isolate, fresult.ToLocalChecked())) << " statTaskNum_="<< statTaskNum_ << std::endl;
                    if(statTaskNum_ > 0 && (--statTaskNum_ <= 0)) {
                        std::cout << "all task done!!!!!!!!!!!!!!!!!!! cost=" << (GetMilliSeconds() - statTick_) << std::endl;
                    }
                }
                else {
                    v8::String::Utf8Value utf8Value(isolate, trycatch.Message()->Get());
                    std::cout << "call function didn't return a value. exception: " << *utf8Value << std::endl;
                }
                //检查一下堆栈大小
                CheckHeapSize(isolate, index);
            }

        }
    }
    else {
        std::cout << target_func_name.c_str() << " not a find function! index= " << index << std::endl;
    }
    std::cout << "thread done! index=" << index << std::endl;
}

// 加载文件
std::string v8engine::ReadFile(const std::string &filename)
{
	std::ifstream input(filename);
	if (!input.is_open()) {
		return "";
	}
	std::string content((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));
	input.close();
	return content;
}

bool v8engine::Create(int threadNum)
{
    //初始化数据
    shutdown_ = false;
    //读取js脚本
    jsScript_.clear();
    jsScript_ = ReadFile("./battle.js");
    if(jsScript_.empty()) {
        std::cout << "jsScript empty!" << std::endl;
        return false;
    }
    //jsScript_ = "function onReceiveBattleRsp(arg) { return arg + ' with from JS';}";
    //初始化v8
    v8::V8::InitializeICUDefaultLocation("");
    v8::V8::InitializeExternalStartupData("");
    v8platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(v8platform.get());
    v8::V8::Initialize();

    //启动工作线程
    tasks_.resize(threadNum, std::list<string>());
    for(int i = 0; i < threadNum; i++) {
        std::thread t1([this, i]() {
            std::cout << "thread run!" << i << std::endl;
            v8::Isolate::CreateParams create_params;
            // create_params.constraints.set_max_old_generation_size_in_bytes((256*1024*1024));
            // create_params.constraints.set_max_young_generation_size_in_bytes((128*1024*1024));
            create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
            v8::Isolate* isolate = v8::Isolate::New(create_params);
            V8ExecuteScript(isolate, jsScript_.c_str(), i+1);
            isolate->Dispose();
            delete create_params.array_buffer_allocator;
        });
        threads_.push_back(std::move(t1));
    }

    return true;
}

void v8engine::Release()
{
	shutdown_ = true;
    m_condi.notify_all();
	for (auto& t : threads_) {
		t.join();
	}
	threads_.clear();
    //清理v8资源
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    v8platform.reset();
    std::cout << "v8 engine release!" << std::endl;
}

void v8engine::PushTask(const std::string& str, int index)
{
    if(index < 0) {
        std::cout << "PushTask error! index=" << index << std::endl;
        return;
    }
    std::unique_lock<std::mutex> guard(m_mutex);
    int i = index % tasks_.size();
    tasks_[i].push_back(str);
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

void v8engine::GarbageCollect()
{
    for(int i = 1; i <= threads_.size(); i++) {
        PushTask(std::string("gc"), i);
    }
}

void v8engine::PrintMemoryInfo()
{
    for(int i = 1; i <= threads_.size(); i++) {
        PushTask(std::string("showmem"), i);
    }
}
