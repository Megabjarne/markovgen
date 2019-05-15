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

using std::cout;
using std::cerr;
using std::ifstream;
using std::string;
using std::stringstream;
using std::map;

const char *values = "abcdefghijklmnopqrstuvwxyz., \n?!:;-\"";
int valuecount = 36;
int memorycount = 6;
bool lowercasing = true;
bool verbose = false;
bool progressreport = false;

map<string, map<char, uint32_t> > rules;

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

void train(string data){
	auto begin = std::chrono::steady_clock::now();

	for (long i=0; i<data.size()-valuecount; i++){

		if (progressreport && (!(i % std::max( (int)((data.size()-valuecount)/500), 1) ) || i == data.size()-valuecount-1)){
			auto end = std::chrono::steady_clock::now();
			long elapsedms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
			long opsleft = data.size() - valuecount - i;
			long msleft = (elapsedms * opsleft) / ((i==0)?1:i);
			cout<<"\rTraining "<<i<<"/"<<data.size()-valuecount<<" "<<(int)((i*100)/(data.size()-valuecount))<<"% Time: "<<(long)(elapsedms/1000)<<"s ETA: "<<(long)(msleft/1000)<<"s      "<<std::flush;
		}

		string index = data.substr(i, memorycount-1);
		char indexc = data[i+memorycount-1];

		auto iter = rules.find(index);
		if (iter == rules.end()){
			rules[index] = map<char, uint32_t>();
			iter = rules.find(index);
		}
		map<char, uint32_t> &m = iter->second;
		auto it = m.find(indexc);
		if (it == m.end()){
			m[indexc] = 1;
		}else{
			(it->second)++;
		}
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

/*	char *current = new char[charcount];
	for (int i=0; i<charcount; i++){
		current[i] = rand()%valuecount;
	}
	int index;
	for (int i=0; i<count; i++){
		index = translate(current);
		uint16_t* probs = tdata[index];
		int sum = 0;
		for (int n=0; n<valuecount; n++){
			sum += probs[n];
		}
		if (sum == 0){
			sum++;
		}
		int rndn = rand() % sum;
		bool found = false;
		for (int n=0; n<valuecount; n++){
			rndn -= probs[n];
			if (rndn <0 || (rndn==0 && (rand()%100 < 0) && probs[n]!=0) ){
				cout<<translate(n);
				for (int j=0; j<charcount-1; j++){
					current[j] = current[j+1];
				}
				current[charcount-1] = n;
				found = true;
				break;
			}
		}
		if (!found){
			for (int k=0; k<charcount; k++){
				current[k] = rand()%valuecount;
			}
		}
	}
	delete current;
*/}

void print_basic_usage(const char *executablename){
	cout<<	"BASIC USAGE: "<<executablename<<" -f [filename]\n";
}
void print_usage(const char *executablename){
	print_basic_usage(executablename);
	cout<<"\n"<<
			"    -f [filename]                  file to be used for training data.\n"<<
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
	const char* filename = nullptr;
	int verbosity = 0;
	int generatelength = 1000;
	// parse arguments
	int index = 1;
	while (index < argn){
		if (strcmp(args[index], "-f") == 0){
			index++;
			if (index == argn){
				cerr<<"missing argument [filename] for -f flag\n";
				print_usage(args[0]);
				exit(1);
			}
			filename = args[index];
		}
		if (strcmp(args[index], "-v")==0 || strcmp(args[index], "--verbose")==0){
			verbose = true;
		}
		if (strcmp(args[index], "-c")==0 || strcmp(args[index], "--characters")==0){
			index++;
			if (index == argn){
				cerr<<"missing argument [character set] for -c flag\n";
				print_usage(args[0]);
				exit(1);
			}
			values = args[index];
			valuecount = strlen(values);
		}
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
		}
		if (strcmp(args[index], "-h")==0 || strcmp(args[index], "--help")==0){
			print_usage(args[0]);
			exit(0);
		}
		if (strcmp(args[index], "-p")==0 || strcmp(args[index], "--progress")==0){
			progressreport = true;
		}
		index++;
	}

	if (filename == nullptr){
		cerr<<"Input file must be specified\n";
		print_basic_usage(args[0]);
		exit(0);
	}

	// read in training data
	ifstream file = ifstream(filename);
	if (!file.good()){
		cerr<<"unable to open/read file '"<<filename<<"'\n";
		exit(2);
	}
	
	if (verbose)
		cout<<"reading file\n";
	
	stringstream buffer;
	buffer << file.rdbuf();
	
	if (verbose)
		cout<<"read "<<buffer.str().size()<<" characters\n";

	string data = clean(buffer.str());

	if (verbose)
		cout<<data.size()<<" characters left after cleaning\n";

	if (verbose)
		cout<<"starting training\n";

	train(data);

	if (verbose)
		cout<<"training finished\n";

	if (verbose)
		cout<<"generating text\n";

	string result = generate(generatelength);

	if (verbose)
		cout<<"text generation finished\n";

	cout<<result<<"\n";
	
	exit(0);
}
