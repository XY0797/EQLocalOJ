/**
 * \file    	Judge.hpp
 * \author  	XY0797
 * \date    	2024.8.6
 * \brief		提供评测控制台程序的接口
 */
#ifndef _XY0797_JUDGE
#define _XY0797_JUDGE 1

#include <regex>
#include <fstream>
#include <string>
#include <sstream>
#include <exception>
#include "ConsoleOJ.hpp"
#include "TestStringCmp.hpp"

// 统一换行符
std::string unifyNewlines(const std::string& input) {
	// 正则表达式匹配各种换行符，包括\r, \r\n
	static std::regex newline_regex("(\\r\\n|\\r)");
	// 使用std::regex_replace替换所有匹配到的换行符为\n
	return std::regex_replace(input, newline_regex, "\n");
}

// 读取整个文件
// 自动统一换行符
std::string read_entire_text_file(const std::string& filename) {
	std::ifstream in(filename);
	if (!in) {
		throw std::runtime_error("无法打开文件");
	}
	std::stringstream buffer;
	buffer << in.rdbuf();
	return buffer.str();
}

// 返回状态码
// 0：AC
// -1：启动失败
// -2：WA
// -3：TLE
// 如果启动失败，会将错误信息写在m_TestERRStr里面
// 如果WA了，会将差异以人类可读形式写在m_TestERRStr里面
// 如果AC，会将用时(小数秒)写在m_TestERRStr里面
int runTest(const std::string& testCaseFilePath, const std::string& ansFilePath,
            const std::string& testExePath, long long timeLimit) {
	std::string myansStr, errorMsg;
	m_TestERRStr.clear();
	long long timecost = 0;
	std::string testCaseStr = read_entire_text_file(testCaseFilePath);
	std::string ansStr = read_entire_text_file(ansFilePath);
	ConsoleOJ myansEXE(testExePath);
	// 获取待检测答案
	if (!myansEXE.launchAndWait(testCaseStr, timeLimit, myansStr, timecost, errorMsg)) {
		if (errorMsg.find("超时") != errorMsg.npos) {
			m_TestERRStr = errorMsg;
			return -3;
		}
		m_TestERRStr = "运行待测程序失败，用时" + std::to_string(timecost)
		               + "ms，原因：" + errorMsg;
		return -1;
	}
	myansStr = unifyNewlines(myansStr);
	ansStr = unifyNewlines(ansStr);
	if (!compareAnsStr(ansStr, myansStr)) {
		return -2;
	}
	m_TestERRStr = std::to_string(timecost) ;
	return 0;
}

#endif /* _XY0797_JUDGE */
