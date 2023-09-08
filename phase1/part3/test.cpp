#include <iostream>
#include <string>
#include <boost/regex.hpp>

int main() {
    std::string input = "The date is 2023-09-07 and it's a sunny day.";

    // Define a regular expression pattern
    boost::regex pattern("\\d{4}-\\d{2}-\\d{2}");

    // Search for matches in the input string
    boost::sregex_iterator it(input.begin(), input.end(), pattern);
    boost::sregex_iterator end;

    // Iterate over the matches and print them
    while (it != end) {
        std::cout << "Match: " << it->str() << std::endl;
        ++it;
    }

    return 0;
}












////// ?////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /*
        instruction_tokens *new_token = new instruction_tokens(instructions[instruction.type]);
        for (auto it : new_token->InstructionTokens)
        {
            if (it.first == "op")
            {
                encoded = it.second + encoded;
            }
            else if (it.first == "rd")
            {
                regex pattern(R"(\d+)");
                smatch match;
                if (regex_search(instruction.variables[0], match, pattern))
                {
                    string extractedNumber = match.str();
                    int number = stoi(extractedNumber);
                    encoded = intToBinary(number, stoi(it.second));
                }
                else
                {
                    cout << red << " INVALID ARGUMENTS IN " << def << "{" << blue << instruction.type << def << "}" << red << " INSTRUCTION TYPE" << def << endl;
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
                regex pattern(R"(\d+)");
                smatch match;
                if (regex_search(instruction.variables[1], match, pattern))
                {
                    string extractedNumber = match.str();
                    int number = stoi(extractedNumber);
                    encoded = intToBinary(number, stoi(it.second));
                }
                else
                {
                    cout << red << " INVALID ARGUMENTS IN " << def << "{" << blue << instruction.type << def << "}" << red << " INSTRUCTION TYPE" << def << endl;
                }
            }
            else if (it.first == "rs2")
            {
                regex pattern(R"(\d+)");
                smatch match;
                if (regex_search(instruction.variables[2], match, pattern))
                {
                    string extractedNumber = match.str();
                    int number = stoi(extractedNumber);
                    encoded = intToBinary(number, stoi(it.second));
                }
                else
                {
                    cout << red << " INVALID ARGUMENTS IN " << def << "{" << blue << instruction.type << def << "}" << red << " INSTRUCTION TYPE" << def << endl;
                }
            }
            else if (startsAtPosition(it.first, "i", 0))
            {
                regex pattern(R"(\d+)");
                smatch match;
                if (regex_search(it.first, match, pattern))
                {
                    string extractedNumber = match.str();
                    int number = stoi(extractedNumber);
                    string binary_form_of_immediate = intToBinary(stoi(instruction.variables[instruction.variables.size() - 1]), 20);
                }
            }
        }
        */





















/*
       class instruction_tokens
{
public:
    vector<pair<string, string>> InstructionTokens;
    instruction_tokens(vector<string> instruction_variables)
    {
        regex pattern(R"((.*?)\{([^}]+)\}(.*?))");
        for (int i = stoi(instruction_variables[1]) + 1; i > 1; i--)
        {
            std::sregex_iterator it(instruction_variables[i].begin(), instruction_variables[i].end(), pattern);
            std::sregex_iterator end;
            while (it != end)
            {
                smatch match = *it;
                string beforeBraces = match[1].str();
                string value = match[2].str();
                InstructionTokens.push_back({beforeBraces, value});
            }
        }
    }
};

*/