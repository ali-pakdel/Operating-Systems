#include "headers.hpp"

using namespace  std;

int main(int argc, char* argv[])
{
    char* reducer_pipe = (char*) malloc(MAX_NAME_LENGTH * sizeof(char));
    reducer_pipe = argv[0];
    int main_pipe_un = atoi(argv[1]);

    char* file_name_ = (char*) malloc(MAX_NAME_LENGTH * sizeof(char));
    read(main_pipe_un, file_name_, MAX_NAME_LENGTH);

    map<string, int> no_each_word;
    no_each_word = count_no_words(file_name_);
    
    send_map_to_reduce(no_each_word, file_name_, reducer_pipe);
    cout << "Mapped of file " << file_name_ << " sended to reducer!" << endl;

    return 0;
}
