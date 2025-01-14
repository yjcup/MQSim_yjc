#include "XMLWriter.h"
#include "../sim/Engine.h"
#include <sys/stat.h>

namespace Utils
{
	bool XmlWriter::exists(const std::string fileName)
	{
		std::fstream checkFile(fileName);
		return checkFile.is_open();
	}

	bool CreateDirectoryIfNeeded(const std::string &dirPath)
	{
		// 创建目录，如果目录已存在，则返回 true
		if (mkdir(dirPath.c_str(), 0777) == 0 || errno == EEXIST)
		{
			return true;
		}
		return false;
	}

	bool XmlWriter::Open(const std::string strFile)
	{

		outFile.open(strFile);
		if (outFile.is_open())
		{
			outFile << "<?xml version=\"1.0\" encoding=\"us-ascii\"?>\n";
			indent = 0;
			openTags = 0;
			openElements = 0;

			return true;
		}

		return false;
	}

	bool XmlWriter::Open(const std::string strFile, const std::string timeStr)
	{
		// 获取当前时间作为文件夹名
		// std::string timeStr = GetCurrentTimeAsString();

		// 基于当前时间构建文件夹路径
		std::string dirPath = "./result/" + timeStr; // 使用时间戳创建文件夹路径，路径格式是 ./2024-12-15_14-30-45

		// 创建目录（如果它不存在）
		if (!CreateDirectoryIfNeeded(dirPath))
		{
			std::cerr << "Error creating directory for time: " << timeStr << std::endl;
			return false;
		}

		// 构建完整的文件路径：文件夹路径 + 文件名
		std::string fullFilePath = dirPath + "/" + strFile; // 假设 strFile 是文件名

		outFile.open(fullFilePath);
		if (outFile.is_open())
		{
			outFile << "<?xml version=\"1.0\" encoding=\"us-ascii\"?>\n";
			indent = 0;
			openTags = 0;
			openElements = 0;

			return true;
		}

		return false;
	}

	void XmlWriter::Close()
	{
		if (outFile.is_open())
		{
			outFile.close();
		}
	}

	void XmlWriter::Write_open_tag(const std::string openTag)
	{
		if (outFile.is_open())
		{
			for (int i = 0; i < indent; i++)
			{
				outFile << "\t";
			}
			tempOpenTag.resize(openTags + 1);
			outFile << "<" << openTag << ">\n";
			tempOpenTag[openTags] = openTag;
			indent += 1;
			openTags += 1;
		}
		else
		{
			PRINT_ERROR("The XML output file is closed. Unable to write to file");
		}
	}

	void XmlWriter::Write_attribute_string(const std::string attribute_name, const std::string attribute_value)
	{
		if (outFile.is_open())
		{
			for (int i = 0; i < indent + 1; i++)
			{
				outFile << "\t";
			}

			outFile << " <" << attribute_name + ">" + attribute_value + "</" << attribute_name + ">\n";
		}
		else
		{
			PRINT_ERROR("The XML output file is closed. Unable to write to file");
		}
	}

	void XmlWriter::Write_close_tag()
	{
		if (outFile.is_open())
		{
			indent -= 1;
			for (int i = 0; i < indent; i++)
			{
				outFile << "\t";
			}
			outFile << "</" << tempOpenTag[openTags - 1] << ">\n";
			tempOpenTag.resize(openTags - 1);
			openTags -= 1;
		}
		else
		{
			PRINT_ERROR("The XML output file is closed. Unable to write to file");
		}
	}

	void XmlWriter::Write_start_element_tag(const std::string elementTag)
	{
		if (outFile.is_open())
		{
			for (int i = 0; i < indent; i++)
			{
				outFile << "\t";
			}
			tempElementTag.resize(openElements + 1);
			tempElementTag[openElements] = elementTag;
			openElements += 1;
			outFile << "<" << elementTag;
		}
		else
		{
			PRINT_ERROR("The XML output file is closed. Unable to write to file");
		}
	}

	void XmlWriter::Write_end_element_tag()
	{
		if (outFile.is_open())
		{
			outFile << "/>\n";
			tempElementTag.resize(openElements - 1);
			openElements -= 1;
		}
		else
		{
			PRINT_ERROR("The XML output file is closed. Unable to write to file");
		}
	}

	void XmlWriter::Write_attribute(const std::string outAttribute)
	{
		if (outFile.is_open())
		{
			outFile << " " << outAttribute;
		}
		else
		{
			PRINT_ERROR("The XML output file is closed. Unable to write to file");
		}
	}

	void XmlWriter::Write_attribute_string_inline(const std::string attribute_name, const std::string attribute_value)
	{
		if (outFile.is_open())
		{
			outFile << " ";
			outFile << attribute_name + "=\"" + attribute_value + "\"";
		}
		else
		{
			PRINT_ERROR("The XML output file is closed. Unable to write to file");
		}
	}

	void XmlWriter::Write_string(const std::string outString)
	{
		if (outFile.is_open())
		{
			outFile << ">" << outString;
		}
		else
		{
			PRINT_ERROR("The XML output file is closed. Unable to write to file");
		}
	}
}