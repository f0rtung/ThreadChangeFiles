#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <sstream>

//��������� �������� ������ ��������� :
//��������� �������� � �������� ����� ���� � �����, � ������� ��������� �����.
//�.�.���������� ������ ������� ����������, � ��������� ������� ���������� ������� ��� ��������� ������� ����� �� �������������\ �� ���������������, ���� ������� ������� ���������� ���������� ������� ������������ ���������.� ������ ���� ��������
//���������� ������� ���� ������� ��� ����������� ��������� � �������.
//��������� ��������� ��� ���� ������, ����� ���� �������� ��������� ����� �
//������� ��������� ������.
//������ ������� ������� ����� ���� ������������ boost::thread_group.

static int sum = 0;

void readfile(std::vector<boost::filesystem::path>::iterator start, std::vector<boost::filesystem::path>::iterator end)
{
	while (start != end)
	{
		std::string path = start->string(); // �������������� ���� � ������
		std::ostringstream buff; // ����� ��� ����������
		std::ifstream file(path); 
		if (file.is_open()) // ��������� ���������� �������� �����, ���� �������, ��:
		{
			int num = 0;
			std::string strnum;
			file >> strnum; // ��������� ���������� ����� � ������
			try
			{
				num = boost::lexical_cast<int>(strnum.c_str()); // �������� ������ � ������, � ������ ������ - ����������
				sum += num; // ���� ���������� ������ �������, �� ����������� ����� ����� �� �������� �� �������� �����
				buff << path.substr(path.find_last_of("\\") + 1) << ": " << num << std::endl; // ��������� ���������� � �����
			}
			catch (const boost::bad_lexical_cast)
			{
				buff << "Error file: " << path << std::endl;
			}
			file.close(); // ��������� ����
			++start; // ��������� ���������
		}
		else
		{
			buff << "Error file: " << path << std::endl;
		}
		std::cout << buff.str(); // ������� ��������� � ����������� ������� ��������
	}
	boost::this_thread::sleep_for(boost::chrono::seconds(10));
}
int main(int argc, char** argv)
{
	if (argc == 1)
	{
		std::cout << "Enter file path!" << std::endl; // �������� ������� ���������� � ������
	}
	boost::filesystem::path filePath = argv[1]; // �������, ��� ���� ������ ����
	if (boost::filesystem::exists(filePath)) // �������� ������������� ����������
	{
		std::vector<boost::filesystem::path> pathVec; // ������ ��� �������� ����� �����
		std::copy_if(boost::filesystem::directory_iterator(filePath),
					 boost::filesystem::directory_iterator(), std::back_inserter(pathVec),	// ��������� � ������ ���� ������, �������������� � ����������
					 [](boost::filesystem::path file){return boost::filesystem::is_regular_file(file); });

		if (!pathVec.empty()) // ���� ������ �� ����, ������ � ���������� ���� �����
		{
			const size_t num_threads = boost::thread::hardware_concurrency() - 1; // ������������ ���������� "���������" �������, ��������� � ���������� �������, �� ������� ����������� ������
			const size_t block_size = pathVec.size() / num_threads; // ���������� ���������� ������, ������� ����� �������������� � ������ ������. 
																	//���� ���-�� ������ < ����� ��������� �������, �� ��� ����� ����� ���������� � ������� ������, ������ �� ���������
			auto block_start = pathVec.begin();
			std::vector<boost::thread> threads;
			if (block_size)
			{
				for (size_t i = 0; i < num_threads; ++i)
				{
					auto block_end = block_start;
					std::advance(block_end, block_size);
					threads.push_back(boost::thread(readfile, block_start, block_end)); // ��������� ����� � �������� ��� �������� �������������� ������
					block_start = block_end;
				}
			}
			readfile(block_start, pathVec.end()); // ����������� ��������� ���������� ������
			if (block_size) std::for_each(threads.begin(), threads.end(), std::mem_fn(&boost::thread::join));
		}
		std::cout << "Total sum: "<<sum;
	}
	else
		std::cout << "Incorrect dir!";
	return 0;
}