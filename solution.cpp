#include <iostream>
#include <string>

using namespace std;

long long convertToNumber(string str) {
    long long result = 0;
    for (char c : str) {
        result = result * 10 + (c - '0');
    }
    return result;
}

void sortArray(long long data[], int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (data[j] > data[j + 1]) {
                long long swapTemp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = swapTemp;
            }
        }
    }
}

int main() {
    long long count;
    cin >> count;
    cin.ignore();
    
    long long numbers[10001];
    
    for (int i = 0; i < count; i++) {
        string inputLine;
        getline(cin, inputLine);
        
        string lastNumberStr = "";
        for (int pos = inputLine.length() - 1; pos >= 0 && inputLine[pos] != ' '; pos--) {
            lastNumberStr = inputLine[pos] + lastNumberStr;
        }
        
        numbers[i] = convertToNumber(lastNumberStr);
    }
    
    sortArray(numbers, count);
    
    int middleIndex = (count - 1) / 2;
    cout << numbers[middleIndex] << endl;
    
    return 0;
}
