#include<iostream>
#include<fstream>
#include<stdint.h>
#include<string>
#include<sstream>
#include<cstdlib>
#include<ctime>
#include<cstring>
#include<map>
#include<chrono>
#include<vector>
#include<thread>

using std::cin;
using std::cout;
using std::cerr;
using std::ifstream;
using std::string;
using std::stringstream;
using std::map;
using std::vector;
using std::istream;

#define DEFAULT_CHARACTERS "abcdefghijklmnopqrstuvwxyz., \n?!:;-\""

const char *values = DEFAULT_CHARACTERS;
int valuecount = sizeof(DEFAULT_CHARACTERS);
int memorycount = 6;
bool lowercasing = true;
bool verbose = false;
bool progressreport = false;
bool readstdin = false;

map<string, map<char, uint32_t> > rules;

class inputstream {
private:
	vector<istream*> *streams;
	int currentindex = 0;
	
public:
	inputstream(vector<istream*>& _streams);
	string read(int n);
	bool empty();
};

char translate(int v){
	return values[v];
}

int translate(char c){
	if (lowercasing && c >= 'A' && c <='Z')
		c = (c-'A') + 'a';
	for (int i=0; i<valuecount; i++)
		if (values[i] == c)
			return i;
	return -1;
}

bool valid(char c){
	return translate(c) != -1;
}

int pow(int n, int e){
	int v=1;
	for (int i=0;i<e;i++){
		v = v*n;
	}
	return v;
}

string clean(string data){
	string result = "";
	for (int i=0; i<data.size(); i++){
		if (valid(data[i])){
			result += data[i];
		}
	}
	return result;
}

inputstream::inputstream(vector<istream*>& _streams){
	streams = &_streams;
}

string inputstream::read(int n){
	string s(n, '\0');
	int index = 0;
	while (index != n){
		// we check that we still have streams to read from
		if (currentindex >= streams->size())
			return "";
		
		// try to fill remaining space in result string
		streams->at(currentindex)->read((char*)&(s.data()[index]), n - index);
		int written = streams->at(currentindex)->gcount();
		if (written == n - index){
			break;
		}
		else
		{
			index += written;
			//find out why it didn't read the full amount
			if (streams->at(currentindex)->eof()){
				// the stream was empty
				if (verbose)
					cout<<"reached EOF on stream #"<<currentindex<<"\n";
				currentindex++;
				continue;
			}
			if (streams->at(currentindex)->bad()){
				// something went wrong, perhaps file isn't avaliable any more
				cerr<<"stream error while reading stream #"<<currentindex<<"\n";
				currentindex++;
				continue;
			}
			
			//if we get here, it just means nothing's avaliable yet, so we wait a bit for characters to become avaliable, then try again
			cout<<"how did you get here?\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}
	return s;
}

bool inputstream::empty(){
	return currentindex >= streams->size();
}

string format(long n){
	if (n > 1024*1024*1024)
		return std::to_string(n/(1024*1024*1024)) + "." + std::to_string(((n/(1024*1024))%1024)/103) + "Gi";
	if (n > 1024*1024)
		return std::to_string(n/(1024*1024)) + "." + std::to_string(((n/1024)%1024)/103) + "Mi";
	if (n > 1024)
		return std::to_string(n/1024) + "." + std::to_string((n%1024)/103) + "Ki";
	return std::to_string(n);
}

void train(inputstream& data){
	auto begin = std::chrono::steady_clock::now();
	string index = data.read(memorycount);
	long bytecount = memorycount;
	
	if (verbose)
		cout<<"Training seed: '"<<index<<"'\n";

	while (!data.empty()){
		string indexc = data.read(1);
		if (indexc.size() == 0)
			break;
		bytecount++;

		if (progressreport && (!(bytecount % 1000) ) ){
			auto end = std::chrono::steady_clock::now();
			long elapsedms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
			if (elapsedms > 0)
				cout<<"\rTraining "<<format(bytecount)<<"B  "<<format((bytecount*1000)/(elapsedms))<<"B/s  Total time: "<<(long)(elapsedms/1000)<<"s     "<<std::flush;
		}

		auto iter = rules.find(index);
		if (iter == rules.end()){
			rules[index] = map<char, uint32_t>();
			iter = rules.find(index);
		}
		map<char, uint32_t> &m = iter->second;
		auto it = m.find(indexc[0]);
		if (it == m.end()){
			m[indexc[0]] = 1;
		}else{
			(it->second)++;
		}
		//move the index register forwards
		index = index.substr(1) + indexc[0];
	}
	if (progressreport)
		cout<<"\n";
}

string generate(int count){
	string resultstring = "";
	string index = "";
	{
		// select random index string from ruleset
		int indexindex = (rand() % rules.size());
		auto it = rules.begin();
		for (int i=0; i<indexindex; i++){
			it++;
		}
		index = it->first;
	}
	if (verbose)
		cout<<"generate seed: "<<index<<"\n";

	for (int i=0; i<count; i++){
		auto charmap = rules[index];
		int sum = 0;
		for (auto it = charmap.begin(); it != charmap.end(); it++){
			sum += it->second;
		}
		if (sum == 0){
			// TODO: Fix automatic recovery from dead ends
			cerr<<"Encountered dead end\n";
			exit(3);
		}
		int randval = rand() % sum;
		for (auto it = charmap.begin(); it != charmap.end(); it++){
			randval -= it->second;
			if (randval < 0){
				resultstring += it->first;
				index = index.substr(1) + it->first;
				cout<<it->first;
				break;
			}
		}
	}
	return resultstring;
}

void print_basic_usage(const char *executablename){
	cout<<	"BASIC USAGE: "<<executablename<<" -f [filename]\n";
}
void print_usage(const char *executablename){
	print_basic_usage(executablename);
	cout<<"\n"<<
			"    -v | --verbose                 extra debug logging.\n"<<
			"    -c | --characters [characters] set of characters allowed. by default \"abcdefghijklmnopqrstuvwxyz., \\n?!:;\"-\"\n"<<
			"    -l [number]                    how many characters to generate.\n"<<
			"    --no-lowercase                 disables automatic conversion of characters to lowercase.\n"<<
			"    -m [size]                      how many characters to take into consideration.\n"<<
			"    -h | --help                    show this help message.\n"<<
			"    -p | --progress                display progress bar while working\n"<<
			"\n";
}

int main(int argn, const char** args){
	srand(time(NULL));
	int verbosity = 0;
	int generatelength = 1000;
	vector<const char*> filenames;
	// parse arguments
	int index = 1;
	while (index < argn){
	
		if (strcmp(args[index], "-v")==0 || strcmp(args[index], "--verbose")==0){
			verbose = true;
		}else
		
		if (strcmp(args[index], "-c")==0 || strcmp(args[index], "--characters")==0){
			index++;
			if (index == argn){
				cerr<<"missing argument [character set] for -c flag\n";
				print_usage(args[0]);
				exit(1);
			}
			values = args[index];
			valuecount = strlen(values);
		}else
		
		if (strcmp(args[index], "-l") == 0){
			index++;
			if (index == argn){
				cerr<<"missing argument [generation length] for -l flag\n";
				print_usage(args[0]);
				exit(1);
			}
			generatelength = atoi(args[index]);
		}
		if (strcmp(args[index], "--no-lowercase") == 0){
			lowercasing = false;
		}
		if (strcmp(args[index], "-m") == 0){
			index++;
			if (index == argn){
				cerr<<"missing argument [memory size] for -m flag \n";
				print_usage(args[0]);
				exit(1);
			}
			memorycount = atoi(args[index]);
		}else
		
		if (strcmp(args[index], "-h")==0 || strcmp(args[index], "--help")==0){
			print_usage(args[0]);
			exit(0);
		}else
		
		if (strcmp(args[index], "-p")==0 || strcmp(args[index], "--progress")==0){
			progressreport = true;
		}else
		
		if (strcmp(args[index], "-")==0){
			readstdin = true;
		}else
		
		{
			// assume it's a file
			filenames.push_back(args[index]);
		}
		index++;
	}

	// open all files, if any given
	vector<istream*> filestreams;
	for (auto f = filenames.begin(); f != filenames.end(); f++){
		if (verbose)
			cout<<"Opening file '"<<*f<<"'\n";
		filestreams.push_back((istream*)new ifstream(*f));
		
		// if opening failed
		if (!filestreams.back()->good()){
			cerr<<"unable to open file '"<<(*f)<<"'\n";
			for (auto fstream = filestreams.begin(); fstream != filestreams.end(); fstream++){
				if (((ifstream*)*fstream)->is_open())
					((ifstream*)*fstream)->close();
				delete (*fstream);
			}
			exit(2);
		}
	}
	
	// add standard input stream
	if (readstdin)
		filestreams.push_back((istream*)&cin);

	if (verbose)
		cout<<"starting training\n";

	inputstream data = inputstream(filestreams);

	train(data);

	if (verbose)
		cout<<"training finished\n";
	
	// if we added it previously, remove it now before closing the files
	if (readstdin)
		filestreams.pop_back();
	
	if (verbose)
		cout<<"closing input files\n";
	
	for (auto it = filestreams.begin(); it != filestreams.end(); it++)
		if (((ifstream*)*it)->is_open())
			((ifstream*)*it)->close();

	if (verbose)
		cout<<"input files closed\n";

	if (verbose)
		cout<<"generating text\n";

	string result = generate(generatelength);

	if (verbose)
		cout<<"text generation finished\n";

	cout<<result<<"\n";
	
	exit(0);
}
