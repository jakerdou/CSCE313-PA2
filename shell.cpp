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

int main () {

    while (true){
        cout << "My Shell$ "; //print a prompt
        string inputline;
        getline (cin, inputline); //get a line from standard input

        char* args[10][50];
        int num_args = 0, num_processes = 0;
        stringstream ss(inputline);
        string arg_input;
        bool output_redir = false, input_redir = false;
        string output_file, input_file;

        while (getline(ss, arg_input, ' ')) {
            if (arg_input == ">") {
                output_redir = true;
                getline(ss, output_file, ' ');
            }
            else if (arg_input == "<") {
                input_redir = true;
                getline(ss, input_file, ' ');
            }
            else if (arg_input == "|") {
                args[num_processes][num_args] = NULL;
                num_processes++;
                num_args = 0;
            }
            else {
                if (arg_input[0] == '\"' || arg_input[0] == '\'') {
                    arg_input = remove_quotation(arg_input);
                }

                char* arg_input_copy = strdup(arg_input.c_str());

                args[num_processes][num_args] = arg_input_copy;
                num_args++;
            }

        }

        cout << "ls: " << args[0][0] << endl;


        if (inputline == string("exit")) {
            cout << "Bye!! End of shell" << endl;
            break;
        }

        int fds[2];
        pipe(fds);

        int pid = fork ();
        if (pid == 0) { //child process

            int fd_out;
            if (output_redir)
            {
                fd_out = open(output_file.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0666);
                dup2(fd_out, 1);
            }

            int fd_in;
            if (input_redir)
            {
                fd_in = open(input_file.c_str(), O_RDONLY, 0666);
                dup2(fd_in, 0);
            }

            dup2(fds[1], 1);


            // cout << "pwd should be here: " << args[1][0] << endl;
            execvp (args [0][0], args [0]);

            close(fd_out);
            close(fd_in);
        }
        else {
            waitpid (pid, 0, 0); //parent waits for child process



            char pipe_buf[100];

            read(fds[0], pipe_buf, 100);
            cout << "from fds[1]: " << pipe_buf << endl;

        }
    }
}
