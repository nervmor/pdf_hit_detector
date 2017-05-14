#pragma once

#include "util_string.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp> 
#include <boost/filesystem/operations.hpp>  
#include <boost/format.hpp>

class util_path
{
public:
	static std::wstring get_image_dir()
	{
		std::wstring str_image_dir = boost::filesystem::initial_path<boost::filesystem::wpath>().wstring();
		if (str_image_dir[str_image_dir.size() - 1] != L'/')
		{
			str_image_dir += L"/";
		}
		return str_image_dir;
	}
	static std::wstring get_dir(const std::wstring& str_file_name)
	{
		boost::filesystem::wpath path(str_file_name);
		std::wstring str_dir = path.parent_path().wstring();
		if (str_dir[str_dir.size() - 1] != L'/')
		{
			str_dir += L"/";
		}
		return str_dir;
	}
	static std::wstring get_base_filename_without_ext(const std::wstring& str_file_name)
	{
		boost::filesystem::wpath path(str_file_name);
		return path.stem().wstring();
	}
	static bool make_sure_dir_exist(const std::wstring& str_dir)
	{
		boost::filesystem::wpath path_dir(str_dir);
		if (!boost::filesystem::exists(path_dir))
		{
			ENSUREf(boost::filesystem::create_directories(path_dir));
		}
		return true;
	}
	static bool delete_file(const std::wstring& str_file)
	{
		boost::filesystem::wpath path_file(str_file);
		ENSUREf(boost::filesystem::remove(path_file));
		return true;
	}
};