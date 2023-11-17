#include "v8engine.h"
#include "libplatform/libplatform.h"
#include "v8.h"

using namespace std;
using namespace v8;

std::unique_ptr<v8::Platform> v8platform; //必须是全局的变量
const string target_func_name = "onReceiveBattleRsp";

void V8ConsoleMessageCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::HandleScope handle_scope(args.GetIsolate());
    v8::String::Utf8Value message(args.GetIsolate(), args[0].As<v8::String>());
    printf("JavaScript Console: %s\n", *message);
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

void v8engine::V8ExecuteScript(v8::Isolate* isolate, const char* jscode, int index) {
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);

    //设置console.log回调
    {
        v8::Local<v8::Object> globalObj = context->Global();
        v8::Local<v8::Object> console = v8::Object::New(isolate);
        auto b1 = globalObj->Set(context, v8::String::NewFromUtf8(isolate, "console").ToLocalChecked(), console);

        v8::Local<v8::FunctionTemplate> error_template = v8::FunctionTemplate::New(isolate, V8ConsoleMessageCallback);
        v8::Local<v8::Function> error_function = error_template->GetFunction(context).ToLocalChecked();
        auto b2 = console->Set(context, v8::String::NewFromUtf8(isolate, "log").ToLocalChecked(), error_function);
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
    v8::String::Utf8Value str(isolate, result);
    std::cout << "JavaScript Run Output: " << *str << ", index=" << index << std::endl;
    v8::Local<v8::String> funcName = v8::String::NewFromUtf8(isolate, target_func_name.c_str()).ToLocalChecked();
    v8::Local<v8::Value> func_value = context->Global()->Get(context, funcName).ToLocalChecked();
    if (func_value->IsFunction())
    {
        std::cout << "run script! index= " << index << std::endl;
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
                    break;
                }
                if(tasks_[i].empty()) {
                    continue;
                }
                str = std::move(tasks_[i].front());
                tasks_[i].pop_front();
            }
            //执行一次业务
            v8::Local<v8::Value> func_arg = v8::String::NewFromUtf8(isolate, str.c_str()).ToLocalChecked();
            {
                v8::TryCatch trycatch(isolate);
                v8::Local<v8::Object> func_object = func_value.As<v8::Object>();
                v8::MaybeLocal<v8::Value> func_result = v8::Function::Cast(*func_object)->Call(context, context->Global(), 1, &func_arg);
                if (!func_result.IsEmpty()) {
                    std::cout << "Call result: " << *(v8::String::Utf8Value(isolate, func_result.ToLocalChecked())) << std::endl;
                }
                else {
                    v8::String::Utf8Value utf8Value(isolate, trycatch.Message()->Get());
                    std::cout << "CallAsFunction didn't return a value, exception: " << *utf8Value << std::endl;
                }
            }
        }
    }
    else {
        std::cout << target_func_name.c_str() << " not a find function! index= " << index << std::endl;
    }
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
}

void v8engine::PushTask(const std::string& str, int index)
{
    std::unique_lock<std::mutex> guard(m_mutex);
    int i = index % tasks_.size();
    tasks_[i].push_back(str);
    std::cout << "push task! cc=" << str << std::endl;
    m_condi.notify_all();
}

void v8engine::WorkerRoutine(int index)
{
    std::cout << "Worker start! " << index << std::endl;
    if (tasks_.empty() || index <= 0 || (index > tasks_.size())) {
        std::cout << "index error! " << index << ", tasks_=" << tasks_.size() << std::endl;
        return;
    }
    auto jscode = jsScript_.c_str();
    // Create a new Isolate and make it the current one.
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    {
        //v8::Locker locker(isolate_);
        v8::Isolate::Scope isolate_scope(isolate);
        // Create a stack-allocated handle scope.
        v8::HandleScope handle_scope(isolate);
        // Create a new context.
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        // Enter the context for compiling and running the hello world script.
        v8::Context::Scope context_scope(context);

        //设置 console.error回调
        {
            v8::Local<v8::Object> globalObj = context->Global();
            v8::Local<v8::Object> console = v8::Object::New(isolate);
            auto b1 = globalObj->Set(context, v8::String::NewFromUtf8(isolate, "console").ToLocalChecked(), console);

            v8::Local<v8::FunctionTemplate> error_template = v8::FunctionTemplate::New(isolate, V8ConsoleMessageCallback);
            v8::Local<v8::Function> error_function = error_template->GetFunction(context).ToLocalChecked();
            auto b2 = console->Set(context, v8::String::NewFromUtf8(isolate, "log").ToLocalChecked(), error_function);
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
        std::cout << "run javascript suc! index=" << index  << std::endl;
        // Take a reference to the created JS function and call it with arguments
        v8::Local<v8::String> funcName = v8::String::NewFromUtf8(isolate, "onReceiveBattleRsp").ToLocalChecked();
        v8::Local<v8::Value> func_value = context->Global()->Get(context, funcName).ToLocalChecked();
        if (func_value->IsFunction()) {
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
                        break;
                    }
                    if(tasks_[i].empty()) {
                        continue;
                    }
                    auto str = std::move(tasks_[i].front());
                    tasks_[i].pop_front();
                }
                //执行一次业务
                v8::Local<v8::Value> func_arg = v8::String::NewFromUtf8(isolate, str.c_str()).ToLocalChecked();
                {
                    v8::TryCatch trycatch(isolate);
                    v8::Local<v8::Object> func_object = func_value.As<v8::Object>();
                    v8::MaybeLocal<v8::Value> func_result = v8::Function::Cast(*func_object)->Call(context, context->Global(), 1, &func_arg);
                    if (!func_result.IsEmpty()) {
                        std::cout << "Call result: " << *(v8::String::Utf8Value(isolate, func_result.ToLocalChecked())) << std::endl;
                    }
                    else {
                        v8::String::Utf8Value utf8Value(isolate, trycatch.Message()->Get());
                        std::cout << "CallAsFunction didn't return a value, exception: " << *utf8Value << std::endl;
                    }
                }
            }
        }
        else {
            std::cerr << "onReceiveBattleRsp is not a function! index=" << index << std::endl;
            return;
        }
    }

    isolate->Dispose();
    delete create_params.array_buffer_allocator;
    std::cout << "Worker done! " << index << std::endl;
}
