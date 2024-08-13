/**
 * \file    	LocalOJ.cpp
 * \author  	XY0797
 * \date    	2024.8.6
 * \brief		程序入口
 */
#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <fstream>
#include "ArtFont.hpp"
#include "JudgeUnit/Judge.hpp"
#include "WindowsFileSysTool.hpp"
#include "EnableWindowsConsoleColor.hpp"

int getNumBits(int num) {
	if (num == 0) {
		return 1;
	}
	int bits = 0;
	while (num) {
		++bits;
		num /= 10;
	}
	return bits;
}

std::string m_TestCaseExtension = ".in";
std::string m_AnsExtension = ".out";

std::string getAnsFilePath(const std::string& testCaseFilePath) {
	size_t i1 = testCaseFilePath.rfind(m_TestCaseExtension);
	return testCaseFilePath.substr(0, i1) + m_AnsExtension;
}

struct JudgeInfo {
	// 测试集输入文件的全路径
	std::string fullTestCasePath;
	// 标准答案文件的全路径
	std::string fullAnsPath;
	// exe文件的全路径
	std::string exeFilePath;
	// 限时(ms)
	int timeLimit;
	// 测试集输入文件的文件名
	std::string name;
	// 文件名是否为纯数字
	bool isNumName;
	// 文件名如果是数字，这里存储数字值
	long long NumName;
	// 评测结果ID
	// 1：未评测，0：AC，-1：启动失败，-2：WA，-3：TLE
	int resID;
	// 评测的结果信息
	std::string ERRmsg;

	// 传入.in文件的完整路径
	JudgeInfo(const std::string& TestCaseFileFullPath,
	          const std::string& ExeFileFullPath, int TimeLimit)
		: fullTestCasePath(TestCaseFileFullPath),
		  fullAnsPath(getAnsFilePath(TestCaseFileFullPath)),
		  exeFilePath(ExeFileFullPath),
		  timeLimit(TimeLimit),
		  name(winfs::getNonExtenFileName(TestCaseFileFullPath)),
		  isNumName(false),
		  resID(1) {
		// 检测数字文件名
		for (char ch : name) {
			if (ch >= '0' && ch <= '9') {
				isNumName = true;
			} else {
				isNumName = false;
				break;
			}
		}
		// 超longlong范围的不当成数字
		if (isNumName) {
			if (name.length() > 19) {
				isNumName = false;
			} else if (name.length() == 19) {
				isNumName = (name[0] != '9');
			}
		}
		// 是数字就字符串转longlong
		if (isNumName) {
			NumName = std::stoll(name);
		}
	}
	// 用于排序
	bool operator<(const JudgeInfo& o) const {
		if (isNumName && o.isNumName) {
			// 都是数字，小的排前面
			return NumName < o.NumName;
		}
		if (isNumName || o.isNumName) {
			// 数字排前面
			return isNumName;
		}
		// 都不是数字就正常字典序
		return name < o.name;
	}
};

std::vector<JudgeInfo> m_JudgeInfoList;
size_t ACcnt = 0;

void doJudge() {
	for (auto& e : m_JudgeInfoList) {
		std::cout << std::endl
		          << "正在评测\x1b[1;37;44m "
		          << e.name << m_TestCaseExtension << " \x1b[0m："
		          << std::endl;
		// 1：未评测，0：AC，-1：启动失败，-2：WA，-3：TLE
		e.resID = runTest(e.fullTestCasePath, e.fullAnsPath,
		                  e.exeFilePath, e.timeLimit);
		switch (e.resID) {
			case 0:
				++ACcnt;
				std::cout << artAC << std::endl;
				e.ERRmsg = m_TestERRStr;
				m_TestERRStr.clear();
				std::cout << "用时：" << e.ERRmsg << "ms" << std::endl;
				break;
			case -1:
				std::cout << artStartFailed << std::endl;
				e.ERRmsg = m_TestERRStr;
				m_TestERRStr.clear();
				std::cout << "报错信息：" << std::endl << e.ERRmsg << std::endl;
				break;
			case -2:
				std::cout << artWA << std::endl;
				e.ERRmsg = m_TestERRStr;
				m_TestERRStr.clear();
				std::cout << "错误原因：" << std::endl << e.ERRmsg << std::endl;
				break;
			case -3:
				std::cout << artTLE << std::endl;
				e.ERRmsg = m_TestERRStr;
				m_TestERRStr.clear();
				std::cout << std::endl << e.ERRmsg << std::endl;
				break;
		}
	}
}

int main() {
	std::string lstJudgeInfoPath =
	    winfs::getCurEXEParentDirectoryPath() + "\\lstjudgeinfo.txt";
	std::cout << "本程序可以作为一个本地的oj使用，指定测试集和程序文件，自动判题"
	          << std::endl
	          << "by XY0797"
	          << std::endl;

	if (!EnableWindowsConsoleColor()) {
		std::cout << "警告：您的系统过于老旧，终端不支持彩色显示，"
		          "这将导致本程序的彩色显示变成乱码！" << std::endl;
	}

	bool isRepeat = false;
	if (winfs::isFileExist(lstJudgeInfoPath)) {
		std::string tmpStr;
		std::cout << "是否\x1b[1;37;42m重复上一次\x1b[0m的评测"
		          "(直接回车表示拒绝，输入任意字符表示同意)：" << std::endl;
		std::getline(std::cin, tmpStr);
		if (!tmpStr.empty()) {
			isRepeat = true;
		}
	}
	std::ifstream fin;
	if (isRepeat) {
		fin.open(lstJudgeInfoPath);
		if (!fin.is_open()) {
			isRepeat = false;
			std::cerr << "\x1b[1;31m"
			          "上一次评测信息文件无法打开，请您手动指定评测信息！"
			          "\x1b[22;0m" << std::endl;
		}
	}

	std::string infilepath;
	std::cout << "请输入测试集\x1b[1;37;42m输入文件\x1b[0m的文件路径"
	          "(可以直接拖进来,文件夹也行)：" << std::endl;
	if (isRepeat) {
		std::getline(fin, infilepath);
		std::cout << infilepath << std::endl;
	} else {
		std::getline(std::cin, infilepath);
	}
	if (!infilepath.empty() && infilepath[0] == '"') {
		infilepath.pop_back();
		infilepath.erase(infilepath.begin());
	}
	if (!winfs::isFileExist(infilepath)) {
		std::cerr << "\x1b[1;31m测试集文件/文件夹 不存在！\x1b[22;0m" << std::endl;
		std::cout << "按回车退出" << std::endl;
		std::cin.get();
		return 0;
	}

	std::string exefilepath;
	std::cout << "请输入\x1b[1;37;42mEXE文件\x1b[0m的文件路径"
	          "(可以直接拖进来)：" << std::endl;
	if (isRepeat) {
		std::getline(fin, exefilepath);
		std::cout << exefilepath << std::endl;
	} else {
		std::getline(std::cin, exefilepath);
	}
	if (!exefilepath.empty() && exefilepath[0] == '"') {
		exefilepath.pop_back();
		exefilepath.erase(exefilepath.begin());
	}
	if (!winfs::isFileExist(exefilepath)) {
		std::cerr << "\x1b[1;31mEXE文件 不存在！\x1b[22;0m" << std::endl;
		std::cout << "按回车退出" << std::endl;
		std::cin.get();
		return 0;
	}

	std::string timeLimitStr;
	int timeLimit;
	std::cout << "请输入程序的\x1b[1;37;42m时间限制\x1b[0m"
	          "(单位毫秒，1000毫秒=1秒，直接回车默认1秒)：" << std::endl;
	if (isRepeat) {
		std::getline(fin, timeLimitStr);
		std::cout << timeLimitStr << std::endl;
	} else {
		std::getline(std::cin, timeLimitStr);
	}
	if (timeLimitStr.empty()) {
		timeLimit = 1000;
	} else {
		try {
			timeLimit = std::stoi(timeLimitStr);
		} catch (const std::exception& e) {
			std::cerr << "\x1b[1;31m输入的时间限制无效，无法转为整数，"
			          "现已指定默认值1秒代替！\x1b[22;0m" << std::endl;
			timeLimit = 1000;
		}
	}

	std::cout << "请输入测试集"
	          "\x1b[1;37;42m答案/预期输出文件\x1b[0m的\x1b[1;37;42m后缀\x1b[0m"
	          "(直接回车则使用默认的.out)：" << std::endl;
	if (isRepeat) {
		std::getline(fin, m_AnsExtension);
		std::cout << m_AnsExtension << std::endl;
	} else {
		std::getline(std::cin, m_AnsExtension);
	}
	if (m_AnsExtension.empty()) {
		m_AnsExtension = ".out";
	}

	try {
		std::vector<std::string> testCaseFiles;
		// strList存储测试样例的输入文件路径
		if (winfs::isDir(infilepath)) {
			std::cout << "请输入测试集"
			          "\x1b[1;37;42m输入文件\x1b[0m的\x1b[1;37;42m后缀\x1b[0m"
			          "(直接回车则使用默认的.in)：" << std::endl;
			if (isRepeat) {
				std::getline(fin, m_TestCaseExtension);
				std::cout << m_TestCaseExtension << std::endl;
			} else {
				std::getline(std::cin, m_TestCaseExtension);
			}
			if (m_TestCaseExtension.empty()) {
				m_TestCaseExtension = ".in";
			}
			testCaseFiles = winfs::getFilesOfDirectory(infilepath,
			                "*" + m_TestCaseExtension);
		} else {
			m_TestCaseExtension = winfs::getFileExtenWithDot(infilepath);
			testCaseFiles.push_back(infilepath);
		}
		// 创建评测列表
		for (const auto& e : testCaseFiles) {
			m_JudgeInfoList.push_back(JudgeInfo(e, exefilepath, timeLimit));
		}
		std::sort(m_JudgeInfoList.begin(), m_JudgeInfoList.end());
		// 开始评测
		doJudge();
		// 评测完成
		std::cout << std::endl << "评测完成，通过情况："
		          << ACcnt << '/' << m_JudgeInfoList.size() << std::endl;
		// 存储上一次评测信息
		{
			std::ofstream fout(lstJudgeInfoPath);
			if (fout.is_open()) {
				fout << infilepath << '\n';
				fout << exefilepath << '\n';
				fout << timeLimit << '\n';
				fout << m_AnsExtension << '\n';
				if (winfs::isDir(infilepath)) {
					fout << m_TestCaseExtension << '\n';
				}
				fout.close();
			}
		}
		// 显示样例评测结果概览
		// 均为白字，绿底AC，红底WA，灰底TLE，蓝底无法启动评测
		std::cout << "\x1b[1;37;42m  AC  \x1b[0m   \x1b[1;37;41m  WA  \x1b[0m"
		          "   \x1b[1;37;100m  TLE  \x1b[0m"
		          "   \x1b[1;37;44m 无法启动评测 \x1b[0m"
		          << std::endl << std::endl;
ShowJudgeRes:
		int caseNumID = 0;
		int caseNumIDMAXLen = getNumBits(m_JudgeInfoList.size());
		int strLineLen = 0;
		for (const auto& e : m_JudgeInfoList) {
			++caseNumID;
			std::string caseNumIDStr = std::to_string(caseNumID);
			caseNumIDStr = std::string(caseNumIDMAXLen - caseNumIDStr.size(), '0')
			               + caseNumIDStr + ":";
			strLineLen += caseNumIDStr.length() + e.name.length()
			              + m_TestCaseExtension.length() + 2;
			std::cout << caseNumIDStr;
			// 1：未评测，0：AC，-1：启动失败，-2：WA，-3：TLE
			switch (e.resID) {
				case 0:
					std::cout << "\x1b[1;37;42m " << e.name << m_TestCaseExtension
					          << " \x1b[0m\t";
					break;
				case -1:
					std::cout << "\x1b[1;37;44m " << e.name << m_TestCaseExtension
					          << " \x1b[0m\t";
					break;
				case -2:
					std::cout << "\x1b[1;37;41m " << e.name << m_TestCaseExtension
					          << " \x1b[0m\t";
					break;
				case -3:
					std::cout << "\x1b[1;37;100m " << e.name << m_TestCaseExtension
					          << " \x1b[0m\t";
					break;
			}
			if (strLineLen >= 40) {
				std::cout << std::endl;
				strLineLen = 0;
			}
		}
		std::cout << std::endl;
		std::cout << "输入0退出程序，输入:左边的序号获取更多信息：" << std::endl;
		int ch;
		if (!(std::cin >> ch)) {
			std::cin.clear();
			ch = -1;
		}
		std::cin.ignore();
		if (ch == 0) {
			return 0;
		}
		if (ch < 1 || ch > static_cast<int>(m_JudgeInfoList.size())) {
			std::cerr << "\x1b[1;31m输入的选择无效！\x1b[22;0m" << std::endl;
			goto ShowJudgeRes;
		}
		--ch;
		switch (m_JudgeInfoList[ch].resID) {
			case 0:
				std::cout << "\x1b[1;37;42m "
				          << m_JudgeInfoList[ch].name << m_TestCaseExtension
				          << " \x1b[0m" << std::endl
				          << artAC << std::endl
				          << "用时："
				          << m_JudgeInfoList[ch].ERRmsg << "ms" << std::endl;
				break;
			case -1:
				std::cout << "\x1b[1;37;44m "
				          << m_JudgeInfoList[ch].name << m_TestCaseExtension
				          << " \x1b[0m" << std::endl
				          << artStartFailed << std::endl
				          << "报错信息：" << std::endl
				          << m_JudgeInfoList[ch].ERRmsg << std::endl;
				break;
			case -2:
				std::cout << "\x1b[1;37;41m "
				          << m_JudgeInfoList[ch].name << m_TestCaseExtension
				          << " \x1b[0m" << std::endl
				          << artWA << std::endl
				          << "错误原因：" << std::endl
				          << m_JudgeInfoList[ch].ERRmsg << std::endl;
				break;
			case -3:
				std::cout << "\x1b[1;37;100m "
				          << m_JudgeInfoList[ch].name << m_TestCaseExtension
				          << " \x1b[0m" << std::endl
				          << artTLE << std::endl
				          << std::endl << m_JudgeInfoList[ch].ERRmsg << std::endl;
				break;
		}
		std::cout << "按回车返回概览界面" << std::endl;
		std::cin.get();
		goto ShowJudgeRes;
	} catch (const std::exception& e) {
		std::cerr << "\x1b[1;31m评测程序遇到异常: "
		          << e.what() << "\x1b[22;0m" << std::endl;
	}
	std::cout << "按回车退出" << std::endl;
	std::cin.get();
	return 0;
}
