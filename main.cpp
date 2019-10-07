#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <queue>
#include <stdlib.h>
#define MAX_LINE 80

struct cmd_map{             //used to hold commands in history 
    public:
        int loc;            //the number next to the command
        std::string cmd;    //the command and args string format
};

void printcmdhistory(std::queue<cmd_map> hist){ //function to print the recent commands
    while(!hist.empty()){                        //iterate and print
        std::cout << "   " << hist.front().loc << "\t" << hist.front().cmd << std::endl;
        hist.pop();                              //pop each time
    }
}

std::string getcmdfromhistory(std::string input, std::queue<cmd_map> hist, int& match){
    int num_cmd = 0;
    match = 0;
    if(input[0] == '!' && isdigit(input[1])){
        input = input.substr(1, input.size());
        std::stringstream ss(input);
        ss >> num_cmd;
    }
    while(!hist.empty()){                    //iterate and check keys for our number
        cmd_map front = hist.front();
        if(num_cmd == front.loc){
            match = 1;
            return front.cmd;                 //change the current command name to the one from hist
        }
        hist.pop();
    }
    return input;
}

int fillargsfromstring(std::string input, std::string args[]){
    std::stringstream ss(input);       //cast that string to a sstream
    int i = 0;
    for(; i < MAX_LINE/2+1; i++)   //make sure to clear args[]
        args[i] = "";
        i = 0;
    while(ss >> args[i])            //parse user input into args[]
        i++;
    return i;
}

int main(){
    std::string args[MAX_LINE/2+1];     //used to store the args from a stringstream
    char *argc[MAX_LINE/2+1];           //used to call execvp()
    std::queue<cmd_map> hist;           //a queue of all recent commands
    int num_cmds = 0;                   //keeps track of current command number
    int should_run = 1;
    while(should_run){
        std::cout << "osh> ";
        std::string in = "";
        getline(std::cin, in);          //get command and args as a string
        int nullterm = fillargsfromstring(in, args);               //null term for the end of the command and args
        if(args[0] == "exit")           //just check if they want to exit now
            should_run = 0;
        else if(args[0] == "history")
            printcmdhistory(hist);
        else if(args[0][0] == '!' && isdigit(args[0][1])){ //checking for format like !3 or !10 etc.                
            int match = 0;
            in = getcmdfromhistory(args[0], hist, match);
            if(!match){
                std::cout << "no such command in history" << std::endl;
                continue;
            }
            std::cout << in << std::endl;
            if(in == "history")
                printcmdhistory(hist);      //if its this command call it here
            nullterm = fillargsfromstring(in, args);               //used to set the end of the args[]
        }else if(args[0] == "!!"){
            if(hist.empty()){               //if hist is empty go to the top of loop
                std::cout << "no commands in history" << std::endl;
                continue;
            }
            in = hist.back().cmd;           //set the current command to the previous
            std::cout << in << std::endl;
            if(in == "history")            //history print call
                printcmdhistory(hist);
            nullterm = fillargsfromstring(in, args);             
        }
        if(args[0] != "history" && args[0] != "exit"){
            int to_wait = 1;                //should the parent wait for child to exit?
            for(int i = 0; i < MAX_LINE/2+1; i++)
                if(args[i] == "&")          //if there's an & then no!
                    to_wait = 0;
            for(int i = 0; i < MAX_LINE/2+1; i++)   //cast string[] to *char[]
                argc[i] = const_cast<char*>(args[i].c_str());
            argc[nullterm] = NULL;          //insert null terminator
            pid_t pid = fork();
            if(args[0] != "history" && args[0] != "exit")
                if(pid == 0){            //child process
                    if(execvp(argc[0], argc) == -1){
                        std::cout << "command not found" << std::endl;
                        exit(0);
                    }
                }else                            //parent
                    if(to_wait)                 //didn't find an &, wait for child
                        wait(NULL);
        }
        num_cmds++;                     //increment the cmd number after processing everything
        cmd_map current;                //a new cmd_map to add to history
        current.loc = num_cmds;         //set its number and command string
        current.cmd = in;
        hist.push(current);             //add it to queue
        if(hist.size() > 10)            //pop queue if it is more than 10S
            hist.pop();
    }
    return 0;
}