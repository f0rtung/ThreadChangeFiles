#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <sstream>

//Небольшое описание работы программы :
//Программа получает в качестве ключа путь к папке, в которой находятся файлы.
//Т.к.количество файлов заранее неизвестно, а создавать большое количество потоков для обработки каждого файла не целесообразно\ не производительно, было принято решение ограничить количество потоков определенным значением.В именно этом варианте
//количество потоков было выбрано как максимально возможное в системе.
//Программа считывает все пути файлов, после чего передает диапазоны путей в
//функцию обработки файлов.
//Вместо вектора потоков можно было использовать boost::thread_group.

static int sum = 0;

void readfile(std::vector<boost::filesystem::path>::iterator start, std::vector<boost::filesystem::path>::iterator end)
{
	while (start != end)
	{
		std::string path = start->string(); // преобразование пути в строку
		std::ostringstream buff; // буфер для накопления
		std::ifstream file(path); 
		if (file.is_open()) // проверяем успешность открытия файла, если успешно, то:
		{
			int num = 0;
			std::string strnum;
			file >> strnum; // считываем содержимое файла в строку
			try
			{
				num = boost::lexical_cast<int>(strnum.c_str()); // приводим строку к целому, в случае ошибки - исключение
				sum += num; // если приведение прошло успешно, то увеличиваем общую сумму на значение из текущего файла
				buff << path.substr(path.find_last_of("\\") + 1) << ": " << num << std::endl; // добавляем информацию в буфер
			}
			catch (const boost::bad_lexical_cast)
			{
				buff << "Error file: " << path << std::endl;
			}
			file.close(); // закрываем файл
			++start; // переводим указатель
		}
		else
		{
			buff << "Error file: " << path << std::endl;
		}
		std::cout << buff.str(); // выводим сообщение о результатах текущей операции
	}
	boost::this_thread::sleep_for(boost::chrono::seconds(10));
}
int main(int argc, char** argv)
{
	if (argc == 1)
	{
		std::cout << "Enter file path!" << std::endl; // проверка запуска приложения с ключем
	}
	boost::filesystem::path filePath = argv[1]; // считаем, что ключ только один
	if (boost::filesystem::exists(filePath)) // проверка существования директории
	{
		std::vector<boost::filesystem::path> pathVec; // вектор для хранения путей фалой
		std::copy_if(boost::filesystem::directory_iterator(filePath),
					 boost::filesystem::directory_iterator(), std::back_inserter(pathVec),	// вставляем в вектор пути файлов, располложенных в директории
					 [](boost::filesystem::path file){return boost::filesystem::is_regular_file(file); });

		if (!pathVec.empty()) // если вектор не пуст, значит в директории есть файлы
		{
			const size_t num_threads = boost::thread::hardware_concurrency() - 1; // максимальное количество "настоящих" потоков, доступных в конкретной системе, за вычетом работающего потока
			const size_t block_size = pathVec.size() / num_threads; // определяем количество файлов, которые будут обрабатываться в каждом потоке. 
																	//Если кол-во файлов < числа возможных потоков, то все файлы можно обработать в текущем потоке, другие не создаются
			auto block_start = pathVec.begin();
			std::vector<boost::thread> threads;
			if (block_size)
			{
				for (size_t i = 0; i < num_threads; ++i)
				{
					auto block_end = block_start;
					std::advance(block_end, block_size);
					threads.push_back(boost::thread(readfile, block_start, block_end)); // добавляем поток и передаем ему диапазон обрабатываемых файлов
					block_start = block_end;
				}
			}
			readfile(block_start, pathVec.end()); // завершающая обработка оставшихся файлов
			if (block_size) std::for_each(threads.begin(), threads.end(), std::mem_fn(&boost::thread::join));
		}
		std::cout << "Total sum: "<<sum;
	}
	else
		std::cout << "Incorrect dir!";
	return 0;
}