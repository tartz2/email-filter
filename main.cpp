/*    Thomas Artz 
 -- UIC Spring 2020 --  */

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "ourvector.h"

using namespace std;

/*function to parse the input to get the first word from input
 *so that you can see which command was given */
void parseInput(string input, string& command) {
    int spaceCount = 0;
    int spaceLocation = -1;        //temporary value to detect error

    for (int i = 0; (unsigned)i < input.size(); i++) {
        if (input[i] == ' ') {
            if (spaceLocation == -1)
                spaceLocation = i;
            spaceCount++;
        }
    }

    /* if there are spaces, command is the first word of the input
     * else the command IS the input */
    if (spaceCount > 0)
        command = input.substr(0, spaceLocation);
    else
        command = input;

    return;
}

/*parses an email that it is given when called assuming it is either username@domain format
 * or domain:username format */
void parseEmail(string word, char letter, string& domain, string& username) {
    int pos = -1;

    for (int i = 0; (unsigned)i < word.size(); i++) {
        if (word[i] == letter) {
            pos = i;
        }
    }

    if (pos != -1) {
        if (letter == '@') {
            domain = word.substr(pos + 1);
            username = word.substr(0, pos);
        }
        else if (letter == ':') {
            domain = word.substr(0, pos);
            username = word.substr(pos + 1);
        }
    }

    return;
}

/*a function that will get the email filename and output filename from the input given
 * if there are still words after the first file, it knows its going to filter
 * so it gets the next word knowing its the output file */
void fileHandler(string input, string& emailFile, string& notSpamFile) {
    int pos[] = { -1, -1 };
    for (int i = 0; (unsigned)i < input.size(); i++) {
        if (input[i] == ' ') {
            if (pos[0] == -1)
                pos[0] = i;
            else
                pos[1] = i;
        }
    }
    if (pos[1] == -1) {
        emailFile = input.substr(pos[0] + 1);
    }
    else {

        emailFile = input.substr(pos[0] + 1, pos[1] - pos[0] - 1);
        notSpamFile = input.substr(pos[1] + 1);
    }
    return;
}

/* function that performs a binary search where it keeps track of 2 searches within 1 loop.
 * It parses the email it is handed, concatenates it so it matches the spamList and
 * then searches. returns true if found, false if it breaks out of the loop */
bool binarySearch(string email, ourvector<string>& emailList) {
    int right = emailList.size() - 1;
    int left = 0;

    string domain, username, subject, id, filename;

    parseEmail(email, '@', domain, username);

    /* setup 2 email strings. one for searching for literal email
     * and one for finding just the domain it belongs to with the '*' */
    email = domain + ":" + username;
    string email2 = domain + ":*";

    //2 strings to break apart the * address
    string tmpUser;
    string tmpDomain;

    /* while the left side is still less than or equal to the right to make sure we havent
     * overstepped the search */
    while (left <= right) {
        int middle = left + (right - left) / 2;

        //assign the spam email its comparing with to a string to limit amount of accesses
        string compareSpam = emailList[middle];
        parseEmail(compareSpam, ':', tmpDomain, tmpUser);

        if (tmpUser.compare("*") == 0) {
            if (tmpDomain.compare(domain) == 0)
                return true;
        }

        //if either email is the current spam email, it returns true
        if (email.compare(compareSpam) == 0 || email2.compare(compareSpam) == 0)
            return true;

        // If email greater then we'll ignore the current left side 
        if (email.compare(compareSpam) > 0)
            left = middle + 1;

        // If email is smaller then we'll ignore the right side 
        if (email.compare(compareSpam) < 0)
            right = middle - 1;
    }

    //resetting variables for second search on the domain:*
    right = emailList.size() - 1;
    left = 0;

    while (left <= right) {
        int middle = left + (right - left) / 2;

        //assign the spam email its comparing with to a string to limit amount of accesses
        string compareSpam = emailList[middle];
        parseEmail(compareSpam, ':', tmpDomain, tmpUser);

        if (tmpUser.compare("*") == 0) {
            if (tmpDomain.compare(domain) == 0)
                return true;
        }

        //if either email is the current spam email, it returns true
        if (email2.compare(compareSpam) == 0)
            return true;

        // If email greater then we'll ignore the current left side 
        if (email2.compare(compareSpam) > 0)
            left = middle + 1;

        // If email is smaller then we'll ignore the right side 
        if (email2.compare(compareSpam) < 0)
            right = middle - 1;
    }

    //if nothing was found return false
    return false;
}


/* small function that reads in 2 words from an email file and then reads
 * the rest of the line for the subject */
void loadEmail(string& id, string& email, string& subject, ifstream& inFS) {
    inFS >> id;
    inFS >> email;
    getline(inFS, subject);
    return;
}

/* small function that opens the file to append, writes the already concatenated
 * id + email + subject and then closes the file */
void writeFile(string line, string filename) {
    ofstream onFS;
    onFS.open(filename, ios_base::app);
    onFS << line << endl;
    onFS.close();
    return;
}

/* loads the directed spam file, reads the contents into a vector,
 * and then closes the file and lets the user know how many spam emails
 * are in the file */
void loadSpamFile(string input, ourvector<string>& spamList) {
    string filename, tmpOutput, email;
    ifstream inFS;

    //clears vector we are using every time we load a new one
    spamList.clear();
    fileHandler(input, filename, tmpOutput);

    inFS.open(filename.c_str());
    if (!inFS.is_open()) {
        cout << "**Error, unable to open '" << filename << "'" << endl << endl;
        return;
    }

    //lets the user know which file it is loading and then starts looping through
    cout << "Loading '" << filename << "'" << endl;
    getline(inFS, email);

    while (!inFS.eof()) {
        spamList.push_back(email);
        getline(inFS, email);
    }

    inFS.close();

    //lets the user know how many spam emails it found
    cout << "# of spam entries: " << spamList.size() << endl << endl;
    return;
}


/* function that checks if an email given from user input is in the currently loaded spam file and it
 *does this by taking the specific email, parsing the domain and username, checks for any errors and if
 *it encounters any it declares the email not spam. Then it calls the binary search function to return
 *true or false whether the email is in the spam list.  */
void checkEmail(string input, ourvector<string>& spamVec) {
    bool isSpam = false;
    int pos = -1; //temporary value so i know if a new value wasn't assigned
    string email;
    string username;
    string domain;


    for (int i = 0; (unsigned)i < input.size(); i++) {
        if (input[i] == ' ')
            pos = i + 1;
    }

    //if the position of the space exists, get the substring for the email from input
    if (pos != -1)
        email = input.substr(pos);
    else
        cout << email << " is not spam" << endl << endl;

    isSpam = binarySearch(email, spamVec);

    if (!isSpam)
        cout << email << " is not spam" << endl << endl;
    else
        cout << email << " is spam" << endl << endl;

    return;

}
/* function that runs after the user specifies the command filter. It first erases the content from the output
 * folder given because I open it in append later, then it opens the file to read from, loops through all the
 * lines parsing the email and id and subject, performing binary search on each email, writing non flagged emails
 * to the outupt file, keeping track of the number of loops and spam emails it finds, and then returns back to main */
void doFilter(string input, ourvector<string>& spamList, int& emailsProcessed, int& nonSpam) {
    int spamAmt = 0;
    emailsProcessed = 0;
    nonSpam = 0;
    string id, email, domain, username, subject, filename;
    string tmpFile = "tmp";

    ifstream inFS;
    ofstream onFS;
    fileHandler(input, filename, tmpFile);

    //erasing contents of the specified output file before appending to it
    onFS.open(tmpFile.c_str());

    onFS.close();

    inFS.open(filename.c_str());
    if (!inFS.is_open()) {
        cout << "**Error, unable to open '" << filename << "'" << endl << endl;
        return;
    }



    //loads email from ifstream by reading in the line
    loadEmail(id, email, subject, inFS);
    while (!inFS.eof()) {
        parseEmail(email, '@', domain, username);

        /* if binary search finds it to be spam, increment spamAmt
         * concatenates the email line back into the proper format and then
         * appends it to the specified file
         */
        if (binarySearch(email, spamList)) {
            spamAmt++;
        }
        else {
            string line = id + " " + username + "@" + domain + subject;
            writeFile(line, tmpFile);
        }

        //either way, it increments the amount of emails its read,
        //and then loads the next email
        emailsProcessed++;
        loadEmail(id, email, subject, inFS);
    }

    //after the file is entirely processed, it closes the stream
    inFS.close();
    nonSpam = emailsProcessed - spamAmt;

    /* saying the # of processed and non-spam here because if i put it in main after
     * the function call, the function could not find the file and it shouldnt say this then */
    cout << "# emails processed: " << emailsProcessed << endl;
    cout << "# non-spam emails: " << nonSpam << endl << endl;
    return;
}

//very small function that loops through the vector and prints the contents
void displayList(ourvector<string>& spamList) {
    for (int i = 0; i < spamList.size(); i++) {
        cout << spamList[i] << endl;
    }
    cout << endl;
}

int main() {

    string input, command;
    ifstream inFS;
    ourvector<string> spamList;
    bool exit = false;
    int emailsProcessed = 0;
    int nonSpam = 0;

    //formatting output and getting user input
    cout << "** Welcome to spam filtering app **" << endl << endl;

    //until the user specifies the only command to make exit true, it will keep looping
    while (exit == false) {
        cout << "Enter command or # to exit> ";
        getline(cin, input);

        //parse input for first command word
        parseInput(input, command);

        //detecting user command
        if (command.compare("display") == 0) {
            displayList(spamList);
        }
        else if (command.compare("check") == 0) {
            checkEmail(input, spamList);
        }
        else if (command.compare("load") == 0) {
            loadSpamFile(input, spamList);
        }
        else if (command.compare("filter") == 0) {
            doFilter(input, spamList, emailsProcessed, nonSpam);
        }
        else if (command.compare("#") == 0) {
            spamList.clear();
            exit = true;
        }
        else {
            cout << "**invalid command" << endl << endl;
        }
    }

    return 0;
}