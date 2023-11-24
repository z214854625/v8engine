// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "libplatform/libplatform.h"
#include "v8.h"

#include "v8engine.h"
#include "base64.h"

using namespace v8;

void WorkerRoutine(int index)
{
    // std::cout << "Worker start! " << index << std::endl;
    // if (tasks_.empty() || index <= 0 || (index > tasks_.size())) {
    //     std::cout << "index error! " << index << ", tasks_=" << tasks_.size() << std::endl;
    //     return;
    // }
    // auto jscode = jsScript_.c_str();
    // // Create a new Isolate and make it the current one.
    // v8::Isolate::CreateParams create_params;
    // create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    // v8::Isolate* isolate = v8::Isolate::New(create_params);
    // {
    //     //v8::Locker locker(isolate_);
    //     v8::Isolate::Scope isolate_scope(isolate);
    //     // Create a stack-allocated handle scope.
    //     v8::HandleScope handle_scope(isolate);
    //     // Create a new context.
    //     v8::Local<v8::Context> context = v8::Context::New(isolate);
    //     // Enter the context for compiling and running the hello world script.
    //     v8::Context::Scope context_scope(context);

    //     //设置 console.error回调
    //     {
    //         v8::Local<v8::Object> globalObj = context->Global();
    //         v8::Local<v8::Object> console = v8::Object::New(isolate);
    //         auto b1 = globalObj->Set(context, v8::String::NewFromUtf8(isolate, "console").ToLocalChecked(), console);

    //         v8::Local<v8::FunctionTemplate> error_template = v8::FunctionTemplate::New(isolate, V8ConsoleMessageCallback);
    //         v8::Local<v8::Function> error_function = error_template->GetFunction(context).ToLocalChecked();
    //         auto b2 = console->Set(context, v8::String::NewFromUtf8(isolate, "log").ToLocalChecked(), error_function);
    //     }
    //     //编译并执行js脚本
    //     v8::TryCatch trycatch(isolate);
    //     v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, jscode, v8::NewStringType::kNormal).ToLocalChecked();
    //     v8::Local<v8::Script> script;
    //     if (!v8::Script::Compile(context, source).ToLocal(&script)) {
    //         V8PrintException(isolate, &trycatch);
    //         return;
    //     }
    //     v8::Local<v8::Value> result;
    //     if (!script->Run(context).ToLocal(&result)) {
    //         V8PrintException(isolate, &trycatch);
    //         return;
    //     }
    //     std::cout << "run javascript suc! index=" << index  << std::endl;
    //     // Take a reference to the created JS function and call it with arguments
    //     v8::Local<v8::String> funcName = v8::String::NewFromUtf8(isolate, "onReceiveBattleRsp").ToLocalChecked();
    //     v8::Local<v8::Value> func_value = context->Global()->Get(context, funcName).ToLocalChecked();
    //     if (func_value->IsFunction()) {
    //         //执行任务
    //         while (true)
    //         {
    //             string str;
    //             {
    //                 //出作用域自动解锁无需调用unlock()
    //                 std::unique_lock<std::mutex> guard(m_mutex);
    //                 int i = index % tasks_.size();
    //                 m_condi.wait(guard, [this, i]() {
    //                     return (shutdown_ || !tasks_[i].empty());
    //                 } );
    //                 if (shutdown_) {
    //                     break;
    //                 }
    //                 if(tasks_[i].empty()) {
    //                     continue;
    //                 }
    //                 auto str = std::move(tasks_[i].front());
    //                 tasks_[i].pop_front();
    //             }
    //             //执行一次业务
    //             v8::Local<v8::Value> func_arg = v8::String::NewFromUtf8(isolate, str.c_str()).ToLocalChecked();
    //             {
    //                 v8::Local<v8::Object> func_object = func_value.As<v8::Object>();
    //                 v8::MaybeLocal<v8::Value> func_result = v8::Function::Cast(*func_object)->Call(context, context->Global(), 1, &func_arg);
    //                 if (!func_result.IsEmpty()) {
    //                     std::cout << "Call result: " << *(v8::String::Utf8Value(isolate, func_result.ToLocalChecked())) << std::endl;
    //                 }
    //                 else {
    //                     v8::String::Utf8Value utf8Value(isolate, trycatch.Message()->Get());
    //                     std::cout << "CallAsFunction didn't return a value, exception: " << *utf8Value << std::endl;
    //                 }
    //             }
    //         }
    //     }
    //     else {
    //         std::cerr << "onReceiveBattleRsp is not a function! index=" << index << std::endl;
    //         return;
    //     }
    // }

    // isolate->Dispose();
    // delete create_params.array_buffer_allocator;
    // std::cout << "Worker done! " << index << std::endl;
}

int srcfunc(int argc, char* argv[])
{
  // Initialize V8.
  printf("main start!%s\n", argv[0]);
  v8::V8::InitializeICUDefaultLocation(""); //argv[0]
  v8::V8::InitializeExternalStartupData(""); //argv[0]
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  // Create a new Isolate and make it the current one.
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate* isolate = v8::Isolate::New(create_params);
  {
    v8::Isolate::Scope isolate_scope(isolate);
    // Create a stack-allocated handle scope.
    v8::HandleScope handle_scope(isolate);
    // Create a new context.
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    // Enter the context for compiling and running the hello world script.
    v8::Context::Scope context_scope(context);
    {
      // Create a string containing the JavaScript source code.
      v8::Local<v8::String> source =
          v8::String::NewFromUtf8Literal(isolate, "'Hello' + ', World!'");
      // Compile the source code.
      v8::Local<v8::Script> script =
          v8::Script::Compile(context, source).ToLocalChecked();
      // Run the script to get the result.
      v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
      // Convert the result to an UTF8 string and print it.
      v8::String::Utf8Value utf8(isolate, result);
      printf("%s\n", *utf8);
    }
    {
      // Use the JavaScript API to generate a WebAssembly module.
      //
      // |bytes| contains the binary format for the following module:
      //
      //     (func (export "add") (param i32 i32) (result i32)
      //       get_local 0
      //       get_local 1
      //       i32.add)
      //
      const char csource[] = R"(
        let bytes = new Uint8Array([
          0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0x01,
          0x60, 0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x03, 0x02, 0x01, 0x00, 0x07,
          0x07, 0x01, 0x03, 0x61, 0x64, 0x64, 0x00, 0x00, 0x0a, 0x09, 0x01,
          0x07, 0x00, 0x20, 0x00, 0x20, 0x01, 0x6a, 0x0b
        ]);
        let module = new WebAssembly.Module(bytes);
        let instance = new WebAssembly.Instance(module);
        instance.exports.add(3, 4);
      )";
      // Create a string containing the JavaScript source code.
      v8::Local<v8::String> source =
          v8::String::NewFromUtf8Literal(isolate, csource);
      // Compile the source code.
      v8::Local<v8::Script> script =
          v8::Script::Compile(context, source).ToLocalChecked();
      // Run the script to get the result.
      v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
      // Convert the result to a uint32 and print it.
      uint32_t number = result->Uint32Value(context).ToChecked();
      printf("3 + 4 = %u\n", number);
    }
  }
  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  delete create_params.array_buffer_allocator;
  return 0;
}

void ConsoleMessageCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
  std::cout  << " ConsoleMessageCallback call back!" << std::endl;
  v8::HandleScope handle_scope(args.GetIsolate());
  v8::String::Utf8Value message(args.GetIsolate(), args[0].As<v8::String>());
  printf("JavaScript Console: %s\n", *message);
}

// 创建一个C++的消息回调函数
void ConsoleErrorCallback(v8::Local<v8::Message> message, v8::Local<v8::Value> error)
{
    v8::HandleScope handle_scope(message->GetIsolate());
    v8::String::Utf8Value error_message(message->GetIsolate(), error);
    v8::String::Utf8Value script_name(message->GetIsolate(), message->GetScriptResourceName());
    int line_number = message->GetLineNumber(message->GetIsolate()->GetCurrentContext()).FromJust();
    int column_number = message->GetStartColumn(message->GetIsolate()->GetCurrentContext()).FromJust();
    
    printf("JavaScript Console Error:\n");
    printf("  Error Message: %s\n", *error_message);
    printf("  Script Name: %s\n", *script_name);
    printf("  Line Number: %d\n", line_number);
    printf("  Column Number: %d\n", column_number);
}

void PrintException(v8::Isolate* isolate, v8::TryCatch* trycatch) {
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

void PrintHeapStats(v8::Isolate* isolate, int index) {
    v8::HeapStatistics heap_stats;
    isolate->GetHeapStatistics(&heap_stats);
    std::cout << "PrintHeapStats-------------------- index=" << index << std::endl;
    std::cout << "Heap size limit: " << heap_stats.heap_size_limit() / 1024 << " KB" << std::endl;
    std::cout << "Total heap size: " << heap_stats.total_heap_size() / 1024 << " KB" << std::endl;
    std::cout << "Total heap size executable: " << heap_stats.total_heap_size_executable() / 1024 << " KB" << std::endl;
    std::cout << "Total physical size: " << heap_stats.total_physical_size() / 1024 << " KB" << std::endl;
    std::cout << "Used heap size: " << heap_stats.used_heap_size() / 1024 << " KB" << std::endl;
}

// 加载文件
std::string ReadFile(const std::string &filename)
{
	std::ifstream input(filename);
	if (!input.is_open()) {
		return "";
	}
	std::string content((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));
	input.close();
	return content;
}

int64_t _GetMilliSeconds(){
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto val = now_ms.time_since_epoch().count();
    return val;
}

void jstest()
{
    // JavaScript代码
  // const char* js_code = R"(
  //     function foo() {
  //         console.log("This is foo function");
  //     }
      
  //     function bar() {
  //         console.log("This is bar function");
  //     }
      
  //     foo();
  //     bar();
  // )";

  // const char js_code[] = "function foo(arg) {"
  //                     "if (Math.random() > 0.5) throw new Error('er');"
  //                     "return arg + ' with foo from JS';"
  //                     "}";

  // const char js_code[] = "function foo(arg) {"
  //                     "return 'i='+ arg;"
  //                     "}";

  const char js_code[] = "function foo(arg) {}";

  // std::string jsCode = ReadFile("./battle.js");
  // if(jsCode.empty()) {
  //   std::cout << "jsCode empty!" << std::endl;
  //   return;
  // }
  //const char* js_code = jsCode.c_str();

  //执行JavaScript代码
  // const char* js_code = R"(
  //     function testFunc() {
  //         console.log("JavaScript Error: Something went wrong!");
  //     }
  //     testFunc();
  // )";

  // Initialize V8.
  printf("jstest start!\n");
  v8::V8::InitializeICUDefaultLocation("");
  v8::V8::InitializeExternalStartupData("");
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  // Create a new Isolate and make it the current one.
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate* isolate = v8::Isolate::New(create_params);
  {
    do
    {
      v8::Isolate::Scope isolate_scope(isolate);
      // Create a stack-allocated handle scope.
      v8::HandleScope handle_scope(isolate);

      // for (int i = 0; i < 500000; i++) {
      //   v8::HandleScope handle_scope1(isolate);
      //   v8::Local<v8::String> foo_arg;
      //   v8::String::NewFromUtf8(isolate, std::to_string(i).c_str()).ToLocal(&foo_arg);
      // }
      // std::cout << "show heap----------------------2:" << std::endl;
      // PrintHeapStats(isolate, 0);
      // break;

      // Create a new context.
      v8::Local<v8::Context> context = v8::Context::New(isolate);
      // Enter the context for compiling and running the hello world script.
      v8::Context::Scope context_scope(context);

      //设置 console.error回调
      {
        v8::Local<v8::Object> globalObj = context->Global();
        v8::Local<v8::Object> console = v8::Object::New(isolate);
        auto b1 = globalObj->Set(context, v8::String::NewFromUtf8(isolate, "console").ToLocalChecked(), console);

        v8::Local<v8::FunctionTemplate> error_template = v8::FunctionTemplate::New(isolate, ConsoleMessageCallback);
        v8::Local<v8::Function> error_function = error_template->GetFunction(context).ToLocalChecked();
        //console->Set(context, v8::String::NewFromUtf8(isolate, "error").ToLocalChecked(), error_function);
        auto b2 = console->Set(context, v8::String::NewFromUtf8(isolate, "log").ToLocalChecked(), error_function);
      }

      v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, js_code, v8::NewStringType::kNormal).ToLocalChecked();

      v8::TryCatch trycatch(isolate);
      v8::Local<v8::Script> script;
      if (!v8::Script::Compile(context, source).ToLocal(&script)) {
          PrintException(isolate, &trycatch);
          break;
      }
      v8::Local<v8::Value> result;
      if (!script->Run(context).ToLocal(&result)) {
          PrintException(isolate, &trycatch);
          break;
      }

      // v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();
      // v8::TryCatch trycatch(isolate);
      // v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
      // if(result->IsUndefined())
      // {
      //   v8::String::Utf8Value utf8Value(isolate, trycatch.Message()->Get());
      //   std::cout << "run script failed! exception=" << *utf8Value << std::endl;
      //   return;
      // }
      v8::String::Utf8Value utf8(isolate, result);

      // This is the result of the evaluation of the code (probably undefined)
      printf("%s\n", *utf8);

      // Take a reference to the created JS function and call it with a single string argument
      v8::Local<v8::Value> foo_value = context->Global()->Get(context, v8::String::NewFromUtf8(isolate, "foo").ToLocalChecked()).ToLocalChecked();
      if (foo_value->IsFunction()) {
          PrintHeapStats(isolate, 0);
          std::cout << "输入命令1继续执行:" << std::endl;
          int a;
          std::cin >> a;
          // argument (string)
          for (int i = 0; i < 500000; i++)
          {
              v8::HandleScope handle_scope1(isolate);
              v8::Local<v8::Value> foo_arg = v8::String::NewFromUtf8(isolate, std::to_string(i).c_str()).ToLocalChecked();
              //Method 1
              v8::TryCatch trycatch(isolate);
              v8::MaybeLocal<v8::Value> foo_ret = foo_value.As<v8::Object>()->CallAsFunction(context, context->Global(), 1, &foo_value);
              if (!foo_ret.IsEmpty()) {
                  v8::String::Utf8Value utf8Value(isolate, foo_ret.ToLocalChecked());
                  std::cout << "CallAsFunction result: " << *utf8Value << std::endl;
              } else {
                  v8::String::Utf8Value utf8Value(isolate, trycatch.Message()->Get());
                  std::cout << "CallAsFunction didn't return a value, exception: " << *utf8Value << std::endl;
              }
              std::cout << "CallAsFunction result: " << i << std::endl;
          }
          PrintHeapStats(isolate, 0);

          {
              // Method 2
              // v8::TryCatch trycatch(isolate);
              // v8::Local<v8::Object> foo_object = foo_value.As<v8::Object>();
              // std::cout << "show 4" << std::endl;
              // v8::MaybeLocal<v8::Value> foo_result = v8::Function::Cast(*foo_object)->Call(context, context->Global(), 1, &foo_arg);
              // std::cout << "show 5" << std::endl;
              // if (!foo_result.IsEmpty()) {
              //     std::cout << "Call result: " << *(v8::String::Utf8Value(isolate, foo_result.ToLocalChecked())) << std::endl;
              // } else {
              //     std::cout << "show 6" << std::endl;
              //     v8::String::Utf8Value utf8Value(isolate, trycatch.Message()->Get());
              //     std::cout << "CallAsFunction didn't return a value, exception: " << *utf8Value << std::endl;
              // }
          }
      } else {
          std::cerr << "foo is not a function" << std::endl;
      }
    }
    while(false);
  }

  while (1) {
    std::cout << "输入命令 1-打印最新内存 2-结束 3-执行gc 其他--" << std::endl;
    int a1;
    std::cin >> a1;
    if (a1 == 1) {
      PrintHeapStats(isolate, 0);
    }
    else if(a1 == 2) {
      break;
    }
    else if(a1==3) {
      int64_t tick1 = _GetMilliSeconds();
      isolate->LowMemoryNotification();
      std::cout << "执行gc结束! cost=" << (_GetMilliSeconds() - tick1) << std::endl;
    }
    else{
      continue;
    }
  }

  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  delete create_params.array_buffer_allocator;
}

void jstest_battle()
{
  //读取脚本
  std::string jsCode = ReadFile("./battle.js");
  if(jsCode.empty()) {
    std::cout << "jsCode empty!" << std::endl;
    return;
  }
  const char* js_code = jsCode.c_str();

  //参数内容
  std::string bt = ReadFile("./bt.txt");
  if(bt.empty()) {
    std::cout << "bt empty!" << std::endl;
    return;
  }
  std::string base64str = base64_encode(bt);
  std::cout << "bt size=" << bt.size() << " base64str="<< base64str.size() << std::endl;

  // Initialize V8.
  printf("jstest_battle start!\n");
  v8::V8::InitializeICUDefaultLocation("");
  v8::V8::InitializeExternalStartupData("");
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  // Create a new Isolate and make it the current one.
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate* isolate = v8::Isolate::New(create_params);
  {
    v8::Isolate::Scope isolate_scope(isolate);
    // Create a stack-allocated handle scope.
    v8::HandleScope handle_scope(isolate);
    // Create a new context.
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    // Enter the context for compiling and running the hello world script.
    v8::Context::Scope context_scope(context);

    do
    {
      //设置 console.error回调
      {
        v8::Local<v8::Object> globalObj = context->Global();
        v8::Local<v8::Object> console = v8::Object::New(isolate);
        auto b1 = globalObj->Set(context, v8::String::NewFromUtf8(isolate, "console").ToLocalChecked(), console);

        v8::Local<v8::FunctionTemplate> error_template = v8::FunctionTemplate::New(isolate, ConsoleMessageCallback);
        v8::Local<v8::Function> error_function = error_template->GetFunction(context).ToLocalChecked();
        //console->Set(context, v8::String::NewFromUtf8(isolate, "error").ToLocalChecked(), error_function);
        auto b2 = console->Set(context, v8::String::NewFromUtf8(isolate, "log").ToLocalChecked(), error_function);
      }
      
      v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, js_code, v8::NewStringType::kNormal).ToLocalChecked();

      v8::TryCatch trycatch(isolate);
      v8::Local<v8::Script> script;
      if (!v8::Script::Compile(context, source).ToLocal(&script)) {
          PrintException(isolate, &trycatch);
          break;
      }
      v8::Local<v8::Value> result;
      if (!script->Run(context).ToLocal(&result)) {
          PrintException(isolate, &trycatch);
          break;
      }

      v8::String::Utf8Value utf8(isolate, result);
      // This is the result of the evaluation of the code (probably undefined)
      printf("%s\n", *utf8);

      v8::Local<v8::Value> objValue2;
      auto b1 = context->Global()->Get(context, v8::String::NewFromUtf8(isolate, "goCallJs").ToLocalChecked()).ToLocal(&objValue2);
      if(objValue2.IsEmpty()) {
        std::cout << "objValue2 empty! " << std::endl;
        break;
      }
      v8::Local<v8::Object> goObj = objValue2.As<v8::Object>();

      v8::Local<v8::Value> funcValue;
      auto b2 = goObj->Get(context, v8::String::NewFromUtf8(isolate, "onReceiveBattleRsp").ToLocalChecked()).ToLocal(&funcValue);
      if(funcValue.IsEmpty()) {
        std::cout << "funcValue empty! " << std::endl;
        break;
      }
      if (funcValue->IsFunction()) {
          v8::Local<v8::Value> args[2] = {
            v8::Integer::New(isolate, 0),
            v8::String::NewFromUtf8(isolate, base64str.c_str()).ToLocalChecked()
          };

          v8::MaybeLocal<v8::Value> fresult = funcValue.As<v8::Object>()->CallAsFunction(context, objValue2, 2, args);
          if (!fresult.IsEmpty()) {
              v8::String::Utf8Value utf8Value(isolate, fresult.ToLocalChecked());
              std::cout << "CallAsFunction result: " << *utf8Value << std::endl;
          } else {
              v8::String::Utf8Value utf8Value(isolate, trycatch.Message()->Get());
              std::cout << "CallAsFunction didn't return a value, exception: " << *utf8Value << std::endl;
          }
      }
      else{
        std::cout << "funcValue not a function!" << std::endl;
      }
    }
    while(false);
  }

  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  delete create_params.array_buffer_allocator;
}

void AddStringToArguments(v8::Isolate* isolate, std::string str, v8::Handle<v8::Value> argList[], unsigned int argPos)
{
  argList[argPos] = v8::String::NewFromUtf8(isolate, str.c_str()).ToLocalChecked();
}

void AddNumberToArguments(v8::Isolate* isolate, double num, v8::Handle<v8::Value> argList[], unsigned int argPos)
{
    argList[argPos] = v8::Number::New(isolate, num);
}

void AddBooleanToArguments(v8::Isolate* isolate, bool value, v8::Handle<v8::Value> argList[], unsigned int argPos)
{
    argList[argPos] = v8::Boolean::New(isolate, value);
}

/**
* CallJSFunction(Handle<v8::Object>, string, Handle<Value>, UINT)
* / Handle of the global that is running the script with desired function
* / Title of the JS fuction
* / List of arguments for function
* / Number of agrguments
* Returns the return value of the JS function
**/
v8::Handle<v8::Value> CallJSFunction(v8::Handle<v8::Context> context,
   v8::Handle<v8::Object> global, std::string funcName, v8::Handle<v8::Value> argList[], unsigned int argCount)
{
  // Create value for the return of the JS function
  v8::Handle<v8::Value> jsResult;
  // Grab JS function out of file
  // v8::Handle<v8::Value> funcKey = v8::String::NewFromUtf8(context->GetIsolate(), funcName.c_str()).ToLocalChecked();
  // v8::Handle<v8::Value> value = global->Get(context, funcKey).ToLocalChecked();

  auto isolate = context->GetIsolate();
  v8::Local<v8::Function> myFunction = v8::Local<v8::Function>::Cast(global->Get(context, v8::String::NewFromUtf8(isolate, "myFunction").ToLocalChecked()).ToLocalChecked());
  v8::Local<v8::Value> args[2] = {v8::Number::New(context->GetIsolate(), 2), v8::Number::New(context->GetIsolate(), 3)};
  //v8::Local<v8::Value> result = myFunction->Call(context, v8::Null(isolate), 2, args).ToLocalChecked();
  v8::Local<v8::Value> result = myFunction->Call(context, context->Global(), 2, args).ToLocalChecked();

  // Cast value to v8::Function
  // v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(value);
  // Call function with all set values
  // v8::Local<v8::Value> args[2] = {v8::Number::New(context->GetIsolate(), 2), v8::Number::New(context->GetIsolate(), 3)};
  // v8::MaybeLocal<v8::Value> result = func->Call(context, v8::Null(context->GetIsolate()), 0, nullptr);
  // Return value from function
  return jsResult;
}

void ExecuteScript(v8::Isolate* isolate, const char* script) {
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);
    // 在这里执行 JavaScript 脚本
    v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, script).ToLocalChecked();
    v8::Local<v8::Script> compiled_script = v8::Script::Compile(context, source).ToLocalChecked();
    v8::Local<v8::Value> result = compiled_script->Run(context).ToLocalChecked();
    // v8::String::Utf8Value str(isolate, result);
    // std::cout << "JavaScript Output: " << *str << std::endl;

    v8::Local<v8::String> funcName = v8::String::NewFromUtf8(isolate, "foo").ToLocalChecked();
    v8::Local<v8::Value> func_value = context->Global()->Get(context, funcName).ToLocalChecked();
    if (func_value->IsFunction())
    {
      int idx = 5;
      while (idx > 0)
      {
        idx--;
        v8::TryCatch trycatch(isolate);
        v8::Local<v8::Object> func_object = func_value.As<v8::Object>();
        v8::MaybeLocal<v8::Value> func_result = v8::Function::Cast(*func_object)->Call(context, context->Global(), 0, nullptr);
        if (!func_result.IsEmpty()) {
            std::cout << "Call result: " << *(v8::String::Utf8Value(isolate, func_result.ToLocalChecked())) << std::endl;
        }
        else {
            v8::String::Utf8Value utf8Value(isolate, trycatch.Message()->Get());
            std::cout << "CallAsFunction didn't return a value, exception: " << *utf8Value << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
      }
    }
}

std::mutex m_mutex;

void thread_test(std::vector<std::thread>& threads_)
{
    // 创建多个线程，并每个线程中创建独立的 Isolate
    std::thread t1([]() {
        //std::unique_lock<std::mutex> guard(m_mutex);
        std::cout << "thread1 run!" << std::endl;
        v8::Isolate::CreateParams create_params;
        std::cout << "thread1 01!" << std::endl;
        create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        std::cout << "thread1 02!" << std::endl;
        v8::Isolate* isolate = v8::Isolate::New(create_params);
        //guard.unlock();
        std::cout << "thread1 1!" << std::endl;
        ExecuteScript(isolate, "function foo() { return 'foo from JS 1'}");
        std::cout << "thread1 2!" << std::endl;
        isolate->Dispose();
        delete create_params.array_buffer_allocator;
        std::cout << "thread1 end!" << std::endl;
    });
    std::thread t2([]() {
        //std::unique_lock<std::mutex> guard(m_mutex);
        std::cout << "thread2 run!" << std::endl;
        v8::Isolate::CreateParams create_params;
        std::cout << "thread2 01!" << std::endl;
        create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        std::cout << "thread2 02!" << std::endl;
        v8::Isolate* isolate = v8::Isolate::New(create_params);
        //guard.unlock();
        std::cout << "thread2 1!" << std::endl;
        ExecuteScript(isolate, "function foo() { return 'foo from JS 2'}");
        std::cout << "thread2 2!" << std::endl;
        isolate->Dispose();
        delete create_params.array_buffer_allocator;
        std::cout << "thread2 end!" << std::endl;
    });

    threads_.push_back(std::move(t1));
    threads_.push_back(std::move(t2));

    // 等待所有线程执行完毕
    //t1.join();
    //t2.join();

    //std::cout << "thread_test end!" << std::endl;

    //v8::V8::Dispose();
    //v8::V8::DisposePlatform();
}

std::unique_ptr<v8::Platform> g_v8platform;

void V8Initialize()
{
  v8::V8::InitializeICUDefaultLocation("");
  v8::V8::InitializeExternalStartupData("");
  //std::unique_ptr<v8::Platform> platform(v8::platform::NewDefaultPlatform());
  g_v8platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(g_v8platform.get());
  v8::V8::Initialize();
}

int main(int argc, char* argv[])
{
  //1、测试
  //执行js脚本
  //jstest();
  //jstest_battle();

  //2、所线程测试
  //V8Initialize();
  // std::vector<std::thread> threads_;
  // thread_test(threads_);
  //std::cout << "thread_test end!" << std::endl;
  //等待所有线程执行完毕
  // for(auto& it : threads_) {
  //   it.join();
  // }
  // v8::V8::Dispose();
  // v8::V8::DisposePlatform();
  // g_v8platform.reset();

  //v8engine 测试
  //参数内容
  std::string bt = ReadFile("./bt.txt");
  std::string base64str = base64_encode(bt);
  std::cout << "bt size=" << bt.size() << " base64str="<< base64str.size() << std::endl;

  int threadNum = 8;
  v8engine v8obj;
  v8obj.Create(threadNum);
  std::this_thread::sleep_for(std::chrono::seconds(2));
  for(int i = 0; i < 3; i++){
    std::cout << " " << std::endl;
  }

  while (1) {
    std::cout << "输入命令:1-继续执行 2-关闭v8engine 3-输出堆栈 4-执行gc" << std::endl;
    int a;
    std::cin >> a;
    if(a == 1) {
      int tasknum = 50000;
      v8obj.StartStat(tasknum);
      for(int i = 1; i <= tasknum; i++) {
          v8obj.PushTask(base64str, i);
      }
    }
    else if (a == 2) {
      v8obj.Release();
      std::cout << "main done!" << std::endl;
      break;
    }
    else if(a == 3){
      v8obj.PrintMemoryInfo();
    }
    else if(a == 4){
      v8obj.GarbageCollect();
    }
    else{
      continue;
    }
  }

  //v8obj.Release();
  //std::cout << "main done!" << std::endl;

  getchar();
  return 0;
}