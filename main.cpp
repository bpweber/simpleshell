#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <queue>
#define MAX_LINE 80

int main(){
    std::string args[MAX_LINE/2+1];     //used to store the args from a stringstream
    char *argc[MAX_LINE/2+1];           //used to call execvp()
    std::queue<std::string> hist;
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
            std::queue<std::string> tmp(hist);
            while(!tmp.empty()){
                std::cout << " " << tmp.front() << std::endl;
                tmp.pop();
            }
        }else if(args[0] == "!!"){
            std::string t;
            std::string last = hist.back();
            std::stringstream lcs(last);
            for(i = 0; i < MAX_LINE/2+1; i++)   //make sure to clear args[]
                args[i] = "";
            i = 0;
            lcs >> t;
            while(lcs >> args[i])            //parse user input into args[]
                i++;
            if(args[0] == "history"){
                std::queue<std::string> tmp(hist);
                while(!tmp.empty()){
                    std::cout << " " << tmp.front() << std::endl;
                    tmp.pop();
                }
            }
            nullterm = i;
            t = "";
            for(i = 0; i < nullterm; i++)
                t += args[i] + " ";
            in = t;
        }
        int to_wait = 1;                //should the parent wait for child to exit?
        for(int i = 0; i < MAX_LINE/2+1; i++)
            if(args[i] == "&")          //if there's an & then no!
                to_wait = 0;
        for(i = 0; i < MAX_LINE/2+1; i++)   //cast string[] to *char[]
            argc[i] = const_cast<char*>(args[i].c_str());
        argc[nullterm] = NULL;          //insert null terminator
        if(fork() == 0)                 //child
            execvp(argc[0], argc);
        else                            //parent
            if(to_wait)                 //didn't find an &, wait for child
                wait(NULL);
        num_cmds++;
        std::ostringstream ncs;
        ncs << num_cmds;
        hist.push(ncs.str() + " " + in);
        if(hist.size() > 10)
            hist.pop();
    }
    return 0;
}