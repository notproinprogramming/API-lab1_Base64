#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>

const std::string BASE64_ALPHABET =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

int DECODE_TABLE[256];

void initDecodeTable() {
    for (int i = 0; i < 256; ++i)
        DECODE_TABLE[i] = -1;

    for (int i = 0; i < 64; ++i)
        DECODE_TABLE[(unsigned char)BASE64_ALPHABET[i]] = i;
}

std::string encodeBase64(const std::vector<unsigned char>& data) {
    std::string result;
    int val = 0, valb = -6;

    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(BASE64_ALPHABET[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        result.push_back(BASE64_ALPHABET[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (result.size() % 4) {
        result.push_back('=');
    }
    return result;
}

bool decodeBase64Line(const std::string& line,
                      std::vector<unsigned char>& output,
                      int lineNumber,
                      bool& paddingEncountered) {

    int val = 0, valb = -8;
    bool localPadding = false;

    for (size_t i = 0; i < line.size(); ++i) {
        unsigned char c = line[i];

        if (c == '=') {
            localPadding = true;
            paddingEncountered = true;
            continue;
        }

        if (localPadding) {
            std::cerr << "Рядок " << lineNumber
                      << ", символ " << i + 1
                      << ": Неправильне використання паддінгу\n";
            return false;
        }

        if (DECODE_TABLE[c] == -1) {
            std::cerr << "Рядок " << lineNumber
                      << ", символ " << i + 1
                      << ": Некоректний вхідний символ ('" << c << "')\n";
            return false;
        }

        val = (val << 6) + DECODE_TABLE[c];
        valb += 6;

        if (valb >= 0) {
            output.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    return true;
}

int FileDecodeFunc(const std::string& inputFile, const std::string& outputFileUser) {

    std::ifstream in(inputFile);
    if (!in) {
        std::cerr << "Cannot open input file\n";
        return 1;
    }

    std::string outputFile = outputFileUser;
    if (outputFile.empty()) {
        if (inputFile.size() > 7 &&
            inputFile.substr(inputFile.size() - 7) == ".base64") {
            outputFile = inputFile.substr(0, inputFile.size() - 7);
            std::cout << "Suggested output file: " << outputFile << "\n";
        } else {
            outputFile = inputFile + ".decoded";
            std::cout << "Suggested output file: " << outputFile << "\n";
        }

        std::cout << "Press ENTER to accept or enter new name: ";
        std::string temp;
        std::getline(std::cin, temp);
        if (!temp.empty())
            outputFile = temp;
    }

    std::vector<unsigned char> decodedData;
    std::string line;
    int lineNumber = 0;
    bool paddingEncountered = false;
    bool finished = false;

    while (std::getline(in, line)) {
        lineNumber++;

        if (line.empty())
            continue;

        if (line[0] == '-')
            continue;

        if (finished) {
            std::cerr << "Наявні дані після кінця повідомлення\n";
            break;
        }


        if (line.length() > 76) {
            std::cerr << "Рядок " << lineNumber
                << ": Рядок занадто довгий ("
                << line.length() << ")\n";
            return 1;
        }

        if (line.length() % 4 != 0) {
            std::cerr << "Рядок " << lineNumber
                << ": Довжина Base64 рядка не кратна 4 ("
                << line.length() << ")\n";
            return 1;
        }

        if (line.find('-') != std::string::npos && line[0] != '-') {
            std::cerr << "Рядок " << lineNumber
                << ": Некоректний вхідний символ ('-')\n";
            return 1;
        }

        if (!decodeBase64Line(line, decodedData, lineNumber, paddingEncountered))
            return 1;

        if (paddingEncountered)
            finished = true;
    }

    in.close();

    std::ofstream out(outputFile, std::ios::binary);
    if (!out) {
        std::cerr << "Cannot open output file\n";
        return 1;
    }

    out.write(reinterpret_cast<const char*>(decodedData.data()),
              decodedData.size());

    out.close();

    std::cout << "Decoding finished. Output: " << outputFile << "\n";
    return 0;
}

int FileChooseFunc(const std::string& inputFile, const std::string& outputFileUser) {
    std::string outputFile = outputFileUser.empty() ? inputFile + ".base64" : outputFileUser;

    std::ifstream in(inputFile, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open input file: " << inputFile << "\n";
        return 1;
    }

    std::vector<unsigned char> buffer(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>()
    );
    in.close();

    std::string encoded = encodeBase64(buffer);

    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Cannot open output file: " << outputFile << "\n";
        return 1;
    }

    out << "- Base64 encoded file\n";

    for (size_t i = 0; i < encoded.size(); i += 76) {
        out << encoded.substr(i, 76) << "\n";
    }

    out.close();

    std::cout << "Encoding finished. Output: " << outputFile << "\n";
    return 0;
}

int main() {

    initDecodeTable();
    while (true) {
        std::cout << "\n=== MENU ===\n";
        std::cout << "1) Base64 encode file\n";
        std::cout << "2) Base64 decode file\n";
        std::cout << "9) Open .base64 file in nvim\n";
        std::cout << "0) Exit\n";

        std::cout << "Choose option: ";

        int choose;
        std::cin >> choose;

        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input.\n";
            continue;
        }

        if (choose == 0) {
            std::cout << "Program terminated.\n";
            break;
        }
        else if (choose == 1) {
            std::string inputFile, outputFile;
            std::cout << "Enter input file name: ";
            std::cin >> inputFile;

            std::cout << "Enter output file name (or leave empty for default): ";
            std::cin.ignore();
            std::getline(std::cin, outputFile);

            FileChooseFunc(inputFile, outputFile);
        }
        else if (choose == 2) {
            std::string inputFile, outputFile;
            std::cout << "Enter encoded file name: ";
            std::cin >> inputFile;
            std::cin.ignore();
            std::cout << "Enter output file name (or leave empty): ";
            std::getline(std::cin, outputFile);
            FileDecodeFunc(inputFile, outputFile);
        }
        else if (choose == 9) {
            std::string fileToOpen;
            std::cout << "Enter .base64 file name to open in nvim: ";
            std::cin >> fileToOpen;

            std::string command = "nvim " + fileToOpen;
            std::cout << "Executing: " << command << "\n";
            system(command.c_str());
        }
        else {
            std::cout << "Unknown option.\n";
        }
    }

    return 0;
}

