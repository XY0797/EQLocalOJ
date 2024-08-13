/**
 * \file    	WindowsFileSysTool.hpp
 * \author  	XY0797
 * \date    	2024.8.6
 * \brief		提供Windows下的文件系统工具函数
 */
#ifndef _XY0797_WINDOWSFILESYSTOOL
#define _XY0797_WINDOWSFILESYSTOOL 1

#include <string>
#include <regex>
#include <vector>
#include <functional>
#include <windows.h>

// 不得修改和读取该命名空间的内容
// 请使用winfs提供的函数读取
namespace INTERNAL_winfs_DO_NOT_READ_OR_EDIT {
	std::string curEXEPath;
	std::string curEXEParentDirectoryPath;
}

namespace winfs {
	// 判断一个文件是否存在
	// 支持判断目录是否存在，末尾有无\皆可
	bool isFileExist(const std::string& filePath) {
		if (filePath.empty()) {
			return false;
		}
		DWORD attributes = GetFileAttributes(filePath.c_str());
		return attributes != INVALID_FILE_ATTRIBUTES;
	}

	// 判断一个windows路径是否为目录
	bool isDir(const std::string& pathStr) {
		if (pathStr == ".") {
			return true;
		}
		size_t i1 = pathStr.rfind("\\");
		if (i1 == pathStr.npos) {
			return false;
		}
		return (pathStr.find(".", i1) == pathStr.npos);
	}

	// 获取文件全路径的父目录，返回不带\的目录路径，失败返回空文本
	std::string getFilesParentDirectory(const std::string& fileFullPath) {
		size_t i1 = fileFullPath.rfind("\\");
		if (i1 == fileFullPath.npos) {
			// 是否为Linux风格
			i1 = fileFullPath.rfind("/");
			if (i1 == fileFullPath.npos) {
				// 说明不是全路径
				return std::string();
			}
		}
		return fileFullPath.substr(0, i1);
	}

	// 获取文件后缀，含.号
	std::string getFileExtenWithDot(const std::string& filePath) {
		size_t i1 = filePath.rfind(".");
		if (i1 == filePath.npos) {
			return std::string();
		}
		return filePath.substr(i1, filePath.length() - i1);
	}

	// 获取文件后缀，不含.号
	std::string getNonDotFileExten(const std::string& filePath) {
		size_t i1 = filePath.rfind(".");
		if (i1 == filePath.npos) {
			return std::string();
		}
		return filePath.substr(i1 + 1, filePath.length() - i1 - 1);
	}

	// 获取无后缀的文件名
	std::string getNonExtenFileName(const std::string& filePath) {
		size_t i1 = filePath.rfind("\\");
		if (i1 == filePath.npos) {
			// 是否为Linux风格
			i1 = filePath.rfind("/");
			if (i1 == filePath.npos) {
				// 说明不是全路径，只有文件名
				i1 = filePath.rfind(".");
				if (i1 == filePath.npos) {
					// 无后缀文件名
					return filePath;
				} else {
					// 去除后缀返回
					return filePath.substr(0, i1);
				}
			}
		}
		// 是全路径，倒找后缀
		size_t i2 = filePath.rfind(".");
		if (i2 != filePath.npos && i2 < i1) {
			// 实际上无后缀
			i2 = filePath.npos;
		}
		if (i2 == filePath.npos) {
			// 无后缀文件名
			return filePath.substr(i1 + 1, filePath.length() - i1 - 1);
		} else {
			// 去除后缀返回
			return filePath.substr(i1 + 1, i2 - i1 - 1);
		}
	}

	// 获取含后缀的文件名
	std::string getFileNameWithExten(const std::string& filePath) {
		size_t i1 = filePath.rfind("\\");
		if (i1 == filePath.npos) {
			// 是否为Linux风格
			i1 = filePath.rfind("/");
			if (i1 == filePath.npos) {
				// 说明不是全路径，只有文件名
				return filePath;
			}
		}
		// 是全路径
		return filePath.substr(i1 + 1, filePath.length() - i1 - 1);
	}

	using StringCombineFunction =
	    std::function < std::string(const std::string&, const std::string&) >;

	// 枚举目录下的文件
	// 文件名示例：*.txt
	// func为压入vector的字符串处理函数
	// 参数1为目录路径，参数2为文件名，返回最终要压入vector的字符串，返回空则不压入
	// 默认行为是拼接为全路径压入vector
	std::vector<std::string> getFilesOfDirectory(const std::string& directoryPath,
	        const std::string& searchFileName,
	        StringCombineFunction func =
	            /* 实现默认行为 */
	            [](const std::string& a, const std::string& b)
	            -> std::string {return a + "\\" + b;}
	                                            ) {
		// 构造查找模式
		std::string searchPattern;
		// 根据末尾字符采用不同方法拼接
		// 同时统一斜杆风格
		static std::regex ForwardSlash_Regex("/");
		auto endChar = directoryPath[directoryPath.size() - 1];
		if (endChar == '\\' || endChar == '/') {
			searchPattern =
			    std::regex_replace(directoryPath + searchFileName,
			                       ForwardSlash_Regex, "\n");
		} else {
			searchPattern =
			    std::regex_replace(directoryPath + "\\" + searchFileName,
			                       ForwardSlash_Regex, "\n");
		}

		// 创建一个 vector 存储所有的文件路径
		std::vector<std::string> inFiles;

		// 初始化 FindFile 搜索句柄
		HANDLE hFind = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAA findData;

		// 开始搜索
		hFind = FindFirstFileA(searchPattern.c_str(), &findData);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				std::string fileName = findData.cFileName;
				// 调用func获取最终压入的字符串
				std::string finalPath = func(directoryPath, fileName);
				if (!finalPath.empty()) {
					// 非空才压入
					inFiles.push_back(finalPath);
				}
			} while (FindNextFileA(hFind, &findData) != 0);
			// 关闭搜索句柄
			FindClose(hFind);
		} else {
			throw std::runtime_error("路径无效或指定路径下无任何测试集文件！");
		}

		return inFiles;
	}

	using namespace INTERNAL_winfs_DO_NOT_READ_OR_EDIT;

	// 获取当前进程的EXE路径
	const std::string& getCurEXEPath() {
		if (curEXEPath.empty()) {
			curEXEPath.assign(_pgmptr);
		}
		return curEXEPath;
	}

	// 获取当前进程的EXE所在目录路径，不以\结尾
	const std::string& getCurEXEParentDirectoryPath() {
		if (curEXEParentDirectoryPath.empty()) {
			curEXEParentDirectoryPath.assign(getFilesParentDirectory(getCurEXEPath()));
		}
		return curEXEParentDirectoryPath;
	}
}

#endif /* _XY0797_WINDOWSFILESYSTOOL */
