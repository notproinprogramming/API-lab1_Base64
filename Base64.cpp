#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>

const std::string BASE64_ALPHABET =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

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
    while (true) {
        std::cout << "\n=== MENU ===\n";
        std::cout << "1) Base64 encode file\n";
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

