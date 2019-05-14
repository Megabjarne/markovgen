#include<iostream>
#include<fstream>
#include<stdint.h>
#include<string>
#include<sstream>
#include<cstdlib>
#include<time.h>
#include<cstring>

using std::cout;
using std::cerr;
using std::ifstream;
using std::string;
using std::stringstream;

const char *values = "abcdefghijklmnopqrstuvwxyz., \n?!:;-\"";
int valuecount = 36;
int charcount = 4;

uint16_t **tdata;

char translate(int v){
	return values[v];
}

int translate(char* c){
	int v=0;
	for (int i=0; i<charcount; i++){
		v = v*valuecount + c[i];
	}
	return v;
}

int translate(char c){
	if (c >= 'A' && c <='Z')
		c = (c-'A') + 'a';
	for (int i=0; i<valuecount; i++){
		if (values[i] == c)
			return i;
	}
	return -1;
}

int pow(int n, int e){
	int v=1;
	for (int i=0;i<e;i++){
		v = v*n;
	}
	return v;
}

void train(string data){
	cout<<"clearing memory\n";
	for (int i=0;i<pow(valuecount, charcount); i++){
		for (int j=0; j<valuecount; j++){
			tdata[i][j] = 0;
		}
	}
	cout<<"clearing done\n";
	cout<<"training starting\n";

	char *currentpos = new char[charcount];
	for (int i=0; i<charcount; i++){
		currentpos[i] = 0;
	}
	int index, cindex;
	for (int i=0; i<data.size(); i++){
		index = translate(currentpos);
		cindex = translate(data[i]);
		if (cindex >= 0){
			tdata[index][translate(data[i])]++;
			for (int j = 0; j<charcount-1; j++){
				currentpos[j] = currentpos[j+1];
			}
			currentpos[charcount-1] = translate(data[i]);
		}
	}
	for (int i=0; i<pow(valuecount, charcount); i++){
		for (int j=0; j<valuecount; j++){
			//tdata[i][j] = tdata[i][j]*tdata[i][j];
		}
	}
	delete currentpos;
	cout<<"training done\n";
}

void generate(int count){
	char *current = new char[charcount];
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
}

void print_usage(){

}

int main(int argn, const char** args){
	const char* filename = "\0";
	int verbosity = 0;
	int generatelength = 8192;
	// parse arguments
	int index = 1;
	while (index < argn){
		if (strcmp(args[index], "-f") == 0){
			index++;
			if (index == argn){
				cerr<<"missing argument [filename] for -f flag\n";
				return 1;
			}
			filename = args[index];
		}
		if (strcmp(args[index], "-v") == 0){
			verbosity++;
		}
		if (strcmp(args[index], "-c") == 0){
			index++;
			if (index == argn){
				cerr<<"missing argument [character set] for -c flag\n";
				return 1;
			}
			values = args[index];
			valuecount = strlen(values);
		}
		if (strcmp(args[index], "-l") == 0){
			index++;
			if (index == argn){
				cerr<<"missing argument [generation length] for -l flag\n";
				return 1;
			}
			generatelength = atoi(args[index]);
		}
		index++; 
	}

	srand(time(NULL));
	ifstream file = ifstream(filename);
	if (!file.good()){
		cerr<<"unable to open/read file '"<<filename<<"'\n";
		return -1;
	}
	
	if (verbosity >= 1)
		cout<<"reading file\n";
	
	stringstream buffer;
	buffer << file.rdbuf();
	
	if (verbosity >= 1)
		cout<<"read "<<buffer.str().size()<<" characters\n";

	if (verbosity >= 1)
		cout<<"allocating training memory ("<< pow(valuecount, charcount)*sizeof(uint16_t*) + pow(valuecount, charcount)*valuecount*sizeof(uint16_t) <<"B)\n";

	tdata = new uint16_t*[pow(valuecount, charcount)];
	
	for (int i=0; i<pow(valuecount, charcount); i++){
		tdata[i] = new uint16_t[valuecount];
	}

	if (verbosity >= 1)
		cout<<"allocation finished\n";

	train(buffer.str());
	
	generate(generatelength);
	cout<<"\n";
	
	for (int i=0; i<pow(valuecount, charcount); i++){
		delete tdata[i];
	}
	delete tdata;
}
