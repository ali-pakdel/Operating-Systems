#include "headers.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    string path_ = TESTS_DIR2;
    char* path = string_to_char_star(path_);
    vector<string> file_names = get_files(path); 

    int no_mappers = file_names.size();
    cout << "We have " << no_mappers << " mapper!" << endl;

    char* reducer_pipe = string_to_char_star("nrp");
    mkfifo(reducer_pipe, PIPE_MODE);

    int reducer_to_main_pipe_un[2];
    pipe(reducer_to_main_pipe_un);

    int map_pipes_un[no_mappers][2];
    for (int i = 0; i < no_mappers; i++)
    {
        pipe(map_pipes_un[i]);
    }

    sendto_or_execute_map(no_mappers, file_names, reducer_pipe, map_pipes_un);
    execute_reducer(no_mappers, reducer_pipe, reducer_to_main_pipe_un[1]);
    create_output_file(reducer_to_main_pipe_un[0]);

    close_pipes(map_pipes_un, no_mappers, reducer_to_main_pipe_un);
    unlink(reducer_pipe);

    wait(NULL);

    return 0;
}