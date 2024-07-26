#include <iostream>
#include<chrono>
#include<thread>
#include<fstream>
#include<vector>
#include <algorithm>
#include<string>
using namespace std;

const int Gx[3][3] = { -1,0,1,
                      -2,0,2,
                      -1,0,1 };

const int Gy[3][3] = { -1,-2,-1,
                      0,0,0,
                      1,2,1 };

const int Gauss[3][3] = { 1,2,1,
                      2,4,2,
                      1,2,1 };


struct Timer {
    chrono::time_point<chrono::steady_clock> start, end;
    chrono::duration<float> duration;

    Timer() {
        start = std::chrono::high_resolution_clock::now();
    }
    ~Timer() {
        end = chrono::high_resolution_clock::now();
        duration = end - start;

        //float ms = duration.count() * 1000.0f;
        cout << "Timer took " << duration.count() << "s" << endl;
    }
};
class Filter {
private:
    ifstream image;
    ofstream newImage;
    string width;
    string height;
    vector<vector<uint8_t>> matrix;

public:
    Filter(string s1, string s2) {
        //file stuff first

        image.open(s1);
        newImage.open(s2);
        string type = "", RGB = "";
        image >> type;
        image >> width;
        image >> height;
        image >> RGB;

        newImage << type << endl;
        newImage << width << " " << height << endl;
        newImage << RGB << endl;

        size_t num_rows = std::stoi(height);
        size_t num_cols = std::stoi(width);


        matrix.resize(num_rows, std::vector<uint8_t>(num_cols, 0));

    }
    ~Filter() {
        image.close();
        newImage.close();
    }
    void GreenFilter() {
        Timer timer;
        int r = 0, g = 0, b = 0;
        while (image >> r >> g >> b) {
            if (g + 50 > 255) {
                g = 255;
            }
            else {
                g += 50;
            }
            newImage << r << " " << g << " " << b << std::endl;
        }
    }
    void GrayFilterDefault() {

        int r = 0, g = 0, b = 0;
        uint8_t gray = 0;
        size_t ROWS = matrix.size();
        size_t COLS = matrix[0].size();
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                if (image >> r >> g >> b) {
                    gray = (0.299 * r + 0.587 * g + 0.114 * b);
                    matrix[i][j] = (gray);
                    //newImage << static_cast<int>(gray) << " " << static_cast<int>(gray) << " " << static_cast<int>(gray) << endl;
                }
            }
        }
        //matrix[100][100] += 2;
        //cout << static_cast<int> (matrix[100][100]);
    }
    int convolveGaus(int i, int j) {
        int sum = 0;
        for (int a = -1; a < 2; a++) {
            for (int b = -1; b < 2; b++) {
                sum += matrix[i + a][j + b] * Gauss[1 + a][1 + b];
            }
        }

        return sum / 16;
    }
    int convolveSibel(int i, int j) {
        int sum1 = 0;
        int sum2 = 0;
        for (int a = -1; a < 2; a++) {
            for (int b = -1; b < 2; b++) {
                sum1 += matrix[i + a][j + b] * Gx[1 + a][1 + b];
                sum2 += matrix[i + a][j + b] * Gy[1 + a][1 + b];
            }
        }
        sum1 = sum1 ^ 2;
        sum2 = sum2 ^ 2;
        sum1 = sum1 + sum2;
        sum1 = sqrt(sum1);
        return sum1;
    }


    void GaussFilter() {

        GrayFilterDefault();
        vector<vector<uint8_t>> tmp = matrix;
        size_t ROWS = matrix.size();
        size_t COLS = matrix[0].size();


        for (int i = 1; i < ROWS - 1; i++) {
            for (int j = 1; j < COLS - 1; j++) {
                tmp[i][j] = convolveGaus(i, j);
            }
        }
        //for (int i = 0; i < ROWS; i ++) {
            //for (int j = 0; j < COLS; j ++) {
                //newImage << static_cast<int>(tmp[i][j]) << " " << static_cast<int>(tmp[i][j]) << " " << static_cast<int>(tmp[i][j]) << endl;
            //}
        //}
        matrix = tmp;
    }
    void SobelFilter() {
        Timer timer;

        GaussFilter();
        vector<vector<uint8_t>> tmp = matrix;
        size_t ROWS = matrix.size();
        size_t COLS = matrix[0].size();

        int maxValue = 0;
        int minValue = 0;

        for (int i = 1; i < ROWS - 1; i++) {
            for (int j = 1; j < COLS - 1; j++) {
                tmp[i][j] = convolveSibel(i, j);
                if (tmp[i][j] > maxValue)maxValue = tmp[i][j];
                if (tmp[i][j] < minValue)minValue = tmp[i][j];
            }
        }
        cout << endl << maxValue << " " << minValue;

        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                if (i == 0 || j == 0 || i == ROWS - 1 || j == COLS - 1) {
                    tmp[i][j] = 0;
                }
                tmp[i][j] *= 5;
                newImage << static_cast<int>(tmp[i][j]) << " " << static_cast<int>(tmp[i][j]) << " " << static_cast<int>(tmp[i][j]) << endl;
            }
        }
    }

};

class FilterThreads {
private:
    ifstream image;
    ofstream newImage;
    string width;
    string height;
    vector<vector<uint8_t>> matrix;

public:
    FilterThreads(string s1, string s2) {
        //file stuff first

        image.open(s1);
        newImage.open(s2);
        string type = "", RGB = "";
        image >> type;
        image >> width;
        image >> height;
        image >> RGB;

        newImage << type << endl;
        newImage << width << " " << height << endl;
        newImage << RGB << endl;

        size_t num_rows = std::stoi(height);
        size_t num_cols = std::stoi(width);


        matrix.resize(num_rows, std::vector<uint8_t>(num_cols, 0));

    }
    ~FilterThreads() {
        image.close();
        newImage.close();
    }
    void GrayFilterDefault() {

        int r = 0, g = 0, b = 0;
        uint8_t gray = 0;
        size_t ROWS = matrix.size();
        size_t COLS = matrix[0].size();
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                if (image >> r >> g >> b) {
                    gray = (0.299 * r + 0.587 * g + 0.114 * b);
                    matrix[i][j] = (gray);
                    //newImage << static_cast<int>(gray) << " " << static_cast<int>(gray) << " " << static_cast<int>(gray) << endl;
                }
            }
        }
        //matrix[100][100] += 2;
        //cout << static_cast<int> (matrix[100][100])
    }

    int convolveGaus(int i, int j, vector<vector<uint8_t>>& temp) {
        int sum = 0;
        for (int a = -1; a < 2; a++) {
            for (int b = -1; b < 2; b++) {
                sum += temp[i + a][j + b] * Gauss[1 + a][1 + b];
            }
        }
        return sum / 16;
    }

    void convolveGausrows(vector<vector<uint8_t>>& temp, int threadID, int numThreads, size_t ROWS, size_t COLS) {
        for (int i = threadID; i < ROWS; i = i + numThreads) {

            for (int j = 0; j < COLS; j++) {
                if (i > 0 && i < ROWS - 1 && j > 0 && j < COLS - 1) {
                    matrix[i][j] = convolveGaus(i, j, temp);
                }
            }
        }
    }



    int convolveSobel(int i, int j, vector<vector<uint8_t>>& temp) {
        int sum1 = 0;
        int sum2 = 0;
        for (int a = -1; a < 2; a++) {
            for (int b = -1; b < 2; b++) {
                sum1 += temp[i + a][j + b] * Gx[1 + a][1 + b];
                sum2 += temp[i + a][j + b] * Gy[1 + a][1 + b];
            }
        }
        sum1 = sum1 ^ 2;
        sum2 = sum2 ^ 2;
        sum1 = sum1 + sum2;
        sum1 = sqrt(sum1);
        return sum1;
    }
    void convolveSobelrows(vector<vector<uint8_t>>& temp, int threadID, int numThreads, size_t ROWS, size_t COLS) {
        cout << threadID << endl;
        for (int i = threadID; i < ROWS; i = i + numThreads) {
            for (int j = 0; j < COLS; j++) {
                if (i > 0 && i < ROWS - 1 && j > 0 && j < COLS - 1) {
                    matrix[i][j] = convolveSobel(i, j, temp);

                }
            }
        }
    }

    void GaussFilter() {
        GrayFilterDefault();
        size_t ROWS = matrix.size();
        size_t COLS = matrix[0].size();

        vector<std::thread> threads;

        vector < vector<uint8_t>> temp = matrix;

        int hardwareThreads = std::thread::hardware_concurrency();
        int numThreads = (hardwareThreads != 0) ? hardwareThreads : 2;

        for (int i = 0; i < numThreads; i++) {
            threads.push_back(std::thread(&FilterThreads::convolveGausrows, this, ref(temp), i, numThreads, ROWS, COLS));
        }
        for (std::thread& t : threads) {
            t.join();
        }
        //for (int i = 0; i < ROWS; i++) {
            //for (int j = 0; j < COLS; j++) {
                //newImage << static_cast<int>(matrix[i][j]) << " " << static_cast<int>(matrix[i][j]) << " " << static_cast<int>(matrix[i][j]) << endl;
            //}
        //}
    }
    void SobelFilter() {
        Timer timer;
        GaussFilter();

        size_t ROWS = matrix.size();
        size_t COLS = matrix[0].size();
        vector<vector<uint8_t>> temp = matrix;

        vector<std::thread> threads;

        int hardwareThreads = std::thread::hardware_concurrency();
        int numThreads = (hardwareThreads != 0) ? hardwareThreads : 2;

        for (int i = 0; i < numThreads; i++) {
            threads.push_back(std::thread(&FilterThreads::convolveSobelrows, this, ref(temp), i, numThreads, ROWS, COLS));

        }
        for (std::thread& t : threads) {
            t.join();
        }



        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                if (i == 0 || j == 0 || i == ROWS - 1 || j == COLS - 1) {
                    matrix[i][j] = 0;
                }
                matrix[i][j] *= 5;
                newImage << static_cast<int>(matrix[i][j]) << " " << static_cast<int>(matrix[i][j]) << " " << static_cast<int>(matrix[i][j]) << endl;
            }
        }
    }

};



int main() {
    //gray scale intoo
    //gaussian into
    //sobel (x and y)


    Filter f1("lakeppm.ppm", "lakesobel.ppm");
    f1.SobelFilter();

    FilterThreads f2("lakeppm.ppm", "lakesobelthr.ppm");
    f2.SobelFilter();


    return 0;
}