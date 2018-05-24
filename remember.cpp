#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>

using namespace std;

string homepath = getenv("HOME");
string commands_path = homepath + "/.commands";
string tags_path = homepath + "/.tags";

struct Info {
	char option;
	bool verbose = false;
	string command;
	vector <string> tags;
};

vector <string> split(const string s) {
	stringstream tag_ss = stringstream(s);
	vector <string> ret;
	for (string str; tag_ss >> str;) {ret.push_back(str);}
	return ret;
}

vector <string> read_f(string fname) {
	ifstream fs;
	fs.open(fname);
	vector <string> returnVec;
	string line;
	while (getline(fs, line) && !line.empty()) {
		returnVec.push_back(line);	
	}
	fs.close();
	return returnVec;
}

void write_f(string fname, vector<string> lines) {
	ofstream ofs;
	ofs.open(fname);
	for (auto line : lines) ofs << line << endl;
	ofs.close();
}

void print_usage_and_exit(bool isError = false){
	string str = "Normal usage:\n";
	str += "\tremember [thing to remember] -t \"tag1 tag2...\"\n";
	str += "Other usages:\n";
	str += "\tremember -r [tags]\n";
	str += "\tremember -r -v [tags]\n";
	str += "\tremember -d [id of command to be deleted]\n";

	if (isError) {
		cerr << str;
		exit(1);
	}
	else {
		cout << str;
		exit(0);
	}

}

Info handle_input (int argc, char ** argv) {
	Info ret;
	ret.option = 'w';

	if (argc < 2) print_usage_and_exit(true);

	string command, tags;
	bool nextIsTags = false;
	for (int i = 1; i < argc; i++){
		if (!string(argv[i]).compare("-t")) {
			nextIsTags = true;
			continue;
		}
		if (!string(argv[i]).compare("-r")) {
			ret.option = 'r';
			continue;
		}
		if (!string(argv[i]).compare("-v")) {
			ret.verbose = true;
			continue;
		}
		if (!string(argv[i]).compare("-d")) {
			ret.option = 'd';
			continue;
		}
		if (argv[i][0] == '-') print_usage_and_exit(true);
		if (nextIsTags){
			tags += string(argv[i]) + " ";
			nextIsTags = false;
		}
		else {
			command += (string(argv[i]) + " ");
		}
	}

	ret.command = command.substr(0, command.size() - 1);
	ret.tags = split(tags);

	if (ret.option == 'r') {
		auto more_tags = split(ret.command);
		ret.tags.insert(end(ret.tags), begin(more_tags), end(more_tags));
	}

	if (ret.tags.empty()) ret.tags.push_back("DEFAULT");
	
	return ret;
}

void new_command(Info &info) {

	// add new command to end of commands file with new id
	vector<string> command_file_vec = read_f(commands_path);
	int new_command_num = 0;
	if (command_file_vec.size() > 0) {
		auto lastline = command_file_vec[command_file_vec.size() - 1];
		stringstream(lastline) >> new_command_num;
		new_command_num++;
	}
	command_file_vec.push_back(to_string(new_command_num) + " " + info.command);
	write_f(commands_path, command_file_vec);

	// write tag information to tag file
	vector<string> tag_file_vec = read_f(tags_path);
	for (auto & tag_line : tag_file_vec) {
		auto t = (split(tag_line)[0]);		
		for (auto & tag : info.tags) {
			if (!t.compare(tag)) {
				tag_line += (" " + to_string(new_command_num));
				tag = "";
			}
		}
	}

	for (auto & tag : info.tags) {
		if (!tag.empty()) {
			tag_file_vec.push_back(tag + " " + to_string(new_command_num));
		}
	}
	write_f(tags_path, tag_file_vec);

}

void retrieve_command (Info &info) {
	set <int> cmd_set;

	auto tag_file_vec = read_f(tags_path);
	for (auto tag_line : tag_file_vec) {
		auto tag_line_split = split(tag_line);
		for (auto search_tag : info.tags) {
			if (!(tag_line_split[0]).compare(search_tag)){
				for (size_t i = 1; i < tag_line_split.size(); i++) {
					cmd_set.insert(stoi(tag_line_split[i]));
				}
			}
		}
	}

	auto command_file_vec = read_f(commands_path);
	for (auto line : command_file_vec) {
		stringstream ss(line);
		int id; ss >> id;
		if (cmd_set.find(id) != cmd_set.end()) {
			if (!info.verbose)
				cout << line.substr(line.find_first_of(" \t")+1) << endl;
			else
				cout << line << endl;
		}
	}
}

void delete_command (Info & info) {
	
	int id = 0;
	try {
		id = stoi(info.command);
	} catch (invalid_argument & ia) {
		print_usage_and_exit(true);
	}

	auto command_line_vec = read_f(commands_path);
	for (size_t i = 0; i < command_line_vec.size(); i++) {
		int curr_id;
		stringstream(command_line_vec[i]) >> curr_id;
		if (curr_id == id) {
			command_line_vec.erase(command_line_vec.begin() + i);
			break;
		}
	}
		
	auto tag_line_vec = read_f(tags_path);
	for (size_t j = 0; j < tag_line_vec.size(); j++) {
		auto & line = tag_line_vec[j];
		auto split_line = split(line);
		for (size_t i = 1; i < split_line.size(); i++){
			if (stoi(split_line[i]) == id) {
				split_line.erase(split_line.begin() + i);
				i--;
			}

			line = "";
			if (split_line.size() > 1) {
				for (auto word : split_line) line += word + " ";
			}
			else {
				tag_line_vec.erase(tag_line_vec.begin() + j);
				j--;
			}

		}
	}

	write_f(commands_path, command_line_vec);
	write_f(tags_path, tag_line_vec);
}

int main(int argc, char ** argv) {
	Info info = handle_input(argc, argv);

	if (info.option == 'w') new_command(info);
	if (info.option == 'r') retrieve_command(info);
	if (info.option == 'd') delete_command(info);

	return 0;
}
