/**
 * \file    	TestStringCmp.hpp
 * \author  	XY0797
 * \date    	2024.8.6
 * \brief		比较程序输出与标准答案中的差异部分，并且以彩色输出差异部分的情况
 */
#ifndef _XY0797_TESTSTRINGCMP
#define _XY0797_TESTSTRINGCMP 1

#include <string>
#include <sstream>
#include <algorithm>

// 差异部分前面字符数的最大容许值
const int MAX_DIFF_PRE_SHOW_MAX = 30;

// 超出容许值后展示的字符数
const int MAX_DIFF_PRE_SHOW = 20;

// 差异部分字符数的最大容许值
const int MAX_DIFF_SHOW_MAX = 80;

// 超出容许值后展示的字符数
const int MAX_DIFF_SHOW = 50;

std::string m_TestERRStr;

// 输出差异到m_TestERRStr里面
// 差异从索引i开始出现
// (i可能是某一字符串的长度，但是不可能两个都是)
void genDiffMsg(const std::string& stdAns, const std::string& testAns, int i) {
	int Alen = stdAns.length();
	int Tlen = testAns.length();
	// 显示标准答案
	m_TestERRStr = "标准答案: \n";
	// 输出差异前面的部分
	if (i > MAX_DIFF_PRE_SHOW_MAX) {
		m_TestERRStr += "\x1b[1;37;100m...还有" + std::to_string(i - MAX_DIFF_PRE_SHOW)
		                + "个字符...\x1b[0m"
		                + stdAns.substr(i - MAX_DIFF_PRE_SHOW, MAX_DIFF_PRE_SHOW);
	} else {
		m_TestERRStr += stdAns.substr(0, i);
	}
	// 输出差异部分
	int ARemainLen = Alen - i;
	// 标答真实显示的差异部分
	int ARealShowLen = 0;
	if (ARemainLen == 0) {
		// 没有剩余的，啥都不干
	} else if (ARemainLen > MAX_DIFF_SHOW_MAX) {
		// 差异部分过长
		ARealShowLen = MAX_DIFF_SHOW;
		m_TestERRStr += stdAns.substr(i, MAX_DIFF_SHOW) + "\x1b[1;37;100m...还有"
		                + std::to_string(ARemainLen - MAX_DIFF_SHOW)
		                + "个字符...\x1b[0m";
	} else {
		// 正常输出所有
		ARealShowLen = ARemainLen;
		m_TestERRStr += stdAns.substr(i, ARemainLen);
	}
	// 显示程序输出，并且高亮显示差异
	m_TestERRStr += "\n程序输出：\n";
	// 显示差异前面的
	if (i > MAX_DIFF_PRE_SHOW_MAX) {
		m_TestERRStr += "\x1b[1;37;100m...还有" + std::to_string(i - MAX_DIFF_PRE_SHOW)
		                + "个字符...\x1b[0m"
		                + testAns.substr(i - MAX_DIFF_PRE_SHOW, MAX_DIFF_PRE_SHOW);
	} else {
		m_TestERRStr += testAns.substr(0, i);
	}
	// 输出差异部分
	int TRemainLen = Tlen - i;
	if (TRemainLen == 0) {
		// 说明末尾缺少了字符
		if (ARemainLen > MAX_DIFF_SHOW_MAX) {
			// 差异部分过长
			// 只输出ARealShowLen个红色空格
			m_TestERRStr += "\x1b[1;37;41m" + std::string(ARealShowLen, ' ')
			                + "\x1b[0m\x1b[1;37;100m..."
			                "还缺失" + std::to_string(ARemainLen - ARealShowLen)
			                + "个字符...\x1b[0m";
		} else {
			// 输出ARealShowLen个红色空格
			m_TestERRStr += "\x1b[1;37;41m" + std::string(ARealShowLen, ' ') + "\x1b[0m";
		}
		return;
	}
	std::string remainStr;
	// 标答真实显示的差异部分
	// 保证不会输出超过标答真实显示的差异部分
	// 因为标答不会超MAX_DIFF_SHOW_MAX，所以说这里也不会超MAX_DIFF_SHOW_MAX字符
	int TRealShowLen = std::min(TRemainLen, ARealShowLen);
	remainStr = testAns.substr(i, TRealShowLen);
	// 下面为isred作用域，通过该标志减少颜色字符的使用
	{
		bool isred = false;
		for (char ch : remainStr) {
			if (i < Alen) {
				// 没有比标答长，正常比较
				if (ch == stdAns[i]) {
					if (isred) {
						m_TestERRStr += "\x1b[0m";
						isred = false;
					}
					m_TestERRStr += ch;
				} else {
					if (!isred) {
						m_TestERRStr += "\x1b[1;37;41m";
						isred = true;
					}
					m_TestERRStr += ch;
				}
				++i;
			} else {
				// 比标答长，一定错
				if (!isred) {
					m_TestERRStr += "\x1b[1;37;41m";
					isred = true;
				}
				m_TestERRStr += ch;
			}
		}
		if (TRealShowLen < ARealShowLen) {
			// 说明末尾缺少了字符
			if (!isred) {
				m_TestERRStr += "\x1b[1;37;41m";
				isred = true;
			}
			m_TestERRStr += std::string(ARealShowLen - TRealShowLen, ' ');
			TRealShowLen = ARealShowLen;
		}
		// 执行到这里，两者实际显示长度一定相等

		// 如果标准答案显示完全了，程序输出却没有
		if (ARemainLen == ARealShowLen && TRealShowLen < TRemainLen) {
			if (TRemainLen <= MAX_DIFF_SHOW_MAX) {
				// 末尾比标准答案多出的字符可完全展示出来
				if (!isred) {
					m_TestERRStr += "\x1b[1;37;41m";
					isred = true;
				}
				m_TestERRStr += testAns.substr(i, TRemainLen - TRealShowLen);
				TRealShowLen = TRemainLen;
			} else if (TRealShowLen < MAX_DIFF_SHOW) {
				// 可显示到MAX_DIFF_SHOW
				if (!isred) {
					m_TestERRStr += "\x1b[1;37;41m";
					isred = true;
				}
				m_TestERRStr += testAns.substr(i, MAX_DIFF_SHOW - TRealShowLen);
				TRealShowLen = MAX_DIFF_SHOW;
			}
		}

		if (isred) {
			m_TestERRStr += "\x1b[0m";
		}
	}

	// 判断折叠的问题
	if (TRemainLen > TRealShowLen) {
		// 需要折叠显示
		m_TestERRStr += "\x1b[1;37;100m...还有"
		                + std::to_string(TRemainLen - TRealShowLen)
		                + "个字符...\x1b[0m";
		if (ARemainLen > TRemainLen) {
			m_TestERRStr += " \x1b[1;37;41m...还缺失"
			                + std::to_string(ARemainLen - TRemainLen)
			                + "个字符...\x1b[0m";
		}
		return;
	} else {
		// TRemainLen已经展示完全了
		if (ARemainLen > TRealShowLen) {
			m_TestERRStr += "\x1b[1;37;100m...还缺失"
			                + std::to_string(ARemainLen - TRealShowLen)
			                + "个字符...\x1b[0m";
		}
	}
}


// 比较是否相等，如果不相等会将差异以人类可读形式写在m_TestERRStr里面
bool testStringCmp(const std::string& stdAns, const std::string& testAns) {
	bool isAEmpty = stdAns.empty();
	bool isTEmpty = testAns.empty();
	int Alen = stdAns.length();
	int Tlen = testAns.length();
	if (isAEmpty && isTEmpty) return true;
	// 有一个为空或都不为空
	if (isAEmpty) {
		// 程序输出有多余
		m_TestERRStr = "标准答案: \n\x1b[1;37;100m   空   \x1b[0m\n程序输出：\n";
		if (Tlen > MAX_DIFF_SHOW_MAX) {
			// 差异部分过长
			m_TestERRStr += "\x1b[1;37;41m"
			                + testAns.substr(0, MAX_DIFF_SHOW)
			                + "\x1b[0m\x1b[1;37;100m..."
			                "还多余" + std::to_string(Tlen - MAX_DIFF_SHOW)
			                + "个字符...\x1b[0m";
		} else {
			// 全部输出
			m_TestERRStr += "\x1b[1;37;41m" + testAns + "\x1b[0m";
		}
		return false;
	} else if (isTEmpty) {
		m_TestERRStr = "标准答案: \n";
		if (Alen > MAX_DIFF_SHOW_MAX) {
			// 差异部分过长
			m_TestERRStr += stdAns.substr(0, MAX_DIFF_SHOW) + "\x1b[1;37;100m..."
			                "还有" + std::to_string(Alen - MAX_DIFF_SHOW)
			                + "个字符...\x1b[0m";
		} else {
			// 全部输出
			m_TestERRStr += "\x1b[1;37;41m" + stdAns + "\x1b[0m";
		}
		m_TestERRStr += "\n程序输出：\n\x1b[1;37;41m   空   \x1b[0m";
		return false;
	}
	// 都不为空
	int i = 0;
	while (i < Alen && i < Tlen) {
		if (stdAns[i] != testAns[i]) {
			// 中间WA了
			genDiffMsg(stdAns, testAns, i);
			return false;
		}
		++i;
	}
	if (i == Alen && i == Tlen) {
		return true; // 说明完全一致
	}
	// 尾部异常
	genDiffMsg(stdAns, testAns, i);
	return false;
}

// 去除末尾空格
void trim(std::string& str) {
	int endi = str.length() - 1;
	while (endi >= 0) {
		if (str[endi] == ' ') {
			str.erase(endi, 1);
			--endi;
		} else {
			return;
		}
	}
}

// 比较标准答案和程序输出的区别，返回是否正确
// 如果不正确会将差异以人类可读形式写在m_TestERRStr里面
bool compareAnsStr(const std::string &stdansStr,
                   const std::string &myansStr, bool isStrict = false) {
	std::istringstream standardStream(stdansStr);
	std::istringstream testStream(myansStr);

	int lencnt = 0;
	std::string standardLine, testLine;
	bool isStdAnsOK = !std::getline(standardStream, standardLine).fail();
	bool isTestAnsOK = !std::getline(testStream, testLine).fail();
	while (isStdAnsOK && isTestAnsOK) {
		++lencnt;
		if (isStrict) {
			if (!testStringCmp(standardLine, testLine)) {
				// 不一致
				m_TestERRStr = "第一处差异在第" + std::to_string(lencnt)
				               + "行\n" + m_TestERRStr;
				return false;
			}
		} else {
			// 去尾空格
			trim(standardLine);
			trim(testLine);
			if (!testStringCmp(standardLine, testLine)) {
				// 不一致
				m_TestERRStr = "第一处差异在第" + std::to_string(lencnt)
				               + "行\n" + m_TestERRStr;
				return false;
			}
		}
		isStdAnsOK = !std::getline(standardStream, standardLine).fail();
		isTestAnsOK = !std::getline(testStream, testLine).fail();
	}

	// 检查两个输出是否同时到达结尾
	if (!isStdAnsOK && !isTestAnsOK) {
		// 一致
		return true;
	}

	int difflen = lencnt + 1;
	if (isStdAnsOK) {
		// 程序输出缺失了行
		int testAllLenCnt = lencnt;
		std::string tmp;
		++lencnt;
		while (!std::getline(standardStream, tmp).fail()) {
			++lencnt;
		}
		m_TestERRStr = "第一处差异在第" + std::to_string(difflen)
		               + "行\n标准答案: \n";
		int ALen = standardLine.length();
		if (ALen > MAX_DIFF_SHOW_MAX) {
			m_TestERRStr += standardLine.substr(0, MAX_DIFF_SHOW)
			                + "\x1b[1;37;100m...还有"
			                + std::to_string(ALen - MAX_DIFF_SHOW)
			                + "个字符...\x1b[0m";
		} else {
			m_TestERRStr += standardLine;
		}
		m_TestERRStr += "\n程序输出：\n\x1b[1;37;41m   空   \x1b[0m\n";
		m_TestERRStr += "标准答案共" + std::to_string(lencnt)
		                + "行，程序输出共" + std::to_string(testAllLenCnt) + "行";
	} else {
		// 程序输出多余了行
		int ansAllLenCnt = lencnt;
		std::string tmp;
		++lencnt;
		while (!std::getline(testStream, tmp).fail()) {
			++lencnt;
		}
		m_TestERRStr = "第一处差异在第" + std::to_string(difflen)
		               + "行\n标准答案: \n\x1b[1;37;100m   空   \x1b[0m\n程序输出：\n";
		int Tlen = testLine.length();
		if (Tlen > MAX_DIFF_SHOW_MAX) {
			m_TestERRStr += "\x1b[1;37;41m" + testLine.substr(0, MAX_DIFF_SHOW)
			                + "\x1b[0m\x1b[1;37;100m..."
			                "还多余" + std::to_string(Tlen - MAX_DIFF_SHOW)
			                + "个字符...\x1b[0m";
		} else {
			m_TestERRStr += "\x1b[1;37;41m" + testLine + "\x1b[0m\n";
		}
		m_TestERRStr += "标准答案共" + std::to_string(ansAllLenCnt)
		                + "行，程序输出共" + std::to_string(lencnt) + "行";
	}
	return false;
}

#endif /* _XY0797_TESTSTRINGCMP */
