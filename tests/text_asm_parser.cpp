#include <sstream>
#include <string>

#include "text_asm_parser.h"

using namespace robot;

using tmp_t =
metaprogramming::sequence
<
    std::integral_constant<char, '['>,
    uint16_t,
    std::integral_constant<char, ']'>,

    std::integral_constant<char, '['>,
    uint16_t,
    std::integral_constant<char, ']'>,

    std::integral_constant<char, '['>,
    value_list<uint16_t>,
    std::integral_constant<char, ']'>
>;

int main()
{
    constexpr size_t buf_size = 256;

    char buf[buf_size];

    std::istringstream input;
    std::ofstream ofs("outfile.txt", std::ios::out|std::ios::app);
	std::ifstream ifs("outfile.txt", std::ios::in);

    while(std::cin && !std::cin.eof()) {
        std::cout << "Enter command" << "\n" << "command format 'command_name [function_number] [parameter_number] [parameter_value]'" << "\n" << "\n" << ">" ;
        std::cin.getline(buf, buf_size);
        std::string s(buf, buf_size);

        input.str(s);

        tmp_t tmp;

        try {
            std::string cmd_name;

            input >> cmd_name;

            if((cmd_name == "write") or (cmd_name == "read") or (cmd_name == "set"))
            {
                input >> tmp;
                
                std::cout << "success\n";
    			ofs <<buf << "\n" << "\n";
            }
            else if (cmd_name == "load")
			{	
				while((ifs.good()) )
				    std::cout<<(char)ifs.get();
			}
            else
                std::cout << "unknown command\n";
        }
        catch(constant_mismatch_error) {
            std::cout << "invalid command format" << std::endl;
        }
    }

    std::cout << "line is too long\n";
    ofs.close();
	ifs.close();
    return 0;
}
