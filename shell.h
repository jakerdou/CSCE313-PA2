#ifndef SHELL_H
#define SHELL_H

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <fcntl.h>
#include <time.h>
#include <wait.h>
#include <signal.h>

#include <sys/time.h>
#include <cassert>
#include <assert.h>

#include <cmath>
#include <numeric>
#include <algorithm>

#include <list>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include <chrono>
#include <ctime>

using namespace std;

string remove_quotation(string str) {
    cout << "here in the function\n";

    str.erase(str.length() - 1, 1);
    str.erase(0, 1);
    return str;
}

string trim_whitespace(string command) {
    if (command[0] == ' ') {
        command.erase(0, 1);
    }
    if (command[command.length()-1] == ' ') {
        command.erase(command.length()-1, 1);
    }
    return command;
}

vector<string> parse_commands(string inputline) {
    vector<string> commands;
    stringstream ss(inputline);
    string command;

    while (getline(ss, command, '|')) {
        command = trim_whitespace(command);
        commands.push_back(command);
    }

    return commands;
}

void execute_command(vector<string> command) {
    char* args[50];
    for (int i=0; i<=command.size(); i++)
    {   
        if (i<command.size()) {
            string arg = command.at(i);
            char* arg_copy = strdup(arg.c_str());
            args[i] = arg_copy;
        }
        else {
            args[i] = NULL;
        }
    }

    try
    {
        execvp(args[0], args);
    }
    catch(const exception& e)
    {
        cerr << e.what() << '\n';
    }    
}

void display_prompt() {
    string user = getenv("USER");
    auto now = chrono::system_clock::now();
    time_t date_time = chrono::system_clock::to_time_t(now);
    char* time_str = ctime(&date_time);

    if (time_str[strlen(time_str) - 1] == '\n')
    {
        time_str[strlen(time_str) - 1] = '\0';
    }
    cout << user << " " << time_str << "$ "; //print a prompt
}

void wait_bg_procs(vector<int>* bg_procs) {
    for (int i = 0; i < bg_procs->size(); i++)
    {
        int pid = bg_procs->at(i);
        int proc_stat = waitpid(pid, 0, WNOHANG);

        if (proc_stat == pid)
        {
            bg_procs->erase(bg_procs->begin() + i);
        }
    }
    
}

void change_dir(string next_dir, vector<string>* prev_dirs) {
    if (next_dir == "-")
    {
        if (prev_dirs->size() > 0)
        {
            string back = prev_dirs->back();
            char* next_dir_cstr = (char*) back.c_str();

            chdir(next_dir_cstr);
            prev_dirs->pop_back();
        }
        else {
            cout << "There is no directory to go back to.\n";
        }
    }
    else {
        const int BUFFER_SIZE = 256;

        char curr_dir_cstr[BUFFER_SIZE];
        getcwd(curr_dir_cstr, BUFFER_SIZE);
        string curr_dir(curr_dir_cstr);
        prev_dirs->push_back(curr_dir);

        char* dir_cstr = (char*) next_dir.c_str();
        chdir(dir_cstr);
    }
    
}

struct command_data {
    bool output_redir, input_redir, bg_proc;
    string output_file, input_file;
    vector<string> args;
};

command_data get_command_data(string command) {
    bool output_redir = false, input_redir = false, bg_proc = false;
    string output_file, input_file;
    vector<string> args;
    
    stringstream ss(command);
    string arg_input;

    while (getline(ss, arg_input, ' ')) {
        if (arg_input == ">") {
            output_redir = true;
            getline(ss, output_file, ' ');
        }
        else if (arg_input == "<") {
            input_redir = true;
            getline(ss, input_file, ' ');
        }
        else if (arg_input == "&")
        {
            bg_proc = true;
        }
        else if (arg_input != "") {
            if (arg_input[0] == '\"' || arg_input[0] == '\'') {
                if (arg_input[0] == arg_input[arg_input.length() - 1])
                {
                    arg_input = remove_quotation(arg_input);
                }
                else
                {
                    string quote_str, extra_space;
                    getline(ss, quote_str, arg_input[0]);
                    arg_input = arg_input.substr(1) + " " + quote_str;
                }
            }

            args.push_back(arg_input);
        }
    }
    command_data cmd_data;
    cmd_data.output_redir = output_redir;
    cmd_data.input_redir = input_redir;
    cmd_data.bg_proc = bg_proc;
    cmd_data.output_file = output_file;
    cmd_data.input_file = input_file;
    cmd_data.args = args;

    return cmd_data;
}

void io_redir(command_data cmd_data) {
    int fd_out;
    if (cmd_data.output_redir)
    {
        fd_out = open(cmd_data.output_file.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0666);
        dup2(fd_out, 1);
        close(fd_out);
    }

    int fd_in;
    if (cmd_data.input_redir)
    {
        fd_in = open(cmd_data.input_file.c_str(), O_RDONLY, 0666);
        dup2(fd_in, 0);
        close(fd_in);
    }
}

#endif