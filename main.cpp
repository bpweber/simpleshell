#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <queue>
#include <stdlib.h>
#define MAX_LINE 80

struct cmd_map{
    public:
        int loc;
        std::string cmd; 
};

void printcmdhistory(std::queue<cmd_map> hist){
    std::queue<cmd_map> tmp(hist);
    while(!tmp.empty()){
        std::cout << "   " << tmp.front().loc << "\t" << tmp.front().cmd << std::endl;
        tmp.pop();
    }
}

int main(){
    std::string args[MAX_LINE/2+1];     //used to store the args from a stringstream
    char *argc[MAX_LINE/2+1];           //used to call execvp()
    std::queue<cmd_map> hist;
    int num_cmds = 0;
    int should_run = 1;
    while(should_run){
        std::cout << "osh> ";
        std::string in = "";
        getline(std::cin, in);          //get command and args as a string
        std::stringstream ss(in);       //cast that string to a sstream
        int i = 0;
        for(i = 0; i < MAX_LINE/2+1; i++)   //make sure to clear args[]
            args[i] = "";
        i = 0;
        while(ss >> args[i])            //parse user input into args[]
            i++;
        int nullterm = i;               //null term for the end of the command and args
        if(args[0] == "exit")           //just check if they want to exit now
            should_run = 0;
        else if(args[0] == "history"){
            printcmdhistory(hist);
        }else if(args[0][0] == '!' && isdigit(args[0][1])){
            std::string cnum = args[0].substr(1,2);
            std::stringstream cmdnum(cnum);
            int c_num = 0;
            cmdnum >> c_num;
            int match = 0;
            std::queue<cmd_map> tmp(hist);
            while(!tmp.empty()){
                cmd_map front = tmp.front();
                if(c_num == front.loc){
                    match = 1;
                    in = front.cmd;
                }
                tmp.pop();
            }
            if(in == "history"){
                printcmdhistory(hist);
            }
            std::stringstream ss(in);       //cast that string to a sstream
            for(i = 0; i < MAX_LINE/2+1; i++)   //make sure to clear args[]
                args[i] = "";
            i = 0;
            while(ss >> args[i])            //parse user input into args[]
                i++;
            int nullterm = i;  
        }else if(args[0] == "!!"){
            if(hist.empty()){
                std::cout << "no commands in history" << std::endl;
                continue;
            }
            in = hist.back().cmd;
            if(in == "history"){
                printcmdhistory(hist);
            }
            std::stringstream ss(in);       //cast that string to a sstream
            for(i = 0; i < MAX_LINE/2+1; i++)   //make sure to clear args[]
                args[i] = "";
            i = 0;
            while(ss >> args[i])            //parse user input into args[]
                i++;
            int nullterm = i;             
        }
        int to_wait = 1;                //should the parent wait for child to exit?
        for(int i = 0; i < MAX_LINE/2+1; i++)
            if(args[i] == "&")          //if there's an & then no!
                to_wait = 0;
        for(i = 0; i < MAX_LINE/2+1; i++)   //cast string[] to *char[]
            argc[i] = const_cast<char*>(args[i].c_str());
        argc[nullterm] = NULL;          //insert null terminator
        pid_t pid = fork();
        if(pid == -1){                 //child
            std::cout << "error occured" << std::endl;
            exit(EXIT_FAILURE);
        }else if(pid == 0){
            execvp(argc[0], argc);
            exit(0);
        }else                            //parent
            if(to_wait)                 //didn't find an &, wait for child
                wait(NULL);
        num_cmds++;
        cmd_map current;
        current.loc = num_cmds;
        current.cmd = in;
        hist.push(current);
        if(hist.size() > 10)
            hist.pop();
    }
    return 0;
}