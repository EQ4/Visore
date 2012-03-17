#include <dirent.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#ifndef MAX_PATH
	#define MAX_PATH 8192
#endif

using namespace std;

void getdir(string directory, vector<string> &files)
{
	DIR *dp;
	struct dirent *dirp;

	if((dp = opendir(directory.c_str())) == NULL)
	{
		return;
	}

	while((dirp = readdir(dp)) != NULL)
	{
		string path(directory+"/"+string(dirp->d_name));
		int lastindex = path.find_last_of("/");
		string file = path.substr(lastindex + 1, strlen(dirp->d_name));
		if(file.find(".") == -1)
		{
			char absolutePath[MAX_PATH]; 
        	realpath(path.c_str(), absolutePath); 
			files.push_back(absolutePath);
			getdir(path, files);
		}
	}

	closedir(dp);
}

int main(int argc, char** argv)
{
	#if !(defined WINDOWS || defined MACOSX || defined LINUX)
		cout << "Visore currently only supports Windows, Linux and Mac OS X." << endl;
		return -1;
	#endif

    char buffer[MAX_PATH];
	getcwd(buffer, MAX_PATH);
	vector<string> files = vector<string>();
	string currentDirectory = string(argv[0]);
	int lastindex = currentDirectory.find_last_of("/");
	currentDirectory = currentDirectory.substr(0, lastindex);
	getdir(currentDirectory, files);

    string command = "";
	char separator;
	#ifdef WINDOWS
		separator = ';';
		command += "@echo off && cd " + currentDirectory + " && set PATH=%PATH%" + separator;
	#elif defined MACOSX
		separator = ':';
		command += "cd " + currentDirectory + " && export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH" + separator;
	#elif defined LINUX
		separator = ':';
		command += "cd " + currentDirectory + " && export LD_LIBRARY_PATH=$LD_LIBRARY_PATH" + separator;
    #endif

	for(int i = 0; i < files.size(); ++i)
	{
		command += files[i] + separator;
cout<<"*"<<files[i]<<"*"<<endl;
	}

	#ifdef WINDOWS
		command += " && start visore.exe";
	#else
		command += " && ./visore";
	#endif

    system(command.c_str());

    return 0;
}