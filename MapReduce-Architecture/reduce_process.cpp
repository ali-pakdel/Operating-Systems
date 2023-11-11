#include "headers.hpp"

using namespace std;

int main(int argc, char* argv[]) {

	int no_maps = atoi(argv[0]);
    char* named_pipe = argv[1];
    int unnamed_pipe = atoi(argv[2]);

	vector<pair<string, string>> words_reapets(no_maps, make_pair(EMPTY_STRING, EMPTY_STRING));
    
    char* all_texts = read_from_map_pipe(no_maps, named_pipe);

   	map<string, int> reduced_text = organize_words_reapets(string(all_texts));
    send_reduce_to_main(reduced_text, unnamed_pipe);

    cout << "Reduced text sended to main!" << endl;

    return 0;
}