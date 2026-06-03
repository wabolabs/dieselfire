#pragma once
// Mock: Utility/DebugPort.h
class DebugPortClass {
public:
  template<typename T> void print(T) {}
  template<typename T> void println(T) {}
  void printf(const char*, ...) {}
  void begin(int) {}
  void handle() {}
  void setWelcomeMsg(const char*) {}
  void setBufferSize(int) {}
};
extern DebugPortClass DebugPort;
