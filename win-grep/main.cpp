#include <iostream>

#include "log_reader.h"
#include <string>


int main(int argc, char *argv[]) {
    CLogReader reader;

    if (argc == 3) {
        bool result = false;
        if (reader.SetFilter(argv[2])) {
            if (reader.Open(argv[1])) {
                char buffer[CLogReader::DEFAULT_BUFFER_SIZE];
                
                // twice getting false on next line means eof
                int fail_count = 0;
                while (fail_count < 2) {
                    if (reader.GetNextLine(buffer, CLogReader::DEFAULT_BUFFER_SIZE))
                        std::cout << buffer << std::endl;
                    else
                        fail_count++;
                }

                reader.Close();

                return 0;
            } else
                std::cout << "opening file '" << argv[1] << "' error" << std::endl;
        } else
            std::cout << "wrong filter '" << argv[2] << "'" << std::endl;
    } else
        std::cout << argv[0] << " path filter" << std::endl;

    return 1; // -----
}
