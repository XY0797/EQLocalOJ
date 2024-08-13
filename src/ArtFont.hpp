/**
 * \file    	ArtFont.hpp
 * \author  	XY0797
 * \date    	2024.8.6
 * \brief		定义AC、WA、TLE、MLE、Start Failed的艺术字
 */
#ifndef _XY0797_ARTFONT
#define _XY0797_ARTFONT 1

#include <string>

// 均为默认底色

// 绿色字体
const std::string artAC
    = "\x1b[1;32;49m"
      "   ___   _____\n"
      "  / _ | / ___/\n"
      " / __ |/ /__  \n"
      "/_/ |_|\\___/  "
      "\x1b[0m";

// 红色字体
const std::string artWA
    = "\x1b[1;31;49m"
      "  _      __   ___ \n"
      " | | /| / /  / _ |\n"
      " | |/ |/ /  / __ |\n"
      " |__/|__/  /_/ |_|"
      "\x1b[0m";

// 默认色字体
const std::string artTLE
    = " ______ __    ____\n"
      "/_  __// /   / __/\n"
      " / /  / /__ / _/  \n"
      "/_/  /____//___/  ";

// 默认色字体
const std::string artMLE
    = "   __  ___ __    ____\n"
      "  /  |/  // /   / __/\n"
      " / /|_/ // /__ / _/  \n"
      "/_/  /_//____//___/  ";

// 蓝色字体
const std::string artStartFailed
    = "\x1b[1;34;49m"
      "   ____ __              __     ____       _  __         __\n"
      "  / __// /_ ___ _ ____ / /_   / __/___ _ (_)/ /___  ___/ /\n"
      " _\\ \\ / __// _ `// __// __/  / _/ / _ `// // // -_)/ _  / \n"
      "/___/ \\__/ \\_,_//_/   \\__/  /_/   \\_,_//_//_/ \\__/ \\_,_/ "
      "\x1b[0m";

#endif /* _XY0797_ARTFONT */
