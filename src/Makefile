test:text.cpp base64.cpp
	g++ -g -o $@ $^ -std=c++17 -lpthread -lstdc++fs -lbundle -levent -Wno-register -D_GLIBCXX_USE_OPEN_MODE -lboost_system -lboost_filesystem -lboost_thread -ljsoncpp -DBOOST_BIND_GLOBAL_PLACEHOLDERS 
gdb_test:Test.cpp
	g++ -g -o $@ $^ -std=c++17 -lpthread -lstdc++fs -ljsoncpp  -lbundle -levent -Wno-register -D_GLIBCXX_USE_OPEN_MODE -lboost_system -lboost_filesystem -lboost_thread -DBOOST_BIND_GLOBAL_PLACEHOLDERS
.PHONY:clean
clean:
	rm -rf test gdb_test ./deep_storage ./low_storage ./logfile storage.data