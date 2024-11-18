
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <stdexcept>
#include <sstream>
#include <ctime>
#include <chrono>
#include <iomanip>

class Config {
public:
    int fieldWidth;
    int fieldHeight;
    double defaultX, defaultY, defaultSx, defaultSy, defaultH;
    std::string logFileName;
    bool loggingInterfaceEnabled;
    bool loggingControlEnabled;

    Config(const std::string& filename) {
        std::ifstream configFile(filename);
        if (!configFile.is_open()) {
            std::cerr << "Failed to open config file." << std::endl;
            return;
        }

     std::string key;
while (configFile >> key) { // Считываем ключи
    if (key == "fieldWidth") configFile >> fieldWidth;
    else if (key == "fieldHeight") configFile >> fieldHeight;
    else if (key == "defaultX") configFile >> defaultX;
    else if (key == "defaultY") configFile >> defaultY;
    else if (key == "defaultSx") configFile >> defaultSx;
    else if (key == "defaultSy") configFile >> defaultSy;
    else if (key == "defaultH") configFile >> defaultH;
    else if (key == "logFileName") configFile >> logFileName;
    else if (key == "loggingInterfaceEnabled") configFile >> std::boolalpha >> loggingInterfaceEnabled;
    else if (key == "loggingControlEnabled") configFile >> std::boolalpha >> loggingControlEnabled;
}

        configFile.close();
    }
};

class Logger {
private:
    std::ofstream logFile;

public:
    Logger(const std::string& fileName) {
        if (!logFile.is_open()) {
            logFile.open(fileName, std::ios::out | std::ios::app);
        }
        
    }
    
     // Конструктор перемещения
    Logger(Logger&& other) noexcept : logFile(std::move(other.logFile)) {
        // Теперь other.logFile указывает на "пустое" место.
    }
    
    // Оператор присваивания перемещения
    Logger& operator=(Logger&& other) noexcept {
        if (this != &other) {
            // Закрываем существующий файл, если он открыт
            if (logFile.is_open()) {
                logFile.close();
            }
            logFile = std::move(other.logFile);
            // Нет необходимости вызывать close() для other
        }
        return *this;
    }


    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void logMessage(const std::string& message, bool b) {
        if (logFile.is_open() && b) {
            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            std::stringstream timeStamp;
            timeStamp << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
            logFile << "[" << timeStamp.str() << "] " << message << std::endl;
        }
    }
    
};

class Gaus {
public:
    double h, x0, y0, sx, sy;

    Gaus(double h, double x0, double y0, double sx, double sy)
        : h(h), x0(x0), y0(y0), sx(sx), sy(sy) {}
};

class Pole {
public:
    std::vector<std::vector<double>> field;

    Pole(int A, int B) {
        field.resize(A, std::vector<double>(B, 0));
    }
};

class Component {
public:
    std::vector<std::vector<double>> componenta;
    Component(const std::vector<std::vector<double>>& inputComponenta) : componenta(inputComponenta) {}
    
    Component(int A, int B) {
        componenta.resize(A, std::vector<double>(B, 0));
    }
};

class Control {
private: 
    Pole* p; // Pointer to Pole 
    std::vector<Gaus> gaussi;
    Logger logger;
    Config config;
    bool b = true;
    int count = 0;
    
    int incrementAndCollect(std::vector<std::vector<double>>& componenta, int x, int y, int i) {
        if (x < 0 || y < 0 || x >= componenta[0].size() || y >= componenta.size() || p->field[y][x] != 1) return -1;

        if (p->field[y][x] == 1) {
            p->field[y][x] = i + 1; // Пометить как посещенное
            count = count < i + 1 ? i + 1 : count;
            componenta[y][x] = 1; // Увеличить значение в Componenta
            incrementAndCollect(componenta, x + 1, y, i + 1);
            incrementAndCollect(componenta, x - 1, y, i + 1);
            incrementAndCollect(componenta, x, y + 1, i + 1);
            incrementAndCollect(componenta, x, y - 1, i + 1);
        }
    return count;
}
    
public: 

std::vector<Component> componenti;

    Control(const std::string& configFileName) : p(nullptr), config(configFileName), logger("") {
    if (!config.logFileName.empty()) {
                logger = Logger(config.logFileName);
            }
            
    if (config.loggingControlEnabled) {
            
                logger.logMessage("Logging Control is enabled.", b);
                std::cout << "Logging Control is enabled." << std::endl;
        } else {
        logger.logMessage("Logging Control is disabled.", b);
        std::cout << "Logging Control is disabled." << std::endl;
        b = false;
      }
        
    }
    
    ~Control() { 
        delete p; // Free memory 
    }
    
    void addgauss(double h, double x0, double y0, double sigma_x, double sigma_y) { 
    logger.logMessage("Gauss addition will begin", b);
        gaussi.emplace_back(h, x0, y0, sigma_x, sigma_y);
        logger.logMessage("Added gauss", b);
    }
    
    void init(int A, int B) { 
        delete p; // Delete previous pointer 
        logger.logMessage("Field generation will begin", b);
        p = new Pole(A, B);
        logger.logMessage("Added field", b);
    }
    
    void generate() {
        double value; 
        for (const auto& g : gaussi) { 
            for (int x = 0; x < p->field[0].size(); ++x) { 
                for (int y = 0; y < p->field.size(); ++y) { 
                    value = g.h * exp(-((pow((x - g.x0) / g.sx, 2)) + (pow((y - g.y0) / g.sy, 2))) / 2); 
                    p->field[y][x] += value; 
                } 
            }
        }
    }
    
    void gnuplot() { 
        int rows = p->field.size(); 
        int cols = p->field[0].size(); 
        // Open a pipe to gnuplot 
        FILE* gnuplotPipe = popen("gnuplot -p", "w"); 
        if (!gnuplotPipe) { 
            std::cerr << "Could not open pipe to gnuplot." << std::endl; 
            logger.logMessage("Could not open pipe to gnuplot.", b);
            return; 
        }
        // Send gnuplot commands 
        fprintf(gnuplotPipe, "set contour base\n"); 
        fprintf(gnuplotPipe, "set view 60,30\n"); 
        fprintf(gnuplotPipe, "set xrange [0:%d]\n", cols - 1); 
        fprintf(gnuplotPipe, "set yrange [0:%d]\n", rows - 1); 
        fprintf(gnuplotPipe, "set terminal png\n"); 
        fprintf(gnuplotPipe, "set output 'landscape.png'\n"); 
        fprintf(gnuplotPipe, "splot '-' with lines\n"); 
        // Write data directly to gnuplot 
        for (int y = 0; y < rows; ++y) { 
            for (int x = 0; x < cols; ++x) { 
                fprintf(gnuplotPipe, "%d %d %f\n", x, y, p->field[y][x]); 
            } 
            fprintf(gnuplotPipe, "\n"); // Newline to separate rows 
        } 
        fprintf(gnuplotPipe, "EOF\n"); // Close the pipe 
        pclose(gnuplotPipe);
    }

     void bmp_write() {
    int width = p->field[0].size();
    int height = p->field.size();
    int padding = (4 - (width * 3) % 4) % 4; // Padding for alignment to 4 bytes
    std::ofstream bmpFile("output.bmp", std::ios::binary);
    if (!bmpFile) {
        std::cerr << "Failed to create BMP file." << std::endl;
        logger.logMessage("Failed to create BMP file.", b);
        return;
    }
    // Write BMP header
    unsigned char bmpHeader[54] = {
        'B', 'M', // Identifier
        0, 0, 0, 0, // Size of file (will be set later)
        0, 0, 0, 0, // Reserved
        54, 0, 0, 0, // Header size
        40, 0, 0, 0, // Info header size
        0, 0, 0, 0, // Width (will be set later)
        0, 0, 0, 0, // Height (will be set later)
        1, 0, // Number of color planes
        24, 0, // Bits per pixel
        0, 0, 0, 0, // Compression
        0, 0, 0, 0, // Image size (will be set later)
        0x13, 0x0B, 0, 0, // Horizontal resolution
        0x13, 0x0B, 0, 0, // Vertical resolution
        0, 0, 0, 0, // Number of colors in palette
        0, 0, 0, 0  // Important colors
    };
    // Set width and height in header
    bmpHeader[18] = (width & 0xFF);
    bmpHeader[19] = (width >> 8) & 0xFF;
    bmpHeader[20] = (width >> 16) & 0xFF;
    bmpHeader[21] = (width >> 24) & 0xFF;
    bmpHeader[22] = (height & 0xFF);
    bmpHeader[23] = (height >> 8) & 0xFF;
    bmpHeader[24] = (height >> 16) & 0xFF;
    bmpHeader[25] = (height >> 24) & 0xFF;
    // Write header
    bmpFile.write(reinterpret_cast<char*>(bmpHeader), 54);
    // Write pixel data
    for (int y = height - 1; y >= 0; --y) { // BMP stores pixels bottom-to-top
        for (int x = 0; x < width; ++x) {
            unsigned char color = 255 - static_cast<unsigned char>(p->field[y][x]); // Color
            bmpFile.put(color); // B
            bmpFile.put(color); // G
            bmpFile.put(color); // R
        }
        // Add padding
        for (int p = 0; p < padding; ++p) {
            bmpFile.put(0);
        }
    }
    bmpFile.close();
}
    
   void bmp_read(const std::string &filename) {
    std::ifstream bmpFile(filename, std::ios::binary);
    if (!bmpFile) {
        std::cerr << "Failed to open BMP file." << std::endl;
        logger.logMessage("Failed to open BMP file.", b);
        return;
    }
    // Читаем заголовок BMP
    unsigned char header[54];
    bmpFile.read(reinterpret_cast<char*>(header), 54);
    // Получаем ширину и высоту изображения
    int width = header[18] | (header[19] << 8) | (header[20] << 16) | (header[21] << 24);
    int height = header[22] | (header[23] << 8) | (header[24] << 16) | (header[25] << 24);
    // Инициализируем новое поле
    init(height, width); // Заметь, что BMP хранит данные от нижней строки к верхней.
    
    // Читаем данные пикселей
    for (int y = height - 1; y >= 0; --y) { // BMP хранит данные снизу вверх
        for (int x = 0; x < width; ++x) {
            unsigned char color = bmpFile.get(); // Читаем B
            bmpFile.get(); // Читаем G
            bmpFile.get(); // Читаем R
            double value = 255 - color; // Цвет в высоту
            p->field[y][x] = value; // Обновляем матрицу значений
        }
        bmpFile.ignore((4 - (width * 3) % 4) % 4); // Пропускаем паддинг
    }
    bmpFile.close();
}

    void bin(int slise) {
        for (const auto& g : gaussi) { 
            for (int x = 0; x < p->field[0].size(); ++x) { 
                for (int y = 0; y < p->field.size(); ++y) { 
                   p->field[y][x] = p->field[y][x] > slise ? 0 : 255;
                } 
            }
        }
        bmp_write();
        logger.logMessage("Created BMP file.", b);
        
        for (const auto& g : gaussi) { 
            for (int x = 0; x < p->field[0].size(); ++x) { 
                for (int y = 0; y < p->field.size(); ++y) { 
                   p->field[y][x] = p->field[y][x] > slise ? 0 : 1;
                } 
            }
        }
    }
    
    void wave() {
        int count;
        
        while (true) {
            Component Componenta(p->field.size(), p->field[0].size());
            bool found = false;

            for (int y = 0; y < p->field.size(); ++y) {
                for (int x = 0; x < p->field[y].size(); ++x) {
                    if (p->field[y][x] == 1) {
                        found = true; // Найдена единица
                        count = incrementAndCollect(Componenta.componenta, x, y, 1);//максимальная длина волны
                        x = p->field[y].size() + 1;
                        y = p->field.size() + 1;
                    }
                }
            }

            if (found) {
               if (count > 6){//Здесь 6 это порог для шума
                componenti.emplace_back(Componenta);
               }
            } else {
                break; // Если единиц больше нет, завершаем цикл
            }
        }
        logger.logMessage("Wave used, amount component = " + std::to_string(componenti.size()), b);
    }
};

class Interface {
private:
bool b = true;

public:
    Control c;
    Logger logger; 
    Config config;
    
    Interface(const std::string& configFileName) : config(configFileName), logger(""), c(configFileName) {
    if (!config.logFileName.empty()) {
                logger = Logger(config.logFileName);
            }
            
        if (config.loggingInterfaceEnabled) {
            
                logger.logMessage("Logging Interface is enabled.", b);
                std::cout << "Logging Interface is enabled." << std::endl;
        } else {
        logger.logMessage("Logging Interface is disabled.", b);
        std::cout << "Logging Interface is disabled." << std::endl;
        b = false;
      }
        
    }
    
    void print() {
        double x, y, sx, sy, h;
        int A, B;
        std::string s;
        bool a;
        std::string filename;
        std::ifstream file;
        std::cout << "Hello, dear user, this program builds Gaussians.\nEnter commands from a text file (PRESS 0) or from the keyboard (PRESS 1)?" << std::endl;
        std::cin >> a;
        logger.logMessage("User chose input method: " + std::to_string(a), b);
        if (a == 0) {
            std::cout << "You will enter commands from a text file.\nEnter filename:" << std::endl;
            std::cin >> filename;
            logger.logMessage("Reading commands from file: " + filename, b);
            file.open(filename);
            if (!file) {
                std::cout << "File not found" << std::endl;
                logger.logMessage("Error: File not found.", b);
                return;
            }
        } else {
            std::cout << "You will enter commands from the keyboard" << std::endl;
            logger.logMessage("User chose to input commands from the keyboard.", b);
        }
        if (a == 0) {
            int n = 0;
            while (file >> s) {
                logger.logMessage("Received command: " + s, b);
    if (s == "init") {
    // Проверяем, была ли уже вызвана команда init
    if (n != 0) {
        std::cout << "The init command has already been called.\nError\n";
        logger.logMessage("Error: Multiple init commands.", b);
        return;
    }
    n = 1; // Устанавливаем флаг, что команда init была вызвана 1 раз
    // Получаем размеры поля из конфигурации
    int A = config.fieldWidth;  // Размеры из конфигурации
    int B = config.fieldHeight;
    // Логируем и инициализируем поле
    logger.logMessage("Initializing field with size: " + std::to_string(A) + " x " + std::to_string(B), b);
    c.init(A, B);
    logger.logMessage("Field initialized.", b);
} else if (s == "g") {

                // Читаем параметры из файла
                file >> x >> y >> sx >> sy >> h;

                // Проверяем, были ли параметры указаны
                if (file.fail()) {
                    // Если не удалось прочитать, значит, используем значения по умолчанию
                    if (file.eof()) { // Желательно проверить конец файла, чтобы избежать повторного чтения
                        x = config.defaultX;
                        y = config.defaultY;
                        sx = config.defaultSx;
                        sy = config.defaultSy;
                        h = config.defaultH;
                    } else {
                        // Если произошла ошибка чтения или недостаточно параметров
                        // Нужно сбросить флаг ошибки для дальнейшего чтения
                        file.clear();

                        // Проверяем, какие параметры были прочитаны
                        if (!(file >> x)) x = config.defaultX; // Если не удалось получить x, берем по умолчанию
                        if (!(file >> y)) y = config.defaultY; // Если не удалось получить y, берем по умолчанию
                        if (!(file >> sx)) sx = config.defaultSx; // ...
                        if (!(file >> sy)) sy = config.defaultSy; // ...
                        if (!(file >> h)) h = config.defaultH; // ...
                    }
                }

                logger.logMessage("Adding Gaussian: x=" + std::to_string(x) +
                    ", y=" + std::to_string(y) + 
                    ", sx=" + std::to_string(sx) + 
                    ", sy=" + std::to_string(sy) + 
                    ", h=" + std::to_string(h), b);
                c.addgauss(h, x, y, sx, sy);
            } else if (s == "generate") {
                    c.generate();
                    logger.logMessage("Generated values in the field.", b);
                } else if (s == "gnuplot") {
                    c.gnuplot();
                    logger.logMessage("Called gnuplot.", b);
                } else if (s == "bmp_write") {
                    c.bmp_write();
                    logger.logMessage("Created BMP file.", b);
                } else if (s == "bmp_read") {
                    file >> filename; // Чтение имени файла для bmp_read
                    c.bmp_read(filename);
                    logger.logMessage("Read BMP file: " + filename, b);
                } else if (s == "bin") {
                    int slice;
                    file >> slice;
                    c.bin(slice);
                    //добавить wave
                    logger.logMessage("Slice applied: slice=" + std::to_string(slice), b);
                }
            }
        } else {
            int n = 0;
            while (true) {
                std::cout << "Enter command (init, g, generate, gnuplot, bmp_write, bmp_read, bin, end):";
                std::cin >> s;
                std::cout << "\n";
                logger.logMessage("Received command: " + s, b);
    if (s == "init") {
    // Проверяем, была ли уже вызвана команда init
    if (n != 0) {
        std::cout << "The init command has already started.\nError\n";
        logger.logMessage("Error: Multiple init commands.", b);
        return;
    }
    n = 1; // Устанавливаем флаг, что команда init была вызвана
    // Получаем размеры поля из конфигурационного файла
    int A = config.fieldWidth;  // Размер по ширине из конфигурации
    int B = config.fieldHeight; // Размер по высоте из конфигурации
    // Логируем и инициализируем поле
    logger.logMessage("Initializing field with size: " + std::to_string(A) + " x " + std::to_string(B), b);
    c.init(A, B);
    logger.logMessage("Field initialized.", b);
}
  if (s == "g") {
    std::string input;
    std::getline(std::cin, input);

    std::istringstream inputStream(input);

    // Инициализируем переменные значениями по умолчанию
    x = config.defaultX;
    y = config.defaultY;
    sx = config.defaultSx;
    sy = config.defaultSy;
    h = config.defaultH;

    // Читаем введенные данные
    if (!(inputStream >> x)) {
        std::cout << "The default value for x is used: " << config.defaultX << std::endl;
    }
    if (!(inputStream >> y)) {
        std::cout << "The default value for y is used: " << config.defaultY << std::endl;
    }
    if (!(inputStream >> sx)) {
        std::cout << "The default value for sx is used: " << config.defaultSx << std::endl;
    }
    if (!(inputStream >> sy)) {
        std::cout << "The default value for sy is used: " << config.defaultSy << std::endl;
    }
    if (!(inputStream >> h)) {
        std::cout << "The default value for h is used: " << config.defaultH << std::endl;
    }

                logger.logMessage("Adding Gaussian: x=" + std::to_string(x) +
                    ", y=" + std::to_string(y) + 
                    ", sx=" + std::to_string(sx) + 
                    ", sy=" + std::to_string(sy) + 
                    ", h=" + std::to_string(h), b);
                c.addgauss(h, x, y, sx, sy);
            }
                  if (s == "generate") {
                    c.generate();
                    std::cout << "Generated values in the field." << std::endl;
                    logger.logMessage("Generated values in the field.", b);
                } if (s == "gnuplot") {
                    c.gnuplot();
                    std::cout << "Called gnuplot." << std::endl;
                    logger.logMessage("Called gnuplot.", b);
                } if (s == "bmp_write") {
                    c.bmp_write();
                    std::cout << "Created BMP file." << std::endl;
                    logger.logMessage("Created BMP file.", b);
                } if (s == "bmp_read") {
                    std::cout << "Enter the filename to read:" << std::endl;
                    std::cin >> filename;
                    c.bmp_read(filename);
                    std::cout << "Read BMP file: " + filename << std::endl;
                    logger.logMessage("Read BMP file: " + filename, b);
                }  if (s == "end") {
                    std::cout << "Ending the program" << std::endl;
                    logger.logMessage("Ending the program.", b);
                    break;
                }
                   if (s == "bin") {
                     int slice;
                     std::cout << "Enter slice level:" << std::endl;
                     std::cin >> slice;
                     c.bin(slice);
                     logger.logMessage("Slice applied: slice=" + std::to_string(slice), b);
                     std::cout << "Slice applied: slice=" << slice << std::endl;
                     logger.logMessage("Wave will be used", b);
                     c.wave();
                     std::cout << "Amount component = " << c.componenti.size() << std::endl;
                }
            }

        if (file.is_open()) {
            file.close();
            logger.logMessage("Closed input file.", b);
        }
     }
   }  
};

int main() {
    // Задаем имя файла конфигурации
    std::string configFileName = "config.txt";

    // Создаем интерфейс, передавая имя файла конфигурации
    Interface i(configFileName);

    // Вызываем метод print() интерфейса
    i.print();

    return 0;
}
