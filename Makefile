.PHONY:clean exec

tt:
	g++ -g -I./include tt.cpp v8engine.cpp base64.cpp -o tt  -L./libv8 -lv8_monolith -lv8_libbase -lv8_libplatform -fno-rtti -ldl -pthread -std=c++17 -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX

clean:
	rm -rf ./tt

exec:
	./tt