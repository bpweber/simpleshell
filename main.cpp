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

void printcmdhistory(std::queue<cmd_map> hist){     //function to print the recent commands
    if(hist.empty())
        std::cout << "-osh: no commands to display" << std::endl;
    while(!hist.empty()){                           //iterate and print
        std::cout << "   " << hist.front().loc << "\t" << hist.front().cmd << std::endl;
        hist.pop();                                 //pop each time
    }
}

std::string getcmdfromhistory(std::string input, std::queue<cmd_map> hist, int& match){ //used to check for command when user inputs !3 etc.
    int num_cmd = 0;
    match = 0;
    if(input[0] == '!' && isdigit(input[1])){       //check for basic formatting
        input = input.substr(1, input.size());      //substr the !    !3 -> 3
        std::stringstream ss(input);                //use sstream to make it an integer
        ss >> num_cmd;
    }
    while(!hist.empty()){                    //iterate and check keys for our number
        cmd_map front = hist.front();
        if(num_cmd == front.loc){               
            match = 1;
            return front.cmd;               //return the matching command from history
        }
        hist.pop();
    }
    return input;                           //no command was found in history so just return what we have
}

int fillargsfromstring(std::string input, std::string args[]){  //parses user input from string into args[]
    std::stringstream ss(input);    //cast that string to a sstream
    int i = 0;
    for(; i < MAX_LINE/2+1; i++)    //make sure to clear args[]
        args[i] = "";
        i = 0;
    while(ss >> args[i])            //parse user input into args[]
        i++;
    return i;                       //return the location of the nullptr
}

int main(){
    std::cout 
    << "*********************************************************************************" << std::endl
    << "*                    CS 433 Programming Assignment 2                            *" << std::endl
    << "*                          Author: Bryce Weber                                  *" << std::endl
    << "*                            Date: 10/9/2019                                    *" << std::endl
    << "*                     Course: CS433 (Operating Systems)                         *" << std::endl
    << "*             Description: A simple shell interface written in C++              *" << std::endl
    << "*********************************************************************************" << std::endl << std::endl
    << "Available osh commands:" << std::endl
    << "   history\tLists previously entered commands" << std::endl
    << "   !!\t\tExecutes most recent command from history" << std::endl
    << "   !n\t\tExecutes the nth command from history" << std::endl << std::endl;

    std::string args[MAX_LINE/2+1];     //used to store the args from a stringstream
    char *argc[MAX_LINE/2+1];           //used to call execvp()
    std::queue<cmd_map> hist;           //a queue of all recent commands
    int num_cmds = 0;                   //keeps track of current command number
    int should_run = 1;
    while(should_run){
        std::cout << "osh> ";
        std::string in = "";
        getline(std::cin, in);                          //get command and args as a string
        int nullterm = fillargsfromstring(in, args);    //null term for the end of the command and args
        if(args[0] == "exit")                           //check if they want to exit now
            should_run = 0;
        else if(args[0] == "history")
            printcmdhistory(hist);
        else if(args[0][0] == '!' && isdigit(args[0][1])){ //checking for format like !3 or !10 etc.                
            int match = 0;
            in = getcmdfromhistory(args[0], hist, match);   //this function returns a string which will be the command matching the number
            if(!match){                                     //no match so go back to the top of the loop
                std::cout << "-osh: no such command in history" << std::endl;
                continue;
            }
            std::cout << in << std::endl;
            if(in == "history")
                printcmdhistory(hist);                      //if its this command call it now (not a unix cmd)
            nullterm = fillargsfromstring(in, args);        //used to set the end of the args[]
        }else if(args[0] == "!!"){
            if(hist.empty()){               //if hist is empty go to the top of loop
                std::cout << "-osh: no commands in history" << std::endl;
                continue;
            }
            in = hist.back().cmd;           //set the current command to the previous
            std::cout << in << std::endl;
            if(in == "history")            //history print call
                printcmdhistory(hist);
            nullterm = fillargsfromstring(in, args);        //fills the args[] and marks where the null terminator should go     
        }
        if(args[0] != "history" && args[0] != "exit"){      //only fork() if were running a system command
            int to_wait = 1;                                //should the parent wait for child to exit?
            for(int i = 0; i < MAX_LINE/2+1; i++)
                if(args[i] == "&")                          //if there's an & then no!
                    to_wait = 0;
            for(int i = 0; i < MAX_LINE/2+1; i++)   //cast string[] to *char[]
                argc[i] = const_cast<char*>(args[i].c_str());
            argc[nullterm] = NULL;                  //insert null terminator
            pid_t pid = fork();
            if(pid == 0){                           //child process
                if(execvp(argc[0], argc) == -1){    //returns -1 if execvp fails
                    std::cout << "-osh: " << argc[0] << ": command not found" << std::endl;
                    exit(0);
                }
            }else                   //parent
                if(to_wait)         //didn't find an &, wait for child
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