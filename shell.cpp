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

int main () {


    while (true){
        int std_in = dup(0);

        cout << getenv("USER") << "$ "; //print a prompt
        string inputline;
        getline (cin, inputline); //get a line from standard input
        vector<string> commands = parse_commands(inputline);        

        for (int i=0; i<commands.size(); i++) {
            // cout << "command: " << commands.at(i) << endl;
            bool output_redir = false, input_redir = false;
            string output_file, input_file;
            string command = commands.at(i);
            vector<string> args;
            
            stringstream ss(command);
            string arg_input;

            // TODO: make parse_args command
            while (getline(ss, arg_input, ' ')) {
                if (arg_input == ">") {
                    output_redir = true;
                    getline(ss, output_file, ' ');
                }
                else if (arg_input == "<") {
                    input_redir = true;
                    getline(ss, input_file, ' ');
                }
                else {
                    if (arg_input[0] == '\"' || arg_input[0] == '\'') {
                        arg_input = remove_quotation(arg_input);
                    }

                    args.push_back(arg_input);
                }
            }
            // execute_command(args);

            if (command == string("exit")) {
                cout << "Bye!! End of shell" << endl;
                break;
            }

            int fds[2];
            pipe(fds);

            int pid = fork ();
            if (pid == 0) { //child process
                cout << "in child\n";
                int fd_out;
                if (output_redir)
                {
                    fd_out = open(output_file.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0666);
                    dup2(fd_out, 1);
                    close(fd_out);
                }

                int fd_in;
                if (input_redir)
                {
                    fd_in = open(input_file.c_str(), O_RDONLY, 0666);
                    dup2(fd_in, 0);
                    close(fd_in);
                }

                if(i<commands.size()-1) {
                    dup2(fds[1], 1);
                    close(fds[1]);
                }

                execute_command(args);
            }
            else {
                dup2(fds[0], 0);
                close(fds[1]);
                
                waitpid (pid, 0, 0); //parent waits for child process
            }
        }
        dup2(std_in, 0);
    }
}
