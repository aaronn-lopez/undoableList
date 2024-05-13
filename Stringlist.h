#pragma once

#include <cassert>
#include <iostream>
#include <string>

using namespace std;

class Stringlist
{
    int cap;     // capacity
    string *arr; // array of strings
    int sz;      // size

    //bool to determine if function call should be written to list.
    bool writeToCallList = true; 

    // Helper function for throwing out_of_range exceptions.
    void bounds_error(const string &s) const
    {
        throw out_of_range("Stringlist::" + s + " index out of bounds");
    }

    // Helper function for checking index bounds.
    void check_bounds(const string &s, int i) const
    {
        if (i < 0 || i >= sz)
        {
            bounds_error(s);
        }
    }

    // Helper function for copying another array of strings.
    void copy(const string *other)
    {
        for (int i = 0; i < sz; i++)
        {
            arr[i] = other[i];
        }
    }

    // Helper function for checking capacity; doubles size of the underlying array if necessary.
    void check_capacity()
    {
        if (sz == cap)
        {
            cap *= 2;
            string *temp = new string[cap];
            for (int i = 0; i < sz; i++)
            {
                temp[i] = arr[i];
            }
            delete[] arr;
            arr = temp;
        }
    }

    //Undo stack created with singly linked list.
    struct Node
    {
        string functionCalled;
        Node *next = nullptr;
    };

    Node *head = nullptr;

    //Function that adds node to list after undoable function calls.
    void writeToList(const string &s)
    {
        if(head == nullptr)
        {
            head = new Node;
            head->functionCalled = s;
            head->next = nullptr;
        }
        else
        {
            Node *lastCall = new Node;
            lastCall->functionCalled = s;
            lastCall->next = head;
            head = lastCall;
        }
    }

    //Function that reads the string at the top of the linked list.
    string readLastCall() const
    {
        if(head != nullptr)
        {
            return(head->functionCalled);
        }
        else
        {
            return("");
        }
    }
   
    //Function that removes the top node in the linked list.
    void removeLastCall()
    {
        if(head != nullptr)
        {
            Node *nexInStack = head->next;
            delete head;
            head = nexInStack;
        }
    }

public:

/*
When writing to the list in functions below, each node will contain up to 3 
pieces of data that will help identify the exact action performed on the string. 
The function below returns such data to assist the undo() function in reverting changes
*/
    string findDataSubstr(int startingPos, int lastCallDataNum)
    {
        //The startingPos argument indicates at which index of the string I want to start the .find iteration from
        //Within each node, the string contains 1-3 pieces of data that is separated by whitespace: 
        //Function call type, Index where data was, and the string itself. 
        //Knowing this, the function is able to identify which of the 1-3 substrings I want to work with. 
        string subStrData;
        string originalStr;
        int subStrIterator = 1;
        originalStr = readLastCall();

        //To use .find() we indicate where the search begins and ends.
        size_t start = startingPos;
        size_t end = originalStr.find(' ', start);

        //Search through string for specific substring based on lastCallDataNum parameter.
        while(subStrIterator != lastCallDataNum)
        {
            subStrData = originalStr.substr(start, end - start); 
            subStrIterator++;         
            start = end + 1;
            end = originalStr.find(' ');
        }
        return(subStrData);
    }

    //Default constructor: makes an empty StringList.
    Stringlist()
        : cap(10), arr(new string[cap]), sz(0)
    {
    
    }

    //Copy constructor: makes a copy of the given StringList.
    //Does NOT  copy the undo stack, or any undo information from other.
    Stringlist(const Stringlist &other)
        : cap(other.cap), arr(new string[cap]), sz(other.sz)
    {
        copy(other.arr);
    }

    //Destructor
    ~Stringlist()
    {
        delete[] arr;
        while(head != nullptr)
        {
            removeLastCall();
        }
    }

    /*
    Assignment operator: makes a copy of the given StringList. (undoable)
    For undoing, when assigning different lists, the undo stack is NOT copied: lst1 = lst2; 
    
    lst1 undo stack is updated to be able to undo the assignment; 
    lst1 does not copy lst2's stack.
    lst2 is not change in any way
    
    Self-assignment is when you assign a list to itself: lst1 = lst1;
    
    In this case, nothing happens to lst1. 
    Both its string functionCalled and undo stack are left as-is.
    */
    Stringlist &operator=(const Stringlist &other)
    {
        if(this != &other)
        {
            if(writeToCallList == true)
            {
                for(int i = 0; i < sz; i++)
                {
                    writeToList("LST_SET " + std::to_string(sz) + ' ' + arr[i]);
                }
            }

            delete[] arr;
            cap = other.capacity();
            arr = new string[cap];
            sz = other.size();
            copy(other.arr);
        }
        return *this;
    }

    //Returns the number of strings in the list.
    int size() const { return sz; }

    //Returns true if the list is empty, false otherwise.
    bool empty() const { return size() == 0; }

    //Returns the capacity of the list, i.e. the size of the underlying array.
    int capacity() const { return cap; }

    //Returns the string at the given index.
    string get(int index) const
    {
        check_bounds("get", index);
        return arr[index];
    }

    //Returns the index of the first occurrence of s in the list, or -1 if s is not in the lst.
    int index_of(const string &s) const
    {
        for (int i = 0; i < sz; i++)
        {
            if (arr[i] == s)
            {
                return i;
            }
        }
        return -1;
    }

    //Returns true if s is in the list, false otherwise.
    bool contains(const string &s) const
    {
        return index_of(s) != -1;
    }

    //Returns a string representation of the list.
    string to_string() const
    {
        string result = "{";
        for (int i = 0; i < size(); i++)
        {
            if (i > 0)
            {
                result += ", ";
            }
            result += "\"" + get(i) + "\"";
        }
        return result + "}";
    }

    //Sets the string at the given index. (undoable)
    void set(int index, string value)
    {
        if(writeToCallList == true)
        {
            writeToList("SET " + std::to_string(index) + ' ' + arr[index]);
        }

        check_bounds("set", index);
        arr[index] = value;
    }

    //Insert s before index; if necessary, the capacity of the underlying array is doubled. (undoable)
    void insert_before(int index, const string &s)
    {
        if(writeToCallList == true)
        {
            writeToList("INS " + std::to_string(index));
        }
        
        if (index < 0 || index > sz)
        { 
            //allows insert at end, i == sz
            bounds_error("insert_before");
        }
        check_capacity();

        for (int i = sz; i > index; i--)
        {
            arr[i] = arr[i - 1];
        }
        arr[index] = s;
        sz++;
    }

    //Appends s to the end of the list; if necessary, the capacity of the underlying array is doubled. (undoable)
    void insert_back(const string &s)
    {
        insert_before(size(), s);
    }

    //Inserts s at the front of the list; if necessary, the capacity of the underlying array is doubled. (undoable)
    void insert_front(const string &s)
    {
        insert_before(0, s);
    }

    // Removes the string at the given index; doesn't change the capacity. (undoable)
    void remove_at(int index)
    {
        if(writeToCallList == true)
        {
            writeToList("REM " + std::to_string(index) + ' ' + arr[index]);
        }
        
        check_bounds("remove_at", index);
        for (int i = index; i < sz - 1; i++)
        {
            arr[i] = arr[i + 1];
        }
        sz--;
    }

    //Removes all strings from the list; doesn't change the capacity. (undoable)
    void remove_all()
    {
        while (sz > 0)
        {
            writeToList("CLR " + arr[sz - 1]);
            writeToCallList = false;
            remove_at(sz - 1);
        }
        writeToCallList = true;

    }

    //Removes the first occurrence of s in the list, and returns true.
    //If s is nowhere in the list, nothing is removed and false is returned. (undoable)
    bool remove_first(const string &s)
    {
        int index = index_of(s);
        if (index == -1)
        {
            return false;
        }
        remove_at(index);
        return true;
    }

    //Undoes the last operation that modified the list. Returns true if a change was undone.
    //If there is nothing to undo, does nothing and returns false.
    //The majority of this function uses other functions and the findDataSubstr function to reverse all changes to the list.
    //The methodology for each differing undo type is more or less the same with a few containing some extra steps to achieve the desired list.
    bool undo()
    {
        //The only time we write to the list is when a function is called from OUTSIDE the undo function.
        writeToCallList = false;

        if(readLastCall().substr(0,3) == "INS")
        {
            //stoi takes the string and converts to an int
            remove_at(stoi(findDataSubstr(4,2)));
            removeLastCall();
            writeToCallList = true;
            return true;
        }
        else if(readLastCall().substr(0,3) == "SET")
        {
            set(stoi(findDataSubstr(4,2)), findDataSubstr(4,3));
            removeLastCall();
            writeToCallList = true;
            return true;
        }
        else if(readLastCall().substr(0,3) == "REM")
        {
            insert_before(stoi(findDataSubstr(4,2)), findDataSubstr(4,3));
            removeLastCall();
            writeToCallList = true;
            return true;
        }
        else if(readLastCall().substr(0,7) == "LST_SET")
        {
            int numIterations = stoi(findDataSubstr(8,2));
            int originalSz = numIterations;

            while(readLastCall().substr(0,7) == "LST_SET")
            {
                set(numIterations - 1, findDataSubstr(8,3));
                numIterations--;
                removeLastCall();
            }

            if(originalSz != sz)
            {
                while(sz > originalSz)
                {
                    remove_at(sz - 1);
                }
            }

            writeToCallList = true;
            return true;
        }
        else if(readLastCall().substr(0,3) == "CLR")
        {
            while(readLastCall().substr(0,3) == "CLR")
            {
                insert_back(findDataSubstr(4,2));
                removeLastCall();
            }

            writeToCallList = true;
            return true;
        }
        return false;
    }
}; //class Stringlist

// Prints list to in the format {"a", "b", "c"}.
ostream &operator<<(ostream &os, const Stringlist &lst)
{
    return os << lst.to_string();
}

//Returns true if the two lists are equal, false otherwise.
//Does NOT consider any undo information when comparing two Stringlists. 
//All that matters is that they have the same strings in the same order.
bool operator==(const Stringlist &a, const Stringlist &b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    for (int i = 0; i < a.size(); i++)
    {
        if (a.get(i) != b.get(i))
        {
            return false;
        }
    }
    return true;
}

//Returns true if the two lists are not equal, false otherwise.
//Does NOT consider any undo information when comparing two Stringlists.
bool operator!=(const Stringlist &a, const Stringlist &b)
{
    return !(a == b);
}