#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "Color.hpp"
#include <regex>
#include <boost/regex.hpp>
#include <bitset>
using namespace std;

const Color::Modifier red(Color::FG_RED);
const Color::Modifier def(Color::FG_DEFAULT);
const Color::Modifier green(Color::FG_GREEN);
const Color::Modifier blue(Color::FG_BLUE);
const Color::Modifier cyan(Color::FG_CYAN);

map<string, vector<string>> instructions;

class token
{
public:
    string type;
    vector<string> variables;
    token()
    {
        this->type = "test";
        vector<string> test;
        this->variables = test;
    }
};

string intToBinary(int number, int length)
{
    if (length <= 0)
    {
        return "";
    }

    bitset<32> binaryRepresentation(number);
    string binaryString = binaryRepresentation.to_string().substr(32 - length);

    return binaryString;
}

bool startsAtPosition(const string &mainStr, const string &subStr, size_t position)
{
    return mainStr.substr(position, subStr.length()) == subStr;
}

token tokenize(string instruction)
{
    stringstream instruction_type_stream(instruction);
    string temp;
    string instruction_variables;
    token instruction_token;
    if (getline(instruction_type_stream, instruction_token.type, ' '))
    {
        getline(instruction_type_stream, instruction_variables);
    }
    instruction_variables.erase(remove_if(instruction_variables.begin(), instruction_variables.end(), ::isspace), instruction_variables.end());
    for (int i = 0; i < instruction_variables.length(); i++)
    {
        string parameters = "";
        while (i < instruction_variables.length() && instruction_variables[i] != ',')
        {
            parameters += instruction_variables[i];
            i++;
        }
        instruction_token.variables.push_back(parameters);
    }
    return instruction_token;
}

string instruction_to_binary(token instruction)
{
    string encoded = "";
    boost::regex pattern(R"((.*?)\{([^}]+)\}(.*?))");
    bool rd_exists = false;
    for (int i = stoi(instructions[instruction.type][1]) + 1; i > 1; i--)
    {
        boost::sregex_iterator MatchitFinder(instructions[instruction.type][i].begin(), instructions[instruction.type][i].end(), pattern);
        boost::smatch match = *MatchitFinder;
        string beforeBraces = match[1].str();
        string value = match[2].str();
        pair<string, string> it = {beforeBraces, value};
        if (it.first == "op")
        {
            encoded = it.second + encoded;
        }
        else if (it.first == "rd")
        {
            rd_exists = true;
            boost::regex pattern(R"(\d+)");
            boost::smatch match;
            if (regex_search(instruction.variables[0], match, pattern))
            {
                string extractedNumber = match.str();
                int number = stoi(extractedNumber);
                encoded = intToBinary(number, stoi(it.second)) + encoded;
            }
            else
            {
                std::cout << red << " INVALID ARGUMENTS IN " << def << "{" << blue << instruction.type << def << "}" << red << " INSTRUCTION TYPE" << def << endl;
            }
        }
        else if (it.first == "funct3")
        {
            encoded = it.second + encoded;
        }
        else if (it.first == "funct7")
        {
            encoded = it.second + encoded;
        }
        else if (it.first == "rs1")
        {
            int offset = 0;
            if (!rd_exists)
            {
                offset++;
            }
            boost::regex pattern(R"(\d+)");
            boost::smatch match;
            if (regex_search(instruction.variables[1 - offset], match, pattern))
            {
                string extractedNumber = match.str();
                int number = stoi(extractedNumber);
                encoded = intToBinary(number, stoi(it.second)) + encoded;
            }
            else
            {
                std::cout << red << " INVALID ARGUMENTS IN " << def << "{" << blue << instruction.type << def << "}" << red << " INSTRUCTION TYPE" << def << endl;
            }
        }
        else if (it.first == "rs2")
        {
            int offset = 0;
            if (!rd_exists)
            {
                offset++;
            }
            boost::regex pattern(R"(\d+)");
            boost::smatch match;
            if (regex_search(instruction.variables[2 - offset], match, pattern))
            {
                string extractedNumber = match.str();
                int number = stoi(extractedNumber);
                encoded = intToBinary(number, stoi(it.second)) + encoded;
            }
            else
            {
                std::cout << red << " INVALID ARGUMENTS IN " << def << "{" << blue << instruction.type << def << "}" << red << " INSTRUCTION TYPE" << def << endl;
            }
        }
        else if (startsAtPosition(it.first, "i", 0))
        {
            boost::regex pattern(R"(\d+)");
            boost::smatch match;
            if (regex_search(it.first, match, pattern))
            {
                string extractedNumber = match.str();
                int number = stoi(extractedNumber);
                string binary_form_of_immediate = intToBinary(stoi(instruction.variables[instruction.variables.size() - 1]), 20);
                std::vector<std::pair<int, int>> offsets;
                std::istringstream ss(it.second);
                std::string token;
                while (std::getline(ss, token, ','))
                {
                    std::istringstream token_ss(token);
                    std::string part1, part2;

                    if (std::getline(token_ss, part1, '-') && std::getline(token_ss, part2))
                    {
                        int first = std::stoi(part1);
                        int second = std::stoi(part2);
                        offsets.emplace_back(first, second);
                    }
                }
                string immediate_value = "";
                std::reverse(binary_form_of_immediate.begin(), binary_form_of_immediate.end());
                for (auto offset_it : offsets)
                {
                    for (int i = offset_it.first; i >= offset_it.second; i--)
                    {
                        immediate_value = immediate_value + binary_form_of_immediate[i];
                    }
                }
                encoded = immediate_value + encoded;
            }
        }
    }
    return encoded;
}

int main()
{
    /* Initialising the instructions from instructions.txt */
    fstream instruction_file;
    instruction_file.open("./instructions.txt", ios::in);
    if (instruction_file.is_open())
    {
        string instruction;
        while (getline(instruction_file, instruction))
        {
            stringstream instruction_stream(instruction);
            string temp;
            bool start = true;
            string instruction = "test_instruction";
            vector<string> instruction_specification;
            while (getline(instruction_stream, temp, ' '))
            {
                if (start)
                {
                    instruction = temp;
                    start = false;
                }
                else
                {
                    instruction_specification.push_back(temp);
                }
            }
            instructions[instruction] = instruction_specification;
        }
    }

    /* Reading the assembly code from test.s */
    fstream code_file;
    code_file.open("./test.s", ios::in);
    vector<string> RISCV_CODE;
    vector<token> TOKENS;
    vector<string> binary_encodings;
    if (code_file.is_open())
    {
        string code;
        while (getline(code_file, code))
        {
            RISCV_CODE.push_back(code);
        }
        code_file.close();
    }

    /* Creating Tokens for each instruction */
    for (auto it : RISCV_CODE)
    {
        TOKENS.push_back(tokenize(it));
    }

    /* Generating Binary encoding for the tokens */
    vector<string> BINARY;
    bool success = true;
    for (auto it : TOKENS)
    {
        if (instructions[it.type].size() == 0)
        {
            std::cout << red << " ERROR : INVALID INSTRUCTION " << def << endl;
            success = false;
        }
        if (stoi(instructions[it.type][0]) != it.variables.size())
        {
            std::cout << red << " ERROR : INVALID NUMBER OF ARGUMENTS WITH " << def << "{" << blue << it.type << def << "}" << red << " INSTRUCTION " << def << endl;
        }
        BINARY.push_back(instruction_to_binary(it));
    }
    std::cout << green << "SUCCESS " << def << endl;
    std::cout << cyan << "The Machine encoding of your program is as follows : " << def << endl;
    string result;
    for (auto it : BINARY)
    {
        result = result + it;
        std::cout << blue << it << def << endl;
    }
    std::cout << endl;
    /* Writing the binary data into a file */
    std::ofstream outputFile("machine_encoding.txt");
    outputFile << result;
    outputFile.close();
    std::cout << cyan << "It is also saved to machine_encoding.txt for your convenience." << endl;
}