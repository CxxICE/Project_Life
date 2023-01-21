#if (_WIN32 || _WIN64)
#include "windows.h"
#define SCREEN_CLEAR "cls"
#endif
#if (unix || __unix || __unix__)
#include <unistd.h>
#define SCREEN_CLEAR "clear"
#endif

#define LIFE '*'
#define DEAD '-'

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <ctime>	//для лучайного заполнения поля игры
#include <thread>	//для альтернативного платформонезависимого таймера
#include <chrono>	//для альтернативного платформонезависимого таймера

void fIntro();	//сигнатура заставки входа в игру
void fExit();	//сигнатура заставки выхода из игры

// функция запроса количества чего-либо (строк, столбцов ...) 
// с проверкой корректности ввода целочисленного значения
int fReadString(std::string requestStr)
{
	std::string tmp;	//переменная приема строки
	bool tmpErr = 0; 	//наличие недопустимых символов в строке = 1
	int requestDigit;	//переменная возврата числового значения из функции
	do
	{
	if (tmpErr)
	{
		std::system(SCREEN_CLEAR);
		std::cout	<< "Incorrect entering! Try again.\n"
					<< "Enter number of " << requestStr << ": ";
	}
	else
	{
		std::cout	<< "Enter number of " << requestStr << ": ";
	}
	tmpErr = 0;
	tmp = "";
	std::getline(std::cin, tmp);
	if (tmp != "")
	{
		//посимвольная проверка строки на остутствие нечисловых символов
		for (int j = 0; j < tmp.length(); ++j)
		{
			if ((static_cast<int>(tmp[j]) < 48) || (static_cast<int>(tmp[j]) > 57))
			{
				tmpErr = 1;
				break;
			}
		}
		if (!tmpErr)
		{
			requestDigit = std::stoi(tmp);
		}
	}
	else
		{
			tmpErr = 1;
		}
	} while (tmpErr);
	return requestDigit;
}

//функция выделения памяти под динамический массив bool c обнулением элементов
bool **fCreateArr(const int ROWS,const int COLUMNS)
{
	bool **createArr;
	createArr = new bool *[ROWS];
	for (int j = 0; j < ROWS; ++j)
	{
		createArr[j] = new bool[COLUMNS] {};
	}
	return createArr;
}


//функция инициализации начального состояния вселенной случайными сислами или числами из файла
//принимает признак startRandom - для случайного заполнения и суфикс файла для неслучайного заполнения
//при успешной инициализации выделяет память под динамический массив и возвращает соответствующий указатель
//также возвращает размер поля и ошибку чтения файла через переменную fileErr по ссылке
bool **fInit(const std::string start, const bool startRandom, int &rows, int &columns, bool &fileErr)
{
	bool **initArr = nullptr;
	if (startRandom) //инициализация случайными числами
	{
		std::system(SCREEN_CLEAR);
		// запрос количества строк с проверкой корректности ввода
		rows = fReadString("rows");

		// запрос количества колонок с проверкой корректности ввода
		columns = fReadString("columns");

		//выделение памяти под динамический массив
		initArr = fCreateArr(rows, columns);

		//заполнение динамического массива случайными числами 0 и 1
		std::srand(time(NULL));
		for (int j = 0; j < rows; ++j)
		{
			for (int k = 0; k < columns; ++k)
			{
				initArr[j][k] = static_cast<bool>(std::rand() % 2);
			}
		}
	}
	else		//инициализация из файла
	{
		std::string fileName;
		std::ifstream initFile;
		fileName = "ini_" + start + ".txt";
		initFile.open(fileName);
		if (initFile.is_open())
		{
			initFile >> rows >> columns;
			//выделение памяти под динамический массив
			initArr = fCreateArr(rows, columns);

			//заполнение динамического массива данными из файла
			{
				int j, k; //переменные с ограниченной областью видимости
				while (initFile >> j >> k)
					{
					initArr[j][k] = 1;
					}
			}
			initFile.close();
		}
		else
		{
			// возврат ошибки, если файл не открылся
			fileErr = 1;
		}
	}
	return initArr;
}

//функция копирования массивов между сосбой из inputArr в outputArr
void fCopy(bool **outputArr, const bool *const *const inputArr, const int ROWS, const int COLUMNS)
{
	for (int i = 0; i < ROWS; ++i)
	{
		for (int j = 0; j < COLUMNS; ++j)
		{
			outputArr[i][j] = inputArr[i][j];
		}
	}
}

//функция расчета следующего поколения, новое состояние записывает в outputArr
//возвращает единицу, если нет изменений относительно предыдущего состояния
bool fGenNext(bool **outputArr, const bool *const *const inputArr, const int ROWS, const int COLUMNS)
{
	short lifes = 0;
	bool uStable = 1;
	for (int i = 0; i < ROWS; ++i)
	{
		for (int j = 0; j < COLUMNS; ++j)
		{	
			//подсчет числа живых клеток вокруг текущей клетки
			for (int k = (i - 1); k <= (i + 1); ++k)
			{
				for (int m = (j - 1); m <= (j + 1); ++m)
				{
					if (!((k == i) && (m == j)) && (k >= 0) && (k < ROWS) && (m >= 0) && (m < COLUMNS))
					{
						if (inputArr[k][m] == 1) { ++lifes; }
					}
				}
			}

			//определение состояния клетки исходя из кол-ва живых вокург нее
			if (lifes == 3)
			{
				outputArr[i][j] = 1;
			}
			else if (lifes == 2)
			{
				outputArr[i][j] = inputArr[i][j];
			}
			else
			{
				outputArr[i][j] = 0;
			}
			uStable = (outputArr[i][j] == inputArr[i][j]) ? uStable : 0;
			lifes = 0;
			}
		}
	return uStable;
}

//функция освобождения памяти (для любого типа данных)
template<typename T1, typename T2>
void fMemFree(T1 **&arrFree, const T2 ROWS)
{
	for (int i = 0; i < ROWS; i++)
	{
		delete[] arrFree[i];
	}
	delete[] arrFree;
	arrFree = nullptr;
}

//функция вывода на экран поля игры
//дополнительно функция возвращает 1, если нет живых клеток
bool fPrintField(const bool *const *const field, const int ROWS, const int COLUMNS, const unsigned long long GEN)
{
	bool uDead = 0;
	std::system(SCREEN_CLEAR);
	int lifes = 0;
	for (int i = 0; i < ROWS; ++i)
	{
		for (int j = 0; j < COLUMNS; ++j)
		{
			if (field[i][j])
			{
				std::cout << LIFE;
				++lifes;
			}
			else
			{
				std::cout << DEAD;
			}
		}
		std::cout << '\n';
	}
	std::cout << "Generation: " << GEN << ". Alive cells: " << lifes << '\n';
	uDead = (lifes == 0) ? 1 : 0;
	return uDead;
}

int main(int argc, char *argv[])
{
#if (_WIN32 || _WIN64)
	setlocale(LC_ALL, "RUS");
#endif
	bool **stateArrNow, **stateArrPrev; //текущее и предыдущее состояния Вселенной
	std::string start, tmp;
	const int SIZE = 7;
	std::string startArr[SIZE] = { "0", "1", "2", "3", "4", "5", "q" };
	//startArr - массив с перечнем доступных режимов начала игры (для переменной start)
	//0-ой элемент - случайное заполнение поля игры, размер поля запрашивается у пользователя
	//1-5 - суфикс к имени файла для инициализации игры из ini_суфикс.txt, можно любые другие суффиксы вписать в середину массива
	//q - последний элемент массива - выход из игры
	int rows = 0, columns = 0;
	bool startRandom, startErr, fileErr, resGame, continueGame = 0;
	bool uDead, uStable, uInfinity; //признаки завершения игры все мертвы/стабильное состояние/игра вероятно зациклена
	unsigned long long gen = 1, maxGen = 100;

	do	//регулярное начало игры с начала по желанию пользователя
	{
		startRandom = 0;	// запуск с рандомного распределения на поле
		startErr = 0;		// признак неверного выбора стартового режима игры
		fileErr = 0;		// признак ошибки открытия файла инициализации
		resGame = 0;		// пользователь начинает игру с начала после ошибки или окончания
		uDead = 0;			// игра завершена, т.к. все клетки мертвы
		uStable = 0;		// игра завершена, т.к. нет изменений относительно предыдущего состояния
		uInfinity = 0;		// игра псевдозавершена, вероятно вселенная зациклена
		start = "";			//переменная выбора способа старта игры
		tmp = "";			//переменная для ввода решения пользователя после завершения игры
		//Заставка
		fIntro();

		//Выбор исходного состояния Вселенной
		//повторяется пока пользователь не введет доступный вариант игры 
		//или не выберет выход из игры		
		do
		{
			if (startErr)
			{
				std::system(SCREEN_CLEAR);
				std::cout << "Incorrect input! Try again.\n";
			}
			else
			{
				std::cout << "Please, Enter symbol to initialize Universe:\n";
			}
			startErr = 1;
			std::cout << startArr[0] << " - random filling of Universe (dimensions choosing by user)\n";
			for (int i = 1; i < (SIZE - 1); ++i)
			{
				std::cout << startArr[i] << " - initialize from file ini_" << startArr[i] << ".txt\n";
			}
			std::cout << startArr[SIZE - 1] << " - exit the game\n\n";
			std::cout << "Enter here: ";
			std::getline(std::cin, start);
			//проверка допустимости введенного значения
			//фиксация random или файл
			for (int i = 0; i < SIZE; ++i)
			{
				if (start == startArr[i])
				{
					startErr = 0;
					if (i == 0) { startRandom = 1; }
				}
			}
		} while (startErr);

			if (start != startArr[SIZE - 1]) //проверка требования выхода из игры
			{
				//инициализация начального состояния Вселенной - Generation 1
				stateArrNow = fInit(start, startRandom, rows, columns, fileErr);

				if (!fileErr) //если нет ошибки открытия файла старт игры
				{
					stateArrPrev = fCreateArr(rows, columns);
					uDead = fPrintField(stateArrNow, rows, columns, gen);
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					do //в случае зацикленного состояния игры пользователь решает продолжать ее или нет через continueGame = 1
					{
						continueGame = 0;
						tmp = "";
						while (!uDead && !uStable && !uInfinity)
						{
							fCopy(stateArrPrev, stateArrNow, rows, columns);
							uStable = fGenNext(stateArrNow, stateArrPrev, rows, columns);
							++gen;
							uDead = fPrintField(stateArrNow, rows, columns, gen);
							std::this_thread::sleep_for(std::chrono::milliseconds(400));
							uInfinity = (gen >= maxGen) ? 1 : 0; //ограничение числа итераций
						}

						if (uDead)
						{
							std::cout	<< "All cells are dead. Game Over!\n\n"
										<< "Try again?\n"
										<< "Enter \"r\" to restart the Game\n"
										<< "Enter any other symbols or just press Enter to exit the Game\n\n"
										<< "Enter here: ";
							std::getline(std::cin, tmp);
							if (tmp == "r")		//начать игру с начала 
							{
								resGame = 1;
								gen = 1;
								std::system(SCREEN_CLEAR);
							}
							else
							{
								fExit();
							}
						}
						else if (uStable)
						{
							std::cout	<< "The world has stagnated. Game Over!\n\n"
										<< "Try again?\n"
										<< "Enter \"r\" to restart the Game\n"
										<< "Enter any other symbols or just press Enter to exit the Game\n\n"
										<< "Enter here: ";
							std::getline(std::cin, tmp);
							if (tmp == "r")		//начать игру с начала 
							{
								resGame = 1;
								gen = 1;
								std::system(SCREEN_CLEAR);
							}
							else
							{
								fExit();
							}
						}
						else if (uInfinity)
						{
							std::cout	<< "This Universe is looped maybe.\n\n"
										<< "Restart or continue?\n"
										<< "Enter \"r\" to restart the Game\n"
										<< "Enter \"g\" to continue this Game\n"
										<< "Enter any other symbols or just press Enter to exit the Game\n\n"
										<< "Enter here: ";
							std::getline(std::cin, tmp);
							if (tmp == "r")				//начать игру с начала 
							{
								resGame = 1;
								gen = 1;
								std::system(SCREEN_CLEAR);
							}
							else if (tmp == "g")		//продолжить наблюдать за Вселенной
							{
								continueGame = 1;
								uInfinity = 0;
								maxGen += maxGen;		//дополнительные maxGen итераций
							}
							else
							{
								fExit();
							}
						}
					} while (continueGame);
					fMemFree(stateArrNow, rows);
					fMemFree(stateArrPrev, rows);
				}
				else
				{
					//ошибка открытия файла, предложить начать игру с нуля или выйти
					std::cout	<< "Open file ini_" << start << " Error!\n\n"
								<< "Try again?\n"
								<< "Enter \"r\" to restart the Game\n"
								<< "Enter any other symbols or just press Enter to exit the Game\n\n"
								<< "Enter here: ";
					std::getline(std::cin, tmp);

					if (tmp == "r")		//начать игру с начала после неудачного открытия файла
					{
						resGame = 1;
						gen = 1;
						std::system(SCREEN_CLEAR);
					}
					else 				//закончить игру после неудачного открытия файла
					{
						fExit();
					}
				}
			}
			else //преждевременный выход из игры по желанию пользователя
			{
				fExit();
			}
	} while (resGame);

#if (_WIN32 || _WIN64 || __CYGWIN__)
	std::system("pause");
#endif
	return 0;
}

void fIntro()
{
	std::string lines = "";
	for (int i = 15; i >= 0; --i)
	{
		for (int j = 0; j < i; ++j) { lines += "\n"; }
		if (i > 0)
		{
			std::system(SCREEN_CLEAR);
			std::cout	<< lines 
						<< "                                 " << '\n'
						<< "  **       **  *******  *******  " << '\n'
						<< "  **       **  *******  *******  " << '\n'
						<< "  **       **  **       **       " << '\n'
						<< "  **       **  ******   *******  " << '\n'
						<< "  **       **  ******   *******  " << '\n'
						<< "  **       **  **       **       " << '\n'
						<< "  *******  **  **       *******  " << '\n'
						<< "  *******  **  **       *******  " << '\n'
						<< "                                 " << '\n';
		}
		else { std::cout << '\a'; }
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		lines = "";
	}
}

void fExit()
{
	std::system(SCREEN_CLEAR);
	std::cout	<< "                                         " << '\n'
				<< "      ***      ***      ***    *****     " << '\n'
				<< "    *******  *******  *******  *******   " << '\n'
				<< "    **   **  **   **  **   **  **   **   " << '\n'
				<< "    **       **   **  **   **  **   **   " << '\n'
				<< "    ** ****  **   **  **   **  **   **   " << '\n'
				<< "    **   **  **   **  **   **  **   **   " << '\n'
				<< "    *******  *******  *******  *******   " << '\n'
				<< "      ***      ***      ***    *****     " << '\n'
				<< "                                         " << '\n'
				<< "      *****    **   **  *******   **   " << '\n'
				<< "      *******  **   **  *******   **   " << '\n'
				<< "      **   **   ** **   **        **   " << '\n'
				<< "      ******    ** **   *******   **   " << '\n'
				<< "      ******     ***    *******   **   " << '\n'
				<< "      **   **    ***    **             " << '\n'
				<< "      *******    ***    *******   **   " << '\n'
				<< "      ******     ***    *******   **   " << '\n'
				<< "                                       " << '\n';
}