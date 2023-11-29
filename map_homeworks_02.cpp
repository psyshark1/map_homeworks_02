#include<iostream>
#include<thread>
#include<chrono>
#include<vector>
#include<random>
#include <execution>
#include<condition_variable>

constexpr const int size1 = 1000;
constexpr const int size2 = 10000;
constexpr const int size3 = 100000;
constexpr const int size4 = 1000000;
std::once_flag fl;

void fill_vector(std::vector<int>& v, const int& size)
{
	std::mt19937 gen;
	std::uniform_int_distribution<int> dist(0, size);
	std::generate(std::execution::par, v.begin(), v.end(), [&]()
		{
			return dist(gen);
		});
}

void print_cores()
{
	std::cout << "Numbers of hardware cores: " << std::thread::hardware_concurrency() << std::endl << std::endl <<
		'\t' << '\t' << 1000 << '\t' << '\t' << 10000 << '\t' << '\t' << 100000 << '\t' << '\t' << 1000000 << std::endl;
}

void calculate_vectors_thread(std::vector<int>& v1, std::vector<int>& v2, int idx_start, int idx_end)
{
	std::call_once(fl, print_cores);
	for (; idx_start <= idx_end; ++idx_start)
	{
		v1[idx_start] + v2[idx_start];
	}
}

void calculate_vectors(std::vector<int>& v1, std::vector<int>& v2, const int& thread_count, const int& size, std::chrono::duration<double>**& table, const int& t_row, const int& t_col)
{
	int parts = size / thread_count;
	
	switch (thread_count)
	{
		case 1:
			{
				auto start = std::chrono::steady_clock::now();
				std::thread t1(calculate_vectors_thread, std::ref(v1), std::ref(v2), 0, size - 1);
				if (t1.joinable()) { t1.join(); }
				auto end = std::chrono::steady_clock::now();
				table[t_row][t_col] = (end - start);
			}
			break;
		case 2:
			{
				auto start = std::chrono::steady_clock::now();
				std::thread t1(calculate_vectors_thread, std::ref(v1), std::ref(v2), 0, parts);
				std::thread t2(calculate_vectors_thread, std::ref(v1), std::ref(v2), parts, size - 1);
				if (t1.joinable()) { t1.join(); }
				if (t2.joinable()) { t2.join(); }
				auto end = std::chrono::steady_clock::now();
				table[t_row][t_col] = (end - start);
			}
			break;
		case 4:
			{
				int end_control{ 0 };
				std::vector<std::thread> vt;
				auto start = std::chrono::steady_clock::now();
				for (int i = 0; i < size; i += parts)
				{
					end_control = i + parts - 1;
					vt.push_back(std::thread(calculate_vectors_thread, std::ref(v1), std::ref(v2), i, end_control));
				}
				for (auto& t : vt)
				{
					t.join();
				}
				auto end = std::chrono::steady_clock::now();
				table[t_row][t_col] = (end - start);
			}
			break;
		case 8:
			{
				int end_control{ 0 };
				std::vector<std::thread> vt;
				auto start = std::chrono::steady_clock::now();
				for (int i = 0; i < size; i += parts)
				{
					end_control = i + parts - 1;
					vt.push_back(std::thread(calculate_vectors_thread, std::ref(v1), std::ref(v2), i, end_control));
				}
				for (auto& t : vt)
				{
					t.join();
				}
				auto end = std::chrono::steady_clock::now();
				table[t_row][t_col] = (end - start);
			}
			break;
		case 16:
			{
				int end_control{ 0 };
				std::vector<std::thread> vt;
				auto start = std::chrono::steady_clock::now();
				for (int i = 0; i < size; i += parts)
				{
					end_control = i + parts - 1;
					if (end_control > size){ end_control = size - 1; }
					vt.push_back(std::thread(calculate_vectors_thread, std::ref(v1), std::ref(v2), i, end_control));
				}
				for (auto& t : vt)
				{
					t.join();
				}
				auto end = std::chrono::steady_clock::now();
				table[t_row][t_col] = (end - start);
			}
			break;
		default:
			break;
	}
}

std::chrono::duration<double>** create_table(const short& rows, const short& cols)
{
	std::chrono::duration<double>** tmp = new std::chrono::duration<double>* [rows];

	for (unsigned short i = 0; i < rows; ++i)
	{
		tmp[i] = new std::chrono::duration<double>[cols];
	}
	return tmp;
}

void delete_table(std::chrono::duration<double>**& arr, const short& rows)
{
	for (unsigned short i = 0; i < rows; ++i)
	{
		delete[] arr[i];
	}
	delete[] arr;
}

void print_result_table(std::chrono::duration<double>**& arr, const short& rows, const short& cols)
{
	short threads{ 1 };
	for (unsigned short i = 0; i < rows; ++i)
	{
		std::cout << threads << " threads:" << '\t';
		for (unsigned short j = 0; j < cols; ++j)
		{
			std::cout << arr[i][j].count() << '\t';
		}
		threads <<= 1;
		std::cout << std::endl;
	}
}

int main()
{
	std::chrono::duration<double>** result_table = create_table(5, 4);

	std::vector<int> v11(size1);
	std::vector<int> v12(size1);
	std::vector<int> v21(size2);
	std::vector<int> v22(size2);
	std::vector<int> v31(size3);
	std::vector<int> v32(size3);
	std::vector<int> v41(size4);
	std::vector<int> v42(size4);

	fill_vector(v11, size1);
	fill_vector(v12, size1);
	fill_vector(v21, size2);
	fill_vector(v22, size2);
	fill_vector(v31, size3);
	fill_vector(v32, size3);
	fill_vector(v41, size4);
	fill_vector(v42, size4);

	for (int i = 0, j = 1; i < 5; ++i, j <<= 1)
	{
		calculate_vectors(v11, v12, j, size1, result_table, i, 0);
		calculate_vectors(v21, v22, j, size2, result_table, i, 1);
		calculate_vectors(v31, v32, j, size3, result_table, i, 2);
		calculate_vectors(v41, v42, j, size4, result_table, i, 3);
	}

	print_result_table(result_table, 5, 4);

	delete_table(result_table, 5);

	return 0;
}
