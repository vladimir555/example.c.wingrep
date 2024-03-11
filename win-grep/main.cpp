#include <cstdio>

#include "log_reader.h"


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
                        printf("%s\n", buffer);
                    else
                        fail_count++;
                }

                reader.Close();

                return 0;
            } else
                printf("opening file '%s' error\n", argv[1]);
        } else
            printf("wrong filter '%s'", argv[2]);
    } else
        printf("%s path filter", argv[0]);

    return 1; // -----
}
