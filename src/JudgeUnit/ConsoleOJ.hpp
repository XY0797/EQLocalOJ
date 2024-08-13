/**
 * \file    	ConsoleOJ.hpp
 * \author  	XY0797
 * \date    	2024.8.6
 * \brief		用于评测控制台程序的工具类
 */
#ifndef _XY0797_CONSOLEOJ
#define _XY0797_CONSOLEOJ 1

#include <iostream>
#include <shared_mutex>
#include <thread>
#include <chrono>
#include <string>
#include <windows.h>

// 安全关闭句柄
void Clhandle_s(HANDLE& hd) {
	if (hd != INVALID_HANDLE_VALUE) {
		CloseHandle(hd);
		hd = INVALID_HANDLE_VALUE;
	}
}

// 文件时间转毫秒
long long fileTime2ms(const FILETIME& fileTime) {
	ULARGE_INTEGER Uint;
	Uint.LowPart = fileTime.dwLowDateTime;
	Uint.HighPart = fileTime.dwHighDateTime;
	return (Uint.QuadPart / 10000ull);
}

class ConsoleOJ {
private:
	// 可执行文件路径、工作目录、命令行参数
	std::string m_programPath;
	std::string m_workingDirectory;

	// 进程信息读写锁，多线程访问时的线程安全
	std::shared_mutex m_rwProcMutex;

	// 读取输出的句柄
	HANDLE m_outputPipeRead = INVALID_HANDLE_VALUE;

	// 写入数据的句柄
	HANDLE m_inputPipeWrite = INVALID_HANDLE_VALUE;

	// 写入数据，保证输入期间不被析构
	const char* m_inputCStr = nullptr;
	DWORD m_inputCStrLen = 0;

	// 是否已经启动
	bool isLaunched = false;

	// 析构事件同步对象
	HANDLE m_hExitEvent = INVALID_HANDLE_VALUE;

	// 将要析构标志位
	bool m_willExit = false;

	// 监视线程对象
	std::thread m_checkProcThread;

	// 写输入文本线程对象
	std::thread m_WriteStrThread;

	// 输出文本内容
	std::string m_output;

	// 监视线程
	static void CheckProcThread(ConsoleOJ* const classthis) {
		classthis->m_output = "";
		bool onemoretime = true;
		while (classthis->isLaunched || onemoretime) {
			if (!classthis->isLaunched) {
				onemoretime = false;
			}
			DWORD availableBytes;
			DWORD bytesRead;
			classthis->m_rwProcMutex.lock();
			PeekNamedPipe(classthis->m_outputPipeRead, NULL, 0, NULL,
			              &availableBytes, NULL);
			if (availableBytes > 0) {
				char* outputBuffer = new char[availableBytes + 1];
				if (!ReadFile(classthis->m_outputPipeRead, outputBuffer,
				              availableBytes, &bytesRead, NULL)) {
					delete[] outputBuffer;
					classthis->m_rwProcMutex.unlock();
					return;
				}
				outputBuffer[bytesRead] = '\0';
				classthis->m_output += std::string(outputBuffer);
				delete[] outputBuffer;
			}
			classthis->m_rwProcMutex.unlock();
			WaitForSingleObject(classthis->m_hExitEvent, 10);
			if (classthis->m_willExit) {
				return;
			}
		}
	}

	static void WriteStrThread(ConsoleOJ* const classthis) {
		DWORD bytesWritten;
		WriteFile(classthis->m_inputPipeWrite, classthis->m_inputCStr,
		          classthis->m_inputCStrLen, &bytesWritten, NULL);
	}

public:
	/*
	 *	构造时传入：exe文件路径
	 */
	explicit ConsoleOJ(const std::string& programPath) : m_programPath(programPath) {
		// 处理工作目录
		size_t found = programPath.find_last_of("/\\");
		if (found != std::string::npos) {
			this->m_workingDirectory = programPath.substr(0, found);
		} else {
			this->m_workingDirectory.clear();
		}
		// 创建事件对象
		m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	~ConsoleOJ() {
		// 析构
		m_willExit = true;
		SetEvent(m_hExitEvent);
		if (isLaunched) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		// 关闭事件同步对象句柄
		CloseHandle(m_hExitEvent);
	}

	/*
	 *	启动进程，返回目标程序是否在时限内成功运行
	 *  输入文本[in]：将压入目标程序输入流的文本，为空则不输入文本
	 *  时间限制[in]：单位毫秒，必须是100ms的倍数，否则向上取整到100ms的倍数
	 *  输出文本[out]：返回程序输出流中的文本
	 *  时间花费[out]：返回程序运行消耗的CPU时间
	 *  错误信息[out]：返回程序运行失败的原因
	 */
	bool launchAndWait(const std::string& inputstr, long long timelimit,
	                   std::string& outputstr, long long& timecosted, std::string& errstr) {
		// 初始化安全标识符，使得管道可被子进程访问
		SECURITY_ATTRIBUTES securityAttributes;
		securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		securityAttributes.bInheritHandle = TRUE;
		securityAttributes.lpSecurityDescriptor = NULL;
		// 管道句柄
		HANDLE inputPipeRead = INVALID_HANDLE_VALUE;
		HANDLE inputPipeWrite = INVALID_HANDLE_VALUE;
		HANDLE outputPipeWrite = INVALID_HANDLE_VALUE;
		// 初始化进程信息结构体
		PROCESS_INFORMATION processInfo;
		ZeroMemory(&processInfo, sizeof(processInfo));
		try {
			// 附带安全标识符创建输入管道
			if (!CreatePipe(&inputPipeRead, &inputPipeWrite, &securityAttributes, 0)) {
				errstr = "创建输入管道失败！";
				throw 1;
			}

			// 附带安全标识符创建输出管道
			m_rwProcMutex.lock();
			if (!CreatePipe(&m_outputPipeRead, &outputPipeWrite, &securityAttributes, 0)) {
				errstr = "创建输出管道失败！";
				throw 2;
			}
			m_rwProcMutex.unlock();

			// 初始化启动信息结构体
			STARTUPINFOA startupInfo;
			ZeroMemory(&startupInfo, sizeof(startupInfo));
			startupInfo.cb = sizeof(startupInfo);

			// 设置输入输出管道，设置使用自定义管道标志位
			startupInfo.hStdInput = inputPipeRead;
			startupInfo.hStdOutput = outputPipeWrite;
			startupInfo.hStdError = outputPipeWrite;
			startupInfo.dwFlags |= STARTF_USESTDHANDLES;

			// 处理命令行信息
			std::string commandLine = "\"" + m_programPath + "\"";
			char* commandLine_c = new char[commandLine.size() + 1];
			strncpy(commandLine_c, commandLine.c_str(), commandLine.size());
			commandLine_c[commandLine.size()] = 0;

			// 创建进程
			const char* workingDirectoryPtr = NULL;
			if (!m_workingDirectory.empty()) {
				workingDirectoryPtr = m_workingDirectory.c_str();
			}
			if (!CreateProcessA(NULL, commandLine_c, NULL, NULL, TRUE,
			                    HIGH_PRIORITY_CLASS | CREATE_NO_WINDOW | CREATE_SUSPENDED,
			                    NULL, workingDirectoryPtr, &startupInfo, &processInfo)) {
				// 释放命令行文本
				delete[] commandLine_c;
				errstr = "创建进程失败！";
				throw 3;
			}

			// 释放命令行文本
			delete[] commandLine_c;

			isLaunched = true;

			// 启动监视线程
			m_checkProcThread = std::thread(&CheckProcThread, this);

			// 写入测试输入
			std::string inputWithEOF;
			if (!inputstr.empty()) {
				// 不为空才需要真写入数据
				inputWithEOF = inputstr + static_cast<char>(26);
				// 异步写入数据
				m_inputCStr = inputWithEOF.c_str();
				m_inputCStrLen = static_cast<DWORD>(inputWithEOF.size());
				m_inputPipeWrite = inputPipeWrite;
				// 启动数据写入线程
				m_WriteStrThread = std::thread(&WriteStrThread, this);
			}

			// 需要等待析构标志句柄与进程句柄
			HANDLE hWaitHandle[2];
			hWaitHandle[0] = this->m_hExitEvent;
			hWaitHandle[1] = processInfo.hProcess;

			// 存储初始CPU周期计数
			ULONG64 stCycleTime = 0ull;
			// 最小增长周期1e6
			const ULONG64 minDCycleTime = 1000000ull;

			// 处理时间限制为100ms的倍数
			long long timeLimitCnt = timelimit / 100;
			if (timelimit % 100) {
				++timeLimitCnt;
			}
			timelimit = timeLimitCnt * 100;

			// 最大真实时间花费
			long long maxRealTimeCost = 40 * timelimit;

			// 继续执行进程
			ResumeThread(processInfo.hThread);

			// 开始计时
			auto start = std::chrono::high_resolution_clock::now();

			// 开始等待句柄信号
			while (1) {
				// 同时等待两个句柄，只要有一个响应就退出阻塞
				DWORD waitResult = WaitForMultipleObjects(2, hWaitHandle, FALSE, 100);

				if (m_willExit) {
					errstr = "类正在析构！";
					throw 4;
				} else if (waitResult == WAIT_TIMEOUT) {
					if (timeLimitCnt == 0) {
						// 需要判断是不是真TLE了
						FILETIME creationTime, exitTime, kernelTime, userTime;
						GetThreadTimes(processInfo.hThread, &creationTime, &exitTime,
						               &kernelTime, &userTime);
						timecosted = fileTime2ms(kernelTime) + fileTime2ms(userTime);
						if (timecosted > timelimit) {
							errstr = "执行超时！";
							throw 4;
						}
						// 先获取真实时间花费
						std::chrono::duration<double> timeElapsed =
						    std::chrono::high_resolution_clock::now() - start;
						timecosted = std::chrono::duration_cast<std::chrono::milliseconds>
						             (timeElapsed).count();
						// 然后判断被阻塞的情况
						ULONG64 curCycleTime = 0ull;
						QueryThreadCycleTime(processInfo.hThread, &curCycleTime);
						if (curCycleTime - stCycleTime < minDCycleTime) {
							errstr = "程序疑似被阻塞，执行超时！";
							throw 4;
						}
						stCycleTime = curCycleTime;
						// 再判断真实耗时过大的情况
						if (timecosted > maxRealTimeCost) {
							errstr = "评测机负载过大，执行超时！";
							throw 4;
						}
					} else {
						--timeLimitCnt;
					}
				} else {
					// 说明正常退出了
					break;
				}
			}

			// 程序时限内退出

			// 获取时间
			FILETIME creationTime, exitTime, kernelTime, userTime;
			GetThreadTimes(processInfo.hThread, &creationTime, &exitTime,
			               &kernelTime, &userTime);
			long long realTimeUsed, CPUTimeUsed;
			realTimeUsed = fileTime2ms(exitTime) - fileTime2ms(creationTime);
			if (realTimeUsed < 0) {
				realTimeUsed = 0;
			}
			CPUTimeUsed = fileTime2ms(kernelTime) + fileTime2ms(userTime);
			if (CPUTimeUsed > realTimeUsed) {
				timecosted = realTimeUsed;
			} else {
				timecosted = CPUTimeUsed;
			}

			// 获取进程退出代码
			DWORD exeCode;
			GetExitCodeProcess(processInfo.hProcess, &exeCode);
			if (exeCode != 0) {
				errstr = "程序返回值为" + std::to_string(exeCode) + "！";
				throw 4;
			}

			// 获取输出
			isLaunched = false;
			m_checkProcThread.join();
			m_WriteStrThread.join();
			outputstr = m_output;
			m_output = "";

			// 关闭管道句柄
			Clhandle_s(inputPipeWrite);
			Clhandle_s(inputPipeRead);
			Clhandle_s(m_outputPipeRead);
			Clhandle_s(outputPipeWrite);

			// 关闭线程句柄
			Clhandle_s(processInfo.hThread);

			// 关闭进程句柄
			Clhandle_s(processInfo.hProcess);
			return true;
		} catch (int errid) {
			switch (errid) {
				case 4:
					// 需要处理写入线程
					if (!inputstr.empty()) {
						// 取消IO请求
						CancelIoEx(m_inputPipeWrite, NULL);
						// 等待线程结束
						m_WriteStrThread.join();
					}
					// 结束进程
					TerminateProcess(processInfo.hProcess, 1);
					// 等待线程
					isLaunched = false;
					m_checkProcThread.join();
					// 关闭线程句柄
					Clhandle_s(processInfo.hThread);
					// 关闭进程句柄
					Clhandle_s(processInfo.hProcess);
				case 3:
					// 关闭输出句柄
					m_rwProcMutex.lock();
					Clhandle_s(m_outputPipeRead);
					m_rwProcMutex.unlock();
					Clhandle_s(outputPipeWrite);
					break;
				case 2:
					// 关闭输出句柄
					Clhandle_s(m_outputPipeRead);
					m_rwProcMutex.unlock();
					Clhandle_s(outputPipeWrite);
					break;
			}
			// 关闭输入句柄
			Clhandle_s(inputPipeRead);
			Clhandle_s(inputPipeWrite);
			return false;
		}
	}
};

#endif /* _XY0797_CONSOLEOJ */
