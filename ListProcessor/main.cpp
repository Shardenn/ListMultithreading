#include <Windows.h>
#include <list>
#include "loop.h"
#include <memory>
#include <time.h> // for random generator
#include <iostream>
#include <vector>
#include <string>
#include <thread>

typedef ListT uint8_t;
#define MAX_ELEMENT_VALUE 100

bool stop_loop = false;
bool add_element = true;
std::list<ListT>* glist = new std::list<ListT>;
CRITICAL_SECTION crit_section;

/*
* 0 : no output at all except exceptions
* 1 : program will print what is put and what is taken from the list
*/
int verbosity_level = 1; 

struct ThreadParams
{
    std::list<ListT>* list;
};

void wait_for_input()
{
    std::string command;
    std::getline(std::cin, command);
    if (command == "s")
    {
        stop_loop = true;
    }
}

DWORD WINAPI edit_list(LPVOID params)
{
    auto args = (ThreadParams*)params;
    
    while (!stop_loop)
    {
        try
        {
            EnterCriticalSection(&crit_section);

            auto list = args->list;

            if (glist->size() < 1 && !add_element)
                add_element = true;
            if (glist->size() > 10 && add_element)
                add_element = false;

            if (add_element || glist->size() < 1)
            {
                ListT new_element = rand() % MAX_ELEMENT_VALUE + 1;

                if (verbosity_level > 0)
                    std::cout << "Putting  " << (int)new_element << " to the end of the list\n";

                glist->push_back(new_element);
            }
            else
            {
                if (verbosity_level > 0)
                    std::cout << "Removing " << (int)glist->back() << " from the end of the list\n";

                glist->pop_back();
            }
            LeaveCriticalSection(&crit_section);
        }
        catch (std::exception& ex)
        {
            std::cout << "Exception in the function put_element_to_list(): \n" << ex.what();
            return -1;
        }
    }
    return 0;
}


int main(int argc, char* argv[])
{
    srand(time(NULL));

    if (argc > 1)
    {
        std::string verb_arg = argv[1];
        verbosity_level = std::stoi(verb_arg);
    }

    if (!InitializeCriticalSectionAndSpinCount(&crit_section, 400))
    {
        std::cout << "Could not initialize critical section.\n";
        return -1;
    }

    std::list<ListT>* list = new std::list<ListT>;

    int elements_to_fill = 0;
    for (int i = 0; i < elements_to_fill; i++)
    {
        glist->push_back(rand() % MAX_ELEMENT_VALUE + 1);
    }

    ThreadParams *params = new ThreadParams;
    params->list = list;

    HANDLE* hThreadArray = new HANDLE[2];
    DWORD* dThreadIDs = new DWORD[2];

    try
    {
        CreateThread(
            NULL,
            0,
            edit_list,
            &params,
            0,
            &(dThreadIDs[0])
        );

        CreateThread(
            NULL,
            0,
            edit_list,
            &params,
            0,
            &(dThreadIDs[1])
        );

        
    }
    catch (std::exception& ex)
    {
        std::cout << "Exception caught in the main func while creating threads: \n" << ex.what();
        return -1;
    }

    std::thread input_loop(wait_for_input);
    
    std::string command;

    while (true)
    {
        int random_num = rand() % 10;
        if (random_num % 2 == 0)
        {
            add_element = !add_element;
        }

        if (stop_loop)
        {
            input_loop.join();
            break;
        }
    }

    WaitForMultipleObjects(2, hThreadArray, true, INFINITY);

    delete glist, list, params;
    delete[] hThreadArray, dThreadIDs;

    return 0;
}