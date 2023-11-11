#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <string.h>
#include <cstring>
#include <map>
#include <sys/wait.h>

#define MAX_NAME_LENGTH 500
#define MAX_TEXT_LENGTH 1000
#define PIPE_MODE 0666

#define COMMA_SEPERATOR ','
#define SHARP_SEPERATOR '#'
#define PERIOD_SEPERATOR '.'
#define STRING_SHARP "#"
#define DOLLAR_SEPERATOR '$'
#define STRING_DOLLAR "$"
#define STRING_COMMA ","
#define COLON ": "

#define WORDS_ID "w,"
#define WORDS_ID_ 'w'
#define REAPETS_ID "r,"
#define REAPETS_ID_ 'r'

#define EMPTY_STRING ""
#define ASCII_ZERO '0'

#define TESTS_DIR1 "./testcases/"
#define TESTS_DIR2 "./testcases"
#define OUTPUT_FILE_PATH "output.csv"

#define REDUCE_COMMAND "./reduce.out"
#define MAP_COMMAND "./map.out"

#define PERROR_DIR "opendir"


char* string_to_char_star(std::string str);
char* int_to_char_star(int n);
std::vector<std::string> get_files(char* path);
std::vector<std::string> seprate_line(std::string line);
std::string create_id(std::string name);
std::pair<int, char> find_id_type(std::string line);
std::map<std::string, int> count_no_words(char* file_name_);
void send_map_to_reduce(std::map<std::string, int> no_each_word, char* file_name, char* reduce_pipe);

std::map<std::string, int> organize_words_reapets(std::string all_texts);
std::pair<std::string, std::string> seprate_each_file(std::string each_file);
std::map<std::string, int> seprate_each_word(std::map<std::string, int> reduced_text, std::pair<std::string, std::string> words_reps);

void sendto_or_execute_map(int no_mappers, std::vector<std::string> file_names, char* reducer_pipe, int (*map_pipes)[2]);
void send_file_name_to_mapper(int mapper_id, std::string file_name_, int map_pipe);
void execute_mapper(int mapper_id, char* reducer_pipe, int main_pipe);

void execute_reducer(int no_mappers, char* reducer_pipe, int main_pipe);

void send_reduce_to_main(std::map<std::string, int> reduced_text, int pipe);
char* read_from_map_pipe(int counter, char* pipe);

void close_pipes(int (*pipes)[2], int no_mappers, int *reducer_to_main_pipe);
void create_output_file(int reducer_to_main_pipe);