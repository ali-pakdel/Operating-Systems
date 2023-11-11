#include "headers.hpp"

using namespace std;

char* string_to_char_star(string str)
{
    char * char_star = new char[str.length() + 1];
    strcpy(char_star, str.c_str());
    return char_star;
}

char* int_to_char_star(int n)
{
    string str = to_string(n);
    return string_to_char_star(str);
}

vector<string> get_files(char* path)
{
    DIR *dir;
    struct dirent *diread;
    vector<string> file_names;

    if ((dir = opendir(path)) != nullptr)
    {
        while ((diread = readdir(dir)) != nullptr)
        {
            if (strlen(diread->d_name) <= 3)
                continue;
            file_names.push_back(diread->d_name);
        }
        closedir (dir);
    }
    else
    {
        perror(PERROR_DIR);
        exit(0);
    }
    cout << "Found " << file_names.size() << " files:" << endl
         << file_names[0];
    for (int i = 0; i < file_names.size(); i++)
        cout << ", " << file_names[i];
    cout << endl;
    
    return file_names;
}

vector<string> seprate_line(string line)
{
	stringstream temp_line;
    string word;
    vector<string> seprated_line;
    
    temp_line << line;

    while (getline(temp_line, word, COMMA_SEPERATOR))
    {
        seprated_line.push_back(word);
    }

    return seprated_line;
}

string create_id(string name)
{
    string id = EMPTY_STRING;
    for(int i = 0; name[i] != PERIOD_SEPERATOR; i++)
    {
        id += name[i];
    }
    return id;
}

pair<int, char> find_id_type(string word)
{   
    pair<int, char> id_type;
    
    id_type.first = word[0] - ASCII_ZERO;
    id_type.second = word[1];

    return id_type;
}

map<string, int> count_no_words(char* file_name_)
{
    map<string, int> no_each_word;
    char* file_name = string_to_char_star(TESTS_DIR1);
    strcat(file_name, file_name_);
    
    ifstream file(file_name);

    string line;
    while(getline(file, line))
    {
        vector<string> seprated_line = seprate_line(line);
        for(int i = 0; i < seprated_line.size(); i++)
        {
            no_each_word[seprated_line[i]]++;
        }
    }
    file.close();
    return no_each_word;
}

void send_map_to_reduce(map<string, int> no_each_word, char* file_name, char* pipe_name)
{
    int reduce_pipe = open(pipe_name, O_WRONLY);
    
    char *words = (char*) malloc(MAX_TEXT_LENGTH * sizeof(char));
    char *reapets = (char*) malloc(MAX_TEXT_LENGTH * sizeof(char));
    string id = create_id(file_name);
    
    strcpy(words, string_to_char_star(id));
    strcat(words, WORDS_ID);
    strcpy(reapets, string_to_char_star(id));
    strcat(reapets, REAPETS_ID);

    map<string, int>::iterator itr;
    for (itr = no_each_word.begin(); itr != no_each_word.end(); ++itr)
    {
        strcat(words, string_to_char_star(itr->first));
        strcat(words, string_to_char_star(STRING_COMMA));

        strcat(reapets, string_to_char_star(to_string(itr->second)));
        strcat(reapets, string_to_char_star(STRING_COMMA));
    }
    strcat(words, STRING_DOLLAR);
    strcat(words, reapets);
    
    write(reduce_pipe, words, MAX_TEXT_LENGTH);
    close(reduce_pipe);
}

pair<string, string> seprate_each_file(string each_file)
{
    stringstream ss_each_file;
    ss_each_file << each_file;
    string words, reapets;
    getline(ss_each_file, words, DOLLAR_SEPERATOR);
    getline(ss_each_file, reapets, DOLLAR_SEPERATOR);
    return make_pair(words, reapets);
}

map<string, int> seprate_each_word(map<string, int> reduced_text, pair<string, string> words_reps)
{
    stringstream ss_words, ss_reapets;
    ss_words << words_reps.first;
    ss_reapets << words_reps.second;
    string word, count;

    getline(ss_words, word, COMMA_SEPERATOR);
    getline(ss_reapets, count, COMMA_SEPERATOR);
    while(getline(ss_words, word, COMMA_SEPERATOR))
    {
        getline(ss_reapets, count, COMMA_SEPERATOR);
        reduced_text[word] += atoi(string_to_char_star(count));
    }

    return reduced_text;
}

map<string, int> organize_words_reapets(string all_texts_)
{
    map<string, int> reduced_text;

    string text = string(all_texts_);
    stringstream ss_text;
    string each_csv;
    ss_text << all_texts_;
    while (getline(ss_text, each_csv, SHARP_SEPERATOR))
    {
        pair<string, string> words_reps = seprate_each_file(each_csv);
        reduced_text = seprate_each_word(reduced_text, words_reps);
    }
    return reduced_text;
}

void send_reduce_to_main(map<string, int> reduced_text, int main_pipe)
{
    string text = EMPTY_STRING;
    map<string, int>::iterator itr;

    for (itr = reduced_text.begin(); itr != reduced_text.end(); ++itr)
    {
        text += (itr->first + COLON + to_string(itr->second) + STRING_COMMA); 
    }
    text.pop_back();
    write(main_pipe, string_to_char_star(text), MAX_TEXT_LENGTH);
}

char* read_from_map_pipe(int counter, char* pipe_name)
{
    int map_pipe = open(pipe_name, O_RDONLY);

    char* all_texts = string_to_char_star(EMPTY_STRING);
    while (counter != 0) {
    	char* words = (char*) malloc(MAX_TEXT_LENGTH * sizeof(char));
        
        if (read(map_pipe, words, MAX_TEXT_LENGTH) <= 0)
            continue;

        counter--;
        strcat(all_texts, words);
        strcat(all_texts, STRING_SHARP);
    }
    close(map_pipe);
    return all_texts;
}

void send_file_name_to_mapper(int mapper_id, string file_name_, int map_pipe)
{
    char *file_name = string_to_char_star(file_name_);
    cout << "Sending " << file_name << " file to mapper number " << mapper_id << endl;
    write(map_pipe, file_name, MAX_NAME_LENGTH);
}

void execute_mapper(int mapper_id, char* reducer_pipe, int main_pipe)
{
    char* arguments[] = {reducer_pipe, int_to_char_star(main_pipe), NULL};
    char* command = string_to_char_star(MAP_COMMAND);

    cout << "Mapper " << mapper_id <<  " started!" << endl;
    execvp(command, arguments);
}

void execute_reducer(int no_mappers, int reducer_to_main_pipe_un, char* reducer_pipe)
{
    string no_mappers_ = to_string(no_mappers);
    string main_pipe_un = to_string(reducer_to_main_pipe_un);

    char* argv_elements[] = {string_to_char_star(no_mappers_), reducer_pipe, string_to_char_star(main_pipe_un), NULL};
    char* command = string_to_char_star(REDUCE_COMMAND);
    execvp(command, argv_elements);
}

void close_pipes(int (*map_pipes_un)[2], int no_mappers, int *reducer_to_main_pipe_un)
{
    for (int i = 0 ; i < no_mappers; i++) 
    {
        close(map_pipes_un[i][0]);
        close(map_pipes_un[i][1]);
        wait(NULL);
    }
    close(reducer_to_main_pipe_un[0]);
    close(reducer_to_main_pipe_un[1]);
}

void create_output_file(int reducer_to_main_pipe)
{
    char* result = (char*) malloc(MAX_TEXT_LENGTH * sizeof(char));
    read(reducer_to_main_pipe, result, MAX_TEXT_LENGTH);
    
    ofstream output_file(OUTPUT_FILE_PATH);
    output_file << result;

    cout << "Main processor created output.csv!" << endl;
}

void sendto_or_execute_map(int no_mappers, vector<string> file_names, char* reducer_pipe, int (*map_pipes_un)[2])
{
    for (int i = 0; i < no_mappers; i++)
    {
        if (fork() != 0)
            send_file_name_to_mapper(i, file_names[i], map_pipes_un[i][1]);
        else
            execute_mapper(i, reducer_pipe, map_pipes_un[i][0]);
    }
}

void execute_reducer(int no_mappers, char* reducer_pipe, int main_pipe)
{
    if (fork() == 0)
    {
        execute_reducer(no_mappers, main_pipe, reducer_pipe);
    }
}